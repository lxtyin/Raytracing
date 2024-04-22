
#version 460 core
#extension GL_ARB_bindless_texture : require


layout(location = 0) out float depthOutput;
layout(location = 1) out vec3 normalOutput;
layout(location = 2) out vec2 uvOutput;
layout(location = 3) out float instanceIndexOutput;

in vec3 view_pos;
in vec3 normal;
in vec2 uv;

uniform int currentInstanceIndex;

void main() {
    // gl_FragCoord: coord in window
//    uvec2 pixelIndex = uvec2(uint(gl_FragCoord.x + 0.1), uint(SCREEN_H - WINDOW_H + gl_FragCoord.y + 0.1));
////    uvec2 pixelIndex = uvec2(uint(screen_uv.x * SCREEN_W), uint(screen_uv.y * SCREEN_H));
//    uint pixelPtr = pixelIndex.y * SCREEN_W + pixelIndex.x;

    depthOutput = length(view_pos);
    normalOutput = normal;
    uvOutput = uv;
    instanceIndexOutput = 1.0 * currentInstanceIndex;

//    FragColor = vec4(1.0 * pixelIndex.x / SCREEN_W, 1.0 * pixelIndex.y / SCREEN_H, 0.0, 1.0);
//    FragColor = vec4(vec3(-view_pos.z) / 100, 1.0);
//    FragColor = vec4((normal + 1) / 2, 1.0);
//    FragColor = vec4(uvGBuffer[pixelPtr * 2 + 0], uvGBuffer[pixelPtr * 2 + 1], 0.0, 1.0);
}

