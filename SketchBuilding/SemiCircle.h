#pragma once

#include <boost/shared_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include "Shape.h"

class RenderManager;

namespace cga {

class SemiCircle : public Shape {
public:
	SemiCircle() {}
	SemiCircle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, const glm::vec3& color);
	boost::shared_ptr<Shape> clone(const std::string& name) const;
	boost::shared_ptr<Shape> offset(const std::string& name, float offsetDistance, int offsetSelector);
	void generateGeometry(RenderManager* renderManager, float opacity) const;
};

}
