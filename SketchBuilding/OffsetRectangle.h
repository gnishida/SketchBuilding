#pragma once

#include <boost/shared_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include "Shape.h"

class RenderManager;

namespace cga {

class OffsetRectangle : public Shape {
private:
	std::vector<glm::vec2> _points;
	float _offsetDistance;
	glm::vec2 _center;

public:
	OffsetRectangle() {}
	OffsetRectangle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, float offsetDistance, const glm::vec3& color, const std::string& texture);
	boost::shared_ptr<Shape> clone(const std::string& name) const;
	void comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes);
	void generateGeometry(RenderManager* renderManager, float opacity) const;
};

}
