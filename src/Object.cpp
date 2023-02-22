//
// Created by lx_tyin on 2023/2/20.
//

#include "Object.h"

mat4 Object::transform() {
    mat4 s{ scale.x, 0, 0, 	0,
            0, scale.y, 0, 	0,
            0, 0, scale.z, 	0,
            0, 0, 0,		1 };
    float x = rotation.x;
    float y = rotation.y;
    float z = rotation.z;
    mat4 rot_x{ 1,		0,		0,		0,
                0,		cos(x),	-sin(x),0,
                0,		sin(x),	cos(x),	0,
                0, 		0,	 	0,		1};
    mat4 rot_y{ cos(y), 0,		sin(y), 0,
                0, 		1,		0,		0,
                -sin(y),0,		cos(y), 0,
                0, 		0,		0, 		1 };
    mat4 rot_z{ cos(z), -sin(z),0, 		0,
                sin(z), cos(z), 0, 		0,
                0, 	 	0,	   	1, 		0,
                0,   	0,  	0, 		1 };

    // x, y, z;
    mat4 res = rot_x * rot_y * rot_z * s;
    res[3] = {position, 1};
    return res;
}


vec3 Object::direction_x() {
    return transform()[0];
}
vec3 Object::direction_y() {
    return transform()[1];
}
vec3 Object::direction_z() {
    return transform()[2];
}
