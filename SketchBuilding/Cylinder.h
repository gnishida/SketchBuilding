#pragma once

#include "Shape.h"

namespace cga {

class Cylinder : public Shape {
public:
	Cylinder() {}
	Cylinder(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float depth, float height, const glm::vec3& color);
	boost::shared_ptr<Shape> clone(const std::string& name) const;
	void generateGeometry(RenderManager* renderManager, float opacity) const;
};

}
