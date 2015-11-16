#include "Cylinder.h"
#include "CGA.h"
#include "GLUtils.h"

namespace cga {

Cylinder::Cylinder(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float depth, float height, const glm::vec3& color) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_scope.x = width;
	this->_scope.y = depth;
	this->_scope.z = height;
	this->_color = color;
}

boost::shared_ptr<Shape> Cylinder::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new Cylinder(*this));
	copy->_name = name;
	return copy;
}

void Cylinder::generateGeometry(std::vector<glutils::Face>& faces, float opacity) const {
	if (_removed) return;

	// top
	{
		std::vector<Vertex> vertices;
		glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5, _scope.y * 0.5, _scope.z));
		glutils::drawCircle(_scope.x * 0.5f, _scope.y * 0.5f, glm::vec4(_color, opacity), mat, vertices, 24);
		faces.push_back(glutils::Face(_name, vertices));
	}

	// base
	if (_scope.z >= 0) {
		std::vector<Vertex> vertices;
		glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5, _scope.y * 0.5, 0));
		glutils::drawCircle(_scope.x * 0.5f, _scope.y * 0.5f, glm::vec4(_color, opacity), mat, vertices, 24);
		faces.push_back(glutils::Face(_name, vertices));
	}

	// side
	{
		std::vector<Vertex> vertices;
		glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5, _scope.y * 0.5, 0));
		glutils::drawCylinderZ(_scope.x * 0.5f, _scope.y * 0.5f, _scope.x * 0.5f, _scope.y * 0.5f, _scope.z, glm::vec4(_color, opacity), mat, vertices, 24);
		faces.push_back(glutils::Face(_name, vertices));
	}
}

}