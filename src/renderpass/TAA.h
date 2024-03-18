//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TAA_H
#define PATH_TRACING_TAA_H

#include "RenderPass.h"

class TAA: public VertexFragmentRenderPass {
public:
    GBuffer history;

    bool firstFrame;

    TAA(const string &fragShaderPath);

    /**
     * Note curFrame will be stolen if saveFrame = false;
     * \saveFrame set false only if it is the final pass.
     */
    void draw(GBuffer &curFrame, bool saveFrame = true);
};


#endif //PATH_TRACING_TAA_H
