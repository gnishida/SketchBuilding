#pragma once

#include <boost/shared_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include "Shape.h"

class RenderManager;

namespace cga {

	class Circle : public Shape {
	public:
		Circle() {}
		Circle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, const glm::vec3& color);
		boost::shared_ptr<Shape> clone(const std::string& name) const;
		boost::shared_ptr<Shape> extrude(const std::string& name, float height);
		void generateGeometry(std::vector<glutils::Face>& faces, float opacity) const;
	};

}
