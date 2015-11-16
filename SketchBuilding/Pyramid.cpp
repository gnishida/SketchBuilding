#include "Pyramid.h"
#include "Rectangle.h"
#include "CGA.h"
#include "GLUtils.h"
#include "Polygon.h"
#include "BoundingBox.h"

namespace cga {

Pyramid::Pyramid(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, const glm::vec2& center, float height, float top_ratio, const glm::vec3& color, const std::string& texture) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_points = points;
	this->_center = center;
	this->_height = height;
	this->_top_ratio = top_ratio;
	this->_color = color;
	this->_texture = texture;

	BoundingBox bbox(points);
	this->_scope = glm::vec3(bbox.maxPt.x, bbox.maxPt.y, height);
}

boost::shared_ptr<Shape> Pyramid::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new Pyramid(*this));
	copy->_name = name;
	return copy;
}

void Pyramid::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	std::vector<glm::vec2> top_points(_points.size());
	for (int i = 0; i < _points.size(); ++i) {
		top_points[i] = (_points[i] - _center) * _top_ratio + _center;
	}

	// front face (To be fixed)
	if (name_map.find("front") != name_map.end() && name_map.at("front") != "NIL") {
		std::vector<glm::vec2> points(3 + (_top_ratio > 0.0f ? 1 : 0));

		float dist = glutils::distance(glm::vec3(_points[0], 0), glm::vec3(_points[1], 0), glm::vec3(top_points[1], _height));
		float angle = asin(_height / dist);
		glm::mat4 mat = glm::rotate(glm::mat4(), -angle, glm::vec3(1, 0, 0));

		points[0] = glm::vec2(mat * glm::vec4(_points[0], 0, 1));
		points[1] = glm::vec2(mat * glm::vec4(_points[1], 0, 1));
		points[2] = glm::vec2(mat * glm::vec4(top_points[1], _height, 1));
		if (points.size() > 3) {
			points[3] = glm::vec2(mat * glm::vec4(top_points[0], _height, 1));
		}

		mat = glm::rotate(_modelMat, angle, glm::vec3(1, 0, 0));
		shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("front"), _pivot, mat, points, _color, _texture)));
	}

	// side faces (To be fixed);
	if (name_map.find("side") != name_map.end() && name_map.at("side") != "NIL") {
		glm::mat4 mat;
		for (int i = 1; i < _points.size(); ++i) {
			glm::vec2 a = _points[i] - _points[i - 1];
			glm::vec2 b = _points[(i + 1) % _points.size()] - _points[i];
			mat = glm::translate(mat, glm::vec3(glm::length(a), 0, 0));
			float theta = acos(glm::dot(a, b) / glm::length(a) / glm::length(b));
			if (a.x * b.y - a.y * b.x < 0) {
				theta = -theta;
			}
			mat = glm::rotate(mat, theta, glm::vec3(0, 0, 1));

			std::vector<glm::vec3> points3d(3 + (_top_ratio > 0.0f ? 1 : 0));
			points3d[0] = glm::vec3(_points[i], 0);
			points3d[1] = glm::vec3(_points[(i+1) % _points.size()], 0);
			points3d[2] = glm::vec3(top_points[(i+1) % _points.size()], _height);
			if (points3d.size() > 3) {
				points3d[3] = glm::vec3(top_points[i], _height);
			}

			glm::mat4 invMat = glm::inverse(mat);
			for (int k = 0; k < points3d.size(); ++k) {
				points3d[k] = glm::vec3(invMat * glm::vec4(points3d[k], 1));
			}

			float dist = glutils::distance(points3d[0], points3d[1], points3d[2]);
			float angle = acos(points3d[2].y / dist);
			glm::mat4 matRot = glm::rotate(glm::mat4(), -angle, glm::vec3(1, 0, 0));

			std::vector<glm::vec2> points(points3d.size());
			for (int k = 0; k < points.size(); ++k) {
				points[k] = glm::vec2(matRot * glm::vec4(points3d[k], 1));
			}

			glm::mat4 mat2 = glm::rotate(mat, angle, glm::vec3(1, 0, 0));
			shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("side"), _pivot, _modelMat * mat2, points, _color, _texture)));
		}
	}

	// top face
	if (_top_ratio > 0.0f && name_map.find("top") != name_map.end() && name_map.at("top") != "NIL") {
		std::vector<glm::vec2> points = top_points;
		glm::vec2 offset = points[0];
		for (int i = 0; i < points.size(); ++i) {
			points[i] -= offset;
		}
		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(offset, _height));

		shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("top"), _pivot, mat, points, _color, _texture)));
	}

	// bottom face
	if (name_map.find("bottom") != name_map.end() && name_map.at("bottom") != "NIL") {
		//std::vector<glm::vec2> basePoints = _points;
		//std::reverse(basePoints.begin(), basePoints.end());
		shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("bottom"), _pivot, _modelMat, _points, _color, _texture)));
	}
}

void Pyramid::generateGeometry(RenderManager* renderManager, float opacity) const {
	if (_removed) return;

	if (_top_ratio == 0.0f) {
		std::vector<Vertex> vertices(_points.size() * 3);

		glm::vec4 p0(_center, _height, 1);
		p0 = _pivot * _modelMat * p0;

		glm::vec4 p1(_points.back(), 0, 1);
		p1 = _pivot * _modelMat * p1;

		for (int i = 0; i < _points.size(); ++i) {
			glm::vec4 p2(_points[i], 0, 1);
			p2 = _pivot * _modelMat * p2;

			glm::vec3 normal = glm::cross(glm::vec3(p1 - p0), glm::vec3(p2 - p0));

			vertices[i * 3] = Vertex(glm::vec3(p0), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));
			vertices[i * 3 + 1] = Vertex(glm::vec3(p1), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));
			vertices[i * 3 + 2] = Vertex(glm::vec3(p2), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));

			p1 = p2;
		}

		renderManager->addObject(_name.c_str(), _texture.c_str(), vertices);
	} else {
		std::vector<Vertex> vertices(_points.size() * 6);

		glm::vec4 p0(_points.back(), 0, 1);
		p0 = _pivot * _modelMat * p0;

		glm::vec4 p1(_points.back() * _top_ratio + _center * (1.0f - _top_ratio), _height, 1);
		p1 = _pivot * _modelMat * p1;

		std::vector<glm::vec3> pts3(_points.size());
		for (int i = 0; i < _points.size(); ++i) {
			glm::vec4 p2(_points[i], 0, 1);
			p2 = _pivot * _modelMat * p2;

			glm::vec4 p3(_points[i] * _top_ratio + _center * (1.0f - _top_ratio), _height, 1);
			pts3[i] = glm::vec3(p3);
			p3 = _pivot * _modelMat * p3;

			glm::vec3 normal = glm::cross(glm::vec3(p2 - p0), glm::vec3(p3 - p0));

			vertices[i * 6 + 0] = Vertex(glm::vec3(p0), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));
			vertices[i * 6 + 1] = Vertex(glm::vec3(p2), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));
			vertices[i * 6 + 2] = Vertex(glm::vec3(p3), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));

			vertices[i * 6 + 3] = Vertex(glm::vec3(p0), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));
			vertices[i * 6 + 4] = Vertex(glm::vec3(p3), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));
			vertices[i * 6 + 5] = Vertex(glm::vec3(p1), normal, glm::vec4(_color, opacity), glm::vec2(0, 0));

			p0 = p2;
			p1 = p3;
		}

		glutils::drawPolygon(pts3, glm::vec4(_color, opacity), _pivot * _modelMat, vertices);

		renderManager->addObject(_name.c_str(), _texture.c_str(), vertices);
	}
}


}
