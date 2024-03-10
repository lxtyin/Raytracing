
#version 460 core

#extension GL_ARB_bindless_texture : require

//#define matrix44_ptr uint
//#define triangle_ptr uint
//
//uniform samplerBuffer uniformbuffer;

#define M_SIZ 8    // 一个material占的vec3数量
#define T_SIZ 9    // 一个triangle占的vec3数量
#define B_SIZ 3    // 一个bvhnode占的vec3数量

#include shader/basic/math.frag

in float pixel_x;
in float pixel_y;
in vec2 screen_uv;


layout(location = 0) out vec3 color_out;
layout(location = 1) out vec3 albedo_out;
layout(location = 2) out vec3 normal_out;
layout(location = 3) out vec3 worldpos_out;


// memory
// ---------------------------------------------- //

uniform samplerBuffer triangles;
uniform samplerBuffer lightidxs;
uniform samplerBuffer bvhnodes;
uniform int light_t_num;
uniform int triangle_num;

uniform mat4 v2w_mat;

uniform int SPP = 1;
uniform int MAX_DEPTH = 3;
uniform float fov = PI / 3;
uniform bool fast_shade = true; // 仅渲染diffuse_color

uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D skybox;
uniform sampler2D skybox_samplecache;
uniform float skybox_Light_SUM;
uniform int SKY_W;
uniform int SKY_H;
uniform uint frameCounter;


layout(binding = 0, std430) readonly buffer ssbo0 {
    sampler2D textureBuffer[];
};

layout(binding = 1, std430) readonly buffer ssbo1 {
    float materialBuffer[];
};

struct Triangle {
    vec4 ver[3];
    vec4 normal[3];
    vec2 uv[3];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    Triangle triangleBuffer[];
};

struct MeshInfo {
    mat4 world2local;
    vec4 emission; // emission.z = is_emission (-1 or 1)
    int materialPtr;
    int emptyblock[11];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    MeshInfo meshInfoBuffer[];
};

struct BVHNode {
    vec4 aa, bb; // actually vec3
    int lsIndex;
    int rsIndex;
    int meshIndex;
    int triangleIndex;
};
layout(binding = 4, std430) readonly buffer ssbo4 {
    BVHNode bvhNodeBuffer[];
};


uint seed = uint(
uint(pixel_x + SCREEN_W / 2) * uint(1973) +
uint(pixel_y + SCREEN_H / 2) * uint(9277) +
uint(frameCounter) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float rand() {
    return float(wang_hash(seed)) / 4294967296.0;
}

#include shader/materials/materials.frag

// math
// ---------------------------------------------- //

struct Coord {
    vec3 s, t, n;
};

Coord create_coord(vec3 n) {
    Coord f;
    if(abs(n.z) > 1 - EPS) f.s = cross(n, vec3(1, 0, 0));
    else f.s = cross(n, vec3(0, 0, 1));
    f.t = cross(n, f.s);
    f.n = n;
    return f;
}

vec3 to_world(in Coord f, vec3 v) {
    return v.x * f.s + v.y * f.t + v.z * f.n;
}
vec3 to_local(in Coord f, vec3 v) {
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

// data source
// ---------------------------------------------- //

// 获取第i个三角形光源的index
int get_light_t_index(int i) {
    return roundint(texelFetch(lightidxs, i).x);
}

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

struct Intersect {
    bool exist;
    vec3 pos;
    vec3 normal;   // 击中平面的法线（向物体外）
    float t;
    vec2 uv;
    int materialPtr;
    int meshIndex;
    int triangleIndex;
};
const Intersect nointersect = Intersect(false, vec3(0), vec3(0), INF, vec2(0), -1, -1, -1);

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
    return tmi < tmx + RAY_EPS;
}

/// 返回与triangle的具体碰撞信息
Intersect intersect_triangle(Ray ray, int triangleIndex) {
    Triangle tri = triangleBuffer[triangleIndex];

    vec3 E1 = tri.ver[1].xyz - tri.ver[0].xyz;
    vec3 E2 = tri.ver[2].xyz - tri.ver[0].xyz;
    vec3 S = ray.ori - tri.ver[0].xyz;
    vec3 S1 = cross(ray.dir, E2);
    vec3 S2 = cross(S, E1);
    float k = 1.0 / dot(S1, E1);
    float t = dot(S2, E2) * k;
    float u = dot(S1, S) * k;
    float v = dot(S2, ray.dir) * k;

    bool exist = (RAY_EPS < t && 0 < u && 0 < v && u + v < 1);

    vec3 pos = ray.ori + t * ray.dir;
    vec3 nor = normalize(tri.normal[0].xyz * (1 - u - v) + tri.normal[1].xyz * u + tri.normal[2].xyz * v);

    return Intersect(exist, pos, nor, t, interpolate_uv(tri, u, v), -1, -1, triangleIndex);
}

int stack_h = 0;
int stack[256];
void intersect_meshBVH(int rootIndex, Ray ray, int meshIndex, inout Intersect result) {
    int starth = stack_h;
    stack[++stack_h] = rootIndex;

    while(stack_h > starth) {
        int id = stack[stack_h--];
        BVHNode cur = bvhNodeBuffer[id];

        if(cur.triangleIndex >= 0) {
            // leaf
            Intersect isect = intersect_triangle(ray, cur.triangleIndex);
            if(isect.exist && isect.t < result.t){
                result = isect;
                result.meshIndex = meshIndex;
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
Intersect intersect_sceneBVH(Ray ray) {

    Intersect result = nointersect;

    stack_h = 0;
    stack[++stack_h] = 0;

    while(stack_h > 0) {
        int id = stack[stack_h--];
        BVHNode cur = bvhNodeBuffer[id];

        if(cur.meshIndex >= 0) {
//            mat4 w2l = meshInfoBuffer[cur.meshIndex].world2local; // todo
//
//            vec3 o = (w2l * vec4(ray.ori, 1)).xyz;
//            vec3 t = normalize((w2l * vec4(ray.dir, 0)).xyz);
//            intersect_meshBVH(id, Ray(o, t), cur.meshIndex, result);
            intersect_meshBVH(id, ray, cur.meshIndex, result);

        } else {
            float tnear;
            if(!intersect_aabb(ray, vec3(cur.aa), vec3(cur.bb), tnear)) continue;
            if(tnear > result.t) continue;

            stack[++stack_h] = cur.lsIndex;
            stack[++stack_h] = cur.rsIndex;
        }
    }

    MeshInfo meshInfo = meshInfoBuffer[result.meshIndex];
    result.materialPtr = meshInfo.materialPtr;
    // TODO: emission and others

    return result;
}

//int stack_h = 0;
//struct RayQuery {
//    int bvhNodeIndex;
//    int meshIndex;
//    float nearT; // intersect t of ray and boundingbox.
//    Ray ray;
//} stack[256];
//
//Intersect intersect_sceneBVH(Ray ray) {
//
//    Intersect result = nointersect;
//
//    stack_h = 0;
//    stack[++stack_h] = RayQuery(0, -1, INF, ray);
//
//    if(stack[1].meshIndex >= 0) {
////        mat4 w2l = meshInfoBuffer[stack[1].meshIndex].world2local;
////        vec3 o = (w2l * vec4(rq.ray.ori, 1)).xyz;
////        vec3 t = (w2l * vec4(rq.ray.dir, 0)).xyz;
////        stack[1].ray = Ray(o, t);
//    }
//
//    while(stack_h > 0) {
//        RayQuery rq = stack[stack_h--];
//        BVHNode cur = bvhNodeBuffer[rq.bvhNodeIndex];
//        if(rq.nearT > result.t) continue;
//
//        if(cur.triangleIndex >= 0) {
//            // leaf
//            Intersect isect = intersect_triangle(rq.ray, cur.triangleIndex);
//            if(isect.exist && isect.t < result.t){
//                result = isect;
//                result.meshIndex = rq.meshIndex;
//            }
//        }  else {
//            BVHNode ls = bvhNodeBuffer[cur.lsIndex];
//            BVHNode rs = bvhNodeBuffer[cur.rsIndex];
//            RayQuery rql = RayQuery(cur.lsIndex, rq.meshIndex, INF, rq.ray);
//            if(ls.meshIndex >= 0) {
////                mat4 w2l = meshInfoBuffer[ls.meshIndex].world2local;
////                vec3 o = (w2l * vec4(rq.ray.ori, 1)).xyz;
////                vec3 t = (w2l * vec4(rq.ray.dir, 0)).xyz;
////                rql.ray = Ray(o, t);
//                rql.meshIndex = ls.meshIndex;
//            }
//
//            RayQuery rqr = RayQuery(cur.rsIndex, rq.meshIndex, INF, rq.ray);
//            if(rs.meshIndex >= 0) {
////                mat4 w2l = meshInfoBuffer[rs.meshIndex].world2local;
////                vec3 o = (w2l * vec4(rq.ray.ori, 1)).xyz;
////                vec3 t = (w2l * vec4(rq.ray.dir, 0)).xyz;
////                rqr.ray = Ray(o, t);
//                rqr.meshIndex = rs.meshIndex;
//            }
//
//            float lt = -1, rt = -1;
//            if(intersect_aabb(rql.ray, vec3(ls.aa), vec3(ls.bb), lt) && lt < result.t) {
//                rql.nearT = lt;
//                stack[++stack_h] = rql;
//            } else lt = -1;
//            if(intersect_aabb(rqr.ray, vec3(rs.aa), vec3(rs.bb), rt) && rt < result.t) {
//                rqr.nearT = rt;
//                stack[++stack_h] = rqr;
//            } else rt = -1;
//            if(lt >= 0 && rt >= 0 && rt > lt) {
//                RayQuery tmp = stack[stack_h];
//                stack[stack_h] = stack[stack_h - 1];
//                stack[stack_h - 1] = tmp;
//            }
//        }
//    }
//
//    MeshInfo meshInfo = meshInfoBuffer[result.meshIndex];
//    result.materialPtr = meshInfo.materialPtr;
//    // TODO: emission and others
//
//    return result;
//}


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

vec3 shade(Ray ray, in Intersect first_isect) {

    vec3 result = vec3(0);
    vec3 history = vec3(1); // 栈上乘积

    Intersect isect = first_isect;
    if(!isect.exist) return get_background_color(ray.dir);

    for(int dep = 0; dep < MAX_DEPTH; dep++) {

//        if(m1.is_emit) {
//            result += history * m1.emission;
//            break;
//        }
        Coord coord = create_coord(isect.normal);
        vec3 wi = to_local(coord, -ray.dir);
        vec3 wo, global_wo;
        float pdf;
        global_wo = skybox_sample(pdf);

        BSDFQueryRecord bRec = BSDFQueryRecord(wi, to_local(coord, global_wo), isect.materialPtr, isect.uv);

        // direct light
        if(pdf > 0) {
            Intersect tsect = intersect_sceneBVH(Ray(isect.pos, global_wo));
            if(!tsect.exist) {
                vec3 fr = eval_material(bRec);
                vec3 recv_col = get_background_color(global_wo) * fr * abs(bRec.wo.z) / (pdf + pdf_material(bRec));
                result += history * recv_col;
            }
        }

        // indirect light
        vec3 fr = sample_material(bRec, pdf);
        global_wo = to_world(coord, bRec.wo);
        if(pdf <= 0) break;

        ray = Ray(isect.pos, global_wo);
        isect = intersect_sceneBVH(ray);
        if(!isect.exist) {  // hit sky
            vec3 recv_col = get_background_color(global_wo) * fr * abs(bRec.wo.z) / (pdf + skybox_sample_pdf(global_wo));
            result += history * recv_col;
            break;
        } else {
            history *= fr * abs(bRec.wo.z) / pdf;
        }
    }
    if(result.x < 0 || result.y < 0 || result.z < 0) result = vec3(100000, 0, 0);
    return result;
}

void main() {

    float dis_z = SCREEN_W * 0.5 / tan(fov / 2);
    vec3 w_ori = vec3(v2w_mat * vec4(0, 0, 0, 1));
    vec3 w_tar = vec3(v2w_mat * vec4(pixel_x, pixel_y, -dis_z, 1));
    vec3 dir = normalize(w_tar - w_ori);
    Ray ray = Ray(w_ori, dir);


    // 预设
    albedo_out = vec3(1, 1, 1);
    normal_out = vec3(1, 1, 1);
    worldpos_out = vec3(1, 1, 1);

//    Intersect ttt = intersect_sceneBVH(ray);
//    if(!ttt.exist) {
//        color_out = get_background_color(ray.dir);
//    } else {
//        vec3 albedo = vec3(materialBuffer[ttt.materialPtr + 1],
//        materialBuffer[ttt.materialPtr + 2],
//        materialBuffer[ttt.materialPtr + 3]);
//        color_out = albedo;
//    }
//    return;

    Intersect isect = intersect_sceneBVH(ray);
    if(!isect.exist) {
        worldpos_out = vec3(10000);
        color_out = get_background_color(ray.dir);
        return;
    }

//    Triangle tr1 = get_triangle(isect.t_index);
//    Material m1 = get_material(tr1.m_index);
//    vec2 uv = interpolate_uv(tr1, isect.u, isect.v);
//    albedo_out = get_diffuse_color(m1, uv);
//    normal_out = isect.normal;
//
//    if(fast_shade) {
//        color_out = get_diffuse_color(m1, uv);
//        return;
//    }

    vec3 result = vec3(0);
    for(int i = 1;i <= SPP;i++) result += shade(ray, isect);
    result /= SPP;

    if(any(isnan(result))) result = vec3(0, 0, 0); // Minimal probability

    worldpos_out = isect.pos;
    color_out = result;

}