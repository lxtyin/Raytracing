
// RoughDielectric ===========
// 参考mitsuba的做法
// 直接忽略那些：因微表面朝向而反射到背面，或折射到正面的光线
// 于是两个积分域完全划分，可以不用MIS解决采样问题。

vec3 eval_RoughDielectric(in BSDFQueryRecord bRec) {

    vec3 albedo = vec3(materialBuffer[bRec.mptr + 1],
    materialBuffer[bRec.mptr + 2],
    materialBuffer[bRec.mptr + 3]);
    float alpha = materialBuffer[bRec.mptr + 4];
    int albedo_map = roundint(materialBuffer[bRec.mptr + 5]);
    if(albedo_map >= 0) {
        albedo = powv(texture(textureBuffer[albedo_map], bRec.uv).rgb, 2.2f);
    }
    float eta = materialBuffer[bRec.mptr + 6];
    if(bRec.wi.z < 0) eta = 1.0 / eta;

    bool isreflect = sign(bRec.wi.z) == sign(bRec.wo.z);

    vec3 H;
    if(isreflect) {
        H = normalize(bRec.wi + bRec.wo);
    } else {
        H = normalize(bRec.wi + eta * bRec.wo);
    }
    H *= sign(H.z);

    float D = eval_GGX(alpha, H);
    float F = fresnel(dot(bRec.wi, H), eta);
    float G = G_GGX(alpha, abs(bRec.wi.z)) * G_GGX(alpha, abs(bRec.wo.z));
    if(isreflect) {
        return albedo * F * D * G / abs(4 * bRec.wi.z * bRec.wo.z);
    } else {
        float cosIH = dot(bRec.wi, H);
        float cosOH = dot(bRec.wo, H);
//        if(sign(cosIH) == sign(cosOH)) return vec3(0.0); // same as mitsuba, ignore this?

        float deno = pow2(cosIH + eta * cosOH);
        float value = ((1 - F) * D * G * eta * eta
                    * cosIH * cosOH) / (bRec.wi.z * bRec.wo.z * deno);

        return sqrt(albedo) * abs(value);
    }

}

float pdf_RoughDielectric(in BSDFQueryRecord bRec) {

    return 0.25 * INV_PI;

    float alpha = materialBuffer[bRec.mptr + 4];
    float eta = materialBuffer[bRec.mptr + 6];
    if(bRec.wi.z < 0) eta = 1.0 / eta;

    bool isreflect = sign(bRec.wi.z) == sign(bRec.wo.z);
    vec3 H;
    float pdf = -1;
    if(isreflect) {
        H = normalize(bRec.wi + bRec.wo);
        pdf = 1.0 / abs(4 * dot(bRec.wi, H));
    } else {
//        if(sign(dot(bRec.wi, H)) == sign(dot(bRec.wo, H))) return 0.0;
        H = normalize(bRec.wi + eta * bRec.wo);
        float deno = pow2(dot(bRec.wi, H) + eta * dot(bRec.wo, H));
        pdf = eta * eta * abs(dot(bRec.wo, H)) / deno;
    }
    H *= sign(H.z);

    float F = fresnel(dot(bRec.wi, H), eta);
    if(!isreflect) F = 1 - F;

    return pdf * pdf_GGX(alpha, H) * F;
}

vec3 sample_RoughDielectric(in out BSDFQueryRecord bRec, out float pdf) {

    bRec.wo = sample_uniformsphere(pdf);
    bRec.wo.z *= sign(bRec.wo.z);
    pdf = 0.25 * INV_PI;
    return eval_RoughDielectric(bRec);

    vec3 albedo = vec3(materialBuffer[bRec.mptr + 1],
    materialBuffer[bRec.mptr + 2],
    materialBuffer[bRec.mptr + 3]);
    float alpha = materialBuffer[bRec.mptr + 4];
    int albedo_map = roundint(materialBuffer[bRec.mptr + 5]);
    if(albedo_map >= 0) {
        albedo = powv(texture(textureBuffer[albedo_map], bRec.uv).rgb, 2.2f);
    }
    float eta = materialBuffer[bRec.mptr + 6];
    if(bRec.wi.z < 0) eta = 1.0 / eta;


    pdf = -1;
    float pdfH;
    vec3 H = sample_GGX(alpha, pdfH);

    float F = fresnel(dot(bRec.wi, H), eta);
    float D = eval_GGX(alpha, H);
    float G = G_GGX(alpha, abs(bRec.wi.z)) * G_GGX(alpha, abs(bRec.wo.z));

    if(rand() > F) {
        // transmission
        bRec.wo = refract(-bRec.wi, H, eta);
        if(sign(bRec.wi.z) == sign(bRec.wo.z)) return vec3(0.0);

        float deno = pow2(dot(bRec.wi, H) + eta * dot(bRec.wo, H));
        float value = ((1 - F) * D * G * eta * eta
                * dot(bRec.wi, H) * dot(bRec.wo, H)) / (bRec.wi.z * bRec.wo.z * deno);

        pdf = (1 - F) * pdfH * eta * eta * abs(dot(bRec.wo, H)) / deno;
        return albedo * abs(value);
    } else {
        // reflect
        vec3 wo = reflect(-bRec.wi, H);
        if(sign(bRec.wi.z) != sign(bRec.wo.z)) return vec3(0.0);

        pdf = F * pdfH / abs(4 * dot(bRec.wi, H));
        return abs(sqrt(albedo) * F * D * G / (4 * bRec.wi.z * bRec.wo.z));
    }
}
