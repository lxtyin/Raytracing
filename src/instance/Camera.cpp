//
// Created by 19450 on 2023/6/17.
//

#include "Camera.h"
#include "../Config.h"

Camera::Camera(float _fov, float _far): fovX(_fov), far(_far), Instance("Camera", nullptr) {
    // near由fov确定
    near = 1.0 / tan(fovX / 2);
}

mat4 Camera::w2v_matrix() {
	return glm::inverse(matrix_to_global());
}

mat4 Camera::v2w_matrix() {
	return matrix_to_global();
}

mat4 Camera::projection() {
    // project to [-1:1, -1:1, near:far]
//	return glm::matbyrow({
//		-near,	0,		0,	0,
//		0,		-near,	0, 	0,
//		0,	 	0,  	-near - far,	-near * far,
//		0,		0, 		1, 	0,
//	});

    float scaleY = 1.0 * SCREEN_H / SCREEN_W;

//     project to [-1:1, -1:1, -1:1]
    float n = -near;
    float f = -far;

	mat4 proj = -glm::matbyrow({
		n,	0,		        0,	0,
		0,	n / scaleY,	    0, 	0,
		0,	0,  	        (n + f) / (f - n),	-2 * n * f / (f - n),
		0,	0, 		        1, 	0,
	});
    return proj;
}
