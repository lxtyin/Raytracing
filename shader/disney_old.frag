/*
    This version samples directional light by light_triangle_index.
*/
#version 330 core

#define M_SIZ 7    // 一个material占的vec3数量
#define T_SIZ 9    // 一个triangle占的vec3数量
#define B_SIZ 3    // 一个bvhnode占的vec3数量
#define INF 1E18
#define PI 3.1415926
#define IVPI 0.3183098
#define EPS 0.0001
#define RAY_EPS 0.01

in float pixel_x;
in float pixel_y;
in vec2 screen_uv;

layout(location = 0) out vec4 color_out;
layout(location = 1) out vec4 albedo_out;
layout(location = 2) out vec4 normal_out;


// memory
// ---------------------------------------------- //

uniform samplerBuffer materials;
uniform samplerBuffer triangles;
uniform samplerBuffer lightidxs;
uniform samplerBuffer bvhnodes;
uniform int light_t_num;
uniform int triangle_num;
uniform mat4 v2w_mat;

uniform float RussianRoulette = 0.8;
uniform int SPP = 1;
uniform float fov = PI / 3;
uniform bool fast_shade = true; // 仅渲染diffuse_color

uniform int SCREEN_W;
uniform int SCREEN_H;
uniform uint frameCounter;
uniform sampler2D last_frame_texture;
uniform sampler2D skybox;
uniform sampler2D skybox_samplecache;
uniform sampler2D texture_list[25];
uniform int SKY_W;
uniform int SKY_H;

// math
// ---------------------------------------------- //
int stack[256];
int stack_h = 0;

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

int roundint(float x) {
    if(x < 0) return int(x - 0.5);
    else return int(x + 0.5);
}

float pow2(float x) {
    return x * x;
}
float pow5(float x) {
    float x2 = x * x;
    return x2 * x2 * x;
}
vec3 powv(vec3 v, float n) {
    return vec3(pow(v.x, n), pow(v.y, n), pow(v.z, n));
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

struct Triangle {
    vec3 ver[3];
    vec2 uv[3];
    vec3 normal[3];
    float area;
    int m_index;
};

struct Material {
    vec3 base_color;
    vec3 emission;
    bool is_emit;
    float metallic;
    float roughness;
    float specular;
    float specular_tint;
    float sheen;
    float sheen_tint;
    float subsurface;
    float clearcoat;
    float clearcoat_gloss;
    float anisotropic;
    float index_of_refraction;
    float spec_trans;

    int diffuse_map_idx;
    int metalness_map_idx;
    int roughness_map_idx;
    int normal_map_idx;
};

struct BVHNode {
    vec3 aa, bb;
    int l, r;
    int t_index;
};

Triangle get_triangle(int i) {
    Triangle r;
    r.ver[0] = texelFetch(triangles, i * T_SIZ).xyz;
    r.ver[1] = texelFetch(triangles, i * T_SIZ + 1).xyz;
    r.ver[2] = texelFetch(triangles, i * T_SIZ + 2).xyz;
    r.normal[0] = texelFetch(triangles, i * T_SIZ + 3).xyz;
    r.normal[1] = texelFetch(triangles, i * T_SIZ + 4).xyz;
    r.normal[2] = texelFetch(triangles, i * T_SIZ + 5).xyz;
    r.area = length(cross(r.ver[1] - r.ver[0], r.ver[2] - r.ver[0])) * 0.5f;
    vec3 u = texelFetch(triangles, i * T_SIZ + 6).xyz;
    vec3 v = texelFetch(triangles, i * T_SIZ + 7).xyz;
    r.uv[0].x = u.x;
    r.uv[1].x = u.y;
    r.uv[2].x = u.z;
    r.uv[0].y = v.x;
    r.uv[1].y = v.y;
    r.uv[2].y = v.z;
    r.m_index = roundint(texelFetch(triangles, i * T_SIZ + 8).x);
    return r;
}

// 获取第i个三角形光源的index
int get_light_t_index(int i) {
    return roundint(texelFetch(lightidxs, i).x);
}

Material get_material(int i) {
    Material r;
    r.base_color = texelFetch(materials, i * M_SIZ).xyz;
    r.emission = texelFetch(materials, i * M_SIZ + 1).xyz;
    vec3 tmp = texelFetch(materials, i * M_SIZ + 2).xyz;
    r.is_emit = tmp.x != 0;
    r.metallic = tmp.y;
    r.roughness = tmp.z;
    tmp = texelFetch(materials, i * M_SIZ + 3).xyz;
    r.specular = tmp.x;
    r.specular_tint = tmp.y;
    r.sheen = tmp.z;
    tmp = texelFetch(materials, i * M_SIZ + 4).xyz;
    r.sheen_tint = tmp.x;
    r.subsurface = tmp.y;
    r.clearcoat = tmp.z;
    tmp = texelFetch(materials, i * M_SIZ + 5).xyz;
    r.clearcoat_gloss = tmp.x;
    r.anisotropic = tmp.y;
    r.diffuse_map_idx = int(tmp.z);
    tmp = texelFetch(materials, i * M_SIZ + 6).xyz;
    r.metalness_map_idx = int(tmp.x);
    r.roughness_map_idx = int(tmp.y);
    r.normal_map_idx = int(tmp.z);

    r.index_of_refraction = 1.33;
    r.spec_trans = 0.7;
    return r;
}

BVHNode get_bvhnode(int i) {
    BVHNode r;
    r.aa = texelFetch(bvhnodes, i * B_SIZ).xyz;
    r.bb = texelFetch(bvhnodes, i * B_SIZ + 1).xyz;
    vec3 tmp = texelFetch(bvhnodes, i * B_SIZ + 2).xyz;
    r.l = roundint(tmp.x);
    r.r = roundint(tmp.y);
    r.t_index = roundint(tmp.z);
    return r;
}

// material accessor
// ---------------------------------------------- //

vec3 get_background_color(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y)); // atan returns [-pi, pi] if input (x, y)
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y; // 翻转y;
    vec3 res = texture(skybox, uv).rgb;
    return res; // hdr不需要grammar矫正
}

vec3 get_diffuse_color(Material m, vec2 uv) {
    if(m.diffuse_map_idx == -1) return m.base_color;
    return powv(texture(texture_list[m.diffuse_map_idx], uv).rgb, 2.2f);
}
float get_metallic(Material m, vec2 uv) {
    if(m.metalness_map_idx == -1) return m.metallic;
    return texture(texture_list[m.metalness_map_idx], uv).x;
}
float get_roughness(Material m, vec2 uv) {
    if(m.roughness_map_idx == -1) return max(0.01f, m.roughness);
    return max(0.01f, texture(texture_list[m.roughness_map_idx], uv).x);
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
    int t_index;
    float u, v, t; // 击中的三角形uv，
};
const Intersect nointersect = Intersect(false, vec3(0), vec3(0), -1, 0, 0, INF);

/// 检测是否与AABB碰撞，返回最早碰撞时间
bool intersect_aabb(Ray ray, vec3 aa, vec3 bb, out float t_near) {
    float tmi = 0, tmx = INF;
    vec3 t1 = (aa - ray.ori) / ray.dir;
    vec3 t2 = (bb - ray.ori) / ray.dir;
    tmi = max(tmi, min(t1.x, t2.x));
    tmi = max(tmi, min(t1.y, t2.y));
    tmi = max(tmi, min(t1.z, t2.z));
    tmx = min(tmx, max(t1.x, t2.x));
    tmx = min(tmx, max(t1.y, t2.y));
    tmx = min(tmx, max(t1.z, t2.z));
    t_near = tmi;
    return tmi < tmx + RAY_EPS;
}

/// 返回与triangle的具体碰撞信息
Intersect intersect_triangle(Ray ray, int i) {
    Triangle tri = get_triangle(i);

    vec3 E1 = tri.ver[1] - tri.ver[0];
    vec3 E2 = tri.ver[2] - tri.ver[0];
    vec3 S = ray.ori - tri.ver[0];
    vec3 S1 = cross(ray.dir, E2);
    vec3 S2 = cross(S, E1);
    float k = 1.0 / dot(S1, E1);
    float t = dot(S2, E2) * k;
    float u = dot(S1, S) * k;
    float v = dot(S2, ray.dir) * k;
    if(RAY_EPS < t && 0 < u && 0 < v && u + v < 1) {
        vec3 pos = ray.ori + t * ray.dir;
        vec3 nor = normalize(tri.normal[0] * (1 - u - v) + tri.normal[1] * u + tri.normal[2] * v);
//        if(dot(ray.dir, nor) > 0) nor = -nor;
        return Intersect(true, pos, nor, i, u, v, t);
    } else {
        return nointersect;
    }
}

/// 在场景中的第一个交点
Intersect get_intersect(Ray ray) {

    float t_near = INF, t_tmp;
    Intersect res = nointersect;

    stack_h = 0;
    stack[++stack_h] = 0;
    while(stack_h > 0) {
        int id = stack[stack_h--];
        BVHNode cur = get_bvhnode(id);

        if(cur.t_index != -1) {  // leaf
            Intersect inter = intersect_triangle(ray, cur.t_index);
            if(inter.exist && inter.t < t_near){
                res = inter;
                t_near = inter.t;
            }
        } else {
            if(!intersect_aabb(ray, cur.aa, cur.bb, t_tmp)) continue;
            if(t_tmp > t_near) continue;

            stack[++stack_h] = cur.l;
            stack[++stack_h] = cur.r;
        }
    }
    return res;
}

// sample
// ---------------------------------------------- //

// 在三角形上均匀采样
void sample_triangle(Triangle t, out vec3 position, out vec3 normal) {
    float r1 = sqrt(rand());
    float r2 = rand();
    normal = t.normal[0] * (1 - r1) + t.normal[1] * r1 * (1 - r2) + t.normal[2] * r1 * r2;
    position = t.ver[0] * (1 - r1) + t.ver[1] * r1 * (1 - r2) + t.ver[2] * r1 * r2;
}

/// 在所有光面上采样
/// in pos 当前位置，用于修改处理pdf
/// out t_index 采样到的发光三角形id
vec3 sample_lightface(vec3 pos, out int t_index, out float pdf) {
    float tot_area = 0;
    for(int i = 0;i < light_t_num;i++) {
        int id = get_light_t_index(i);
        Triangle t = get_triangle(id);
        tot_area += t.area;
    }
    float rnd = rand() * tot_area;
    for(int i = 0;i < light_t_num;i++) {
        int id = get_light_t_index(i);
        Triangle t = get_triangle(id);
        rnd -= t.area;
        if(rnd < EPS) {
            vec3 light_normal, light_pos;
            sample_triangle(t, light_pos, light_normal);
            vec3 dir = normalize(light_pos - pos);
            float dis = length(light_pos - pos);
            pdf = 1.0 / t.area / (abs(dot(light_normal, dir)) / (dis * dis));
            t_index = id;
            return dir;
        }
    }
    return vec3(0);
}

// 球面均匀采样
vec3 simple_sample(out float pdf) {
    float z = rand() * 2 - 1;
    float r = max(0.0, sqrt(1.0 - z * z));
    float phi = 2 * PI * rand();
    vec3 wo = vec3(r * cos(phi), r * sin(phi), z);
    pdf = 0.25 * IVPI;
    return wo;
}

// D项 半法线 重要性采样 半球面采样
vec3 importance_sample_GTR2(Material m, vec2 uv, out float pdf) {
    float m_roughness = get_roughness(m, uv);
    float alpha2 = max(0.01, m_roughness * m_roughness);
    float x = rand();

    float cos_theta = sqrt((1. - x) / (x * (alpha2 - 1) + 1));
    float cos2 = cos_theta * cos_theta;
    float phi = rand();
    float r = sqrt(1. - cos2);

    vec3 h = normalize(vec3(r * cos(phi), r * sin(phi), cos_theta));
    pdf = 2. * alpha2 * cos_theta / pow2(cos2 * (alpha2 - 1.) + 1.) / (2 * PI);
    return h;
}
// 环境光重要性采样 全球面上采样
vec3 importance_sample_skybox(out float pdf) {
    float x = rand();
    float y = rand();
    vec3 samp = texture(skybox_samplecache, vec2(x, y)).xyz;
    float theta = (samp.x - 0.5) * 2 * PI;
    float phi = (1.0 - samp.y - 0.5) * PI;
    float r = cos(phi);
    vec3 wi = vec3(r * cos(theta), sin(phi), r * sin(theta));
    float w2a = (2 * PI * PI * sqrt(1.0 - wi.y * wi.y)) / (SKY_H * SKY_W); // 积分域转换项
    pdf = samp.z / w2a;
    return wi;
}

// bxdf
// ---------------------------------------------- //

float smithG_GGX(float NdotV, float a) {
    float a2 = a * a;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a2 + b - a2*b));
}
float GTR2(float NdotH, float a) {
    float a2 = a * a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}
vec3 SchlickFresnel(vec3 f0, float c) {
    return f0 + (vec3(1) - f0) * pow5(1 - c);
}
float fresnel(float cosI, float etaI, float etaT) {
    float sinI = sqrt(1.0 - cosI * cosI);
    float sinT = etaI / etaT * sinI;
    float cosT = sqrt(1.0 - sinT * sinT);
    float Rl = pow2((etaI * cosI - etaT * cosT) / (etaI * cosI + etaT * cosT));
    float Rp = pow2((etaI * cosT - etaT * cosI) / (etaI * cosT + etaT * cosI));
    return (Rl + Rp) / 2;
}

// L(in), N(same direction with L), V(out)
vec3 btdf(Material m, vec3 L, vec3 V, vec3 N, vec2 uv, float ior1, float ior2) {

    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    if(NdotL <= 0 || NdotV >= 0) return vec3(0);
    float eta = ior2 / ior1;
    vec3 H = -normalize(L + eta * V);
    float NdotH = dot(N, H);
    if(NdotH <= 0) H = -H;
    float LdotH = dot(L, H);
    float VdotH = dot(V, H);
    if(LdotH * VdotH >= 0) return vec3(0);

    float m_roughness = get_roughness(m, uv);
    float m_metallic = get_metallic(m, uv);
    m_roughness = 0.1;
    m_metallic = 0.5;

    vec3 m_transmission = sqrt(get_diffuse_color(m, uv)) * m.spec_trans;

    float sqrtDenom = LdotH + eta * VdotH;
    float Ds = GTR2(NdotH, m_roughness);
    float iFs = 1 - fresnel(LdotH, ior1, ior2);
    if(iFs <= 0) return vec3(0); // 全反射
    float Gs = smithG_GGX(abs(NdotL), m_roughness);
    Gs *= smithG_GGX(abs(NdotV), m_roughness);

    vec3 refraction = abs((LdotH * VdotH * pow2(eta) * Gs * iFs * Ds * m_transmission)
    / (NdotV * NdotL * pow2(sqrtDenom)));

    return refraction;
}

vec3 brdf(Material m, vec3 L, vec3 V, vec3 N, vec2 uv) {
    vec3 H = normalize(L + V);
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);
    if(NdotL <= 0 || NdotV <= 0) return vec3(0); // assert!

    float m_roughness = get_roughness(m, uv);
    float m_metallic = get_metallic(m, uv);

    vec3 Cdlin = get_diffuse_color(m, uv);  // 线性空间下的漫反射颜色
    float Cdlum = .3*Cdlin.x + .6*Cdlin.y  + .1*Cdlin.z; // 计算亮度
    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1);     // normalize lum. to isolate hue+sat
    vec3 Cspec = m.specular * mix(vec3(1.0f), Ctint, m.specular_tint); // 插值得到镜面反射颜色
    vec3 Cspec0 = mix(0.08 * Cspec, Cdlin, m_metallic);     // 0° 镜面反射颜色, 即菲涅尔项中的F0

    // diffuse
    float FL = pow5(1 - NdotL), FV = pow5(1 - NdotV);
    float Fd90 = 0.5 + 2 * LdotH*LdotH * m_roughness;
    float Fd = mix(1.0f, Fd90, FL) * mix(1.0f, Fd90, FV);
    vec3 diffuse = IVPI * Fd * Cdlin * (1 - m.spec_trans);

    // subsurface
    //    float Fss90 = LdotH * LdotH * m_roughness;
    //    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    //    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);
    //    vec3 diffuse = IVPI * mix(Fd, ss, m.subsurface) * Cdlin;

    // specular
    float Ds = GTR2(NdotH, m_roughness);
    vec3 Fs = SchlickFresnel(Cspec0, LdotH);
    float Gs = smithG_GGX(NdotL, m_roughness);
    Gs *= smithG_GGX(NdotV, m_roughness);

    vec3 specular = Gs * Fs * Ds / (4 * NdotV * NdotL);

    // 菲涅尔项已经蕴含了金属度
    return diffuse * (1 - m_metallic) + specular;
}

// 在m上，L方向入射，V方向出射，面法向为N（指向外），纹理坐标uv
vec3 bxdf(Material m, vec3 L, vec3 V, vec3 N, vec2 uv) {
    vec3 LN = dot(L, N) > 0 ? N : -N;
    if(dot(V, LN) >= 0) {
        return brdf(m, L, V, LN, uv);
    } else {
        float ior1 = 1, ior2 = m.index_of_refraction;
        if(LN != N) { // swap
            ior1 += ior2;
            ior2 = ior1 - ior2;
            ior1 -= ior2;
        }
        return btdf(m, L, V, LN, uv, ior1, ior2);
    }
}

// path tracing
// ---------------------------------------------- //

vec3 shade(Ray ray) {

    vec3 result = vec3(0);
    vec3 history = vec3(1); // 栈上乘积

    Intersect inter1 = get_intersect(ray);
    if(!inter1.exist) return get_background_color(ray.dir);

    Triangle tr1 = get_triangle(inter1.t_index);
    Material m1 = get_material(tr1.m_index);

    while(true) {

        // 自发光
        if(m1.is_emit) {
            result += history * m1.emission;
            break;
        }

        float pdf;
        int light_t_index = -1;

        vec3 pos = inter1.pos;
        vec3 wi = sample_lightface(pos, light_t_index, pdf); // 光源采样
        vec3 wo = -ray.dir;
        vec3 nor = inter1.normal;                      // 指向物体外的法线
        vec2 uv = tr1.uv[0] * (1 - inter1.u - inter1.v) +
                  tr1.uv[1] * inter1.u + tr1.uv[2] * inter1.v;

        Triangle tr2 = get_triangle(light_t_index);
        Material m2 = get_material(tr2.m_index);

        // 直接光
        Intersect test = get_intersect(Ray(pos, wi));
        if(test.exist && test.t_index == light_t_index) {
            vec3 f_r = bxdf(m1, wi, wo, nor, uv);
            vec3 recv_col = m2.emission * f_r * abs(dot(nor, wi)) / pdf;
            result += history * recv_col;
        }

        // 间接光
        if(rand() < RussianRoulette) {

//            vec3 h = importance_sample_GTR2(m1, uv, pdf);
//            wi = reflect(-wo, to_world(h, nor));

//            wi = simple_sample(pdf);
//            wi = to_world(wi, nor);

            wi = importance_sample_skybox(pdf);

            vec3 f_r = bxdf(m1, wi, wo, nor, uv);

            ray = Ray(pos, wi);
            Intersect test = get_intersect(ray);
            if(!test.exist) {
                vec3 recv_col = get_background_color(wi) * f_r * abs(dot(nor, wi)) / pdf / RussianRoulette;
                result += history * recv_col;
                break;
            }

            history *= f_r * abs(dot(nor, wi)) / pdf / RussianRoulette;
            Triangle tr3 = get_triangle(test.t_index);
            Material m3 = get_material(tr3.m_index);
            if(test.t_index != light_t_index) {
                inter1 = test;
                tr1 = tr3;
                m1 = m3;
            } else break;
        } else break;
    }
    return result;
}

void main() {

    float dis_z = SCREEN_W * 0.5 / tan(fov / 2);
    vec3 w_ori = vec3(v2w_mat * vec4(0, 0, 0, 1));
    vec3 w_tar = vec3(v2w_mat * vec4(pixel_x, pixel_y, -dis_z, 1));
    vec3 dir = normalize(w_tar - w_ori);
    Ray ray = Ray(w_ori, dir);

    // 预设
    albedo_out = vec4(1, 1, 1, 1);
    normal_out = vec4(1, 1, 1, 1);
    Intersect inter1 = get_intersect(ray);
    if(!inter1.exist) {
        color_out = vec4(get_background_color(ray.dir), 1);
        return;
    }

    Triangle tr1 = get_triangle(inter1.t_index);
    Material m1 = get_material(tr1.m_index);
    vec2 uv = tr1.uv[0] * (1 - inter1.u - inter1.v) +
            tr1.uv[1] * inter1.u + tr1.uv[2] * inter1.v;
    albedo_out = vec4(get_diffuse_color(m1, uv), 1);
    normal_out = vec4(inter1.normal, 1);

    if(fast_shade) {
        color_out = vec4(get_diffuse_color(m1, uv), 1);
        return;
    }

    vec3 result = vec3(0);
    for(int i = 0;i < SPP;i++) result += shade(ray);
    result /= SPP;

    // mix last frame
    vec3 last_col = texture(last_frame_texture, screen_uv).xyz;

//    color_out = vec4(result, 1.0);
    color_out = vec4(mix(last_col, result, 1.0 / frameCounter), 1); // 所有帧混合
//    color_out = vec4(mix(last_col, result, 0.2), 1); // 所有帧混合
}