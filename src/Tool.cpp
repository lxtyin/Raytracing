//
// Created by lx_tyin on 2023/2/22.
//

#include "Tool.h"
#include "stb_image.h"
#include "glad/glad.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
using std::map;

string read_file(const string &path){
    std::ifstream fin;
    string res;
    //这两行确保ifstream能抛出异常
    fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        fin.open(path);
        std::stringstream stream;
        stream << fin.rdbuf();
        res = stream.str();
        fin.close();
    } catch(std::ifstream::failure e) {
        std::cerr << "Tool: Fail to read " << path << ", " << e.what() << std::endl;
    }
    return res;
}

vector<Object*> load_obj(const string &fo, const string &fm){
    std::ifstream fin;

    map<string, Material> materials;
    if(!fm.empty()) {
        fin.open(fm);
        string type, name = "";
        Material cur;
        while(fin >> type) {
            if(type == "newmtl") {
                materials[name] = cur;
                fin >> name;
                cur = Material();
            } else if(type == "Ns") {
                float ns;
                fin >> ns;
                cur.roughness = sqrt(2.0 / (ns + 2)); // Blinn-Phong 高光系数 - roughness
            } else if(type == "Kd") {
                vec3 kd;
                fin >> kd[0] >> kd[1] >> kd[2];
                cur.color = kd;
            } else if(type == "Ks") {
                vec3 ks;
                fin >> ks[0] >> ks[1] >> ks[2];
                // 先随便读一下 todo
                cur.metallic = 0.3;
            } else if(type == "Ke") {
                vec3 ke;
                fin >> ke[0] >> ke[1] >> ke[2];
                cur.emission = ke;
                cur.is_emit = ke.length() > 0.01;
            } else {
                std::getline(fin, type);
                continue;
            }
        }
        materials[name] = cur;
        fin.close();
    }

    fin.open(fo);
    vector<vec3> vertexs = {vec3(0)};
    vector<vec2> uvs = {vec2(0)};
    vector<vec3> normals = {vec3(0)};

    vector<Object*> result;
    Object *o = new Object;
    string type;
    while(fin >> type) {
        if(type == "v") {
            float x, y, z;
            fin >> x >> y >> z;
            vertexs.emplace_back(x, y, z);
        } else if(type == "vt") {
            float x, y;
            fin >> x >> y;
            uvs.emplace_back(x, y);
        } else if(type == "vn") {
            float x, y, z;
            fin >> x >> y >> z;
            normals.emplace_back(x, y, z);
        } else if(type == "f") {
            Triangle t;
            int v_i = 0, vt_i = 0, vn_i = 0;
            for(int i = 0;i < 3;i++) {
                char c;
                fin >> v_i; fin.get(c);
                if(c == '/') {
                    fin >> vt_i; fin.get(c);
                    if(c == '/') fin >> vn_i;
                }
                t.vertex[i] = vertexs[v_i];
                t.uv[i] = uvs[vt_i];
                t.normal[i] = normals[vn_i];
            }
            o->triangles.push_back(t);
        } else if(type == "usemtl") {
            if(o->material) result.push_back(o);
            string name;
            fin >> name;
            o = new Object;
            o->material = new Material(materials[name]);
        } else {
            std::getline(fin, type);
            continue;
        }
    }
    if(!o->material) o->material = new Material;
    result.push_back(o);

    fin.close();
    return result;
}

uint load_texture(const string &file) {
    stbi_set_flip_vertically_on_load(true);

    uint tto;
    int width, height, channel;
    try {
        unsigned char* data = stbi_load(file.c_str(), &width, &height, &channel, 0);
        int format;
        if(channel == 1) format = GL_RED;
        if(channel == 3) format = GL_RGB;
        if(channel == 4) format = GL_RGBA;

        glGenTextures(1, &tto);
        glBindTexture(GL_TEXTURE_2D, tto);//和VBO同理，绑定后通过GL_TEXTURE_2D赋值
        glTexImage2D(GL_TEXTURE_2D,     //目标
                     0,                 //多级渐进纹理级别
                     format,            //保存的格式
                     width, height,     //长宽
                     0,                 //历史遗留
                     format,            //原图格式
                     GL_UNSIGNED_BYTE,  //存储为Byte数组
                     data);             //数据源
        //设置环绕方式，放大缩小时的过滤模式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //第二个参数s,t,r相当于x,y,z，对单一轴设置环绕方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //此处为简单复制，也是默认方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //设置放大缩小时的插值方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST 为邻近
        glGenerateMipmap(GL_TEXTURE_2D);//生成mipmap

        stbi_image_free(data);
    }  catch(std::ifstream::failure e) {
        std::cerr << "Tool: Fail to load " << file << ", " << e.what() << std::endl;
    }
}


uint load_cubebox(const string &px, const string &nx, const string &py,
                  const string &ny, const string &pz, const string &nz){

    stbi_set_flip_vertically_on_load(true);

    int width, height, channel;
    auto load = [&](const string &f, int i) {
        try {
            unsigned char* data = stbi_load(f.c_str(), &width, &height, &channel, 0);
            int format;
            if(channel == 1) format = GL_RED;
            if(channel == 3) format = GL_RGB;
            if(channel == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,     //目标
                         0,                                      //多级渐进纹理级别
                         format,                                 //保存的格式
                         width, height,                          //长宽
                         0,                                      //历史遗留
                         format,                                 //原图格式
                         GL_UNSIGNED_BYTE,                       //存储为Byte数组
                         data);                                  //数据源
            stbi_image_free(data);
        }  catch(std::ifstream::failure e) {
            std::cerr << "Tool: Fail to load " << f << ", " << e.what() << std::endl;
        }
    };

    uint tto;
    glGenTextures(1, &tto);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tto);
    load(px, 0), load(nx, 1), load(py, 2);
    load(ny, 3), load(pz, 4), load(nz, 5);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return tto;
}
