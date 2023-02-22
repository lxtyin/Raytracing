#version 330 core

layout (location = 0) in vec2 vpos;

uniform int SCREEN_W;
uniform int SCREEN_H;

out float pixel_x;
out float pixel_y;

void main() {
    pixel_x = vpos.x * SCREEN_W / 2;
    pixel_y = vpos.y * SCREEN_H / 2;
    gl_Position = vec4(vpos, 0, 1.0);
}
