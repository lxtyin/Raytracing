
#version 330 core
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D prev_texture;

in vec2 tex_uv;
out vec4 FragColor;

void main() {

    FragColor = texture(prev_texture, tex_uv);
}