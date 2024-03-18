
// Temporal accumulation (in HDR).
// Update both current moment and color.

#version 460 core

in vec2 screen_uv;
uniform int SCREEN_W;
uniform int SCREEN_H;

layout(binding = 0, std430) buffer ssbo0 {
    float colorGBuffer[];
};
layout(binding = 1, std430) buffer ssbo1 {
    float momentGBuffer[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    float normalGBuffer[];
};
layout(binding = 3, std430) readonly buffer ssbo3 {
    float meshIndexGBuffer[];
};
layout(binding = 4, std430) readonly buffer ssbo4 {
    float motionGBuffer[];
};

layout(binding = 5, std430) readonly buffer ssbo5 {
    float historyColorGBuffer[];
};
layout(binding = 6, std430) readonly buffer ssbo6 {
    float historyMomentGBuffer[];
};
layout(binding = 7, std430) readonly buffer ssbo7 {
    float historyNormalGBuffer[];
};
layout(binding = 8, std430) readonly buffer ssbo8 {
    float historyMeshIndexGBuffer[];
};

vec3 vmin(vec3 a, vec3 b) {
    return vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z));
}
vec3 vmax(vec3 a, vec3 b) {
    return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}
float lenv(vec3 x) {
    return x.x * x.x + x.y * x.y + x.z * x.z;
}

bool geometry_test(uint ptr1, uint ptr2) {

}

void main() {
    ivec2 pixelIndex = ivec2(int(screen_uv.x * SCREEN_W), int(screen_uv.y * SCREEN_H));
    int pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    vec3 color = vec3(
        colorGBuffer[pixelPtr * 3 + 0],
        colorGBuffer[pixelPtr * 3 + 1],
        colorGBuffer[pixelPtr * 3 + 2]
    );
    vec2 moment = vec2(
        momentGBuffer[pixelPtr * 1 + 0],
        momentGBuffer[pixelPtr * 2 + 1]
    );
    vec2 motion = vec2(
        motionGBuffer[pixelPtr * 2 + 0],
        motionGBuffer[pixelPtr * 2 + 1]
    );


    vec3 resultColor;
    vec2 resultMoment;
    vec2 last_uv = screen_uv - motion;
    if(last_uv.x >= 1 || last_uv.y >= 1 || last_uv.x < 0 || last_uv.y < 0) {
        resultColor = color;
        resultMoment = moment;
    } else {
        ivec2 lastPixelIndex = ivec2(int(last_uv.x * SCREEN_W), int(last_uv.y * SCREEN_H));
        int lastPixelPtr = lastPixelIndex.y * SCREEN_W + lastPixelIndex.x;

        vec3 lastcolor = vec3(
            historyColorGBuffer[lastPixelPtr * 3 + 0],
            historyColorGBuffer[lastPixelPtr * 3 + 1],
            historyColorGBuffer[lastPixelPtr * 3 + 2]
        );
        vec2 lastmoment = vec2(
            historyMomentGBuffer[lastPixelPtr * 2 + 0],
            historyMomentGBuffer[lastPixelPtr * 2 + 1]
        );



        result = mix(lastcolor, cur, 0.05);
    }
    if(result.x < 0 || result.y < 0 || result.z < 0) result = vec3(10000, 0, 0);

    colorGBuffer[pixelPtr * 3 + 0] = result.x;
    colorGBuffer[pixelPtr * 3 + 1] = result.y;
    colorGBuffer[pixelPtr * 3 + 2] = result.z;
}