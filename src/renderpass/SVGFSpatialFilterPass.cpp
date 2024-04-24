//
// Created by 19450 on 2024/3/17.
//

#include "SVGFSpatialFilterPass.h"
#include "../tool/tool.h"
#include "../Config.h"

SVGFSpatialFilterPass::SVGFSpatialFilterPass(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    outputColorGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    tmpColorGBufferSSBO(SCREEN_H * SCREEN_W * 3){}

void SVGFSpatialFilterPass::draw(const SSBOBuffer<float> &colorGBufferSSBO,
                                 const SSBOBuffer<float> &momentGBufferSSBO,
                                 const SSBOBuffer<float> &normalGBufferSSBO,
                                 const SSBOBuffer<float> &depthGBufferSSBO,
                                 const SSBOBuffer<float> &numSamplesGBufferSSBO) {

    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_W"), SCREEN_W);
    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_H"), SCREEN_H);

    outputColorGBufferSSBO.copy(&colorGBufferSSBO);

    for(int i = 0;i < Config::SVGFSpatialFilterLevel;i++) {
        glUniform1i(glGetUniformLocation(shaderProgram, "step"), 1 << i);

        outputColorGBufferSSBO.bind_current_shader(0); // in
        normalGBufferSSBO.bind_current_shader(1);
        depthGBufferSSBO.bind_current_shader(2);
        momentGBufferSSBO.bind_current_shader(3);
        numSamplesGBufferSSBO.bind_current_shader(4);
        tmpColorGBufferSSBO.bind_current_shader(5);     // out

        drawcall();
        std::swap(tmpColorGBufferSSBO, outputColorGBufferSSBO);
    }
}

SVGFSpatialFilterPass::~SVGFSpatialFilterPass() {
    outputColorGBufferSSBO.release();
    tmpColorGBufferSSBO.release();
}
