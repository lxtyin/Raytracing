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

void save_file(const string &path, const string &info);

string localtimestring();


string show_file_open_dialog() ;

string show_save_dir_dialog() ;


string show_file_open_dialog() ;

string show_save_dir_dialog() ;