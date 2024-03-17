
// Test compute shader

#version 460 core
#extension GL_ARB_bindless_texture : require

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (std430, binding = 6) buffer OutColor {
    float outputColor[];
};

uniform int SCREEN_W;
uniform int SCREEN_H;

void main() {
    uvec3 id = gl_GlobalInvocationID;
    if(id.y >= SCREEN_W || id.x >= SCREEN_H) return;

    uint indexInMap = id.x * SCREEN_W + id.y;
    outputColor[indexInMap * 3 + 0] = float(id.x) / float(SCREEN_H);
    outputColor[indexInMap * 3 + 1] = float(id.y) / float(SCREEN_W);
    outputColor[indexInMap * 3 + 2] = 0.5;
}

