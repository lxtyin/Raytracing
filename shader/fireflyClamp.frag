
#version 330 core
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D prevpass_color;
uniform bool is_filter_enabled;

in vec2 screen_uv;
layout(location = 0) out vec4 FinalColor;

float len23(vec3 x) {
    return x.x * x.x + x.y * x.y + x.z * x.z;
}
float len22(vec2 x) {
    return x.x * x.x + x.y * x.y;
}
float lumin(vec3 x) {
    return dot(x, vec3(0.2, 0.7, 0.1));
}

void main() {

    if(!is_filter_enabled) {
        FinalColor = texture(prevpass_color, screen_uv);
        return;
    }

    float iW = 1.0 / SCREEN_W, iH = 1.0 / SCREEN_H;
    float sigma = 0, miu = 0;
    int r = 7;

    for(int i = -r;i <= r;i++) {
        for(int j = -r;j <= r;j++){
            vec2 uv2 = screen_uv + vec2(i * iW, j * iH);
            float lum = lumin(texture(prevpass_color, uv2).xyz);
            miu += lum;
            sigma += lum * lum;
        }
    }
    miu /= (r + 1) * (r + 1);
    sigma /= (r + 1) * (r + 1);
    sigma = sqrt(sigma - miu * miu);

    vec3 cur = texture(prevpass_color, screen_uv).xyz;
    float lum = lumin(cur);
    float clum = clamp(lum, miu - 2 * sigma, miu + 2 * sigma);
    cur *= clum / lum;

    FinalColor = vec4(cur, 1);
}