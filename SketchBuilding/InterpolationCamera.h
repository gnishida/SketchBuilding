#pragma once

#include "Camera.h"

class InterpolationCamera {
public:
	Camera camera_start;
	Camera camera_end;

	float t;

public:
	InterpolationCamera();
	InterpolationCamera(const Camera& camera_start, const Camera& camera_end);

	bool forward();
	Camera currentCamera();
};

