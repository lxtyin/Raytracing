//
// Created by 19450 on 2023/5/22.
//

#ifndef PATH_TRACING__NERFCREATOR_H_
#define PATH_TRACING__NERFCREATOR_H_

#include "src/tool/json.hpp"
#include "src/tool/tool.h"
#include "src/Config.h"
#include "src/instance/Instance.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <fstream>
using std::string;

const string data_prefix = "nerfdata/";

class NerfCreator {
 public:
	JSON::Value nerf_json;
	int file_id = 0;

	NerfCreator() {
		nerf_json["camera_angle_x"] = 1.047197;
		nerf_json["camera_angle_y"] = 1.047197;
		nerf_json["fl_x"] = (double)SCREEN_W / 2 / tan(SCREEN_FOV / 2);
		nerf_json["fl_y"] = (double)SCREEN_W / 2 / tan(SCREEN_FOV / 2);
		nerf_json["k1"] = 0.0;
		nerf_json["k2"] = 0.0;
		nerf_json["p1"] = 0.0;
		nerf_json["p2"] = 0.0;
		nerf_json["cx"] = (double)SCREEN_W / 2;
		nerf_json["cy"] = (double)SCREEN_H / 2;
		nerf_json["w"] = (double)SCREEN_W;
		nerf_json["h"] = (double)SCREEN_H;
		nerf_json["aabb_scale"] = 4.0;
		nerf_json["frames"] = std::vector<JSON::Value>(0);
	}

	void screenshot(Instance* camera) {
		string file_path = str_format("images/%d.png", ++file_id);

		mat4 T = camera->matrix_to_global();
		T = glm::rotate((float)M_PI / 2, vec3(1, 0, 0), T); // rot(x, 90) * T
		JSON::Value mj(vector<JSON::Value>(0));
		for(int i = 0;i < 4;i++) {
			JSON::Value lj(vector<JSON::Value>(0));
			for(int j = 0;j < 4;j++) lj.append(T[j][i]);
			mj.append(lj);
		}
		JSON::Value cur;
		cur["file_path"] = file_path;
		cur["sharpness"] = 30.0;
		cur["transform_matrix"] = mj;
		nerf_json["frames"].append(cur);

		cv::Mat screenshot(SCREEN_W, SCREEN_H, CV_8UC3);
		glReadPixels(0, 0, SCREEN_W, SCREEN_H, GL_BGR, GL_UNSIGNED_BYTE, screenshot.data);
		cv::Mat fliped;
		cv::flip(screenshot, fliped, 0);
		cv::imwrite(data_prefix + file_path, fliped);

		cout << "ScreenShot " << file_path << " ok." << '\n';
	}

	void writejson() {
		JSON::StyleWriter sw;
		std::ofstream fout(data_prefix + "transforms.json");
		fout << sw.write(nerf_json) << std::endl;
		fout.close();
		cout << "Json file saved ok." << '\n';
	}

};

#endif //PATH_TRACING__NERFCREATOR_H_
