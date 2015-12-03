#include "InterpolationCamera.h"


InterpolationCamera::InterpolationCamera() {
}

InterpolationCamera::InterpolationCamera(const Camera& camera_start, const Camera& camera_end) : camera_start(camera_start), camera_end(camera_end), t(0) {
}

bool InterpolationCamera::forward() {
	t += 0.1f;
	if (t >= 1.0f) {
		t = 1.0f;
		return true;
	}
	else return false;
}

Camera InterpolationCamera::currentCamera() {
	Camera ret = camera_start;
	ret.pos = camera_start.pos * (1.0f - t) + camera_end.pos * t;
	ret.xrot = camera_start.xrot * (1.0f - t) + camera_end.xrot * t;
	ret.yrot = camera_start.yrot * (1.0f - t) + camera_end.yrot * t;
	ret.zrot = camera_start.zrot * (1.0f - t) + camera_end.zrot * t;
	return ret;
}