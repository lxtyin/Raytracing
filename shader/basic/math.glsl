#define INF 1E18
#define PI 3.1415926
#define INV_PI 0.3183098
#define EPS 0.0001

int roundint(float x) {
    float b = step(x, 0);
    return int(x - b + 0.5);
}
float len2(vec3 x) {
    return x.x * x.x + x.y * x.y + x.z * x.z;
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

// N(same direction with I)
vec3 refract(vec3 I, vec3 N, float eta) {
    float c1 = dot(N, -I);
    float s1 = sqrt(1 - c1 * c1);
    float s2 = s1 / eta;
    if(s2 >= 1) return vec3(0); // full reflect
    float c2 = sqrt(1 - s2 * s2);
    return -N * c2 + (I + N * c1) / eta;
}

float luminance(vec3 c) {
    return c.x * 0.212671f + c.y * 0.715160f + c.z * 0.072169f;
}
