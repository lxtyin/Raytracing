
// Input HDR -> Tonemapping -> gamma correct -> TAA -> Output LDR

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

vec3 vmin(vec3 a, vec3 b) {
    return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}
vec3 vmax(vec3 a, vec3 b) {
    return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}
float lenv(vec3 x) {
    return x.x * x.x + x.y * x.y + x.z * x.z;
}

vec3 readColorGBuffer(uvec2 uv) {
    if(uv.x >= SCREEN_W || uv.y >= SCREEN_H) return vec3(0);
    uint pixelPtr = uv.y * SCREEN_W + uv.x;
    return vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
}

void main() {
    uvec2 pixelIndex = uvec2(uint(screen_uv.x * SCREEN_W), uint(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    uint pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 cur = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
    vec2 motion = vec2(
        motionGBuffer[pixelPtr * 2 + 0],
        motionGBuffer[pixelPtr * 2 + 1]
    );

    // TAA
    vec3 mu = vec3(0), var = vec3(0);
    for(int i = 0;i < 9;i++) {
        vec3 r = readColorGBuffer(pixelIndex + uvec2(i / 3, i % 3));
        r = RGB2YCoCgR(r);
        mu += r;
        var += r * r;
    }
    mu /= 9;
    var = var / 9 - mu * mu;
    vec3 sigma = sqrt(var);
//    vec3 aabbMin = mu - sigma;
//    vec3 aabbMax = mu + sigma;

    vec3 result;
    uvec2 lastPixelIndex = uvec2(pixelIndex - motion);
    uint lastPixelPtr = lastPixelIndex.y * SCREEN_W + lastPixelIndex.x;
    if(lastPixelIndex.x >= SCREEN_W || lastPixelIndex.y >= SCREEN_H) {
        result = cur;
    } else {
        vec3 lastcolor = RGB2YCoCgR(vec3(
            historyColorGBuffer[lastPixelPtr * 3 + 0],
            historyColorGBuffer[lastPixelPtr * 3 + 1],
            historyColorGBuffer[lastPixelPtr * 3 + 2]
        ));

        // variance clip
        vec3 dir = lastcolor - mu;
        float milen = 1.0;
        milen = min(milen, sigma.x / abs(dir.x));
        milen = min(milen, sigma.y / abs(dir.y));
        milen = min(milen, sigma.z / abs(dir.z));
        lastcolor = YCoCgR2RGB(mu + dir * milen);

        result = mix(lastcolor, cur, 0.1);
    }

    colorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = result.z;
}