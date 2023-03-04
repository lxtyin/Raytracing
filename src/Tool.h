//
// Created by lx_tyin on 2023/2/22.
//

#ifndef PATH_TRACING_TOOL_H
#define PATH_TRACING_TOOL_H

#include "Object.h"
#include <string>
using std::string;
using uint = unsigned int ;

string read_file(const string &path);

vector<Object*> load_obj(const string &file, const string &fm = "");

uint load_texture(const string &fo, const string &fm = "");

uint load_cubebox(const string &px, const string &nx, const string &py,
                  const string &ny, const string &pz, const string &nz);

#endif //PATH_TRACING_TOOL_H
