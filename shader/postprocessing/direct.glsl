
#version 460 core

uniform int channel;
layout(binding = 0, std430) readonly buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    float instanceIndexGBuffer[];
};

in vec2 screen_uv;
out vec4 FragColor;

uniform int SCREEN_W;
uniform int SCREEN_H;
uniform int selectedInstanceIndex;

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H));
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 outputcolor;
    if(channel == 3) {
        outputcolor = vec3(colorGBuffer[pixelPtr * 3 + 0],
            colorGBuffer[pixelPtr * 3 + 1],
            colorGBuffer[pixelPtr * 3 + 2]);
    } else if(channel == 1) {
        outputcolor = vec3(colorGBuffer[pixelPtr],
                            colorGBuffer[pixelPtr],
                            colorGBuffer[pixelPtr]);
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

