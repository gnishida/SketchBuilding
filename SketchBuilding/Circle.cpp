#include "Circle.h"
#include "CGA.h"
#include "Cylinder.h"
#include "Polygon.h"
#include "Pyramid.h"

namespace cga {

	Circle::Circle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, const glm::vec3& color) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_scope = glm::vec3(width, height, 0);
	this->_color = color;
	this->_textureEnabled = false;
}

boost::shared_ptr<Shape> Circle::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new Circle(*this));
	copy->_name = name;
	return copy;
}

boost::shared_ptr<Shape> Circle::extrude(const std::string& name, float height) {
	return boost::shared_ptr<Shape>(new Cylinder(name, _pivot, _modelMat, _scope.x, _scope.y, height, _color));
}

void Circle::offset(const std::string& name, float offsetDistance, const std::string& inside, const std::string& border, std::vector<boost::shared_ptr<Shape> >& shapes) {
	// inner shape
	if (!inside.empty()) {
		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(-offsetDistance, -offsetDistance, 0));

		shapes.push_back(boost::shared_ptr<Shape>(new Circle(inside, _pivot, mat, _scope.x + offsetDistance * 2, _scope.y + offsetDistance * 2, _color)));
	}

	// border shape
	if (!border.empty()) {
		for (int i = 0; i < CIRCLE_SLICES; ++i) {
			float theta1 = (float)i / CIRCLE_SLICES * M_PI * 2.0f;
			float theta2 = (float)(i + 1) / CIRCLE_SLICES * M_PI * 2.0f;

			std::vector<glm::vec2> pts;
			pts.push_back(glm::vec2(cosf(theta1) * _scope.x * 0.5 + _scope.x * 0.5, sinf(theta1) * _scope.y * 0.5 + _scope.y * 0.5));
			pts.push_back(glm::vec2(cosf(theta2) * _scope.x * 0.5 + _scope.x * 0.5, sinf(theta2) * _scope.y * 0.5 + _scope.y * 0.5));
			pts.push_back(glm::vec2(cosf(theta2) * (_scope.x * 0.5 + offsetDistance) + _scope.x * 0.5, sinf(theta2) * (_scope.y * 0.5 + offsetDistance) + _scope.y * 0.5));
			pts.push_back(glm::vec2(cosf(theta1) * (_scope.x * 0.5 + offsetDistance) + _scope.x * 0.5, sinf(theta1) * (_scope.y * 0.5 + offsetDistance) + _scope.y * 0.5));
			shapes.push_back(boost::shared_ptr<Shape>(new Polygon(border, _pivot, _modelMat, pts, _color, _texture)));
		}

	}
}

boost::shared_ptr<Shape> Circle::pyramid(const std::string& name, float height) {
	std::vector<glm::vec2> points;
	for (int i = 0; i < CIRCLE_SLICES; ++i) {
		float theta = (float)i / CIRCLE_SLICES * M_PI * 2.0f;
		points.push_back(glm::vec2(cosf(theta) * _scope.x * 0.5 + _scope.x * 0.5, sinf(theta) * _scope.y * 0.5 + _scope.y * 0.5));
	}
	return boost::shared_ptr<Shape>(new Pyramid(name, _pivot, _modelMat, points, glm::vec2(_scope.x * 0.5, _scope.y * 0.5), height, 0, _color, _texture));
}

void Circle::generateGeometry(std::vector<glutils::Face>& faces, float opacity) const {
	if (_removed) return;

	std::vector<Vertex> vertices;

	glm::mat4 mat = _pivot * glm::translate(_modelMat, glm::vec3(_scope.x * 0.5f, _scope.y * 0.5f, 0));
	glutils::drawCircle(_scope.x * 0.5f, _scope.y * 0.5f, glm::vec4(_color, opacity), mat, vertices, 24);

	/*
	glm::vec3 p0 = glm::vec3(_pivot * _modelMat * glm::vec4(_scope.x * 0.5, 0, 0, 1));

	glm::vec3 normal = glm::vec3(_pivot * _modelMat * glm::vec4(0, 0, 1, 0));

	int numSlices = 12;
	for (int i = 0; i < numSlices; ++i) {
		float theta1 = (float)i / numSlices * M_PI * 2.0f;
		float theta2 = (float)(i + 1) / numSlices * M_PI * 2.0f;

		glm::vec4 p1(_scope.x * 0.5 * cosf(theta1) + _scope.x * 0.5, _scope.y * sinf(theta1), 0.0f, 1.0f);
		p1 = _pivot * _modelMat * p1;
		glm::vec4 p2(_scope.x * 0.5 * cosf(theta2) + _scope.x * 0.5, _scope.y * sinf(theta2), 0.0f, 1.0f);
		p2 = _pivot * _modelMat * p2;

		vertices.push_back(Vertex(p0, normal, glm::vec4(_color, opacity)));
		if (i < numSlices) {
			vertices.push_back(Vertex(glm::vec3(p1), normal, glm::vec4(_color, opacity), 1));
		}
		else {
			vertices.push_back(Vertex(glm::vec3(p1), normal, glm::vec4(_color, opacity)));
		}
		if (i > 0) {
			vertices.push_back(Vertex(glm::vec3(p2), normal, glm::vec4(_color, opacity), 1));
		}
		else {
			vertices.push_back(Vertex(glm::vec3(p2), normal, glm::vec4(_color, opacity)));
		}
	}
	*/

	faces.push_back(glutils::Face(_name, vertices));
}

}
