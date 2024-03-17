
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

vec3 ACESToneMapping(vec3 color) {
    const float A = 2.51;
    const float B = 0.03;
    const float C = 2.43;
    const float D = 0.59;
    const float E = 0.14;
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}

vec3 ReinhardToneMapping(vec3 color) {
    return color / (color + 1.0);
}

vec3 igamma(vec3 c) {
    return vec3(
        pow(c.x, 0.45f),
        pow(c.y, 0.45f),
        pow(c.z, 0.45f)
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

    cur = ACESToneMapping(cur);
    cur = igamma(cur);

    // TAA
    vec3 result;
    uvec2 lastPixelIndex = uvec2(pixelIndex - motion);
    uint lastPixelPtr = lastPixelIndex.y * SCREEN_W + lastPixelIndex.x;
    if(lastPixelIndex.x >= SCREEN_W || lastPixelIndex.y >= SCREEN_H) {
        result = cur;
    } else {
        vec3 lastcolor = vec3(
            historyColorGBuffer[lastPixelPtr * 3 + 0],
            historyColorGBuffer[lastPixelPtr * 3 + 1],
            historyColorGBuffer[lastPixelPtr * 3 + 2]
        );
        result = mix(lastcolor, cur, 0.1);
    }

    colorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = result.z;
}