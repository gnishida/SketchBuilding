#include "OffsetSemiCircle.h"
#include "CGA.h"
#include "SemiCircle.h"
#include "Polygon.h"

namespace cga {

OffsetSemiCircle::OffsetSemiCircle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, float offsetDistance, const glm::vec3& color) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_scope = glm::vec3(width, height, 0);
	this->_offsetDistance = offsetDistance;
	this->_color = color;
}

boost::shared_ptr<Shape> OffsetSemiCircle::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new OffsetSemiCircle(*this));
	copy->_name = name;
	return copy;
}

void OffsetSemiCircle::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	// inside face
	if (name_map.find("inside") != name_map.end() && name_map.at("inside") != "NIL") {
		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(-_offsetDistance, 0, 0));
		shapes.push_back(boost::shared_ptr<Shape>(new SemiCircle(name_map.at("inside"), _pivot, mat, _scope.x + _offsetDistance * 2.0f, _scope.y + _offsetDistance, _color)));
	}

	// border face
	if (name_map.find("border") != name_map.end() && name_map.at("border") != "NIL") {
		std::vector<glm::vec2> points;
		int numSlices = 12;

		float rx = _scope.x * 0.5f;
		float ry = _scope.y;
		for (int i = 0; i <= numSlices; ++i) {
			float theta = (float)i / numSlices * M_PI;

			points.push_back(glm::vec2(_scope.x * 0.5f + rx * cosf(theta), ry * sinf(theta)));
		}

		rx = _scope.x * 0.5f + _offsetDistance;
		ry = _scope.y + _offsetDistance;
		for (int i = numSlices; i >= 0; --i) {
			float theta = (float)i / numSlices * M_PI;

			points.push_back(glm::vec2(_scope.x * 0.5f + rx * cosf(theta), ry * sinf(theta)));
		}
		
		shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("border"), _pivot, _modelMat, points, _color, _texture)));
	}
}

}
