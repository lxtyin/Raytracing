
#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) buffer ssbo0 {
    float depthGBuffer[];
};
layout(binding = 1, std430) buffer ssbo1 {
    float normalGBuffer[];
};
layout(binding = 2, std430) buffer ssbo2 {
    float uvGBuffer[];
};
layout(binding = 3, std430) buffer ssbo3 {
    float instanceIndexGBuffer[];
};

in vec3 view_pos;
in vec3 normal;
in vec2 uv;

out vec4 FragColor;

// viewport in left upper corner.
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform int WINDOW_W;
uniform int WINDOW_H;
uniform int currentInstanceIndex;

void main() {
    // gl_FragCoord: coord in window
    uvec2 pixelIndex = uvec2(uint(gl_FragCoord.x + 0.1), uint(SCREEN_H - WINDOW_H + gl_FragCoord.y + 0.1));
//    uvec2 pixelIndex = uvec2(uint(screen_uv.x * SCREEN_W), uint(screen_uv.y * SCREEN_H));
    uint pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    if (gl_FragCoord.z < gl_DepthRange.near) discard;

    depthGBuffer[pixelPtr] = gl_FragCoord.z;
    instanceIndexGBuffer[pixelPtr] = currentInstanceIndex;
    uvGBuffer[pixelPtr * 2 + 0] = uv[0];
    uvGBuffer[pixelPtr * 2 + 1] = uv[1];

    normalGBuffer[pixelPtr * 3 + 0] = (normal[0] + 1) / 2;
    normalGBuffer[pixelPtr * 3 + 1] = (normal[1] + 1) / 2;
    normalGBuffer[pixelPtr * 3 + 2] = (normal[2] + 1) / 2;

//    FragColor = vec4(1.0 * pixelIndex.x / SCREEN_W, 1.0 * pixelIndex.y / SCREEN_H, 0.0, 1.0);
//    FragColor = vec4(vec3(-view_pos.z) / 100, 1.0);
//    FragColor = vec4((normal + 1) / 2, 1.0);
//    FragColor = vec4(uvGBuffer[pixelPtr * 2 + 0], uvGBuffer[pixelPtr * 2 + 1], 0.0, 1.0);
    FragColor = vec4((normal + 1) / 2, 1.0);
}

