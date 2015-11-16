#include "BoundingBox.h"

namespace cga {

BoundingBox::BoundingBox(const std::vector<glm::vec2>& points) {
	minPt.x = (std::numeric_limits<float>::max)();
	minPt.y = (std::numeric_limits<float>::max)();
	minPt.z = 0.0f;
	maxPt.x = -(std::numeric_limits<float>::max)();
	maxPt.y = -(std::numeric_limits<float>::max)();
	maxPt.z = 0.0f;

	for (int i = 0; i < points.size(); ++i) {
		minPt.x = std::min(minPt.x, points[i].x);
		minPt.y = std::min(minPt.y, points[i].y);
		maxPt.x = std::max(maxPt.x, points[i].x);
		maxPt.y = std::max(maxPt.y, points[i].y);
	}
}

BoundingBox::BoundingBox(const std::vector<glm::vec3>& points) {
	minPt.x = (std::numeric_limits<float>::max)();
	minPt.y = (std::numeric_limits<float>::max)();
	minPt.z = (std::numeric_limits<float>::max)();
	maxPt.x = -(std::numeric_limits<float>::max)();
	maxPt.y = -(std::numeric_limits<float>::max)();
	maxPt.z = -(std::numeric_limits<float>::max)();

	for (int i = 0; i < points.size(); ++i) {
		minPt.x = std::min(minPt.x, points[i].x);
		minPt.y = std::min(minPt.y, points[i].y);
		minPt.z = std::min(minPt.z, points[i].z);
		maxPt.x = std::max(maxPt.x, points[i].x);
		maxPt.y = std::max(maxPt.y, points[i].y);
		maxPt.z = std::max(maxPt.z, points[i].z);
	}
}

BoundingBox::BoundingBox(const std::vector<std::vector<glm::vec3> >& points) {
	minPt.x = (std::numeric_limits<float>::max)();
	minPt.y = (std::numeric_limits<float>::max)();
	minPt.z = (std::numeric_limits<float>::max)();
	maxPt.x = -(std::numeric_limits<float>::max)();
	maxPt.y = -(std::numeric_limits<float>::max)();
	maxPt.z = -(std::numeric_limits<float>::max)();

	for (int i = 0; i < points.size(); ++i) {
		for (int k = 0; k < points[i].size(); ++k) {
			minPt.x = std::min(minPt.x, points[i][k].x);
			minPt.y = std::min(minPt.y, points[i][k].y);
			minPt.z = std::min(minPt.z, points[i][k].z);
			maxPt.x = std::max(maxPt.x, points[i][k].x);
			maxPt.y = std::max(maxPt.y, points[i][k].y);
			maxPt.z = std::max(maxPt.z, points[i][k].z);
		}
	}
}

}
