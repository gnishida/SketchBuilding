#include "OffsetPolygon.h"
#include "Polygon.h"
#include "GeneralObject.h"
#include "CGA.h"
#include "GLUtils.h"
#include "BoundingBox.h"

namespace cga {

OffsetPolygon::OffsetPolygon(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, const std::vector<glm::vec2>& points, float offsetDistance, const glm::vec3& color, const std::string& texture) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_points = points;
	this->_offsetDistance = offsetDistance;
	this->_color = color;
	this->_texture = texture;

	BoundingBox bbox(points);
	this->_scope = glm::vec3(bbox.maxPt.x, bbox.maxPt.y, 0);

	this->_center = glm::vec2(0, 0);
	for (int i = 0; i < points.size(); ++i) {
		_center += points[i];
	}
	_center /= points.size();
}

boost::shared_ptr<Shape> OffsetPolygon::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new OffsetPolygon(*this));
	copy->_name = name;
	return copy;
}

void OffsetPolygon::comp(const std::map<std::string, std::string>& name_map, std::vector<boost::shared_ptr<Shape> >& shapes) {
	std::vector<glm::vec2> offset_points;
	glutils::offsetPolygon(_points, _offsetDistance, offset_points);

	// inside face
	if (name_map.find("inside") != name_map.end() && name_map.at("inside") != "NIL") {
		std::vector<glm::vec2> pts = offset_points;
		glm::vec2 t = pts[0] - _points[0];
		for (int i = 0; i < pts.size(); ++i) {
			pts[i] -= t;
		}

		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(t, 0));
		shapes.push_back(boost::shared_ptr<Shape>(new Polygon(name_map.at("inside"), _pivot, mat, pts, _color, _texture)));
	}

	// border face
	if (name_map.find("border") != name_map.end() && name_map.at("border") != "NIL") {
		std::vector<glm::vec3> pts;
		std::vector<glm::vec3> normals;
		for (int i = 0; i < _points.size(); ++i) {
			pts.push_back(glm::vec3(offset_points[i], 0));
			pts.push_back(glm::vec3(_points[i], 0));
			pts.push_back(glm::vec3(_points[(i+1) % _points.size()], 0));

			pts.push_back(glm::vec3(offset_points[i], 0));
			pts.push_back(glm::vec3(_points[(i+1) % _points.size()], 0));
			pts.push_back(glm::vec3(offset_points[(i+1) % offset_points.size()], 0));

			normals.push_back(glm::vec3(0, 0, 1));
			normals.push_back(glm::vec3(0, 0, 1));
			normals.push_back(glm::vec3(0, 0, 1));
			normals.push_back(glm::vec3(0, 0, 1));
			normals.push_back(glm::vec3(0, 0, 1));
			normals.push_back(glm::vec3(0, 0, 1));
		}
		
		shapes.push_back(boost::shared_ptr<Shape>(new GeneralObject(name_map.at("border"), _pivot, _modelMat, pts, normals, _color)));
	}
}

void OffsetPolygon::generateGeometry(RenderManager* renderManager, float opacity) const {
	if (_removed) return;

	std::vector<glm::vec2> offset_points;
	glutils::offsetPolygon(_points, _offsetDistance, offset_points);

	std::vector<Vertex> vertices;
	glutils::drawConcavePolygon(offset_points, glm::vec4(_color, opacity), _pivot * _modelMat, vertices);

	for (int i = 0; i < _points.size(); ++i) {
		std::vector<glm::vec2> pts(4);
		pts[0] = offset_points[i];
		pts[1] = _points[i];
		pts[2] = _points[(i+1) % _points.size()];
		pts[3] = offset_points[(i+1) % offset_points.size()];
		glutils::drawPolygon(pts, glm::vec4(_color, opacity), _pivot * _modelMat, vertices);
	}

	renderManager->addObject(_name.c_str(), _texture.c_str(), vertices);
}

}
