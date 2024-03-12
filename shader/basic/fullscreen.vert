#version 460 core

layout (location = 0) in vec2 vpos;

uniform int SCREEN_W;
uniform int SCREEN_H;

out vec2 screen_uv;   // (0, 0) -> (1, 1), 第一象限

void main() {
    screen_uv = (vpos + 1.0) / 2;
    gl_Position = vec4(vpos, 0, 1.0); // [-1, 1]^3 is required.
}
