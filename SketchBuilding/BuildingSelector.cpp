#include "BuildingSelector.h"
#include "Scene.h"
#include "RenderManager.h"
#include "GLUtils.h"
#include <iostream>

namespace sc {

float BuildingSelector::CONTROL_POINT_SIZE = 0.5f;

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

int BuildingSelector::selectBuilding(const glm::vec3& cameraPos, const glm::vec3& viewDir) {
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

	return _selectedBuilding;
}

/**
 * Find the control point that is close to the mouse pointer.
 * If such a control point exists, return the id of the corresponding building mass.
 * Otherwise, find the building mass that is close to the mouse pointer and return its id.
 * If no such a building mass exists, return -1.
 */
int BuildingSelector::selectBuildingControlPoint(const glm::vec3& cameraPos, const glm::vec3& viewDir, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height) {
	this->_mouseStartPt = mousePt;

	_selectedBuilding = selectBuilding(cameraPos, viewDir);
	_selectedBuildingControlPoint = -1;

	float min_dist = (std::numeric_limits<float>::max)();

	const float angle_threshold = 0.99993f;

	for (int i = 0; i < _scene->_objects.size(); ++i) {
		if (_selectedBuilding >= 0 && i != _selectedBuilding) continue;

		{
			float x = _scene->_objects[i].offset_x;
			float y = _scene->_objects[i].offset_y + _scene->_objects[i].object_depth * 0.5;
			float z = _scene->_objects[i].offset_z + _scene->_objects[i].height * 0.5;
			glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));

			glm::vec3 dir = p - cameraPos;
			glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(-1, 0, 0, 0));
			if (glm::dot(glm::normalize(dir), n) < 0.2 && hitTestForControlPoint(p, mousePt, mvpMatrix, screen_width, screen_height)) {
				float dist = glm::dot(glm::normalize(viewDir), dir);
				if (dist < min_dist) {
					min_dist = dist;
					_selectedBuilding = i;
					_selectedBuildingControlPoint = 1;

					// compute the projected direction vector for this control point
					glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
					glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x - 1, y, z, 1);
					_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
				}
			}
		}

		{
			float x = _scene->_objects[i].offset_x + _scene->_objects[i].object_width;
			float y = _scene->_objects[i].offset_y + _scene->_objects[i].object_depth * 0.5;
			float z = _scene->_objects[i].offset_z + _scene->_objects[i].height * 0.5;
			glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));

			glm::vec3 dir = p - cameraPos;
			glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(1, 0, 0, 0));
			if (glm::dot(glm::normalize(dir), n) < 0.2 && hitTestForControlPoint(p, mousePt, mvpMatrix, screen_width, screen_height)) {
				float dist = glm::dot(glm::normalize(viewDir), dir);
				if (dist < min_dist) {
					min_dist = dist;
					_selectedBuilding = i;
					_selectedBuildingControlPoint = 2;

					// compute the projected direction vector for this control point
					glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
					glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x + 1, y, z, 1);
					_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
				}
			}
		}

		{
			float x = _scene->_objects[i].offset_x + _scene->_objects[i].object_width * 0.5;
			float y = _scene->_objects[i].offset_y;
			float z = _scene->_objects[i].offset_z + _scene->_objects[i].height * 0.5;
			glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));

			glm::vec3 dir = p - cameraPos;
			glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(0, -1, 0, 0));
			if (glm::dot(glm::normalize(dir), n) < 0.2 && hitTestForControlPoint(p, mousePt, mvpMatrix, screen_width, screen_height)) {
				float dist = glm::dot(glm::normalize(viewDir), dir);
				if (dist < min_dist) {
					min_dist = dist;
					_selectedBuilding = i;
					_selectedBuildingControlPoint = 3;

					// compute the projected direction vector for this control point
					glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
					glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y - 1, z, 1);
					_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
				}
			}
		}

		{
			float x = _scene->_objects[i].offset_x + _scene->_objects[i].object_width * 0.5;
			float y = _scene->_objects[i].offset_y + _scene->_objects[i].object_depth;
			float z = _scene->_objects[i].offset_z + _scene->_objects[i].height * 0.5;
			glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));

			glm::vec3 dir = p - cameraPos;
			glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(0, 1, 0, 0));
			if (glm::dot(glm::normalize(dir), n) < 0.2 && hitTestForControlPoint(p, mousePt, mvpMatrix, screen_width, screen_height)) {
				float dist = glm::dot(glm::normalize(viewDir), dir);
				if (dist < min_dist) {
					min_dist = dist;
					_selectedBuilding = i;
					_selectedBuildingControlPoint = 4;

					// compute the projected direction vector for this control point
					glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
					glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y + 1, z, 1);
					_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
				}
			}
		}

		{
			float x = _scene->_objects[i].offset_x + _scene->_objects[i].object_width * 0.5;
			float y = _scene->_objects[i].offset_y + _scene->_objects[i].object_depth * 0.5;
			float z = _scene->_objects[i].offset_z + _scene->_objects[i].height;
			glm::vec3 p = glm::vec3(_scene->system.modelMat * glm::vec4(x, y, z, 1));

			glm::vec3 dir = p - cameraPos;
			glm::vec3 n = glm::vec3(_scene->system.modelMat * glm::vec4(0, 0, 1, 0));
			if (glm::dot(glm::normalize(dir), n) < 0.2 && hitTestForControlPoint(p, mousePt, mvpMatrix, screen_width, screen_height)) {
				float dist = glm::dot(glm::normalize(viewDir), dir);
				if (dist < min_dist) {
					min_dist = dist;
					_selectedBuilding = i;
					_selectedBuildingControlPoint = 5;

					// compute the projected direction vector for this control point
					glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
					glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y, z + 1, 1);
					_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
				}
			}
		}
	}

	if (_selectedBuildingControlPoint > 0) {
		return _selectedBuilding;
	}
	else {
		if (_selectedBuilding >= 0) {
			// select the entire building
			_selectedBuildingControlPoint = 0;

			float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
			float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
			float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
			glm::vec4 screen_p1 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y, z, 1);
			glm::vec4 screen_p2 = mvpMatrix * _scene->system.modelMat * glm::vec4(x + 1, y, z, 1);
			glm::vec4 screen_p3 = mvpMatrix * _scene->system.modelMat * glm::vec4(x, y + 1, z, 1);

			_xDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			_yDir = glm::vec2((screen_p3.x / screen_p3.w + 1.0) * 0.5f * screen_width, (1 - screen_p3.y / screen_p3.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
		}

		return _selectedBuilding;
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

void BuildingSelector::remove() {
	if (_selectedBuilding == -1) return;

	_scene->removeObject(_selectedBuilding);
	unselectBuilding();
}

void BuildingSelector::resize(const glm::vec2& mousePt, bool conflictAllowed, bool sameWidthDepth) {
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
		if (!sameWidthDepth) {
			_scene->_objects[_selectedBuilding].offset_x -= diff;
			_scene->_objects[_selectedBuilding].object_width += diff;
		}
		else {
			_scene->_objects[_selectedBuilding].offset_x -= diff;
			_scene->_objects[_selectedBuilding].object_width += diff * 2.0f;
			_scene->_objects[_selectedBuilding].offset_y -= (_scene->_objects[_selectedBuilding].object_width - _scene->_objects[_selectedBuilding].object_depth) * 0.5f;
			_scene->_objects[_selectedBuilding].object_depth = _scene->_objects[_selectedBuilding].object_width;
		}
	}
	else if (_selectedBuildingControlPoint == 2) {
		float diff = glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		if (!sameWidthDepth) {
			_scene->_objects[_selectedBuilding].object_width += diff;
		}
		else {
			_scene->_objects[_selectedBuilding].offset_x -= diff;
			_scene->_objects[_selectedBuilding].object_width += diff * 2.0f;
			_scene->_objects[_selectedBuilding].offset_y -= (_scene->_objects[_selectedBuilding].object_width - _scene->_objects[_selectedBuilding].object_depth) * 0.5f;
			_scene->_objects[_selectedBuilding].object_depth = _scene->_objects[_selectedBuilding].object_width;
		}
	}
	else if (_selectedBuildingControlPoint == 3) {
		float diff = glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		if (!sameWidthDepth) {
			_scene->_objects[_selectedBuilding].offset_y -= diff;
			_scene->_objects[_selectedBuilding].object_depth += diff;
		}
		else {
			_scene->_objects[_selectedBuilding].offset_y -= diff;
			_scene->_objects[_selectedBuilding].object_depth += diff * 2.0f;
			_scene->_objects[_selectedBuilding].offset_x -= (_scene->_objects[_selectedBuilding].object_depth - _scene->_objects[_selectedBuilding].object_width) * 0.5f;
			_scene->_objects[_selectedBuilding].object_width = _scene->_objects[_selectedBuilding].object_depth;
		}
	}
	else if (_selectedBuildingControlPoint == 4) {
		float diff = _scene->_objects[_selectedBuilding].object_depth - _scene->_objects[_selectedBuilding].object_width;
		if (!sameWidthDepth) {
			_scene->_objects[_selectedBuilding].object_depth += diff;
		}
		else {
			_scene->_objects[_selectedBuilding].offset_y -= diff;
			_scene->_objects[_selectedBuilding].object_depth += diff * 2.0f;
			_scene->_objects[_selectedBuilding].offset_x -= (_scene->_objects[_selectedBuilding].object_depth - _scene->_objects[_selectedBuilding].object_width) * 0.5f;
			_scene->_objects[_selectedBuilding].object_width = _scene->_objects[_selectedBuilding].object_depth;
		}
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

void BuildingSelector::alignObjects(float threshold) {
	if (_selectedBuilding >= 0) {
		_scene->alignObjects(_selectedBuilding, _selectedBuildingControlPoint, threshold);
	}
}

void BuildingSelector::generateGeometry(RenderManager* renderManager) {
	std::vector<Vertex> vertices;

	{
		float x = _scene->_objects[_selectedBuilding].offset_x;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 1) {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 2) {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 3) {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 4) {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = _scene->_objects[_selectedBuilding].offset_x + _scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = _scene->_objects[_selectedBuilding].offset_y + _scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = _scene->_objects[_selectedBuilding].offset_z + _scene->_objects[_selectedBuilding].height;
		glm::mat4 mat = glm::translate(_scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 5) {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(CONTROL_POINT_SIZE, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	renderManager->addObject("controler", "", vertices, false);
}

bool BuildingSelector::hitTestForControlPoint(const glm::vec3& point, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height) {
	glm::vec3 p1 = point + glm::vec3(-CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE);
	glm::vec3 p2 = point + glm::vec3(CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE);
	glm::vec3 p3 = point + glm::vec3(CONTROL_POINT_SIZE, CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE);
	glm::vec3 p4 = point + glm::vec3(-CONTROL_POINT_SIZE, CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE);
	glm::vec3 p5 = point + glm::vec3(-CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE, CONTROL_POINT_SIZE);
	glm::vec3 p6 = point + glm::vec3(CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE, CONTROL_POINT_SIZE);
	glm::vec3 p7 = point + glm::vec3(CONTROL_POINT_SIZE, CONTROL_POINT_SIZE, CONTROL_POINT_SIZE);
	glm::vec3 p8 = point + glm::vec3(-CONTROL_POINT_SIZE, CONTROL_POINT_SIZE, CONTROL_POINT_SIZE);

	glm::vec4 pp1 = mvpMatrix * glm::vec4(p1, 1);
	glm::vec4 pp2 = mvpMatrix * glm::vec4(p2, 1);
	glm::vec4 pp3 = mvpMatrix * glm::vec4(p3, 1);
	glm::vec4 pp4 = mvpMatrix * glm::vec4(p4, 1);
	glm::vec4 pp5 = mvpMatrix * glm::vec4(p5, 1);
	glm::vec4 pp6 = mvpMatrix * glm::vec4(p6, 1);
	glm::vec4 pp7 = mvpMatrix * glm::vec4(p7, 1);
	glm::vec4 pp8 = mvpMatrix * glm::vec4(p8, 1);

	glutils::BoundingBox bbox;
	bbox.addPoint(glm::vec2((pp1.x / pp1.w * 0.5 + 0.5) * screen_width, (0.5 - pp1.y / pp1.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp2.x / pp2.w * 0.5 + 0.5) * screen_width, (0.5 - pp2.y / pp2.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp3.x / pp3.w * 0.5 + 0.5) * screen_width, (0.5 - pp3.y / pp3.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp4.x / pp4.w * 0.5 + 0.5) * screen_width, (0.5 - pp4.y / pp4.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp5.x / pp5.w * 0.5 + 0.5) * screen_width, (0.5 - pp5.y / pp5.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp6.x / pp6.w * 0.5 + 0.5) * screen_width, (0.5 - pp6.y / pp6.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp7.x / pp7.w * 0.5 + 0.5) * screen_width, (0.5 - pp7.y / pp7.w * 0.5) * screen_height));
	bbox.addPoint(glm::vec2((pp8.x / pp8.w * 0.5 + 0.5) * screen_width, (0.5 - pp8.y / pp8.w * 0.5) * screen_height));

	return bbox.contains(mousePt, 2.0f);
}

}