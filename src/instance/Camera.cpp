//
// Created by 19450 on 2023/6/17.
//

#include "Camera.h"
#include "../Config.h"

Camera::Camera(float _fov, float _far): fov(_fov), far(_far), Instance("Camera", nullptr) {
    // near由fov确定
    near = 1.0 / tan(fov / 2);
}

mat4 Camera::w2v_matrix() {
	return glm::inverse(matrix_to_global());
}

mat4 Camera::v2w_matrix() {
	return matrix_to_global();
}

mat4 Camera::projection() {
	return glm::matbyrow({
		-near,	0,		0,	0,
		0,		-near,	0, 	0,
		0,	 	0,  	-near - far,	-near * far,
		0,		0, 		1, 	0,
	});
}
