#pragma once

#include "Camera.h"

class InterpolationCamera {
public:
	Camera camera_start;
	Camera camera_end;

	float t;

public:
	InterpolationCamera();
	InterpolationCamera(const Camera& camera_start, float target_xrot, float target_yrot, float target_zrot, const glm::vec3& target_pos);

	bool forward();
	Camera currentCamera();
};

