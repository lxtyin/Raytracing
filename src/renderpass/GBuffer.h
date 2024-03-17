//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_GBUFFER_H
#define PATH_TRACING_GBUFFER_H

#include "glad/glad.h"

class GBuffer {
public:
    /**
     * GBuffers, store in texture order (Down is x and right is y).
     * The buffers will not be released automaticly
     */
    GLuint colorGBufferSSBO;           // 3 per pixel
    GLuint normalGBufferSSBO;          // 3
    GLuint depthGBufferSSBO;           // 1
    GLuint meshIndexGBufferSSBO;       // 1
    GLuint motionGBufferSSBO;          // 2, the unit is pixel.

    GBuffer();
    ~GBuffer();
};


#endif //PATH_TRACING_GBUFFER_H
