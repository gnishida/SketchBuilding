#include "InterpolationCamera.h"


InterpolationCamera::InterpolationCamera() {
}

InterpolationCamera::InterpolationCamera(const Camera& camera_start, float target_xrot, float target_yrot, float target_zrot, const glm::vec3& target_pos) : camera_start(camera_start), t(0) {
	camera_end = camera_start;
	camera_end.xrot = target_xrot;
	camera_end.yrot = target_yrot;
	camera_end.zrot = target_zrot;
	camera_end.pos = target_pos;

	if (camera_end.yrot - camera_start.yrot > 180.0f) {
		camera_end.yrot -= 360.0f;
	}
	else if (camera_start.yrot - camera_end.yrot > 180.0f) {
		camera_end.yrot += 360.0f;
	}
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