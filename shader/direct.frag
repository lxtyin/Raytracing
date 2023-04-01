
#version 330 core
uniform sampler2D prev_texture;

in vec2 screen_uv;
out vec4 FragColor;

void main() {
    FragColor = texture(prev_texture, screen_uv);
}