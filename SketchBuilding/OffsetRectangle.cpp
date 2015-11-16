#include "OffsetRectangle.h"
#include "Rectangle.h"
#include "GeneralObject.h"
#include "CGA.h"
#include "GLUtils.h"
#include "BoundingBox.h"

namespace cga {

OffsetRectangle::OffsetRectangle(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, float offsetDistance, const glm::vec3& color, const std::string& texture) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_scope.x = width; 
	this->_scope.y = height;
	this->_offsetDistance = offsetDistance;
	this->_color = color;
	this->_texture = texture;
}

boost::shared_ptr<Shape> OffsetRectangle::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new OffsetRectangle(*this));
	copy->_name = name;
	return copy;
}

void OffsetRectangle::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	// inside face
	if (name_map.find("inside") != name_map.end() && name_map.at("inside") != "NIL") {
		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(-_offsetDistance, -_offsetDistance, 0));
		shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(name_map.at("inside"), _pivot, mat, _scope.x + _offsetDistance * 2.0f, _scope.y + _offsetDistance * 2.0f, _color)));
	}

	// border face
	if (name_map.find("border") != name_map.end() && name_map.at("border") != "NIL") {
		std::vector<std::vector<glm::vec3> > points;
		std::vector<std::vector<glm::vec3> > normals;

		std::vector<glm::vec3> pts(4);
		std::vector<glm::vec3> pts2(4);
		pts[0] = glm::vec3(0, 0, 0);
		pts[1] = glm::vec3(_scope.x, 0, 0);
		pts[2] = glm::vec3(_scope.x, _scope.y, 0);
		pts[3] = glm::vec3(0, _scope.y, 0);

		pts2[0] = glm::vec3(-_offsetDistance, -_offsetDistance, 0);
		pts2[1] = glm::vec3(_scope.x + _offsetDistance, - _offsetDistance, 0);
		pts2[2] = glm::vec3(_scope.x + _offsetDistance, _scope.y + _offsetDistance, 0);
		pts2[3] = glm::vec3(-_offsetDistance, _scope.y + _offsetDistance, 0);

		glm::vec3 normal(0, 0, 1);

		for (int i = 0; i < 4; ++i) {
			int next = (i + 1) % 4;

			std::vector<glm::vec3> ps;
			ps.push_back(pts2[i]);
			ps.push_back(pts[i]);
			ps.push_back(pts[next]);
			ps.push_back(pts2[next]);
			points.push_back(ps);

			std::vector<glm::vec3> ns;
			ns.push_back(glm::vec3(0, 0, 1));
			ns.push_back(glm::vec3(0, 0, 1));
			ns.push_back(glm::vec3(0, 0, 1));
			ns.push_back(glm::vec3(0, 0, 1));
			normals.push_back(ns);
		}
		
		shapes.push_back(boost::shared_ptr<Shape>(new GeneralObject(name_map.at("border"), _pivot, _modelMat, points, normals, _color)));
	}
}

void OffsetRectangle::generateGeometry(RenderManager* renderManager, float opacity) const {
	if (_removed) return;

	std::vector<Vertex> vertices;
	glm::mat4 mat = glm::translate(_pivot * _modelMat, glm::vec3(_scope.x * 0.5f, _scope.y * 0.5f, 0.0f));
	glutils::drawQuad(_scope.x - _offsetDistance * 2.0f, _scope.y  - _offsetDistance * 2.0f, glm::vec4(_color, opacity), mat, vertices);

	std::vector<glm::vec3> pts(4);
	std::vector<glm::vec3> pts2(4);
	pts[0] = glm::vec3(0, 0, 0);
	pts[1] = glm::vec3(_scope.x, 0, 0);
	pts[2] = glm::vec3(_scope.x, _scope.y, 0);
	pts[3] = glm::vec3(0, _scope.y, 0);
	pts2[0] = glm::vec3(_offsetDistance, _offsetDistance, 0);
	pts2[1] = glm::vec3(_scope.x - _offsetDistance, _offsetDistance, 0);
	pts2[2] = glm::vec3(_scope.x - _offsetDistance, _scope.y - _offsetDistance, 0);
	pts2[3] = glm::vec3(_offsetDistance, _scope.y - _offsetDistance, 0);

	for (int i = 0; i < 4; ++i) {
		pts[i] = glm::vec3(_pivot * _modelMat * glm::vec4(pts[i], 1));
		pts2[i] = glm::vec3(_pivot * _modelMat * glm::vec4(pts2[i], 1));
	}

	glm::vec3 normal(0, 0, 1);
	normal = glm::vec3(_pivot * _modelMat * glm::vec4(normal, 1));

	for (int i = 0; i < pts.size(); ++i) {
		int next = (i + 1) % pts.size();

		vertices.push_back(Vertex(pts2[i], normal, glm::vec4(_color, opacity)));
		vertices.push_back(Vertex(pts[i], normal, glm::vec4(_color, opacity), 1));
		vertices.push_back(Vertex(pts[next], normal, glm::vec4(_color, opacity)));

		vertices.push_back(Vertex(pts2[i], normal, glm::vec4(_color, opacity)));
		vertices.push_back(Vertex(pts[next], normal, glm::vec4(_color, opacity)));
		vertices.push_back(Vertex(pts2[next], normal, glm::vec4(_color, opacity), 1));
	}

	renderManager->addObject(_name.c_str(), _texture.c_str(), vertices);
}

}
