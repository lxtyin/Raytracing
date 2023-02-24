#version 330 core

layout (location = 0) in vec2 vpos;

uniform int SCREEN_W;
uniform int SCREEN_H;

out float pixel_x; // (-W/2, -H/2) - (W/2, H/2)
out float pixel_y;
out vec2 tex_uv;   // (0, 0) - (1, 1)

void main() {
    pixel_x = vpos.x * SCREEN_W / 2;
    pixel_y = vpos.y * SCREEN_H / 2;
    tex_uv = (vpos + vec2(1)) / 2;
    gl_Position = vec4(vpos, 0, 1.0);
}
