
// Input LDR -> TAA.
// This version use variance guided clip, in YCoCg color space. So only LDR input is supported.

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform uint frameCounter;

layout(binding = 0, std430) readonly buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) buffer ssbo1 {
    float historyColorGBuffer[];
};
layout(binding = 2, std430) buffer ssbo2 {
    float historyMomentGBuffer[];
};

float luminance(vec3 c) {
    return c.x * 0.212671f + c.y * 0.715160f + c.z * 0.072169f;
}

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 color = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
    float lum = luminance(color);
    vec2 moment = vec2(lum, lum * lum);
    vec3 lastcolor = vec3(
        historyColorGBuffer[pixelPtr * 3 + 0],
        historyColorGBuffer[pixelPtr * 3 + 1],
        historyColorGBuffer[pixelPtr * 3 + 2]
    );
    vec2 lastmoment = vec2(
        historyMomentGBuffer[pixelPtr * 2 + 0],
        historyMomentGBuffer[pixelPtr * 2 + 1]
    );

    float sigma = sqrt(max(1.0, lastmoment.y - lastmoment.x * lastmoment.x));

    if(frameCounter != 1 && abs(moment.x - lastmoment.x) > sigma * 3) {
        color = lastcolor;
    } else {
        color = mix(lastcolor, color, 1.0 / frameCounter);
    }
    moment = mix(lastmoment, moment, 1.0 / frameCounter);

    historyColorGBuffer[pixelPtr * 3 + 0] = color.x;
    historyColorGBuffer[pixelPtr * 3 + 1] = color.y;
    historyColorGBuffer[pixelPtr * 3 + 2] = color.z;
    historyMomentGBuffer[pixelPtr * 2 + 0] = moment.x;
    historyMomentGBuffer[pixelPtr * 2 + 1] = moment.y;
}