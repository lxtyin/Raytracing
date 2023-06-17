
#version 330 core
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D prevpass_color;
uniform sampler2D prevpass_albedo;
uniform sampler2D prevpass_normal;
uniform bool is_filter_enabled;

in vec2 screen_uv;
layout(location = 0) out vec4 FinalColor;

float len23(vec3 x) {
    return x.x * x.x + x.y * x.y + x.z * x.z;
}
float len22(vec2 x) {
    return x.x * x.x + x.y * x.y;
}

void main() {

    if(!is_filter_enabled) {
        FinalColor = texture(prevpass_color, screen_uv);
        return;
    }

    vec3 color =  texture(prevpass_color, screen_uv).xyz;
    vec3 normal = texture(prevpass_normal, screen_uv).xyz;
    vec3 albedo = texture(prevpass_albedo, screen_uv).xyz;

    // joint bilateral filter
    float sum_weight = 0;
    vec3 sum_color = vec3(0);

    int r = 16;
    // copy parameter from games202.
    float m_sigmaCoord = 4.0f;
    float m_sigmaColor = 0.6;
    float m_sigmaNormal = 0.1;
    float sigma_color2 =  2 * m_sigmaColor * m_sigmaColor;
    float sigma_normal2 = 2 * m_sigmaNormal * m_sigmaNormal;
    float sigma_coord2 =  2 * m_sigmaCoord * m_sigmaCoord;

    float iH = 1.0 / SCREEN_H;

    for(int j = -r;j <= r;j++){
        vec2 uv2 = screen_uv + vec2(0, j * iH);

        vec3 p_color  = texture(prevpass_color, uv2).xyz;
        vec3 p_normal = texture(prevpass_normal, uv2).xyz;
        vec3 p_albedo = texture(prevpass_albedo, uv2).xyz;

        float dis = j * j        / sigma_coord2  +
        len23(p_color - color)   / sigma_color2  +
        len23(p_albedo - albedo) / sigma_color2 +
        len23(p_normal - normal) / sigma_normal2;

        float w = exp(-dis);
        sum_color += w * p_color;
        sum_weight += w;
    }
    vec3 res = sum_color / sum_weight;

    FinalColor = vec4(res, 1);
}