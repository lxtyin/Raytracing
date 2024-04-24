//
// Created by 19450 on 2024/4/20.
//

#ifndef PATH_TRACING_RASTERPASS_H
#define PATH_TRACING_RASTERPASS_H

#include "RenderPass.h"
#include "../instance/Camera.h"
using glm::vec2;
class Instance;

class RasterPass: public RenderPass {
    GLuint frameBufferObject;
    GLuint depthTestRenderBuffer; // use for depth test (store gl_FragCoord.z).

    void init_fbo();

public:
    // Output GBuffers, (store as texture).
    GLuint depthGBufferTexture;
    GLuint normalGBufferTexture;
    GLuint uvGBufferTexture;
    GLuint instanceIndexGBufferTexture;

    void init_shader(const string &vertexShaderPath, const string &fragShaderPath);

    RasterPass(const string &vertexShaderPath, const string &fragShaderPath);
    ~RasterPass();

    void draw(Camera *camera, vec2 jitter);
};


#endif //PATH_TRACING_RASTERPASS_H
