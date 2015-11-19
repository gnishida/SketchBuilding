#pragma once

#include "Shape.h"

namespace cga {

class Cylinder : public Shape {
public:
	Cylinder() {}
	Cylinder(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float depth, float height, const glm::vec3& color);
	boost::shared_ptr<Shape> clone(const std::string& name) const;
	void comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes);
	void generateGeometry(std::vector<glutils::Face>& faces, float opacity) const;
};

}
