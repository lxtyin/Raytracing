// compute shader ver.

#version 460 core
#extension GL_ARB_bindless_texture : require
//#define COMPUTE_SHADER

#include shader/basic/math.glsl
#include shader/basic/sobol.glsl

const uniform float RAY_MINDIS = 0.001;

#ifdef COMPUTE_SHADER
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
#else
in vec2 screen_uv;
#endif

layout(binding = 0, std430) readonly buffer ssbo0 {
    sampler2D textureBuffer[];
};

layout(binding = 1, std430) readonly buffer ssbo1 {
    float materialBuffer[];
};

struct Triangle {
    vec3 ver[3];
    vec3 normal[3];
    vec2 uv[3];
};

struct TriangleX {
    float ver[9];
    float normal[9];
    float uv[6];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    TriangleX triangleBuffer[];
};
Triangle get_triangle(int idx) {
    TriangleX x = triangleBuffer[idx];
    Triangle y;
    for(int i = 0;i < 3;i++) {
        y.ver[i] = vec3(x.ver[i * 3 + 0], x.ver[i * 3 + 1], x.ver[i * 3 + 2]);
        y.normal[i] = vec3(x.normal[i * 3 + 0], x.normal[i * 3 + 1], x.normal[i * 3 + 2]);
        y.uv[i] = vec2(x.uv[i * 2 + 0], x.uv[i * 2 + 1]);
    }
    return y;
}

struct InstanceInfo {
    mat4 world2local;
    int materialPtr;
    int meshIndex;
    int emptyblock[10];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    InstanceInfo instanceInfoBuffer[];
};

struct BVHNode {
    vec4 aa, bb; // actually vec3
    int lsIndex;
    int rsIndex;
    int instanceIndex;
    int triangleIndex;
};
layout(binding = 4, std430) readonly buffer ssbo4 {
    BVHNode meshBVHBuffer[];
};
layout(binding = 5, std430) readonly buffer ssbo5 {
    BVHNode sceneBVHBuffer[];
};

struct Light {
    // point light (0) or directional light (1).
    int type;
    float x, y, z;
    float r, g, b;
    float emptyblock;
};
layout(binding = 6, std430) readonly buffer ssbo6 {
    Light lightBuffer[];
};
uniform int lightCount;


layout(binding = 7) buffer ssbo7 {
    float albedoGBuffer[];
};

layout(binding = 9) writeonly buffer ssbo9 {
    float motionGBuffer[];
};
layout(binding = 11) writeonly buffer ssbo11 {
    float depthGBuffer[];
};
layout(binding = 12) writeonly buffer ssbo12 {
    float normalGBuffer[];
};
layout(binding = 13) writeonly buffer ssbo13 {
    float instanceIndexGBuffer[];
};

layout(binding = 14) buffer ssbo14 {
    float directLumGBuffer[];
};
layout(binding = 15) buffer ssbo15 {
    float indirectLumGBuffer[];
};

uniform sampler2D depthGBufferTexture;
uniform sampler2D normalGBufferTexture;
uniform sampler2D uvGBufferTexture;
uniform sampler2D instanceIndexGBufferTexture;


// Input infos ===
uniform mat4 v2wMat;
uniform mat4 backprojMat; // Last frame matrix to project worldposition -> [-1, 1]
uniform int MAX_DEPTH = 2;
uniform float fov = PI / 3;
uniform float cameraNear, cameraFar;
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform int WINDOW_W;
uniform int WINDOW_H;
uniform sampler2D skybox;
uniform sampler2D skybox_samplecache;
uniform float skybox_Light_SUM;
uniform int SKY_W;
uniform int SKY_H;
uniform uint frameCounter;
uniform uint currentspp;
uniform vec2 jitter;

uniform bool BRDFSampling;
uniform bool SkyboxSampling;
uniform bool SkyboxLighting;

uniform bool RasterizaionFor1st;

#include shader/materials/materials.glsl


// math
// ---------------------------------------------- //
struct Frame {
    vec3 s, t, n;
};

Frame create_coord(vec3 n) {
    Frame f;
    if(abs(n.z) > 1 - EPS) f.s = normalize(cross(n, vec3(1, 0, 0)));
    else f.s = normalize(cross(n, vec3(0, 0, 1)));
    f.t = cross(n, f.s);
    f.n = n;
    return f;
}

vec3 to_world(in Frame f, vec3 v) {
    return v.x * f.s + v.y * f.t + v.z * f.n;
}
vec3 to_local(in Frame f, vec3 v) {
    return vec3(dot(v, f.s), dot(v, f.t), dot(v, f.n));
}

// v原先在n = +z下表示
vec3 to_world(vec3 v, vec3 n) {
    vec3 help = vec3(1, 0, 0);
    if(abs(n.x) > 0.999) help = vec3(0, 0, 1);
    vec3 t = normalize(cross(n, help));
    vec3 b = normalize(cross(n, t));
    return normalize(v.x * t + v.y * b + v.z * n);
}

// 获取第i个三角形光源的index
//int get_light_t_index(int i) {
//    return roundint(texelFetch(lightidxs, i).x);
//}

// 参数中u，v为重心坐标，输出为纹理映射（uv）的插值
vec2 interpolate_uv(in Triangle tr1, float u, float v) {
    return tr1.uv[0] * (1 - u - v) +
    tr1.uv[1] * u + tr1.uv[2] * v;
}

// intersect
// ---------------------------------------------- //

struct Ray {
    vec3 ori;
    vec3 dir;
};

struct Intersection {
    bool exist;
    vec3 position; // in world space.
    vec3 normal; // in world space.
    float t;
    vec2 uv;
    int instanceIndex;
    int triangleIndex;
};
const Intersection nointersect = Intersection(false, vec3(0), vec3(0, 0, 1), INF, vec2(0), -1, -1);

/// 检测是否与AABB碰撞，返回最早碰撞时间
bool intersect_aabb(Ray ray, vec3 aa, vec3 bb, out float nearT) {
    float tmi = 0, tmx = INF;
    vec3 t1 = (aa - ray.ori) / ray.dir;
    vec3 t2 = (bb - ray.ori) / ray.dir;
    tmi = max(tmi, min(t1.x, t2.x));
    tmi = max(tmi, min(t1.y, t2.y));
    tmi = max(tmi, min(t1.z, t2.z));
    tmx = min(tmx, max(t1.x, t2.x));
    tmx = min(tmx, max(t1.y, t2.y));
    tmx = min(tmx, max(t1.z, t2.z));
    nearT = tmi;

    return tmx >= tmi && tmx >= RAY_MINDIS;
}

/// 返回与triangle的具体碰撞信息
Intersection intersect_triangle(Ray ray, int triangleIndex) {
    Triangle tri = get_triangle(triangleIndex);

    vec3 E1 = tri.ver[1].xyz - tri.ver[0].xyz;
    vec3 E2 = tri.ver[2].xyz - tri.ver[0].xyz;
    vec3 S = ray.ori - tri.ver[0].xyz;
    vec3 S1 = cross(ray.dir, E2);
    vec3 S2 = cross(S, E1);
    float k = 1.0 / dot(S1, E1);
    float t = dot(S2, E2) * k;
    float u = dot(S1, S) * k;
    float v = dot(S2, ray.dir) * k;

    bool exist = (RAY_MINDIS <= t && 0 <= u && 0 <= v && u + v <= 1);

    vec3 pos = ray.ori + t * ray.dir;
    vec3 nor = normalize(tri.normal[0].xyz * (1 - u - v) + tri.normal[1].xyz * u + tri.normal[2].xyz * v);

    // TODO: geo/shading frame (if tangent is required)
    //    vec2 D1 = tri.uv[1] - tri.uv[0];
    //    vec2 D2 = tri.uv[2] - tri.uv[0];
    //    frame.t = 1.0 / (D1.x * D2.y - D2.x * D1.y) * (E1 * D1.x - E2 * D2.x);

    // returns in local frame, then updated to world frame.
    return Intersection(exist, pos, nor, t, interpolate_uv(tri, u, v), -1, triangleIndex);
}


int stack_h = 0;
int stack[256];
void intersect_meshBVH(int instanceIndex, Ray _ray, inout Intersection result) {
    int root = instanceInfoBuffer[instanceIndex].meshIndex;
    int starth = stack_h;
    stack[++stack_h] = root;

    mat4 w2l = instanceInfoBuffer[instanceIndex].world2local;
    mat3 rs = mat3(w2l);

    Ray ray;
    ray.ori = (w2l * vec4(_ray.ori, 1)).xyz;
    ray.dir = w2l[0].xyz * _ray.dir[0] + w2l[1].xyz * _ray.dir[1] + w2l[2].xyz * _ray.dir[2]; // keep scale.

    while(stack_h > starth) {
        int id = stack[stack_h--];
        BVHNode cur = meshBVHBuffer[id];

        if(cur.triangleIndex >= 0) {
            // leaf
            Intersection isect = intersect_triangle(ray, cur.triangleIndex);
            if(isect.exist && isect.t < result.t){
                // (l2w^-1)^T * V = w2l^T * V -> V * w2l
                isect.normal = normalize(isect.normal * rs);
                isect.position = _ray.ori + isect.t * _ray.dir;
                result = isect;
                result.instanceIndex = instanceIndex;
            }
        } else {
            float tnear;
            if(!intersect_aabb(ray, vec3(cur.aa), vec3(cur.bb), tnear)) continue;
            if(tnear > result.t) continue;

            stack[++stack_h] = cur.lsIndex;
            stack[++stack_h] = cur.rsIndex;
        }
    }
}

/// 在场景中的第一个交点
Intersection intersect_sceneBVH(Ray ray) {
    Intersection result = nointersect;

    stack_h = 0;
    stack[++stack_h] = 0;

    while(stack_h > 0) {
        int id = stack[stack_h--];
        BVHNode cur = sceneBVHBuffer[id];

        float tnear;
        if(!intersect_aabb(ray, vec3(cur.aa), vec3(cur.bb), tnear)) continue;
        if(tnear > result.t) continue;

        if(cur.instanceIndex >= 0) {
            intersect_meshBVH(cur.instanceIndex, ray, result);
            continue;
        }

        stack[++stack_h] = cur.lsIndex;
        stack[++stack_h] = cur.rsIndex;
    }

    return result;
}


// sample
// ---------------------------------------------- //

// 在三角形上均匀采样
void sample_triangle(in Triangle t, out vec3 position, out vec3 normal) {
    float r1 = sqrt(rand());
    float r2 = rand();
    normal = (t.normal[0] * (1 - r1) + t.normal[1] * r1 * (1 - r2) + t.normal[2] * r1 * r2).xyz;
    position = (t.ver[0] * (1 - r1) + t.ver[1] * r1 * (1 - r2) + t.ver[2] * r1 * r2).xyz;
}

/// 在所有光面上采样 [unused]
/// in pos 当前位置，用于修改处理pdf
/// out t_index 采样到的发光三角形id
//vec3 sample_lightface(vec3 pos, out int t_index, out float pdf) {
//    float tot_area = 0;
//    for(int i = 0;i < light_t_num;i++) {
//        int id = get_light_t_index(i);
//        Triangle t = get_triangle(id);
//        tot_area += t.area;
//    }
//    float rnd = rand() * tot_area;
//    int id; Triangle t;
//    for(int i = 0;i < light_t_num;i++) {
//        id = get_light_t_index(i);
//        t = get_triangle(id);
//        rnd -= t.area;
//        if(rnd < EPS) break;
//    }
//    vec3 light_normal, light_pos;
//    sample_triangle(t, light_pos, light_normal);
//    vec3 dir = normalize(light_pos - pos);
//    float dis = length(light_pos - pos);
//    pdf = 1.0 / t.area / (abs(dot(light_normal, dir)) / (dis * dis));
//    t_index = id;
//    return dir;
//}

// 球面均匀采样
vec3 spherical_sample(out float pdf) {
    float z = rand() * 2 - 1;
    float r = max(0.0, sqrt(1.0 - z * z));
    float phi = 2 * PI * rand();
    vec3 wo = vec3(r * cos(phi), r * sin(phi), z);
    pdf = 0.25 * INV_PI;
    return wo;
}
float spherical_sample_pdf() {
    return 0.25 * INV_PI;
}

vec3 get_background_color(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); // atan returns [-pi, pi] if input (x, y)
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y; // 翻转y;
    vec3 res = texture(skybox, uv).rgb;
    return res; // hdr不需要grammar矫正
}

// 环境光重要性采样 全球面上采样
vec3 skybox_sample(out float pdf) {
    float x = rand();
    float y = rand();
    vec3 samp = texture(skybox_samplecache, vec2(x, y)).xyz;
    float theta = (samp.x - 0.5) * 2 * PI;
    float phi = (1.0 - samp.y - 0.5) * PI;
    float r = cos(phi);
    vec3 wi = vec3(r * cos(theta), sin(phi), r * sin(theta));
    float w2a = (2 * PI * PI * sqrt(1.0 - wi.y * wi.y)) / (SKY_H * SKY_W); // 积分域转换项
    if(w2a == 0) w2a = SKY_W * PI / SKY_H; // 极点情况
    pdf = samp.z / w2a;
    return wi;
}
float skybox_sample_pdf(vec3 v) {
    vec3 l = get_background_color(v);
    float lw = (l.x * 0.2 + l.y * 0.7 + l.z * 0.1) / skybox_Light_SUM;
    float w2a = (2 * PI * PI * sqrt(1.0 - v.y * v.y)) / (SKY_H * SKY_W); // 积分域转换项
    if(w2a == 0) w2a = SKY_W * PI / SKY_H; // 极点情况
    return lw / w2a;
}



// path tracing
// ---------------------------------------------- //

//void shade_mis(Ray ray, in Intersection first_isect, out vec3 resultDI, out vec3 resultGI) {
//
//    resultDI = vec3(0);
//    resultGI = vec3(0);
//    vec3 history = vec3(1); // 栈上乘积
//
//    Intersection isect = first_isect;
//    if(!isect.exist) {
//        resultDI = resultGI = get_background_color(ray.dir);
//        return;
//    }
//
//    for(int dep = 0; dep < MAX_DEPTH; dep++) {
////        InstanceInfo hitMesh = instanceInfoBuffer[isect.meshIndex];
////        if(hitMesh.emission.w > 0) {
////            resultGI += history * hitMesh.emission.xyz;
////            break;
////        }
//        int materialPtr = instanceInfoBuffer[isect.instanceIndex].materialPtr;
//        Frame coord = create_coord(isect.normal);
//        vec3 wi = to_local(coord, -ray.dir);
//        vec3 wo, global_wo;
//        float pdf;
//
//        BSDFQueryRecord bRec = BSDFQueryRecord(wi, vec3(0), materialPtr, isect.uv, 1.0);
//
//        // independent direct lighting
//        for(int li = 0;li < lightCount;li++) {
//
//            Light light = lightBuffer[li];
//            vec3 dir, radiance;
//            if(light.type == 0) {
//                dir = vec3(light.x, light.y, light.z) - isect.position;
//                radiance = vec3(light.r, light.g, light.b) / dot(dir, dir);
//                dir = normalize(dir);
//            } else {
//                dir = -normalize(vec3(light.x, light.y, light.z));
//                radiance = vec3(light.r, light.g, light.b);
//            }
//
//            Intersection tsect = intersect_sceneBVH(Ray(isect.position, dir));
//            if(!tsect.exist) {
//                bRec.wo = to_local(coord, dir);
//                vec3 fr = eval_material(bRec);
//                vec3 recv_col = radiance * fr * abs(bRec.wo.z);
//                resultGI += history * recv_col;
//                if(dep == 0) resultDI = recv_col;
//            }
//        }
//
//        // MIS sampling
//        float pSky = 0.5;
//        vec3 fr;
//        if(rand() < pSky) {
//            global_wo = skybox_sample(pdf);
//            bRec.wo = to_local(coord, global_wo);
//            if(pdf <= 0) break;
//            pdf = pdf * pSky + pdf_material(bRec) * (1 - pSky);
//            fr = eval_material(bRec);
//        } else {
//            //        if(!BRDFSampling) {
//            //            bRec.wo = spherical_sample(pdf);
//            //            fr = eval_material(bRec);
//            //        }
//            fr = sample_material(bRec, pdf);
//            if(pdf <= 0) break;
//            global_wo = to_world(coord, bRec.wo);
//            pdf = skybox_sample_pdf(global_wo) * pSky + pdf * (1 - pSky);
//        }
//        if(pdf <= 0) break;
//
//        ray = Ray(isect.position, global_wo);
//        isect = intersect_sceneBVH(ray);
//        history *= fr * abs(bRec.wo.z) / pdf;
//        if(!isect.exist) {
//            vec3 recv_col = get_background_color(global_wo);
//            resultGI += history * recv_col;
//            if(dep == 0) resultDI = resultGI;
//            break;
//        }
//    }
//}

void shade_mis_advanced(Ray ray, in Intersection first_isect, out vec3 resultDI, out vec3 resultGI) {

    resultDI = vec3(0);
    resultGI = vec3(0);
    vec3 history = vec3(1); // 栈上乘积

    Intersection isect = first_isect;
    if(!isect.exist) {
        resultDI = resultGI = get_background_color(ray.dir);
        return;
    }

    for(int dep = 0; dep < MAX_DEPTH; dep++) {
        //        InstanceInfo hitMesh = instanceInfoBuffer[isect.meshIndex];
        //        if(hitMesh.emission.w > 0) {
        //            resultGI += history * hitMesh.emission.xyz;
        //            break;
        //        }
        int materialPtr = instanceInfoBuffer[isect.instanceIndex].materialPtr;
        Frame coord = create_coord(isect.normal);
        vec3 wi = to_local(coord, -ray.dir);
        vec3 wo, global_wo;
        float pdf;

        BSDFQueryRecord bRec = BSDFQueryRecord(wi, vec3(0), materialPtr, isect.uv, 1.0);

        // skybox direct light
        if(SkyboxLighting && SkyboxSampling) {
            global_wo = skybox_sample(pdf);
            if(pdf > 0) {
                Intersection tsect = intersect_sceneBVH(Ray(isect.position, global_wo));
                if(!tsect.exist) {
                    bRec.wo = to_local(coord, global_wo);
                    vec3 fr = eval_material(bRec);
                    vec3 recv_col = get_background_color(global_wo) * fr * abs(bRec.wo.z) / (pdf + pdf_material(bRec));
                    resultGI += history * recv_col;
                    if(dep == 0) resultDI = recv_col;
                }
            }
        }

        // independent direct lighting
        for(int li = 0;li < lightCount;li++) {

            Light light = lightBuffer[li];
            vec3 dir, radiance; float dis2;
            if(light.type == 0) {
                dir = vec3(light.x, light.y, light.z) - isect.position;
                dis2 = dot(dir, dir);
                radiance = vec3(light.r, light.g, light.b) / dis2;
                dir = normalize(dir);
            } else {
                dir = -normalize(vec3(light.x, light.y, light.z));
                dis2 = INF;
                radiance = vec3(light.r, light.g, light.b);
            }

            Intersection tsect = intersect_sceneBVH(Ray(isect.position, dir));
            if(!tsect.exist || tsect.t * tsect.t > dis2) {
                bRec.wo = to_local(coord, dir);
                vec3 fr = eval_material(bRec);
                vec3 recv_col = radiance * fr * abs(bRec.wo.z);
                resultGI += history * recv_col;
                if(dep == 0) resultDI = recv_col;
            }
        }


        // indirect light
        vec3 fr = sample_material(bRec, pdf);
        if(!BRDFSampling) {
            bRec.wo = spherical_sample(pdf);
            fr = eval_material(bRec);
        }

        global_wo = to_world(coord, bRec.wo);
        if(pdf <= 0) break;

        ray = Ray(isect.position, global_wo);
        isect = intersect_sceneBVH(ray);
        if(!isect.exist) {  // hit sky
            if(SkyboxLighting && SkyboxSampling) {
                vec3 recv_col = get_background_color(global_wo) * fr * abs(bRec.wo.z) / (pdf + skybox_sample_pdf(global_wo));
                resultGI += history * recv_col;
            } else if(SkyboxLighting) {
                vec3 recv_col = get_background_color(global_wo) * fr * abs(bRec.wo.z) / pdf;
                resultGI += history * recv_col;
            }
            break;
        } else {
            history *= fr * abs(bRec.wo.z) / pdf;
        }
    }
}


void main() {
#ifdef COMPUTE_SHADER
    uvec2 pixelIndex = gl_GlobalInvocationID.xy;
    if(pixelIndex.x >= SCREEN_W || pixelIndex.y >= SCREEN_H) return;
    uint pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;
#else
    uvec2 pixelIndex = uvec2(uint(screen_uv.x * SCREEN_W), uint(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    uint pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;
#endif

    uint seed = uint(
    pixelIndex.x * uint(1973) +
    pixelIndex.y * uint(9277) +
    frameCounter * currentspp * uint(26699)) | uint(1);

//    uint seed = frameCounter * SCREEN_W * SCREEN_H + pixelPtr;
    sobolseed = seed;
    sobolcurdim = 1u;

    vec2 p = pixelIndex + jitter;
    float disz = SCREEN_W * 0.5 / tan(fov / 2);
    vec3 ori = vec3(v2wMat * vec4(0, 0, 0, 1));
    vec3 dir = normalize(vec3(v2wMat * vec4(p.x - SCREEN_W / 2, p.y - SCREEN_H / 2, -disz, 0)));
    Ray ray = Ray(ori, dir);


    // viewport at left upper corner.
    vec2 window_uv = vec2(screen_uv.x * SCREEN_W / WINDOW_W,
            (screen_uv.y - 1) * SCREEN_H / WINDOW_H + 1);

    float depth = texture(depthGBufferTexture, window_uv).r;
    int instanceIndex = int(texture(instanceIndexGBufferTexture, window_uv).r + 0.01);

    Intersection first_isect;
    first_isect.t = depth;
    first_isect.exist = depth <= 100000;
    first_isect.normal = normalize(texture(normalGBufferTexture, window_uv).xyz);
    first_isect.uv = texture(uvGBufferTexture, window_uv).xy;
    first_isect.position = ray.ori + ray.dir * first_isect.t;
    first_isect.instanceIndex = instanceIndex;

    if(!RasterizaionFor1st) {
        first_isect = intersect_sceneBVH(ray);
    }

    vec3 resultDI, resultGI, albedo;
    shade_mis_advanced(ray, first_isect, resultDI, resultGI);

    if(any(isnan(resultDI))) resultDI = vec3(0.0);
    if(any(isnan(resultGI))) resultGI = vec3(0.0);

    vec3 resultIDI = resultGI - resultDI;

    // TODO avoid cameraNear clip ?

    if(first_isect.exist) {
        int materialPtr = instanceInfoBuffer[instanceIndex].materialPtr;
        BSDFQueryRecord bRec = BSDFQueryRecord(vec3(0), vec3(0), materialPtr, first_isect.uv, 1.0);
        albedo = albedo_material(bRec);
    } else {
        albedo = resultGI;
    }

    resultDI /= max(vec3(EPS), albedo);
    resultIDI /= max(vec3(EPS), albedo);

    // calculate motion vector
    vec4 lastNDC = backprojMat * vec4(first_isect.position, 1);
    lastNDC /= lastNDC.w;
    vec2 last_suv = (lastNDC.xy + 1.0) / 2;
    vec2 motion = screen_uv - last_suv;

    if(currentspp != 1) {
        vec3 lastdi = vec3(directLumGBuffer[pixelPtr * 3 + 0],
                           directLumGBuffer[pixelPtr * 3 + 1],
                           directLumGBuffer[pixelPtr * 3 + 2]);
        vec3 lastidi = vec3(indirectLumGBuffer[pixelPtr * 3 + 0],
                            indirectLumGBuffer[pixelPtr * 3 + 1],
                            indirectLumGBuffer[pixelPtr * 3 + 2]);
        vec3 lastalbedo = vec3(albedoGBuffer[pixelPtr * 3 + 0],
                               albedoGBuffer[pixelPtr * 3 + 1],
                               albedoGBuffer[pixelPtr * 3 + 2]);
        resultDI = mix(lastdi, resultDI, 1.0 / currentspp);
        resultIDI = mix(lastidi, resultIDI, 1.0 / currentspp);
        albedo = mix(lastalbedo, albedo, 1.0 / currentspp);
    }

    directLumGBuffer[pixelPtr * 3 + 0] = resultDI.x;
    directLumGBuffer[pixelPtr * 3 + 1] = resultDI.y;
    directLumGBuffer[pixelPtr * 3 + 2] = resultDI.z;
    indirectLumGBuffer[pixelPtr * 3 + 0] = resultIDI.x;
    indirectLumGBuffer[pixelPtr * 3 + 1] = resultIDI.y;
    indirectLumGBuffer[pixelPtr * 3 + 2] = resultIDI.z;
    albedoGBuffer[pixelPtr * 3 + 0] = albedo.x;
    albedoGBuffer[pixelPtr * 3 + 1] = albedo.y;
    albedoGBuffer[pixelPtr * 3 + 2] = albedo.z;

    motionGBuffer[pixelPtr * 2 + 0] = motion.x;
    motionGBuffer[pixelPtr * 2 + 1] = motion.y;
    depthGBuffer[pixelPtr] = depth;
    normalGBuffer[pixelPtr * 3 + 0] = first_isect.normal.x;
    normalGBuffer[pixelPtr * 3 + 1] = first_isect.normal.y;
    normalGBuffer[pixelPtr * 3 + 2] = first_isect.normal.z;
    instanceIndexGBuffer[pixelPtr] = first_isect.instanceIndex;

    return;
}