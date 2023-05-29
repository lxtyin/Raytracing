
#version 330 core
uniform sampler2D prevpass_color;

in vec2 screen_uv;
out vec4 FragColor;

void main() {
    FragColor = texture(prevpass_color, screen_uv);
}