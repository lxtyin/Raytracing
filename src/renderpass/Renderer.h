//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "../instance/Triangle.h"
#include "RenderPass.h"
#include <vector>
#include <map>

class Scene;
class Instance;

class Renderer: public VertexFragmentRenderPass {

public:

    Renderer(const string &shaderPath);

    void draw(GBuffer &gbuffer);
};


#endif //PATH_TRACING_RENDERER_H
