//
// Created by 19450 on 2024/3/4.
//

#ifndef PATH_TRACING_GPURESOURCE_H
#define PATH_TRACING_GPURESOURCE_H


class GPUResource {
public:
    bool isonGPU = false;
    virtual void load_to_gpu() = 0;
    virtual void unload_from_gpu() = 0;
};


#endif //PATH_TRACING_GPURESOURCE_H
