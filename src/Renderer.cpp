//
// Created by lx_tyin on 2023/2/20.
//

#include "Renderer.h"
#include "Tool.h"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <queue>

void Renderer::init() {

    frame = 0;
    float canvas_data[] = {-1, 1, -1, -1, 1, -1,
                           1, -1, 1, 1, -1, 1};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), canvas_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);//unbind VAO

    string vertexShaderCode = read_file("shader/standard.vert");
    string fragmentShaderCode = read_file("shader/standard.frag");
    const char *vertexCodePointer = vertexShaderCode.c_str();
    const char *fragmentCodePointer = fragmentShaderCode.c_str();

    int success;
    char infoLog[512];
    //编译上述shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCodePointer, nullptr);
    glCompileShader(vertexShader);
    //编译当然要检查是否成功，后面的也差不多
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "vertexShader compile failed: " << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCodePointer, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "fragmentShader compile failed: " << infoLog << std::endl;
    }

    //连接，得到一个完整的shaderProgram
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    //检查连接是否成功
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "link failed: " << infoLog << std::endl;
    }

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Renderer::load_to_gpu(vector<vec3> &buff, const char *name, int idx) {

    if(shaderProgram == -1) {
        std::cerr << "Renderer: Has not init yet." << std::endl;
        return;
    }
    glUseProgram(shaderProgram);

    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glBufferData(GL_TEXTURE_BUFFER, buff.size() * 3 * sizeof(float), buff.data(), GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0 + idx);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_BUFFER, tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);

    glUniform1i(glGetUniformLocation(shaderProgram, name), idx);
}

void Renderer::reload_material(Scene *scene) {
    // <color, emission, (is_emit, 0, 0)> 3x
    auto &buff = material_buff;
    buff.clear();
    for(auto *obj: scene->objects) {
        auto *m = obj->material;
        buff.emplace_back(m->color);
        buff.emplace_back(m->emission);
        buff.emplace_back(m->is_emit, 0, 0);
    }
    load_to_gpu(buff, "materials", 0);
}

void Renderer::reload_triangles(Scene *scene) {
    // <v0, v1, v2, (u0, u1, u2), (v0, v1, v2), (m_index, 0, 0)> 6x
    auto &buff = triangle_buff;
    buff.clear();
    lightindex_buff.clear();
    triangle_index.clear();
    int n = 0, m = 0;
    for(auto *o: scene->objects) {
        for(auto &tr: o->triangles) {
            auto &t = scene->triangles[n];
            if(o->material->is_emit) lightindex_buff.emplace_back(n, 0, 0);
            triangle_index[&t] = n++;
            for(auto & i : t.vertex) buff.push_back(i);
            buff.emplace_back(t.uv[0][0], t.uv[1][0], t.uv[2][0]);
            buff.emplace_back(t.uv[0][1], t.uv[1][1], t.uv[2][1]);
            buff.emplace_back(m, 0, 0);
        }
        m++;
    }
    load_to_gpu(buff, "triangles", 1);
    load_to_gpu(lightindex_buff, "lightindexs", 2);

    assert(lightindex_buff.size() > 0); // 场景至少需要一个光源

    glUniform1i(glGetUniformLocation(shaderProgram, "light_t_num"), lightindex_buff.size());
    glUniform1i(glGetUniformLocation(shaderProgram, "triangles_num"), scene->triangles.size());
}

void Renderer::reload_bvhnodes(Scene *scene) {
    // <aa, bb, (l_index, r_index, t_index)> 3x
    auto &buff = bvhnodes_buff;
    buff.resize(scene->bvh_root->siz * 3);

    std::queue<std::pair<BVHNode*, int>> q; // <node, index>
    q.emplace(scene->bvh_root, 0);
    int n = 0;
    while(!q.empty()) {
        auto &[p, index] = q.front();
        q.pop();
        int il = -1, ir = -1, it = -1;
        if(p->ls) il = ++n, q.emplace(p->ls, il);
        if(p->rs) ir = ++n, q.emplace(p->rs, ir);
        if(p->isleaf) it = triangle_index[p->triangle];
        buff[index * 3] = p->aa;
        buff[index * 3 + 1] = p->bb;
        buff[index * 3 + 2] = {il, ir, it};
    }
    load_to_gpu(buff, "bvhnodes", 3);
}

void Renderer::reload_scene(Scene *scene) {
    reload_material(scene);
    reload_triangles(scene);
    reload_bvhnodes(scene);
}


void Renderer::set_screen(int w, int h) {
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_W"), w);
    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_H"), h);
}

void Renderer::draw(Scene *scene, mat4 view) {

    if(shaderProgram == -1) {
        std::cerr << "Renderer: Has not init yet." << std::endl;
        return;
    }

    frame++;
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "v2w_mat"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform1i(glGetUniformLocation(shaderProgram, "frameCounter"), frame);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


