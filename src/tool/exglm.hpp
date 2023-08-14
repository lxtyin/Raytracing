#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
using std::ostream;
using std::cout;

namespace glm {
    //扩展矩阵左乘 仿照glm加速。
    template<typename T, qualifier Q>
    GLM_FUNC_QUALIFIER mat<4, 4, T, Q> translate(vec<3, T, Q> const& v, mat<4, 4, T, Q> const& m) {
        mat4 Result(m);
        Result[3] = m[3] + vec4(v, 0);
        return Result;
    }
    template<typename T, qualifier Q>
    GLM_FUNC_QUALIFIER mat<4, 4, T, Q> rotate(T angle, vec<3, T, Q> const& v, mat<4, 4, T, Q> const& m) {
        // 这边比较麻烦 先不管效率这么放这
        return rotate(mat4(1.0f), angle, v) * m;
    }

    //单点的旋转
    template<typename T, qualifier Q>
    GLM_FUNC_QUALIFIER vec<3, T, Q> point_rotate(vec<3, T, Q> const& ori, T angle, vec<3, T, Q> const& v) {
        return rotate(mat4(1.0f), angle, v) * vec4(ori, 1.0);
    }

	inline ostream& operator << (ostream &f, const mat4 &m) {
		for(int i = 0;i < 4;i++){
			for(int j = 0;j < 4;j++){
				f << m[i][j] << " \n"[j == 3];
			}
		}
		return f;
	}
	inline ostream& operator << (ostream &f, const vec3 &v) {
		for(int i = 0;i < 3;i++){
			f << v[i] << " \n"[i == 2];
		}
		return f;
	}

	inline mat4 matbyrow(const std::initializer_list<float> &a) {
		assert(a.size() == 16);
		const float *d = data(a);
		return {
			d[0], d[4], d[8], d[12],
			d[1], d[5], d[9], d[13],
			d[2], d[6], d[10], d[14],
			d[3], d[7], d[11], d[15]
		};
	}
}
