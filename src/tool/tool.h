#pragma once

#include "glad/glad.h"
#include <cstdio>
#include <cstdarg>
#include <string>
using std::string;
using uint = unsigned int;

template<typename... T>
inline string str_format(const char* fmt, T... args) {
    char buf[128] = {0};
    sprintf(buf, fmt, args...);
    return buf;
}

string read_file(const string &path);

string read_shader(const string &path);

string localtimestring();

void copySSBO(GLuint frm, GLuint tar, uint siz);
