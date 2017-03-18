#ifndef VSB_GPG_NORMAL_SHADER_H
#define VSB_GPG_NORMAL_SHADER_H
#include "vector3.h"
#include <opencv2/core/matx.hpp>

class NormalShader
{
public:
	NormalShader() {};

	cv::Vec3f static getNormal(Vector3 normal) {
			Vector3 result = normal / 2.f;
			result.x += 0.5f;
			result.y += 0.5f;
			result.z += 0.5f;
			return cv::Vec3f(result.z, result.y, result.x);	
	}
};

#endif