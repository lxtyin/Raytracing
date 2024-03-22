
// RoughConductor ===========
vec3 eval_RoughConductor(in BSDFQueryRecord bRec) {
    if(bRec.wi.z <= 0 || bRec.wo.z <= 0) return vec3(0.0);

    vec3 albedo = vec3(materialBuffer[bRec.mptr + 1],
                       materialBuffer[bRec.mptr + 2],
                       materialBuffer[bRec.mptr + 3]);
    float alpha = materialBuffer[bRec.mptr + 4];
    float metallic = materialBuffer[bRec.mptr + 5];
    int albedo_map = roundint(materialBuffer[bRec.mptr + 6]);
    if(albedo_map >= 0) {
        albedo = powv(texture(textureBuffer[albedo_map], bRec.uv).rgb, 2.2f);
    }

    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 H = normalize(bRec.wi + bRec.wo);
    float LdotH = dot(bRec.wo, H);
    float D = eval_GGX(alpha, H);
    vec3 F = SchlickFresnel(F0, LdotH);

    float k = pow2(alpha + 1) / 8;
    float Gi4io = 0.25 / ((bRec.wi.z * (1 - k) + k) * (bRec.wo.z * (1 - k) + k));


    float FL = pow5(1 - bRec.wi.z), FV = pow5(1 - bRec.wo.z);
    float Fd90 = 0.5 + 2 * pow2(LdotH) * alpha;
    float Fd = mix(1.0f, Fd90, FL) * mix(1.0f, Fd90, FV);

    return D * F * Gi4io + (1 - metallic) * albedo * Fd * INV_PI;
}

float pdf_RoughConductor(in BSDFQueryRecord bRec) {
    if(bRec.wi.z <= 0 || bRec.wo.z <= 0) return 0.0;
    float alpha = materialBuffer[bRec.mptr + 4];

    vec3 H = normalize(bRec.wi + bRec.wo);
    float LdotH = dot(bRec.wo, H);
    float pdf = pdf_GGX(alpha, H);
    return pdf / (4 * LdotH);
}

vec3 sample_RoughConductor(in out BSDFQueryRecord bRec, out float pdf) {
    if(bRec.wi.z <= 0) {
        pdf = 0.0;
        return vec3(0.0);
    }

    float alpha = materialBuffer[bRec.mptr + 4];

    vec3 H = sample_GGX(alpha, pdf);
    bRec.wo = reflect(-bRec.wi, H);
    bRec.eta = 1.0;
    if(bRec.wi.z * bRec.wo.z <= 0) {
        pdf = -1;
        return vec3(0);
    }
    float LdotH = dot(bRec.wo, H);
    pdf /= (4 * LdotH);
    return eval_RoughConductor(bRec);
}

vec3 albedo_RoughConductor(in BSDFQueryRecord bRec) {
    vec3 albedo = vec3(materialBuffer[bRec.mptr + 1],
                    materialBuffer[bRec.mptr + 2],
                    materialBuffer[bRec.mptr + 3]);
    int albedo_map = roundint(materialBuffer[bRec.mptr + 6]);
    if(albedo_map >= 0) {
        albedo = powv(texture(textureBuffer[albedo_map], bRec.uv).rgb, 2.2f);
    }
    return albedo;
}