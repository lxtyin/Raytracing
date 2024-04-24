
// Spatial filter: 5x5 a'trous wavelet filter.

#version 460 core
#include shader/basic/math.glsl

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;

layout(binding = 0, std430) readonly buffer ssbo0 {
    float momentGBuffer[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    float normalGBuffer[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    float depthGBuffer[];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    float numSamplesGBuffer[];
};
layout(binding = 4, std430) writeonly buffer ssbo4 {
    float varianceGBuffer[];
};

const float kernel[4] = {3.0 / 8, 1.0 / 4.0, 1.0 / 16, 1.0 / 32};

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H));
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 normal = vec3(
        normalGBuffer[pixelPtr * 3 + 0],
        normalGBuffer[pixelPtr * 3 + 1],
        normalGBuffer[pixelPtr * 3 + 2]
    );
    vec2 moment = vec2(
        momentGBuffer[pixelPtr * 2 + 0],
        momentGBuffer[pixelPtr * 2 + 1]
    );
    float numSamples = numSamplesGBuffer[pixelPtr];
    float depth = depthGBuffer[pixelPtr];

    // filter
    if(numSamples <= 4) {
        float totalWeight = 0;
        vec2 result = vec2(0);
        for(int i = -3;i <= 3;i++) {
            for(int j = -3;j <= 3;j++) {
                ivec2 index = pixelIndex + ivec2(i, j);
                if(index.x >= SCREEN_W || index.x < 0 || index.y >= SCREEN_H || index.y < 0) continue;
                int ptr = index.y * SCREEN_W + index.x;

                vec2 _moment = vec2(
                momentGBuffer[ptr * 2 + 0],
                momentGBuffer[ptr * 2 + 1]
                );
                vec3 _normal = vec3(
                normalGBuffer[ptr * 3 + 0],
                normalGBuffer[ptr * 3 + 1],
                normalGBuffer[ptr * 3 + 2]
                );
                float _depth = depthGBuffer[ptr];

                float w = kernel[abs(i)] * kernel[abs(j)] *
                pow(max(0, dot(normal, _normal)), 128) *
                exp(-abs(depth - _depth) / 2);

                totalWeight += w;
                result += w * _moment;
            }
        }
        moment = result / totalWeight;
    } else {
        // 3x3 blur kernel.
        float totalWeight = 0;
        vec2 result = vec2(0);
        for(int i = -1;i <= 1;i++) {
            for(int j = -1;j <= 1;j++) {
                ivec2 index = pixelIndex + ivec2(i, j);
                if(index.x >= SCREEN_W || index.x < 0 || index.y >= SCREEN_H || index.y < 0) continue;
                int ptr = index.y * SCREEN_W + index.x;

                vec2 _moment = vec2(
                    momentGBuffer[ptr * 2 + 0],
                    momentGBuffer[ptr * 2 + 1]
                );

                float w = kernel[abs(i)] * kernel[abs(j)];

                totalWeight += w;
                result += w * _moment;
            }
        }
        moment = result / totalWeight;
    }

    float variance = max(0, moment.y - moment.x * moment.x);
    varianceGBuffer[pixelPtr] = variance;

}

