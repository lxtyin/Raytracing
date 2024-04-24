//
// Created by 19450 on 2024/4/19.
//

#ifndef PATH_TRACING_SSBOBUFFER_H
#define PATH_TRACING_SSBOBUFFER_H

#include "glad/glad.h"
#include "tool/tool.h"
#include "texture/Texture.h"
#include <vector>
#include <opencv2/opencv.hpp>
using uint = unsigned int;
using uchar = unsigned char;

template <typename T> struct Checker;


template<class T>
class SSBOBuffer {
    uint m_size;
    GLuint m_ssbo;
public:
    SSBOBuffer();
    SSBOBuffer(uint siz);
    SSBOBuffer(uint siz, T* data);

    void release();

    bool isempty() const;
    void bind_current_shader(int index) const;
    void copy(const SSBOBuffer<T> *buffer);
    void copy(const T *data);
    void fill(const T &val);

    void save_as_image(int width, int height, int channel, const string &path) const;
};


template<class T>
void SSBOBuffer<T>::save_as_image(int width, int height, int channel, const string &path) const {
    // T must be float here !
    assert(width * height * channel == m_size);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);
    float* data = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, m_size * sizeof(float), GL_MAP_READ_BIT);
    uchar *datau = new uchar[m_size];
    for(int i = 0;i < m_size;i++) datau[i] = std::min(255, int(data[i] * 255));

    int format, convert = 0;
    if(channel == 1) format = CV_8U;
    if(channel == 3) format = CV_8UC3, convert = cv::COLOR_RGB2BGR;
    if(channel == 4) format = CV_8UC4, convert = cv::COLOR_RGBA2BGRA;

    cv::Mat img(height, width, format, datau);
    cv::flip(img, img, 0);
    cv::cvtColor(img, img, convert);
    cv::imwrite(path, img);

    delete[] datau;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

template<class T>
void SSBOBuffer<T>::copy(const T *data) {
    if(isempty()) return;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_size * sizeof(T), data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

template<class T>
void SSBOBuffer<T>::fill(const T &val) {
    if(isempty()) return;
    std::vector<T> filldata(m_size, val);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_size * sizeof(T), filldata.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

template<class T>
void SSBOBuffer<T>::release() {
    if(!isempty()) glDeleteBuffers(1, &m_ssbo);
    m_ssbo = 0;
}

template<class T>
SSBOBuffer<T>::SSBOBuffer() {
    m_size = 0;
    m_ssbo = 0;
}

template<class T>
SSBOBuffer<T>::SSBOBuffer(uint siz) {
    m_size = siz;
    T* tmpdata = new T[siz];

    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_size * sizeof(T), tmpdata, GL_DYNAMIC_COPY);
//    glCreateBuffers(1, &m_ssbo);
//    glNamedBufferStorage(
//            m_ssbo,
//            m_size * sizeof(T),
//            (const void *)tmpdata,
//            GL_DYNAMIC_STORAGE_BIT
//    );
}

template<class T>
SSBOBuffer<T>::SSBOBuffer(uint siz, T *data) {
    m_size = siz;

    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_size * sizeof(T), data, GL_DYNAMIC_COPY);
//    glCreateBuffers(1, &m_ssbo);
//    glNamedBufferStorage(
//            m_ssbo,
//            m_size * sizeof(T),
//            (const void *)data,
//            GL_DYNAMIC_STORAGE_BIT
//    );
}


template<class T>
void SSBOBuffer<T>::bind_current_shader(int index) const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_ssbo);
}

template<class T>
void SSBOBuffer<T>::copy(const SSBOBuffer<T> *buffer) {
    assert(m_size == buffer->m_size);
    glBindBuffer(GL_COPY_READ_BUFFER, buffer->m_ssbo);
    glBindBuffer(GL_COPY_WRITE_BUFFER, m_ssbo);

    glBufferData(GL_COPY_WRITE_BUFFER, m_size * sizeof(T), NULL, GL_DYNAMIC_COPY);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, m_size * sizeof(T));

    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}


template<class T>
bool SSBOBuffer<T>::isempty() const {
    return m_ssbo == 0;
}



#endif //PATH_TRACING_SSBOBUFFER_H
