#include "BuildingSelector.h"
#include "Scene.h"
#include "RenderManager.h"
#include "GLUtils.h"
#include <iostream>

namespace sc {

BuildingSelector::BuildingSelector(sc::Scene* scene) {
	_scene = scene;
	_selectedBuilding = -1;
	_selectedBuildingControlPoint = -1;
}

bool BuildingSelector::isBuildingSelected() {
	if (_selectedBuilding >= 0) return true;
	else return false;
}

bool BuildingSelector::isBuildingControlPointSelected() {
	if (_selectedBuilding >= 0 && _selectedBuildingControlPoint >= 0) {
		return true;
	}
	else {
		return false;
	}
}

bool BuildingSelector::selectBuilding(const glm::vec3& cameraPos, const glm::vec3& viewDir) {
	glm::vec3 intPt;
	float min_dist = (std::numeric_limits<float>::max)();

	_selectedBuilding = -1;
	_selectedBuildingControlPoint = -1;

	for (int i = 0; i < _scene->_objects.size(); ++i) {
		for (int j = 0; j < _scene->_objects[i].faces.size(); ++j) {
			if (_scene->_objects[i].faces[j]->vertices.size() < 3) continue;

			if (_scene->_objects[i].faces[j]->grammar_type != "building") continue;

			for (int k = 0; k < _scene->_objects[i].faces[j]->vertices.size(); k += 3) {
				if (glutils::rayTriangleIntersection(cameraPos, viewDir, _scene->_objects[i].faces[j]->vertices[k].position, _scene->_objects[i].faces[j]->vertices[k + 1].position, _scene->_objects[i].faces[j]->vertices[k + 2].position, intPt)) {
					float dist = glm::length(intPt - cameraPos);

					if (dist < min_dist) {
						min_dist = dist;
						_selectedBuilding = i;
					}
				}

			}
		}
	}

	if (_selectedBuilding >= 0) {
		return true;
	}
	else {
		return false;
	}
}

bool BuildingSelector::selectBuildingControlPoint(const glm::vec3& cameraPos, const glm::vec3& viewDir, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height) {
	this->_mouseStartPt = mousePt;

	_selectedBuildingControlPoint = -1;

	if (_selectedBuilding == -1) return false;

	float min_dist = (std::numeric_limits<float>::max)();

	const float angle_threshold = 0.99993f;

	{
		float x = _scene->_objects[_selectedBuilding].offset_x;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(-1, 0, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > angle_threshold) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 1;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x - 1, y, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(1, 0, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > angle_threshold) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 2;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x + 1, y, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(0, -1, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > angle_threshold) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 3;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y - 1, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(0, 1, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > angle_threshold) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 4;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y + 1, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height;
		glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(0, 0, 1, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > angle_threshold) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 5;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y, z + 1, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	if (_selectedBuildingControlPoint >= 1) {
		return true;
	}
	else {
		// select the entire building
		if (selectBuilding(cameraPos, viewDir)) {
			_selectedBuildingControlPoint = 0;

			float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
			float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
			float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
			glm::vec4 screen_p1 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y, z, 1);
			glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x + 1, y, z, 1);
			glm::vec4 screen_p3 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y + 1, z, 1);

			_xDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			_yDir = glm::vec2((screen_p3.x / screen_p3.w + 1.0) * 0.5f * screen_width, (1 - screen_p3.y / screen_p3.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);

			return true;
		}
		else {
			return false;
		}
	}
}

void BuildingSelector::unselectBuilding() {
	_selectedBuilding = -1;
	_selectedBuildingControlPoint = -1;
}

void BuildingSelector::unselectBuildingControlPoint() {
	_selectedBuildingControlPoint = -1;
}

void BuildingSelector::copy() {
	if (_selectedBuilding == -1) return;

	_scene->newObject();
	_scene->currentObject().setFootprint(_scene->_objects[_selectedBuilding].offset_x + 14, _scene->_objects[_selectedBuilding].offset_y, _scene->_objects[_selectedBuilding].offset_z, _scene->_objects[_selectedBuilding].object_width, _scene->_objects[_selectedBuilding].object_depth);
	_scene->currentObject().setHeight(_scene->_objects[_selectedBuilding].height);

	// copy the grammar from the selected building mass to the new one
	for (auto it = _scene->_objects[_selectedBuilding].grammars.begin(); it != _scene->_objects[_selectedBuilding].grammars.end(); ++it) {
		_scene->currentObject().setGrammar(it->first, it->second);
	}
}

void BuildingSelector::resize(const glm::vec2& mousePt, bool conflictAllowed) {
	if (!isBuildingControlPointSelected()) return;

	if (_selectedBuildingControlPoint == 0) {
		float den = _xDir.x * _yDir.y - _xDir.y * _yDir.x;
		if (den != 0.0f) {
			_scene->_objects[_selectedBuilding].offset_x += ((mousePt - _mouseStartPt).x * _yDir.y - (mousePt - _mouseStartPt).y * _yDir.x) / den;
			_scene->_objects[_selectedBuilding].offset_y += ((mousePt - _mouseStartPt).y * _xDir.x - (mousePt - _mouseStartPt).x * _xDir.y) / den;

			if (!conflictAllowed) {
				// check the conflict. 
				// If exists, this building will be elevated such that the bottom face will be the same height of the other in order to avoid the conflict.
				avoidBuildingConflict(_selectedBuilding);
			}
		}
	}
	else if (_selectedBuildingControlPoint == 1) {
		float diff = glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		_scene->_objects[_selectedBuilding].offset_x -= diff;
		_scene->_objects[_selectedBuilding].object_width += diff;
	}
	else if (_selectedBuildingControlPoint == 2) {
		_scene->_objects[_selectedBuilding].object_width += glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
	}
	else if (_selectedBuildingControlPoint == 3) {
		float diff = glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		_scene->_objects[_selectedBuilding].offset_y -= diff;
		_scene->_objects[_selectedBuilding].object_depth += diff;
	}
	else if (_selectedBuildingControlPoint == 4) {
		_scene->_objects[_selectedBuilding].object_depth += glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
	}
	else if (_selectedBuildingControlPoint == 5) {
		float height = _scene->_objects[_selectedBuilding].height + glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		
		_scene->_objects[_selectedBuilding].setHeight(height);
	}

	_mouseStartPt = mousePt;
}

void BuildingSelector::avoidBuildingConflict(int currentBuilding) {
	bool conflicted = false;

	for (int i = 0; i < _scene->_objects.size(); ++i) {
		if (i == currentBuilding) continue;

		if (_scene->_objects[currentBuilding].offset_x + _scene->_objects[currentBuilding].object_width < _scene->_objects[i].offset_x) continue;
		if (_scene->_objects[currentBuilding].offset_x > _scene->_objects[i].offset_x + _scene->_objects[i].object_width) continue;

		if (_scene->_objects[currentBuilding].offset_y + _scene->_objects[currentBuilding].object_depth < _scene->_objects[i].offset_y) continue;
		if (_scene->_objects[currentBuilding].offset_y > _scene->_objects[i].offset_y + _scene->_objects[i].object_depth) continue;

		if (_scene->_objects[currentBuilding].offset_z + _scene->_objects[currentBuilding].height < _scene->_objects[i].offset_z) continue;
		if (_scene->_objects[currentBuilding].offset_z > _scene->_objects[i].offset_z + _scene->_objects[i].height) continue;

		conflicted = true;
		_scene->_objects[currentBuilding].offset_z = _scene->_objects[i].offset_z + _scene->_objects[i].height;
	}

	if (!conflicted) {
		_scene->_objects[currentBuilding].offset_z = 0;
	}
}

void BuildingSelector::alignObjects() {
	if (_selectedBuilding >= 0) {
		_scene->alignObjects(_selectedBuilding, _selectedBuildingControlPoint);
	}
}

void BuildingSelector::generateGeometry(RenderManager* renderManager) {
	std::vector<Vertex> vertices;
	const float controlPointSize = 0.7f;

	{
		float x = _scene->_objects[_selectedBuilding].offset_x;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 1) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 2) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 3) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 4) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 5) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	renderManager->addObject("controler", "", vertices, false);
}

}