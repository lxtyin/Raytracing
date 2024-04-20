//
// Created by 19450 on 2024/4/19.
//

#ifndef PATH_TRACING_BUFFER_H
#define PATH_TRACING_BUFFER_H

#include "glad/glad.h"
#include "tool/tool.h"
using uint = unsigned int;

template<class T>
class SSBOBuffer {
    uint m_size;
    GLuint m_ssbo;
public:
    SSBOBuffer();
    SSBOBuffer(uint siz);
    SSBOBuffer(uint siz, T* data);

    void release();

    bool isempty();
    void bind_current_shader(int index);
    void copy(SSBOBuffer<T> *buffer);
};

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

//    glGenBuffers(1, &m_ssbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, m_size * sizeof(T), tmpdata, GL_DYNAMIC_COPY);
    glCreateBuffers(1, &m_ssbo);
    glNamedBufferStorage(
            m_ssbo,
            m_size * sizeof(T),
            (const void *)tmpdata,
            GL_DYNAMIC_STORAGE_BIT
    );
}

template<class T>
SSBOBuffer<T>::SSBOBuffer(uint siz, T *data) {
    m_size = siz;

//    glGenBuffers(1, &m_ssbo);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
//    glBufferData(GL_SHADER_STORAGE_BUFFER, m_size * sizeof(T), data, GL_DYNAMIC_COPY);
    glCreateBuffers(1, &m_ssbo);
    glNamedBufferStorage(
            m_ssbo,
            m_size * sizeof(T),
            (const void *)data,
            GL_DYNAMIC_STORAGE_BIT
    );
}


template<class T>
void SSBOBuffer<T>::bind_current_shader(int index) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_ssbo);
}

template<class T>
void SSBOBuffer<T>::copy(SSBOBuffer<T> *buffer) {
    m_size = buffer->m_size;
    copySSBO(buffer->m_ssbo, m_ssbo, m_size * sizeof(T));
}


template<class T>
bool SSBOBuffer<T>::isempty() {
    return m_ssbo == 0;
}



#endif //PATH_TRACING_BUFFER_H
