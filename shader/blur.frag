
#version 330 core
uniform int SCREEN_W;
uniform int SCREEN_H;
uniform sampler2D prev_texture;

in vec2 screen_uv;
out vec4 FragColor;

void main() {

    float kernl[9] = {
            1, 2, 1,
            2, 4, 2,
            1, 2, 1
    };

    int k = 0;
    vec3 res = vec3(0);
    for(int i = -1;i <= 1;i++){
        for(int j = -1;j <= 1;j++){
            vec2 uv2 = screen_uv + vec2(0.001 * i, 0.001 * j);
            res += vec3(texture(prev_texture, uv2)) * kernl[k++];
        }
    }
    res /= 16;
    FragColor = vec4(res, 1);
}