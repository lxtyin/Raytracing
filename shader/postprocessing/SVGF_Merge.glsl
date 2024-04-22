
// Temporal accumulation (in HDR).
// Update both current moment and color.

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;

layout(binding = 0, std430) writeonly buffer ssbo0 {
    float colorGBuffer[];
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

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H));
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 di = vec3(
        directLumGBuffer[pixelPtr * 3 + 0],
        directLumGBuffer[pixelPtr * 3 + 1],
        directLumGBuffer[pixelPtr * 3 + 2]
    );
    vec3 idi = vec3(
        indirectLumGBuffer[pixelPtr * 3 + 0],
        indirectLumGBuffer[pixelPtr * 3 + 1],
        indirectLumGBuffer[pixelPtr * 3 + 2]
    );
    vec3 albedo = vec3(
        albedoGBuffer[pixelPtr * 3 + 0],
        albedoGBuffer[pixelPtr * 3 + 1],
        albedoGBuffer[pixelPtr * 3 + 2]
    );

    vec3 result = (di + idi) * albedo;

    colorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = result.z;
}