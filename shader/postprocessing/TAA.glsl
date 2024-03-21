
// Input LDR -> TAA.
// This version use variance guided clip, in YCoCg color space. So only LDR input is supported.

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;

layout(binding = 0, std430) buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    float historyColorGBuffer[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    float motionGBuffer[];
};


layout(binding = 3, std430) readonly buffer ssbo3 {
    float normalGBuffer[];
};
layout(binding = 4, std430) readonly buffer ssbo4 {
    float meshIndexGBuffer[];
};
layout(binding = 5, std430) readonly buffer ssbo5 {
    float historyNormalGBuffer[];
};
layout(binding = 6, std430) readonly buffer ssbo6 {
    float historyMeshIndexGBuffer[];
};


vec3 RGB2YCoCgR(vec3 rgbColor) {
    vec3 YCoCgRColor;

    YCoCgRColor.y = rgbColor.r - rgbColor.b;
    float temp = rgbColor.b + YCoCgRColor.y / 2;
    YCoCgRColor.z = rgbColor.g - temp;
    YCoCgRColor.x = temp + YCoCgRColor.z / 2;

    return YCoCgRColor;
}

vec3 YCoCgR2RGB(vec3 YCoCgRColor) {
    vec3 rgbColor;

    float temp = YCoCgRColor.x - YCoCgRColor.z / 2;
    rgbColor.g = YCoCgRColor.z + temp;
    rgbColor.b = temp - YCoCgRColor.y / 2;
    rgbColor.r = rgbColor.b + YCoCgRColor.y;

    return rgbColor;
}


bool geometry_test(uint ptr1, uint ptr2) {
    vec3 normal1 = vec3(
        normalGBuffer[ptr1 * 3 + 0],
        normalGBuffer[ptr1 * 3 + 1],
        normalGBuffer[ptr1 * 3 + 2]
    );
    float id1 = meshIndexGBuffer[ptr1];
    vec3 normal2 = vec3(
        historyNormalGBuffer[ptr2 * 3 + 0],
        historyNormalGBuffer[ptr2 * 3 + 1],
        historyNormalGBuffer[ptr2 * 3 + 2]
    );
    float id2 = historyMeshIndexGBuffer[ptr2];

    return dot(normal1, normal2) > 0.9 && abs(id1 - id2) < 0.1;
}

bool interpolate(vec2 suv, uint testptr, vec3 mu, vec3 sigma, out vec3 color) {
    ivec2 leftbottom = ivec2(int(suv.x * SCREEN_W - 0.5),
    int(suv.y * SCREEN_H - 0.5));
    vec2 point = vec2(suv.x * SCREEN_W, suv.y * SCREEN_H);

    vec3 outcolor = vec3(0);
    vec2 outmoment = vec2(0);
    float totweight = 0;
    for(int i = 0;i < 4;i++) {
        ivec2 idx = leftbottom + ivec2(i / 2, i % 2);
        if(idx.x < 0 || idx.x >= SCREEN_W || idx.y < 0 || idx.y >= SCREEN_H) continue;
        int ptr = idx.y * SCREEN_W + idx.x;
        if(!geometry_test(testptr, ptr)) continue;

        float dx = abs(point.x - idx.x - 0.5);
        float dy = abs(point.y - idx.y - 0.5);
        float weight = max(0, (1 - dx) * (1 - dy));

        vec3 lastcolor = RGB2YCoCgR(vec3(
            historyColorGBuffer[ptr * 3 + 0],
            historyColorGBuffer[ptr * 3 + 1],
            historyColorGBuffer[ptr * 3 + 2]
        ));

        // variance clip
        vec3 dir = lastcolor - mu;
        float milen = 1.0;
        milen = min(milen, sigma.x / abs(dir.x));
        milen = min(milen, sigma.y / abs(dir.y));
        milen = min(milen, sigma.z / abs(dir.z));
        lastcolor = YCoCgR2RGB(mu + dir * milen);

        outcolor += lastcolor * weight;
        totweight += weight;
    }
    color = outcolor / totweight;
    return totweight > 0.00001;
}

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 cur = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
    vec2 motion = vec2(
        motionGBuffer[pixelPtr * 2 + 0],
        motionGBuffer[pixelPtr * 2 + 1]
    );

    // variance guide clip
    vec3 mu = vec3(0), var = vec3(0);
    int nv = 0;
    for(int i = 0;i < 9;i++) {
        ivec2 index = pixelIndex + ivec2(i / 3 - 1, i % 3 - 1);
        if(index.x < 0 || index.x >= SCREEN_W || index.y < 0 || index.y >= SCREEN_H) continue;
        int ptr = index.y * SCREEN_W + index.x;
        vec3 r = RGB2YCoCgR(vec3(
            colorGBuffer[ptr * 3 + 0],
            colorGBuffer[ptr * 3 + 1],
            colorGBuffer[ptr * 3 + 2]
        ));
        nv++;
        mu += r;
        var += r * r;
    }
    mu /= nv;
    var = max(vec3(0), var / nv - mu * mu);
    vec3 sigma = sqrt(var);
//    vec3 aabbMin = mu - sigma;
//    vec3 aabbMax = mu + sigma;

    vec3 result;
    vec2 last_uv = screen_uv - motion;

//    interpolate version.
    vec3 lastcolor;
    bool valid = interpolate(last_uv, pixelPtr, mu, sigma, lastcolor);
    if(valid) result = mix(lastcolor, cur, 0.05);
    else result = cur;

//    if(last_uv.x >= 1 || last_uv.y >= 1 || last_uv.x < 0 || last_uv.y < 0) {
//        result = cur;
//    } else {
//        ivec2 lastPixelIndex = ivec2(int(last_uv.x * SCREEN_W), int(last_uv.y * SCREEN_H));
//        int lastPixelPtr = lastPixelIndex.y * SCREEN_W + lastPixelIndex.x;
//
//        vec3 lastcolor = RGB2YCoCgR(vec3(
//            historyColorGBuffer[lastPixelPtr * 3 + 0],
//            historyColorGBuffer[lastPixelPtr * 3 + 1],
//            historyColorGBuffer[lastPixelPtr * 3 + 2]
//        ));
//
//        // variance clip
//        vec3 dir = lastcolor - mu;
//        float milen = 1.0;
//        milen = min(milen, sigma.x / abs(dir.x));
//        milen = min(milen, sigma.y / abs(dir.y));
//        milen = min(milen, sigma.z / abs(dir.z));
//        lastcolor = YCoCgR2RGB(mu + dir * milen);
//
//        result = mix(lastcolor, cur, 0.05);
//    }
    if(result.x < 0 || result.y < 0 || result.z < 0) result = vec3(10000, 0, 0);

    colorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = result.z;
}