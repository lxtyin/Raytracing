
// utils material function


vec3 sample_GGX(float alpha, out float pdf) {
    float alpha2 = alpha * alpha;
    float x = rand();

    float cos_theta = sqrt((1. - x) / (x * (alpha2 - 1) + 1));
    float cos2 = cos_theta * cos_theta;
    float phi = rand() * 2 * PI;
    float r = sqrt(1. - cos2);
    pdf = 2. * alpha2 * cos_theta / pow2(cos2 * (alpha2 - 1.) + 1.) / (2 * PI);
    vec3 h = normalize(vec3(r * cos(phi), r * sin(phi), cos_theta));
    return h;
}
float eval_GGX(float alpha, vec3 h) {
    float cos_theta = abs(h.z);
    float alpha2 = alpha * alpha;
    float cos2 = cos_theta * cos_theta;
    float pdf = alpha2 * cos_theta / pow2(cos2 * (alpha2 - 1.) + 1.) / PI;
    return pdf;
}
float pdf_GGX(float alpha, vec3 h) {
    return eval_GGX(alpha, h);
}

float G_GGX(float alpha, float cosI) {
    float k = pow2(alpha + 1) / 8;
    return cosI / (cosI * (1 - k) + k);
}


vec3 sample_uniformsphere(out float pdf) {
    vec2 s = rand2D();
    float z = s.x * 2 - 1;
    float r = sqrt(1 - z * z), phi = s.y * 2 * PI;
    pdf = 0.25 * INV_PI;
    return vec3(cos(s.y) * r, sin(s.y) * r, z);
}


vec3 SchlickFresnel(vec3 f0, float c) {
    return f0 + (vec3(1) - f0) * pow5(1 - c);
}

float SchlickFresnel(float f0, float c) {
    return f0 + (1 - f0) * pow5(1 - c);
}

float fresnel(float cosI, float eta) {
    cosI = abs(cosI);
    float sinT = 1.0 / eta * sqrt(1.0 - cosI * cosI);
    if(sinT > 1) return 1.0; // total internal reflection
    float cosT = sqrt(1.0 - sinT * sinT);
    float Rs = pow2((cosI - eta * cosT) / (cosI + eta * cosT));
    float Rp = pow2((cosT - eta * cosI) / (cosT + eta * cosI));
    return (Rs + Rp) / 2;
}


struct BSDFQueryRecord {
    // light direction L = wo; view direction V = wi
    vec3 wi, wo;
    int mptr;
    vec2 uv;

    // wo_ior / wi_ior
    // For tracking basic radiance: L * ior ^ 2
    float eta;
};

// Material
// same as mitsuba, wo is light direction.
// eval: returns bsdf (without cos)
// pdf: returns pdf
// sample: returns eval(), out pdf

#include shader/materials/RoughConductor.glsl
#include shader/materials/RoughDielectric.glsl

vec3 eval_material(in BSDFQueryRecord bRec) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return eval_RoughConductor(bRec);
    else if(type == 2) return eval_RoughDielectric(bRec);
    else return vec3(0.0);
}

float pdf_material(in BSDFQueryRecord bRec) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return pdf_RoughConductor(bRec);
    else if(type == 2) return pdf_RoughDielectric(bRec);
    else return 0.0;
}

vec3 sample_material(in out BSDFQueryRecord bRec, out float pdf) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return sample_RoughConductor(bRec, pdf);
    else if(type == 2) return sample_RoughDielectric(bRec, pdf);
    else {
        pdf = -1;
        return vec3(0.0);
    }
}

vec3 albedo_material(in BSDFQueryRecord bRec) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return albedo_RoughConductor(bRec);
    else if(type == 2) return albedo_RoughDielectric(bRec);
    else return vec3(0);
}