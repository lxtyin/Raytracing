
#version 330 core
uniform sampler2D cur_colorT, cur_wposT;
uniform sampler2D last_colorT, last_wposT;
uniform uint frameCounter;
uniform mat4 back_proj;
uniform bool is_motionvector_enabled;

in vec2 screen_uv;
layout(location = 0) out vec3 MapedColor;
layout(location = 1) out vec3 MixedHdrColor;

vec3 tonemapping(vec3 x) {
//    return min(x, vec3(1.0));
    return x / (x + 1);
}

void main() {

    vec3 cur_color = texture(cur_colorT, screen_uv).xyz;
    vec3 cur_wpos = texture(cur_wposT, screen_uv).xyz;
    vec3 result;
    if(!is_motionvector_enabled) {
        // static mix frame.
        vec3 last_color = texture(last_colorT, screen_uv).xyz;
        result = mix(last_color, cur_color, 1.0 / frameCounter);
    } else {
        // motion vector
        vec4 Hcoord = back_proj * vec4(cur_wpos, 1); // homogeneous coordinates
        vec2 last_xy = Hcoord.xy / Hcoord.w;
        vec3 last_color = texture(last_colorT, last_xy).xyz;
        vec3 last_wpos =  texture(last_wposT, last_xy).xyz;

        if(0. < last_xy.x && last_xy.x < 1. && 0. < last_xy.y && last_xy.y < 1. && length(cur_wpos - last_wpos) < 1) {
            result = mix(last_color, cur_color, 0.2);
        } else {
            result = cur_color;
        }
    }
    MixedHdrColor = result;

    // tone mapping
    result = tonemapping(result);

    // gamma correct
    result.x = pow(result.x, 0.45f);
    result.y = pow(result.y, 0.45f);
    result.z = pow(result.z, 0.45f);
//    if(isnan(result.x)) result.x = 100000;

    MapedColor = result;
}