
#version 330 core
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D prev_texture;

in vec2 screen_uv;
out vec4 FragColor;

void main() {

    vec3 result = vec3(texture(prev_texture, screen_uv));
    result = result / (result + 1);

    result.x = pow(result.x, 0.45f);
    result.y = pow(result.y, 0.45f);
    result.z = pow(result.z, 0.45f);

    FragColor = vec4(result, 1);
}