
#version 330 core
uniform sampler2D prevpass_color;
uniform sampler2D last_frame_hdr;
uniform uint frameCounter;
uniform bool is_mix_frame;

in vec2 screen_uv;
layout(location = 0) out vec3 MapedColor;
layout(location = 1) out vec3 MixedHdrColor;

vec3 tonemapping(vec3 x) {
    return x / (x + 1);
}

void main() {

    vec3 color =  texture(prevpass_color, screen_uv).xyz;
    if(any(isnan(color))) color = vec3(0);

    // mix with last frame in HDR space.
    vec3 last_col = texture(last_frame_hdr, screen_uv).xyz;
    vec3 mixed_col = mix(last_col, color, 1.0 / frameCounter);
    if(!is_mix_frame) mixed_col = color;
    MixedHdrColor = mixed_col;

    // tone mapping
    vec3 res = tonemapping(mixed_col);

    // gamma correct
    res.x = pow(res.x, 0.45f);
    res.y = pow(res.y, 0.45f);
    res.z = pow(res.z, 0.45f);

    MapedColor = res;
}