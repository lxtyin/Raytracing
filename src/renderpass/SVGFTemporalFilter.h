//
// Created by 19450 on 2024/3/18.
//

#ifndef PATH_TRACING_SVGFTEMPORALFILTER_H
#define PATH_TRACING_SVGFTEMPORALFILTER_H

#include "RenderPass.h"

class SVGFTemporalFilter: public VertexFragmentRenderPass {
public:
    GBuffer history;

    SVGFTemporalFilter(const string &fragShaderPath);

    void draw(GBuffer &curFrame);
};


#endif //PATH_TRACING_SVGFTEMPORALFILTER_H