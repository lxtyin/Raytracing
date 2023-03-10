
#version 330 core

#define M_SIZ 7    // 一个material占的vec3数量
#define T_SIZ 9    // 一个triangle占的vec3数量
#define B_SIZ 3    // 一个bvhnode占的vec3数量
#define INF 1E18
#define PI 3.1415926
#define IVPI 0.3183098
#define EPS 0.0001

in float pixel_x;
in float pixel_y;
in vec2 screen_uv;
out vec4 FragColor;

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
uniform sampler2D texture_list[16];

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

// n = +z
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
    // hdr取样
    // todo 环境根据亮度的重要性采样
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y; // 翻转y;
    vec3 res = texture(skybox, uv).rgb;
    res = min(res, vec3(10));
    return powv(res, 2.2f);
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
    vec3 normal; // 击中平面的法线（向光线方向）
    int t_index;
    float u, v, t;
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
    return tmi < tmx + EPS;
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
    if(EPS < t && 0 < u && 0 < v && u + v < 1) {
        vec3 pos = ray.ori + t * ray.dir;
        // 插值法线
        vec3 nor = normalize(tri.normal[0] * (1 - u - v) + tri.normal[1] * u + tri.normal[2] * v);
        if(dot(ray.dir, nor) > 0) nor = -nor;
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
vec3 sample_triangle(Triangle t) {
    float r1 = sqrt(rand());
    float r2 = rand();
    return t.ver[0] * (1 - r1) + t.ver[1] * r1 * (1 - r2) + t.ver[2] * r1 * r2;
}

// 在半球面上采样(+z方向)
vec3 sample_hemisphere() {
    float z = rand();
    float r = sqrt(1.0 - z * z);
    float phi = 2.0 * PI * rand();
    return vec3(r * cos(phi), r * sin(phi), z);
}

// 在所有光面上采样
void sample_light(out vec3 pos, out int t_index, out float pdf) {
    float tot_area = 0;
    for(int i = 0;i < light_t_num;i++) {
        int id = get_light_t_index(i);
        Triangle t = get_triangle(id);
        tot_area += t.area;
    }
    pdf = 1.0 / tot_area;
    float rnd = rand() * tot_area;
    for(int i = 0;i < light_t_num;i++) {
        int id = get_light_t_index(i);
        Triangle t = get_triangle(id);
        rnd -= t.area;
        if(rnd < EPS) {
            pos = sample_triangle(t);
            t_index = id;
            return;
        }
    }
}

vec3 simple_sample(vec3 wi, vec3 n, out float pdf) {
    float z = rand();
    float r = max(0.0, sqrt(1.0 - z * z));
    float phi = 2 * PI * rand();
    vec3 wo = vec3(r * cos(phi), r * sin(phi), z);
    pdf = 0.5 * IVPI;
    return to_world(wo, n);
}

// 采样出射光（重要性采样
vec3 importance_sample(Material m, vec3 wi, vec3 n, vec2 uv, out float pdf) {
    float m_roughness = get_roughness(m, uv);
    float alpha2 = max(0.01, m_roughness * m_roughness);
    float x = rand();

    float cos_theta = sqrt((1. - x) / (x * (alpha2 - 1) + 1));
    float cos2 = cos_theta * cos_theta;
    float phi = rand();
    float r = sqrt(1. - cos2);

    vec3 h = normalize(vec3(r * cos(phi), r * sin(phi), cos_theta));
    vec3 wo = reflect(-wi, to_world(h, n));
    pdf = 2. * alpha2 * cos_theta / pow2(cos2 * (alpha2 - 1.) + 1.) / (2 * PI);
    return wo;
}

// brdf
// ---------------------------------------------- //

float SchlickFresnel(float u) {
    float m = clamp(1-u, 0, 1);
    float m2 = m * m;
    return m2 * m2 * m;
}
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

vec3 brdf(Material m, vec3 L, vec3 V, vec3 N, vec2 uv) {
    vec3 H = normalize(L + V);
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);
    if(NdotL < 0 || NdotV < 0) return vec3(0); // 反向

    float m_roughness = get_roughness(m, uv);
    float m_metallic = get_metallic(m, uv);

    vec3 Cdlin = get_diffuse_color(m, uv);  // 线性空间下的漫反射颜色
    float Cdlum = .3*Cdlin.r + .6*Cdlin.g  + .1*Cdlin.b; // 计算亮度
    vec3 Ctint = Cdlum > 0 ? Cdlin/Cdlum : vec3(1);     // normalize lum. to isolate hue+sat
    vec3 Cspec = m.specular * mix(vec3(1), Ctint, m.specular_tint); // 插值得到镜面反射颜色
    vec3 Cspec0 = mix(0.08 * Cspec, Cdlin, m_metallic);     // 0° 镜面反射颜色, 即菲涅尔项中的F0

    // diffuse
    float FL = SchlickFresnel(NdotL), FV = SchlickFresnel(NdotV);
    float Fd90 = 0.5 + 2 * LdotH*LdotH * m_roughness;
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // subsurface
    float Fss90 = LdotH * LdotH * m_roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NdotL + NdotV) - .5) + .5);
    vec3 diffuse = IVPI * mix(Fd, ss, m.subsurface) * Cdlin;

    // specular
    float Ds = GTR2(NdotH, m_roughness);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs = smithG_GGX(NdotL, m_roughness);
    Gs *= smithG_GGX(NdotV, m_roughness);

    vec3 specular = Gs * Fs * Ds / (4 * NdotL * NdotV);

    return diffuse * (1 - m_metallic) + specular;
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
        vec3 light_pos;
        int light_t_index = -1;

        sample_light(light_pos, light_t_index, pdf);        // 光源采样

        Triangle tr2 = get_triangle(light_t_index);
        Material m2 = get_material(tr2.m_index);

        vec3 pos = inter1.pos;
        vec3 wi = normalize(light_pos - inter1.pos);
        vec3 wo = -ray.dir;
        vec3 n = inter1.normal;
        vec2 uv = tr1.uv[0] * (1 - inter1.u - inter1.v) +
                  tr1.uv[1] * inter1.u + tr1.uv[2] * inter1.v;

        // 直接光
        Intersect test = get_intersect(Ray(pos, wi));
        if(test.exist && test.t_index == light_t_index) {
            vec3 ln = test.normal;
            if(dot(wi, ln) > 0) ln = -ln;
            vec3 f_r = brdf(m1, wo, wi, n, uv);
            float dis = length(light_pos - inter1.pos);
            vec3 L_dir = m2.emission * f_r * dot(n, wi) * dot(ln, -wi) / (dis * dis) / pdf;
            result += history * L_dir;
        }

        // 间接光
        if(rand() < RussianRoulette) {
            wi = importance_sample(m1, wo, n, uv, pdf);
            ray = Ray(pos, wi);
            vec3 f_r = brdf(m1, wi, wo, n, uv);
            history *= f_r * dot(n, wi) / pdf / RussianRoulette;

            Intersect test = get_intersect(ray);
            if(!test.exist) {
                result += history * get_background_color(wi);
                break;
            }

            Triangle tr3 = get_triangle(test.t_index);
            Material m3 = get_material(tr3.m_index);
            if(!m3.is_emit) {
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

    vec3 result = vec3(0);
    if(fast_shade) {
        Intersect inter1 = get_intersect(ray);
        if(!inter1.exist) result = get_background_color(ray.dir);
        else {
            Triangle tr1 = get_triangle(inter1.t_index);
            Material m1 = get_material(tr1.m_index);
            vec2 uv = tr1.uv[0] * (1 - inter1.u - inter1.v) +
                    tr1.uv[1] * inter1.u + tr1.uv[2] * inter1.v;
            result = get_diffuse_color(m1, uv);
        }
        FragColor = vec4(result, 1);
    } else {
        for(int i = 0;i < SPP;i++) result += shade(ray);
        result /= SPP;

        // mix last frame
        vec4 last_col = texture(last_frame_texture, screen_uv);
        FragColor = mix(last_col, vec4(result, 1.0), 1.0 / frameCounter); // 所有帧混合
//        FragColor = mix(last_col, vec4(result, 1.0), 0.2); // 所有帧混合
    }

}