#pragma once

#include "RenderManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

namespace cga {

class BoundingBox {
public:
	glm::vec3 minPt;
	glm::vec3 maxPt;

public:
	BoundingBox(const std::vector<glm::vec2>& points);
	BoundingBox(const std::vector<glm::vec3>& points);
	BoundingBox(const std::vector<std::vector<glm::vec3> >& points);
	float sx() { return maxPt.x - minPt.x; }
	float sy() { return maxPt.y - minPt.y; }
	float sz() { return maxPt.z - minPt.z; }
};

}