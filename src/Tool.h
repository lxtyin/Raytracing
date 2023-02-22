//
// Created by lx_tyin on 2023/2/22.
//

#ifndef PATH_TRACING_TOOL_H
#define PATH_TRACING_TOOL_H

#include "Object.h"
#include <string>
using std::string;

string read_file(const string &path);

Object* load_obj(const string &file);

#endif //PATH_TRACING_TOOL_H
