
#version 330 core
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D prev_color;
uniform sampler2D prev_albedo;
uniform sampler2D prev_normal;

in vec2 screen_uv;
out vec4 FragColor;

float len23(vec3 x) {
    return x.x * x.x + x.y * x.y + x.z * x.z;
}
float len22(vec2 x) {
    return x.x * x.x + x.y * x.y;
}

void main() {

    vec3 color = texture(prev_color, screen_uv).xyz;
    vec3 normal = texture(prev_normal, screen_uv).xyz;
    vec3 albedo = texture(prev_albedo, screen_uv).xyz;

    float mean = 0, sigma = 0;

    for(int i = -1;i <= 1;i++) {
        for (int j = -1;j <= 1; j++) {
            vec2 uv2 = screen_uv + vec2(i * 1.0 / SCREEN_W, j * 1.0 / SCREEN_H);
            vec3 p_color = texture(prev_color, uv2).xyz;
            mean += length(p_color);
            sigma += len23(p_color);
        }
    }
    mean /= 25;
    sigma = sqrt(sigma / 25 - mean * mean);

    float totw = 0;
    vec3 res = vec3(0);
    for(int i = -1;i <= 1;i++){
        for(int j = -1;j <= 1;j++){
            vec2 uv2 = screen_uv + vec2(i * 1.0 / SCREEN_W, j * 1.0 / SCREEN_H);

            vec3 p_color = texture(prev_color, uv2).xyz;

            if(abs(length(p_color) - mean) > max(0.1, sigma)) continue;
            vec3 p_normal = texture(prev_normal, uv2).xyz;
            vec3 p_albedo = texture(prev_albedo, uv2).xyz;

            float w = exp(-(len23(p_color - color) / 100 + len23(p_normal - normal) * 2 + len22(screen_uv - uv2) / 3));
            res += w * p_color;
            totw += w;
        }
    }
    if(totw == 0) res = color;
    else res /= totw;
    FragColor = vec4(res, 1);
}