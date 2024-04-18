//
// Created by lx_tyin on 2022/11/24.
//

#include "tool.h"
#include "../Config.h"
#include "stb_image.h"
#include "glad/glad.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <sstream>
#include <windows.h>
#include <commdlg.h>
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

void save_file(const string &path, const string &info) {
    ofstream fout;
    string res;
    fout.open(path);
    fout << info;
    fout.close();
}

string read_shader(const string& path) {
//	string text = read_file(path);
//	std::istringstream iss(text);
//	string result = "", line;
//	while(getline(iss, line)) {
//		if(line.substr(0, 8) == "#include") {
//            string target = line.substr(8, line.size() - 8);
//            result += read_shader(target);
//		} else result += line + '\n';
//	}
//	return result;
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

string localtimestring() {
    // 获取当前时间点
    std::time_t currentTime = std::time(nullptr);

    // 转换为本地时间
    std::tm* localTime = std::localtime(&currentTime);

    // 格式化时间字符串
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%Y-%m-%d-%H-%M-%S", localTime);
    return string(timeString);
}


void copySSBO(GLuint frm, GLuint tar, uint siz) {
    glBindBuffer(GL_COPY_READ_BUFFER, frm);
    glBindBuffer(GL_COPY_WRITE_BUFFER, tar);

    glBufferData(GL_COPY_WRITE_BUFFER, siz, NULL, GL_STATIC_COPY);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, siz);

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}


string show_file_open_dialog() {
    OPENFILENAME ofn;
    TCHAR szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrTitle = "Select a File";
    ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        return szFile;
    } else {
        return "";
    }
}


string show_save_dir_dialog() {
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrDefExt = "txt";
    ofn.lpstrTitle = "Save File As";
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn) == TRUE) {
        return szFile;
    } else {
        return "";
    }
}
