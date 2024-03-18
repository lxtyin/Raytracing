
// Spatial filter: 5x5 a'trous wavelet filter.

#version 460 core
#include shader/basic/math.glsl

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform int step;           // for differen level of a'trous wavelet filter.

layout(binding = 0, std430) buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    float normalGBuffer[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    float depthGBuffer[];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    float momentGBuffer[];
};


vec3 readColorGBuffer(uvec2 uv) {
    if(uv.x >= SCREEN_W || uv.y >= SCREEN_H) return vec3(0);
    uint pixelPtr = uv.y * SCREEN_W + uv.x;
    return vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
}

const float kernel[3] = {1.0, 2.0 / 3.0, 1.0 / 6};

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H)); // 像素的纹理坐标 第一象限
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 color = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
    vec3 normal = vec3(
        normalGBuffer[pixelPtr * 3 + 0],
        normalGBuffer[pixelPtr * 3 + 1],
        normalGBuffer[pixelPtr * 3 + 2]
    );
    float depth = depthGBuffer[pixelPtr];

    // calculate variance
    float mu = 0, var = 0;
    for(int i = 0;i < 9;i++) {
        float r = luminance(readColorGBuffer(pixelIndex + uvec2(i / 3, i % 3)));
        mu += r;
        var += r * r;
    }
    mu /= 9;
    var = max(0, var / 9 - mu * mu);
    float sigma = sqrt(var);

    // filter
    float totalWeight = 0;
    vec3 result = vec3(0);
    for(int i = -2;i <= 2;i++) {
        for(int j = -2;j <= 2;j++) {
            ivec2 index = pixelIndex + ivec2(i, j) * step;
            if(index.x >= SCREEN_W || index.x < 0 || index.y >= SCREEN_H || index.y < 0) continue;
            int ptr = index.y * SCREEN_W + index.x;

            vec3 _color = vec3(
                colorGBuffer[ptr * 3 + 0],
                colorGBuffer[ptr * 3 + 1],
                colorGBuffer[ptr * 3 + 2]
            );
            vec3 _normal = vec3(
                normalGBuffer[ptr * 3 + 0],
                normalGBuffer[ptr * 3 + 1],
                normalGBuffer[ptr * 3 + 2]
            );
            float _depth = depthGBuffer[ptr];

            // No depth now.
            float w = kernel[abs(i)] * kernel[abs(j)] *
                    pow(max(0, dot(normal, _normal)), 128) *
                    exp(-abs(luminance(color) - luminance(_color)) / (sigma * 4.0 + 0.00001));
            totalWeight += w;
            result += w * _color;
        }
    }
    result /= totalWeight;
    if(any(isnan(result)) || any(isinf(result))) result = vec3(10000, 0, 0);

    colorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = result.z;
}

