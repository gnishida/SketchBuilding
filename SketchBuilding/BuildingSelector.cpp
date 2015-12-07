#include "BuildingSelector.h"
#include "Scene.h"
#include "RenderManager.h"
#include "GLUtils.h"
#include <iostream>

namespace sc {

	BuildingSelector::BuildingSelector() {
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

	bool BuildingSelector::selectBuilding(sc::Scene* scene, const glm::vec3& cameraPos, const glm::vec3& viewDir) {
		glm::vec3 intPt;
		float min_dist = (std::numeric_limits<float>::max)();

		_selectedBuilding = -1;
		_selectedBuildingControlPoint = -1;

		for (int i = 0; i < scene->_objects.size(); ++i) {
			for (int j = 0; j < scene->_objects[i].faces.size(); ++j) {
				if (scene->_objects[i].faces[j]->vertices.size() < 3) continue;

				if (scene->_objects[i].faces[j]->grammar_type != "building") continue;

				for (int k = 0; k < scene->_objects[i].faces[j]->vertices.size(); k += 3) {
					if (glutils::rayTriangleIntersection(cameraPos, viewDir, scene->_objects[i].faces[j]->vertices[k].position, scene->_objects[i].faces[j]->vertices[k + 1].position, scene->_objects[i].faces[j]->vertices[k + 2].position, intPt)) {
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

	bool BuildingSelector::selectBuildingControlPoint(sc::Scene* scene, const glm::vec3& cameraPos, const glm::vec3& viewDir, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height) {
		this->_mouseStartPt = mousePt;

		_selectedBuildingControlPoint = -1;

		if (_selectedBuilding == -1) return false;

		float min_dist = (std::numeric_limits<float>::max)();

		{
			float x = scene->_objects[_selectedBuilding].offset_x;
			float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
			float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
			glm::vec3 p = glm::vec3(scene->system.modelMat * glm::vec4(x, y, z, 1));
			glm::vec3 n = glm::vec3(scene->system.modelMat * glm::vec4(-1, 0, 0, 0));

			glm::vec3 dir = p - cameraPos;
			if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > 0.9995f) {
				float dist = glm::length(dir);
				if (dist < min_dist) {
					min_dist = dist;
					_selectedBuildingControlPoint = 1;

					// compute the projected direction vector for this control point
					glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
					glm::vec4 screen_p2 = mvpMatrix * scene->system.modelMat * glm::vec4(x - 1, y, z, 1);
					_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
				}
			}
		}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width;
		float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(scene->system.modelMat * glm::vec4(1, 0, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > 0.9995f) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 2;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * scene->system.modelMat * glm::vec4(x + 1, y, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = scene->_objects[_selectedBuilding].offset_y;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(scene->system.modelMat * glm::vec4(0, -1, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > 0.9995f) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 3;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * scene->system.modelMat * glm::vec4(x, y - 1, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
		glm::vec3 p = glm::vec3(scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(scene->system.modelMat * glm::vec4(0, 1, 0, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > 0.9995f) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 4;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * scene->system.modelMat * glm::vec4(x, y + 1, z, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height;
		glm::vec3 p = glm::vec3(scene->system.modelMat * glm::vec4(x, y, z, 1));
		glm::vec3 n = glm::vec3(scene->system.modelMat * glm::vec4(0, 0, 1, 0));

		glm::vec3 dir = p - cameraPos;
		if (glm::dot(dir, n) < 0 && glm::dot(glm::normalize(viewDir), glm::normalize(dir)) > 0.9995f) {
			float dist = glm::length(dir);
			if (dist < min_dist) {
				min_dist = dist;
				_selectedBuildingControlPoint = 5;

				// compute the projected direction vector for this control point
				glm::vec4 screen_p1 = mvpMatrix * glm::vec4(p, 1);
				glm::vec4 screen_p2 = mvpMatrix * scene->system.modelMat * glm::vec4(x, y, z + 1, 1);
				_controlPointDir = glm::vec2((screen_p2.x / screen_p2.w + 1.0) * 0.5f * screen_width, (1 - screen_p2.y / screen_p2.w) * 0.5 * screen_height) - glm::vec2((screen_p1.x / screen_p1.w + 1.0) * 0.5 * screen_width, (1 - screen_p1.y / screen_p1.w) * 0.5 * screen_height);
			}
		}
	}

		if (_selectedBuildingControlPoint >= 1) {
			return true;
		}
		else {
			// select the entire building
			if (selectBuilding(scene, cameraPos, viewDir)) {
				_selectedBuildingControlPoint = 0;

				float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
				float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
				float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
				glm::vec4 screen_p1 = mvpMatrix * scene->system.modelMat * glm::vec4(x, y, z, 1);
				glm::vec4 screen_p2 = mvpMatrix * scene->system.modelMat * glm::vec4(x + 1, y, z, 1);
				glm::vec4 screen_p3 = mvpMatrix * scene->system.modelMat * glm::vec4(x, y + 1, z, 1);

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

	void BuildingSelector::copy(Scene* scene) {
		if (_selectedBuilding == -1) return;

		scene->newObject();
		scene->currentObject().setFootprint(scene->_objects[_selectedBuilding].offset_x + 14, scene->_objects[_selectedBuilding].offset_y, scene->_objects[_selectedBuilding].offset_z, scene->_objects[_selectedBuilding].object_width, scene->_objects[_selectedBuilding].object_depth);
		scene->currentObject().setHeight(scene->_objects[_selectedBuilding].height);

		// copy the grammar from the selected building mass to the new one
		for (auto it = scene->_objects[_selectedBuilding].grammars.begin(); it != scene->_objects[_selectedBuilding].grammars.end(); ++it) {
			scene->currentObject().setGrammar(it->first, it->second);
		}
	}

	void BuildingSelector::resize(Scene* scene, const glm::vec2& mousePt) {
		if (!isBuildingControlPointSelected()) return;

		if (_selectedBuildingControlPoint == 0) {
			float den = _xDir.x * _yDir.y - _xDir.y * _yDir.x;
			if (den != 0.0f) {
				scene->_objects[_selectedBuilding].offset_x += ((mousePt - _mouseStartPt).x * _yDir.y - (mousePt - _mouseStartPt).y * _yDir.x) / den;
				scene->_objects[_selectedBuilding].offset_y += ((mousePt - _mouseStartPt).y * _xDir.x - (mousePt - _mouseStartPt).x * _xDir.y) / den;

				// check the conflict. 
				// If exists, this building will be elevated such that the bottom face will be the same height of the other in order to avoid the conflict.
				avoidBuildingConflict(scene, _selectedBuilding);
			}
		}
		else if (_selectedBuildingControlPoint == 1) {
			float diff = glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
			scene->_objects[_selectedBuilding].offset_x -= diff;
			scene->_objects[_selectedBuilding].object_width += diff;
		}
		else if (_selectedBuildingControlPoint == 2) {
			scene->_objects[_selectedBuilding].object_width += glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		}
		else if (_selectedBuildingControlPoint == 3) {
			float diff = glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
			scene->_objects[_selectedBuilding].offset_y -= diff;
			scene->_objects[_selectedBuilding].object_depth += diff;
		}
		else if (_selectedBuildingControlPoint == 4) {
			scene->_objects[_selectedBuilding].object_depth += glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);
		}
		else if (_selectedBuildingControlPoint == 5) {
			float height = scene->_objects[_selectedBuilding].height + glm::dot(mousePt - _mouseStartPt, _controlPointDir) / glm::length(_controlPointDir) / glm::length(_controlPointDir);

			scene->_objects[_selectedBuilding].setHeight(height);
		}

		_mouseStartPt = mousePt;
	}

	void BuildingSelector::avoidBuildingConflict(Scene* scene, int currentBuilding) {
		bool conflicted = false;

		for (int i = 0; i < scene->_objects.size(); ++i) {
			if (i == currentBuilding) continue;

			if (scene->_objects[currentBuilding].offset_x + scene->_objects[currentBuilding].object_width < scene->_objects[i].offset_x) continue;
			if (scene->_objects[currentBuilding].offset_x > scene->_objects[i].offset_x + scene->_objects[i].object_width) continue;

			if (scene->_objects[currentBuilding].offset_y + scene->_objects[currentBuilding].object_depth < scene->_objects[i].offset_y) continue;
			if (scene->_objects[currentBuilding].offset_y > scene->_objects[i].offset_y + scene->_objects[i].object_depth) continue;

			if (scene->_objects[currentBuilding].offset_z + scene->_objects[currentBuilding].height < scene->_objects[i].offset_z) continue;
			if (scene->_objects[currentBuilding].offset_z > scene->_objects[i].offset_z + scene->_objects[i].height) continue;

			conflicted = true;
			scene->_objects[currentBuilding].offset_z = scene->_objects[i].offset_z + scene->_objects[i].height;
		}

		if (!conflicted) {
			scene->_objects[currentBuilding].offset_z = 0;
		}
	}

	void BuildingSelector::alignObjects(Scene* scene) {
		if (_selectedBuilding >= 0) {
			scene->alignObjects(_selectedBuilding);
		}
	}

	void BuildingSelector::generateGeometry(RenderManager* renderManager, Scene* scene) {
		std::vector<Vertex> vertices;
		const float controlPointSize = 0.7f;

		{
			float x = scene->_objects[_selectedBuilding].offset_x;
			float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
			float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
			glm::mat4 mat = glm::translate(scene->system.modelMat, glm::vec3(x, y, z));
			if (_selectedBuildingControlPoint == 1) {
				glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
			}
			else {
				glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
			}
		}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width;
		float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 2) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = scene->_objects[_selectedBuilding].offset_y;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 3) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height * 0.5;
		glm::mat4 mat = glm::translate(scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 4) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

	{
		float x = scene->_objects[_selectedBuilding].offset_x + scene->_objects[_selectedBuilding].object_width * 0.5;
		float y = scene->_objects[_selectedBuilding].offset_y + scene->_objects[_selectedBuilding].object_depth * 0.5;
		float z = scene->_objects[_selectedBuilding].offset_z + scene->_objects[_selectedBuilding].height;
		glm::mat4 mat = glm::translate(scene->system.modelMat, glm::vec3(x, y, z));
		if (_selectedBuildingControlPoint == 5) {
			glutils::drawSphere(controlPointSize, glm::vec4(1, 0, 0, 1), mat, vertices);
		}
		else {
			glutils::drawSphere(controlPointSize, glm::vec4(0, 0, 1, 1), mat, vertices);
		}
	}

		renderManager->addObject("controler", "", vertices);
	}

}