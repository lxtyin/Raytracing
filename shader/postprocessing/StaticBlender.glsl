
// Input LDR -> TAA.
// This version use variance guided clip, in YCoCg color space. So only LDR input is supported.

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform uint frameCounter;

layout(binding = 0, std430) buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) buffer ssbo1 {
    float historyColorGBuffer[];
};


void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 cur = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );

    vec3 lastcolor = vec3(
        historyColorGBuffer[pixelPtr * 3 + 0],
        historyColorGBuffer[pixelPtr * 3 + 1],
        historyColorGBuffer[pixelPtr * 3 + 2]
    );

    vec3 result = mix(lastcolor, cur, 1.0 / frameCounter);

    colorGBuffer[pixelPtr * 3 + 0] = historyColorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = historyColorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = historyColorGBuffer[pixelPtr * 3 + 2] = result.z;
}