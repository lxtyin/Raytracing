#version 460 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_uv;

out vec3 view_pos;
out vec3 normal;
out vec2 uv;

uniform mat3 normal_matrix;
uniform mat4 view_model_matrix;
uniform mat4 proj_matrix;

void main() {
    vec4 vp = view_model_matrix * vec4(in_pos, 1.0);
    vec4 fp = proj_matrix * vp;

    view_pos = vec3(vp);
    uv = in_uv;
    normal = normalize(normal_matrix * in_normal);

    gl_Position = fp;
}
