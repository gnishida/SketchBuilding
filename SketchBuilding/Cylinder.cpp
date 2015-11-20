#include "Cylinder.h"
#include "CGA.h"
#include "Circle.h"
#include "Rectangle.h"
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

void Cylinder::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	// top face
	if (name_map.find("top") != name_map.end() && name_map.at("top") != "NIL") {
		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(0, 0, _scope.z));
		shapes.push_back(boost::shared_ptr<Shape>(new Circle(name_map.at("top"), _pivot, mat, _scope.x, _scope.y, _color)));
	}

	// bottom face
	if (name_map.find("bottom") != name_map.end() && name_map.at("bottom") != "NIL") {
		glm::mat4 mat = glm::rotate(glm::translate(_modelMat, glm::vec3(0, _scope.y, 0)), M_PI, glm::vec3(1, 0, 0));
		shapes.push_back(boost::shared_ptr<Shape>(new Circle(name_map.at("bottom"), _pivot, mat, _scope.x, _scope.y, _color)));
	}

	// side face
	if (name_map.find("side") != name_map.end() && name_map.at("side") != "NIL") {
		int slices = 24;

		for (int i = 0; i < slices; ++i) {
			float theta1 = (float)i / slices * M_PI * 2.0f;
			float theta2 = (float)(i + 1) / slices * M_PI * 2.0f;

			glm::vec3 p0(cosf(theta1) * _scope.x * 0.5f + _scope.x * 0.5, sinf(theta1) * _scope.y * 0.5f + _scope.y * 0.5, 0.0f);
			glm::vec3 p1(cosf(theta2) * _scope.x * 0.5f + _scope.x * 0.5, sinf(theta2) * _scope.y * 0.5f + _scope.y * 0.5, 0.0f);
			//glm::vec3 p2(cosf(theta2) * _scope.x * 0.5f, sinf(theta2) * _scope.y * 0.5f, _scope.z);
			//glm::vec3 p3(cosf(theta1) * _scope.x * 0.5f, sinf(theta1) * _scope.y * 0.5f, _scope.z);

			// set the conversion matrix
			float rot_z = atan2f(p1.y - p0.y, p1.x - p0.x);
			glm::mat4 mat = glm::rotate(glm::rotate(glm::translate(_modelMat, p0), rot_z, glm::vec3(0, 0, 1)), M_PI * 0.5f, glm::vec3(1, 0, 0));
			//glm::mat4 invMat = glm::inverse(convMat);
			//glm::mat4 mat = _modelMat * convMat;

			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(name_map.at("side"), _pivot, mat, glm::length(p0 - p1), _scope.z, _color)));
		}
	}
}

void Cylinder::generateGeometry(std::vector<boost::shared_ptr<glutils::Face> >& faces, float opacity) const {
	if (_removed) return;

	// top
	{
		std::vector<Vertex> vertices;
		glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5, _scope.y * 0.5, _scope.z));
		glutils::drawCircle(_scope.x * 0.5f, _scope.y * 0.5f, glm::vec4(_color, opacity), mat, vertices, 24);
		faces.push_back(boost::shared_ptr<glutils::Face>(new glutils::Face(_name, vertices)));
	}

	// base
	if (_scope.z >= 0) {
		std::vector<Vertex> vertices;
		glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5, _scope.y * 0.5, 0));
		glutils::drawCircle(_scope.x * 0.5f, _scope.y * 0.5f, glm::vec4(_color, opacity), mat, vertices, 24);
		faces.push_back(boost::shared_ptr<glutils::Face>(new glutils::Face(_name, vertices)));
	}

	// side
	{
		std::vector<Vertex> vertices;
		glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5, _scope.y * 0.5, 0));
		glutils::drawCylinderZ(_scope.x * 0.5f, _scope.y * 0.5f, _scope.x * 0.5f, _scope.y * 0.5f, _scope.z, glm::vec4(_color, opacity), mat, vertices, 24);
		faces.push_back(boost::shared_ptr<glutils::Face>(new glutils::Face(_name, vertices)));
	}
}

}