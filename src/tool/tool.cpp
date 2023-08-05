//
// Created by lx_tyin on 2022/11/24.
//

#include "tool.h"
#include "../Config.h"
#include "stb_image.h"
#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

string read_file(const string &path){
    ifstream fin;
    string res;
    //这两行确保ifstream能抛出异常
    fin.exceptions(ifstream::failbit | ifstream::badbit);
    try {
        fin.open(path);
        stringstream stream;
        stream << fin.rdbuf();
        res = stream.str();
        fin.close();
    } catch(std::ifstream::failure e) {
        cerr << "tool: Fail to read " << path << ", " << e.what() << endl;
    }
    return res;
}

string read_shader(const string& path) {
	string text = read_file(path);
	std::istringstream iss(text);
	string result = "";
	char c;
	while(iss.get(c)) {
		if(c == '#') {
			string s;
			iss >> s;
			if(s == "include") {
				iss >> s;
				result += read_shader(s);
			} else result += "#" + s;
		} else result.push_back(c);
	}
	return result;
}

