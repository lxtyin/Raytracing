//
// Created by lx_tyin on 2023/2/22.
//

#include "Tool.h"
#include <iostream>
#include <fstream>
#include <sstream>

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


Object* load_obj(const string &file){
    std::ifstream fin(file);
    vector<vec3> vertexs = {vec3(0)};
    Object *o = new Object;

    string type;
    while(fin >> type) {
        if(type == "v"){
            vec3 x;
            fin >> x[0] >> x[1] >> x[2];
            vertexs.push_back(x);
        } else if(type == "f") {
            int i, j, k;
            fin >> i >> j >> k;
            o->triangles.emplace_back(vertexs[i], vertexs[j], vertexs[k]);
        }
    }
    fin.close();
    return o;
}
