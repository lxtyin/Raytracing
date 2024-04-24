
// Input HDR -> Tonemapping -> gamma correct -> Output LDR

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;

layout(binding = 0, std430) buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) buffer ssbo1 {
    float outputColorGBuffer[];
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
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 cur = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );

    cur = ACESToneMapping(cur);
    cur = igamma(cur);

    outputColorGBuffer[pixelPtr * 3 + 0] = cur.x;
    outputColorGBuffer[pixelPtr * 3 + 1] = cur.y;
    outputColorGBuffer[pixelPtr * 3 + 2] = cur.z;
}