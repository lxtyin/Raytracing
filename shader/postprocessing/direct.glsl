
#version 460 core


layout(binding = 0, std430) readonly buffer ssbo0 {
    float renderedGBuffer[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    float directLumGBuffer[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    float indirectLumGBuffer[];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    float albedoGBuffer[];
};
layout(binding = 4, std430) readonly buffer ssbo4 {
    float depthGBuffer[];
};
layout(binding = 5, std430) readonly buffer ssbo5 {
    float normalGBuffer[];
};
layout(binding = 6, std430) readonly buffer ssbo6 {
    float instanceIndexGBuffer[];
};
uniform int visualType;

in vec2 screen_uv;
out vec4 FragColor;

uniform int SCREEN_W;
uniform int SCREEN_H;
uniform int selectedInstanceIndex;
uniform float scaling;

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H));
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 outputcolor;
    if(visualType == 0) {
        outputcolor = vec3(
        renderedGBuffer[pixelPtr * 3 + 0],
        renderedGBuffer[pixelPtr * 3 + 1],
        renderedGBuffer[pixelPtr * 3 + 2]);
    }
    if(visualType == 1) {
        outputcolor = vec3(
        directLumGBuffer[pixelPtr * 3 + 0],
        directLumGBuffer[pixelPtr * 3 + 1],
        directLumGBuffer[pixelPtr * 3 + 2]);
    }
    if(visualType == 2) {
        outputcolor = vec3(
        indirectLumGBuffer[pixelPtr * 3 + 0],
        indirectLumGBuffer[pixelPtr * 3 + 1],
        indirectLumGBuffer[pixelPtr * 3 + 2]);
    }
    if(visualType == 3) {
        outputcolor = vec3(
        albedoGBuffer[pixelPtr * 3 + 0],
        albedoGBuffer[pixelPtr * 3 + 1],
        albedoGBuffer[pixelPtr * 3 + 2]);
    }
    if(visualType == 4) {
        outputcolor = vec3(depthGBuffer[pixelPtr + 0]) / 100;
    }
    if(visualType == 5) {
        outputcolor = (vec3(
        normalGBuffer[pixelPtr * 3 + 0],
        normalGBuffer[pixelPtr * 3 + 1],
        normalGBuffer[pixelPtr * 3 + 2]) + 1) / 2;
    }
    if(visualType == 6) {
        outputcolor = vec3(instanceIndexGBuffer[pixelPtr + 0]) / 30;
    }

    if(selectedInstanceIndex >= 0) {

        int instanceIndex = int(instanceIndexGBuffer[pixelPtr] + 0.01);

        if(instanceIndex == selectedInstanceIndex) {
//            if((pixelIndex.x + pixelIndex.y) % 10 == 0)
//                outputcolor = vec3(255, 175, 69) / 255;
        } else {
            for(int i = 0;i < 25;i++) {
                ivec2 index = pixelIndex + ivec2(i / 5 - 2, i % 5 - 2);
                if(index.x < 0 || index.x >= SCREEN_W || index.y < 0 || index.y >= SCREEN_H) continue;
                int ptr = index.y * SCREEN_W + index.x;

                int idx = int(instanceIndexGBuffer[ptr] + 0.01);
                if(idx == selectedInstanceIndex) {
                    outputcolor = vec3(251, 109, 72) / 255;
                    break;
                }
            }
        }
    }

    FragColor = vec4(outputcolor, 0);
}

