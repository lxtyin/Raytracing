
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

vec3 SchlickFresnel(vec3 f0, float c) {
    return f0 + (vec3(1) - f0) * pow5(1 - c);
}

float SchlickFresnel(float f0, float c) {
    return f0 + (1 - f0) * pow5(1 - c);
}

// unupdated
float fresnel(float cosI, float etaI, float etaT) {
    float sinI = sqrt(1.0 - cosI * cosI);
    float sinT = etaI / etaT * sinI;
    if(sinT > 1) return 1; // Full reflect
    float cosT = sqrt(1.0 - sinT * sinT);
    float Rl = pow2((etaI * cosI - etaT * cosT) / (etaI * cosI + etaT * cosT));
    float Rp = pow2((etaI * cosT - etaT * cosI) / (etaI * cosT + etaT * cosI));
    return (Rl + Rp) / 2;
}


layout(binding = 2, std430) readonly buffer ssbo2 {
    float materialBuffer[];
};

struct BSDFQueryRecord {
    // 同mitsuba：wo 光源方向 (L), wi 观察方向 (V)
    vec3 wi, wo;
    int mptr;
    vec2 uv;
};


// Material
// eval: returns bsdf (without cos), legal input is required.
// pdf: returns pdf, legal input is required
// sample: returns eval(), out pdf


#include shader/materials/RoughConductor.frag


vec3 eval_material(in BSDFQueryRecord bRec) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return eval_RoughConductor(bRec);
    else return vec3(0.0);
}

float pdf_material(in BSDFQueryRecord bRec) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return pdf_RoughConductor(bRec);
    else return 0.0;
}

vec3 sample_material(in out BSDFQueryRecord bRec, out float pdf) {
    int type = roundint(materialBuffer[bRec.mptr]);
    if(type == 1) return sample_RoughConductor(bRec, pdf);
    else {
        pdf = -1;
        return vec3(0.0);
    }
}

