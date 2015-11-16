#include "UShape.h"
#include "GLUtils.h"
#include "Circle.h"
#include "Pyramid.h"
#include "HipRoof.h"
#include "GableRoof.h"
#include "UShapePrism.h"
#include "Polygon.h"
#include "Cuboid.h"
#include "SemiCircle.h"
#include "Rectangle.h"
#include "CGA.h"

namespace cga {

UShape::UShape(const std::string& name, const glm::mat4& pivot, const glm::mat4& modelMat, float width, float height, float front_width, float back_height, const glm::vec3& color) {
	this->_name = name;
	this->_removed = false;
	this->_pivot = pivot;
	this->_modelMat = modelMat;
	this->_scope = glm::vec3(width, height, 0);
	this->_front_width = front_width;
	this->_back_height = back_height;
	this->_color = color;
	this->_textureEnabled = false;
}

boost::shared_ptr<Shape> UShape::clone(const std::string& name) const {
	boost::shared_ptr<Shape> copy = boost::shared_ptr<Shape>(new UShape(*this));
	copy->_name = name;
	return copy;
}

boost::shared_ptr<Shape> UShape::extrude(const std::string& name, float height) {
	return boost::shared_ptr<Shape>(new UShapePrism(name, _pivot, _modelMat, _scope.x, _scope.y, height, _front_width, _back_height, _color));
}

void UShape::offset(const std::string& name, float offsetDistance, const std::string& inside, const std::string& border, std::vector<boost::shared_ptr<Shape> >& shapes) {
	// inner shape
	if (!inside.empty()) {
		float offset_width = _scope.x + offsetDistance * 2.0f;
		float offset_height = _scope.y + offsetDistance * 2.0f;
		glm::mat4 mat = glm::translate(_modelMat, glm::vec3(-offsetDistance, -offsetDistance, 0));
		/*if (_textureEnabled) {
			float offset_u1 = _texCoords[0].x;
			float offset_v1 = _texCoords[0].y;
			float offset_u2 = _texCoords[2].x;
			float offset_v2 = _texCoords[2].y;
			if (offsetDistance < 0) {
				float offset_u1 = (_texCoords[2].x - _texCoords[0].x) * (-offsetDistance) / _scope.x + _texCoords[0].x;
				float offset_v1 = (_texCoords[2].y - _texCoords[0].y) * (-offsetDistance) / _scope.y + _texCoords[0].y;
				float offset_u2 = (_texCoords[2].x - _texCoords[0].x) * (_scope.x + offsetDistance) / _scope.x + _texCoords[0].x;
				float offset_v2 = (_texCoords[2].y - _texCoords[0].y) * (_scope.y + offsetDistance) / _scope.y + _texCoords[0].y;
			}
			shapes.push_back(boost::shared_ptr<Shape>(new UShape(inside, _pivot, mat, offset_width, offset_height, _color, _texture, offset_u1, offset_v1, offset_u2, offset_v2)));
		}
		else {*/
		shapes.push_back(boost::shared_ptr<Shape>(new UShape(inside, _pivot, mat, offset_width, offset_height, _front_width + offsetDistance * 2.0f, _back_height + offsetDistance * 2.0f, _color)));
		//}
	}

	// border shape
	if (!border.empty()) {
		if (offsetDistance < 0) {
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, _modelMat, _front_width, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::rotate(glm::translate(_modelMat, glm::vec3(_front_width, -offsetDistance, 0)), M_PI * 0.5f, glm::vec3(0, 0, 1)), _scope.y - _back_height + offsetDistance, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::translate(_modelMat, glm::vec3(_front_width + offsetDistance, _scope.y - _back_height, 0)), _scope.x - _front_width * 2 - offsetDistance * 2, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::rotate(glm::translate(_modelMat, glm::vec3(_scope.x - _front_width, _scope.y - _back_height, 0)), -M_PI * 0.5f, glm::vec3(0, 0, 1)), _scope.y - _back_height + offsetDistance, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::translate(_modelMat, glm::vec3(_scope.x - _front_width, 0, 0)), _front_width, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::rotate(glm::translate(_modelMat, glm::vec3(_scope.x, -offsetDistance, 0)), M_PI * 0.5f, glm::vec3(0, 0, 1)), _scope.y + offsetDistance * 2, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::rotate(glm::translate(_modelMat, glm::vec3(_scope.x, _scope.y, 0)), M_PI, glm::vec3(0, 0, 1)), _scope.x, -offsetDistance, _color)));
			shapes.push_back(boost::shared_ptr<Shape>(new Rectangle(border, _pivot, glm::rotate(glm::translate(_modelMat, glm::vec3(0, _scope.y + offsetDistance, 0)), -M_PI * 0.5f, glm::vec3(0, 0, 1)), _scope.y + offsetDistance * 2, -offsetDistance, _color)));
		}
		else {
			// not supported
		}
	}
}

void UShape::setupProjection(int axesSelector, float texWidth, float texHeight) {
	_texCoords.resize(8);
	_texCoords[0] = glm::vec2(0, 0);
	_texCoords[1] = glm::vec2(_front_width / texWidth, 0);
	_texCoords[2] = glm::vec2(_front_width / texWidth, (_scope.y - _back_height) / texHeight);
	_texCoords[3] = glm::vec2((_scope.x - _front_width) / texWidth, (_scope.y - _back_height) / texHeight);
	_texCoords[4] = glm::vec2((_scope.x - _front_width) / texWidth, 0);
	_texCoords[5] = glm::vec2(_scope.x / texWidth, 0);
	_texCoords[6] = glm::vec2(_scope.x / texWidth, _scope.y / texHeight);
	_texCoords[7] = glm::vec2(0, _scope.y / texHeight);
}

void UShape::size(float xSize, float ySize, float zSize, bool centered) {
	_prev_scope = _scope;

	if (centered) {
		_modelMat = glm::translate(_modelMat, glm::vec3((_scope.x - xSize) * 0.5, (_scope.y - ySize) * 0.5, (_scope.z - zSize) * 0.5));
	}

	_front_width *= xSize / _scope.x;
	_back_height *= ySize / _scope.y;

	_scope.x = xSize;
	_scope.y = ySize;
	_scope.z = zSize;
}

void UShape::generateGeometry(RenderManager* renderManager, float opacity) const {
	if (_removed) return;

	std::vector<Vertex> vertices;

	if (_textureEnabled) {
		glutils::drawQuad(_front_width, _scope.y - _back_height, _texCoords[0], _texCoords[1], _texCoords[2], (_texCoords[0] + _texCoords[7]) * 0.5f, glm::translate(_pivot * _modelMat, glm::vec3(_front_width * 0.5, (_scope.y - _back_height) * 0.5, 0)), vertices);
		glutils::drawQuad(_scope.x, _back_height, (_texCoords[0] + _texCoords[7]) * 0.5f, (_texCoords[5] + _texCoords[6]) * 0.5f, _texCoords[6], _texCoords[7], glm::translate(_pivot * _modelMat, glm::vec3(_scope.x * 0.5, _scope.y - _back_height * 0.5, 0)), vertices);
		glutils::drawQuad(_front_width, _scope.y - _back_height, _texCoords[4], _texCoords[5], (_texCoords[5] + _texCoords[6]) * 0.5f, _texCoords[3], glm::translate(_pivot * _modelMat, glm::vec3(_scope.x - _front_width * 0.5, (_scope.y - _back_height) * 0.5, 0)), vertices);
		renderManager->addObject(_name.c_str(), _texture.c_str(), vertices);
	}
	else {
		glutils::drawQuad(_front_width, _scope.y - _back_height, glm::vec4(_color, opacity), glm::translate(_pivot * _modelMat, glm::vec3(_front_width * 0.5, (_scope.y - _back_height) * 0.5, 0)), vertices);
		glutils::drawQuad(_scope.x, _back_height, glm::vec4(_color, opacity), glm::translate(_pivot * _modelMat, glm::vec3(_scope.x * 0.5, _scope.y - _back_height * 0.5, 0)), vertices);
		glutils::drawQuad(_front_width, _scope.y - _back_height, glm::vec4(_color, opacity), glm::translate(_pivot * _modelMat, glm::vec3(_scope.x - _front_width * 0.5, (_scope.y - _back_height) * 0.5, 0)), vertices);
		renderManager->addObject(_name.c_str(), "", vertices);
	}


	/*
	std::vector<glm::vec2> pts(8);
	pts[0] = glm::vec2(0, 0);
	pts[1] = glm::vec2(_front_width, 0);
	pts[2] = glm::vec2(_front_width, _scope.y - _back_height);
	pts[3] = glm::vec2(_scope.x - _front_width, _scope.y - _back_height);
	pts[4] = glm::vec2(_scope.x - _front_width, 0);
	pts[5] = glm::vec2(_scope.x, 0);
	pts[6] = glm::vec2(_scope.x, _scope.y);
	pts[7] = glm::vec2(0, _scope.y);

	glutils::drawConcavePolygon(pts, glm::vec4(_color, opacity), _pivot * _modelMat, vertices);

	renderManager->addObject(_name.c_str(), "", vertices);
	*/
}

}