
#version 330 core

#define M_SIZ 4    // 一个material占的vec3数量
#define T_SIZ 6    // 一个triangle占的vec3数量
#define B_SIZ 3    // 一个bvhnode占的vec3数量
#define INF 1E18
#define PI 3.1415926
#define IVPI 0.3183098
#define EPS 0.01

in float pixel_x;
in float pixel_y;
in vec2 tex_uv;
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

uniform int SCREEN_W;
uniform int SCREEN_H;
uniform uint frameCounter;
uniform sampler2D last_frame_texture;
uniform samplerCube skybox;

int stack[256];
int stack_h = 0;

// util
// ---------------------------------------------- //
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

// data source
// ---------------------------------------------- //

struct Triangle {
    vec3 ver[3];
    vec2 uv[3];
    vec3 normal;
    float area;
    int m_index;
};

struct Material {
    vec3 color;
    vec3 emission;
    bool is_emit;

    float specular;     // 镜面光强度
    float roughness;    // 粗糙度 [0, 1]
    float metallic;     // 金属度 缩减漫反射 [0, 1]
    float subsurface;
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
    r.normal = normalize(cross(r.ver[1] - r.ver[0], r.ver[2] - r.ver[1]));
    r.area = length(cross(r.ver[1] - r.ver[0], r.ver[2] - r.ver[0])) * 0.5f;
    vec3 u = texelFetch(triangles, i * T_SIZ + 3).xyz;
    vec3 v = texelFetch(triangles, i * T_SIZ + 4).xyz;
    r.uv[0].x = u.x;
    r.uv[1].x = u.y;
    r.uv[2].x = u.z;
    r.uv[0].y = v.x;
    r.uv[1].y = v.y;
    r.uv[2].y = v.z;
    r.m_index = roundint(texelFetch(triangles, i * T_SIZ + 5).x);
    return r;
}

// 获取第i个三角形光源的index
int get_light_t_index(int i) {
    return roundint(texelFetch(lightidxs, i).x);
}

Material get_material(int i) {
    Material r;
    r.color = texelFetch(materials, i * M_SIZ).xyz;
    r.emission = texelFetch(materials, i * M_SIZ + 1).xyz;
    vec3 tmp = texelFetch(materials, i * M_SIZ + 2).xyz;
    r.is_emit = tmp.x != 0;
    r.specular = tmp.y;
    tmp = texelFetch(materials, i * M_SIZ + 3).xyz;
    r.roughness = tmp.x;
    r.metallic = tmp.y;
    r.subsurface = tmp.z;
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
        vec3 nor = tri.normal;
        if(dot(ray.dir, nor) > 0) nor = -nor;
        return Intersect(true, pos, nor, i, u, v, t);
        // 可在此插值
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

// path tracing
// ---------------------------------------------- //

vec3 shade(Ray ray) {

    vec3 result = vec3(0);

    Intersect inter1 = get_intersect(ray);
    if(!inter1.exist) return vec3(texture(skybox, ray.dir));

    Triangle tr1 = get_triangle(inter1.t_index);
    Material m1 = get_material(tr1.m_index);

    result = m1.color;
    return result;
}

void main() {

    float dis_z = SCREEN_W * 0.5 / tan(fov / 2);
    vec3 w_ori = vec3(v2w_mat * vec4(0, 0, 0, 1));

    vec3 result = vec3(0);

    vec3 w_tar = vec3(v2w_mat * vec4(pixel_x, pixel_y, -dis_z, 1));
    vec3 dir = normalize(w_tar - w_ori);
    Ray ray = Ray(w_ori, dir);
    result = shade(ray);

    FragColor = vec4(result, 1);
}