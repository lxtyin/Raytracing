//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_GBUFFER_H
#define PATH_TRACING_GBUFFER_H

#include "glad/glad.h"

class GBuffer {
public:
    /**
     * GBuffers, store in texture space (Quadrant 1).
     * The buffers will not be released automaticly
     */
    GLuint colorGBufferSSBO;           // 3 per pixel
    GLuint normalGBufferSSBO;          // 3
    GLuint depthGBufferSSBO;           // 1
    GLuint motionGBufferSSBO;          // 2, motion in screen space ([0, 1]^2).
    GLuint albedoGBufferSSBO;
    GLuint momentGBufferSSBO;
    GLuint meshIndexGBufferSSBO;

    GBuffer();
    ~GBuffer();
};


#endif //PATH_TRACING_GBUFFER_H
