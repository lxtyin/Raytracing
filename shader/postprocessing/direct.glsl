
#version 460 core

layout(binding = 0, std430) readonly buffer ssbo0 {
    float colorBuffer[];
};


in vec2 screen_uv;
out vec4 FragColor;

uniform int SCREEN_W;
uniform int SCREEN_H;

void main() {
    uvec2 uv = uvec2(uint((1.0 - screen_uv.y) * SCREEN_H),
                    uint(screen_uv.x * SCREEN_W));

    uint storePtr = uv.x * SCREEN_W + uv.y;

    FragColor = vec4(colorBuffer[storePtr * 3 + 0],
                    colorBuffer[storePtr * 3 + 1],
                    colorBuffer[storePtr * 3 + 2],  0);
}

