//
// Created by 19450 on 2024/4/20.
//

#ifndef PATH_TRACING_RASTERPASS_H
#define PATH_TRACING_RASTERPASS_H

#include "RenderPass.h"
#include "../instance/Camera.h"

class Instance;

class RasterPass: public RenderPass {
    GLuint frameBufferObject;
    GLuint depthRenderBuffer;    // GBuffer, store distance (float).
    GLuint normalRenderBuffer;
    GLuint uvRenderBuffer;
    GLuint instanceRenderBuffer;
    GLuint depthTestRenderBuffer; // use for depth test (store gl_FragCoord.z).

    void init_fbo();
    void copy_fbodata_to_ssbo();

public:
    SSBOBuffer<float> depthGBufferSSBO;
    SSBOBuffer<float> normalGBufferSSBO;
    SSBOBuffer<float> uvGBufferSSBO;
    SSBOBuffer<float> instanceIndexGBufferSSBO;

    void init_shader(const string &vertexShaderPath, const string &fragShaderPath);

    RasterPass(const string &vertexShaderPath, const string &fragShaderPath);
    ~RasterPass();

    void draw(Camera *camera);
};


#endif //PATH_TRACING_RASTERPASS_H
