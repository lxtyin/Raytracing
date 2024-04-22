//
// Created by 19450 on 2023/6/17.
//

#ifndef PATH_TRACING_SRC_INSTANCE_CAMERA_H_
#define PATH_TRACING_SRC_INSTANCE_CAMERA_H_

#include "Instance.h"

// perspective camera.
class Camera: public Instance {
 public:

    float fovX;
    float near, far;

	Camera(float _near, float _far);

	mat4 w2v_matrix();
	mat4 v2w_matrix();
	mat4 projection();

};

#endif //PATH_TRACING_SRC_INSTANCE_CAMERA_H_
