
#version 330 core

#define M_SIZ 3    // 一个material占的vec3数量
#define T_SIZ 6    // 一个triangle占的vec3数量
#define B_SIZ 3    // 一个bvhnode占的vec3数量
#define INF 1E18
#define PI 3.1415926

in float pixel_x;
in float pixel_y;
out vec4 FragColor;

// memory
// ---------------------------------------------- //

uniform samplerBuffer materials;
uniform samplerBuffer triangles;
uniform samplerBuffer lightindexs;
uniform samplerBuffer bvhnodes;
uniform int light_t_num;
uniform int triangles_num;
uniform mat4 v2w_mat;

uniform float RussianRoulette = 0.8;
uniform int SPP = 4;
uniform float fov = PI / 2;

uniform int SCREEN_W;
uniform int SCREEN_H;
uniform uint frameCounter;

int stack[256];
int stack_h = 0;

// util
// ---------------------------------------------- //
uint seed = uint(
    uint(pixel_x + 19293) * uint(1973) +
    uint(pixel_y + 42931) * uint(9277) +
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

// data source
// ---------------------------------------------- //

struct Triangle {
    vec3 ver[3];
    vec2 uv[3];
    vec3 normal;
    int m_index;
};

struct Material {
    vec3 color;
    vec3 emission;
    bool is_emit;
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
    return roundint(texelFetch(lightindexs, i).x);
}

Material get_material(int i) {
    Material r;
    r.color = texelFetch(materials, i * M_SIZ).xyz;
    r.emission = texelFetch(materials, i * M_SIZ + 1).xyz;
    r.is_emit = texelFetch(materials, i * M_SIZ + 2).x != 0;
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

/// 若有交点，返回与aabb的第一个交点
Intersect intersect_aabb(Ray ray, vec3 aa, vec3 bb) {
    float tmi = 0, tmx = INF;
    vec3 t1 = (aa - ray.ori) / ray.dir;
    vec3 t2 = (bb - ray.ori) / ray.dir;
    tmi = max(tmi, min(t1.x, t2.x));
    tmi = max(tmi, min(t1.y, t2.y));
    tmi = max(tmi, min(t1.z, t2.z));
    tmx = min(tmx, max(t1.x, t2.x));
    tmx = min(tmx, max(t1.y, t2.y));
    tmx = min(tmx, max(t1.z, t2.z));
    if(tmi - 0.01 > tmx) return nointersect;
    return Intersect(true, ray.ori + ray.dir * tmi, vec3(0), -1, 0, 0, tmi);
}

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
    if(0 < t && 0 < u && 0 < v && u + v < 1) {
        vec3 pos = ray.ori + t * ray.dir;
        vec3 nor = tri.normal;
        if(dot(ray.dir, nor) > 0) nor = -nor;
        return Intersect(true, pos, nor, i, u, v, t);
    } else {
        return nointersect;
    }
}

/// 在场景中的第一个交点
Intersect get_intersect(Ray ray) {

    BVHNode root = get_bvhnode(0);
    if(!intersect_aabb(ray, root.aa, root.bb).exist){
        return nointersect;
    }

    Intersect res = nointersect;
    stack_h = 0;
    stack[++stack_h] = 0;

    while(stack_h > 0) {
        int id = stack[stack_h--];
        BVHNode cur = get_bvhnode(id);

        // 叶子结点
        if(cur.t_index != -1) {
            Intersect inter = intersect_triangle(ray, cur.t_index);
            if(inter.exist && inter.t < res.t) res = inter;
        } else {
            BVHNode ls = get_bvhnode(cur.l);
            BVHNode rs = get_bvhnode(cur.r);
            Intersect inter1 = intersect_aabb(ray, ls.aa, ls.bb);
            Intersect inter2 = intersect_aabb(ray, rs.aa, rs.bb);

            // 仅在与包围盒交点小于最优值时 才考虑
            if(inter1.exist && inter1.t < res.t) {
                stack[++stack_h] = cur.l;
            }
            if(inter2.exist && inter2.t < res.t) {
                stack[++stack_h] = cur.r;
            }
        }
    }
    return res;
}

// sample
// ---------------------------------------------- //

// 三角形面积
float area(Triangle t) {
    return length(cross(t.ver[1] - t.ver[0], t.ver[2] - t.ver[1])) / 2;
}

// 在三角形上均匀采样
vec3 sample_triangle(Triangle t) {
    float r1 = sqrt(rand());
    float r2 = rand();
    return t.ver[0] * (1 - r1) + t.ver[1] * r1 * (1 - r2) + t.ver[2] * r1 * r2;
}

// 在半球面上采样(+y方向)
vec3 sample_hemisphere() {
    float y = rand();
    float r = sqrt(1.0 - y * y);
    float phi = 2.0 * PI * rand();
    return vec3(r * cos(phi), y, -r * sin(phi));
}

// 在所有光面上采样
void sample_light(out vec3 pos, out int t_index, out float pdf) {
    float tot_area = 0;
    for(int i = 0;i < light_t_num;i++) {
        int id = get_light_t_index(i);
        Triangle t = get_triangle(id);
        tot_area += area(t);
    }
    float rnd = rand() * tot_area;
    for(int i = 0;i < light_t_num;i++) {
        int id = get_light_t_index(i);
        Triangle t = get_triangle(id);
        rnd -= area(t);
        if(rnd < 1e-2) { 
            pdf = 1.0 / area(t);
            pos = sample_triangle(t);
            t_index = id;
            return;
        }
    }
}

// 采样出射光（重要性采样
vec3 sample_direction(Material material, vec3 wi, vec3 normal, out float pdf) {
    // 暂时均匀采样
    vec3 dir = sample_hemisphere();
    vec3 _y = normalize(normal);
    vec3 _x = normalize(cross(vec3(0, 1, 0), normal));
    vec3 _z = normalize(cross(_x, _y));
    pdf = 1.0 / (2 * PI);
    return dir.x * _x + dir.y * _y + dir.z * _z;
}

// 材质的brdf
vec3 brdf(Material material, vec3 wi, vec3 wo, vec3 normal) {
    // 暂时不管纹理
    if(dot(wo, normal) < 0 || dot(wi, normal) < 0) return vec3(0); // 反向
    return material.color / PI;
}

// path tracing
// ---------------------------------------------- //

vec3 shade(Ray ray) {

//    Intersect inter1 = get_intersect(ray);
//    if(inter1.exist) {
//        Triangle t = get_triangle(inter1.t_index);
//        Material m = get_material(t.m_index);
//        return m.color;
//    } else {
//        return vec3(0, 0, 0);
//    }


    vec3 result = vec3(0);
    vec3 history = vec3(1); // 累乘值

    while(true) {        

        Intersect inter1 = get_intersect(ray);
        if(!inter1.exist) break;

        Triangle tr1 = get_triangle(inter1.t_index);
        Material m1 = get_material(tr1.m_index);

        // 自发光
        if(m1.is_emit) {
            result += history * m1.emission;
            break;
        }

        float pdf;
        vec3 light_pos;
        int light_t_index = -1;
        sample_light(light_pos, light_t_index, pdf);        // 光源采样点

        Triangle tr2 = get_triangle(light_t_index);
        Material m2 = get_material(tr2.m_index);

        vec3 pos = inter1.pos;
        vec3 wi = normalize(light_pos - inter1.pos);
        vec3 wo = ray.dir;
        vec3 n = inter1.normal;
        vec3 ln = tr2.normal;
        if(dot(wi, ln) > 0) ln = -ln;

        // 直接光
        Intersect test = get_intersect(Ray(pos, wi));
        if(test.exist && length(test.pos - light_pos) < 1e-2) {
            vec3 f_r = brdf(m1, wi, wo, n);
            float dis = length(light_pos - inter1.pos) / 10; //todo
            vec3 L_dir = m2.emission * f_r * abs(dot(n, wi) * dot(ln, -wi)) / (dis * dis) / pdf;
            result += history * L_dir;
        }

        // 间接光
        if(rand() < RussianRoulette) {
            wi = sample_direction(m1, wo, n, pdf);
            test = get_intersect(Ray(pos, wi));
            if(test.exist) {
                Material m3 = get_material(get_triangle(test.t_index).m_index);
                if(!m3.is_emit) {
                    vec3 f_r = brdf(m1, wi, wo, n);
                    history *= f_r * abs(dot(n, wi)) / pdf / RussianRoulette;
                    ray = Ray(pos, wi);
                } else break;
            } else break;
        } else break;
    }
    return result;
}

void main() {

    float dis_z = SCREEN_W * 0.5 / tan(fov / 2);
    vec3 w_ori = vec3(v2w_mat * vec4(0, 0, 0, 1));

    vec3 result = vec3(0);

    for(int i = 0;i < SPP;i++) {
        float px = pixel_x + rand() - 0.5;
        float py = pixel_y + rand() - 0.5;

        vec3 w_tar = vec3(v2w_mat * vec4(px, py, -dis_z, 1));
        vec3 dir = normalize(w_tar - w_ori);
        Ray ray = Ray(w_ori, dir);
        result += shade(ray);
    }
    result /= SPP;

    FragColor = vec4(result, 1.0);
}