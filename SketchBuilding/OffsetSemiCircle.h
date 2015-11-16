#pragma once

#include <boost/shared_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include "Shape.h"

class RenderManager;

namespace cga {

class OffsetSemiCircle : public Shape {
public:
	float _offsetDistance;

public:
	OffsetSemiCircle() {}
	OffsetSemiCircle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, float offsetDistance, const glm::vec3& color);
	boost::shared_ptr<Shape> clone(const std::string& name) const;
	void comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes);
};

}