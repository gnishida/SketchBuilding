#include "Scene.h"
#include "RenderManager.h"
#include "Rectangle.h"
#include <iostream>

namespace sc {

void ShapeLayer::setFootprint(float offset_x, float offset_y, float offset_z, float object_width, float object_depth) {
	this->offset_x = offset_x;
	this->offset_y = offset_y;
	this->offset_z = offset_z;
	this->object_width = object_width;
	this->object_depth = object_depth;
}

void ShapeLayer::setHeight(float height) {
	this->height = height;
}

void ShapeLayer::setGrammar(const cga::Grammar& grammar, const std::vector<float>& params) {
	this->grammar = grammar;
	cga::CGA::setParamValues(this->grammar, params);
}

void ShapeLayer::generateGeometry(cga::CGA* system, RenderManager* renderManager) {
	faces.clear();

	if (height == 0.0f) return;

	// footprint
	cga::Rectangle* footprint = new cga::Rectangle("Start", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(offset_x, offset_y, offset_z)), glm::mat4(), object_width, object_depth, glm::vec3(1, 1, 1));
	system->stack.push_back(boost::shared_ptr<cga::Shape>(footprint));

	system->derive(grammar, true);
	
	system->generateGeometry(faces);

	renderManager->addFaces(faces);
}

void ShapeLayer::updateGeometry(RenderManager* renderManager) {
	renderManager->addFaces(faces);
}

BuildingMass::BuildingMass() {
	_layers.resize(1);
	_currentLayer = 0;
	_selectedFace = NULL;
}

void BuildingMass::clear() {
	_layers.clear();
	_layers.push_back(ShapeLayer());
	_currentLayer = 0;
}

void BuildingMass::newLayer() {
	_layers.push_back(ShapeLayer());
	_currentLayer++;
}

void BuildingMass::alignLayers() {
	if (_currentLayer == 0) return;

	if (fabs(_layers[_currentLayer].offset_x - _layers[_currentLayer - 1].offset_x) < 2.0f) {
		_layers[_currentLayer].offset_x = _layers[_currentLayer - 1].offset_x;
	}
	if (fabs(_layers[_currentLayer].offset_y - _layers[_currentLayer - 1].offset_y) < 2.0f) {
		_layers[_currentLayer].offset_y = _layers[_currentLayer - 1].offset_y;
	}
	if (fabs(_layers[_currentLayer].offset_x + _layers[_currentLayer].object_width - _layers[_currentLayer - 1].offset_x - _layers[_currentLayer - 1].object_width) < 2.0f) {
		_layers[_currentLayer].object_width = _layers[_currentLayer - 1].offset_x + _layers[_currentLayer - 1].object_width - _layers[_currentLayer].offset_x;
	}
	if (fabs(_layers[_currentLayer].offset_y + _layers[_currentLayer].object_depth - _layers[_currentLayer - 1].offset_y - _layers[_currentLayer - 1].object_depth) < 2.0f) {
		_layers[_currentLayer].object_depth = _layers[_currentLayer - 1].offset_y + _layers[_currentLayer - 1].object_depth - _layers[_currentLayer].offset_y;
	}
}

/**
 * Select a face by hit testing.
 *
 * @param p			the point that emits ray.
 * @param v			the ray vector
 */
bool BuildingMass::selectFace(const glm::vec3& p, const glm::vec3& v, glutils::Face** selectedFace) {
	glm::vec3 intPt;
	float min_dist = (std::numeric_limits<float>::max)();
	bool selected = false;

	glutils::Face* newSelectedFace = NULL;

	for (int i = 0; i < _layers.size(); ++i) {
		if (i == _currentLayer) continue;

		for (int j = 0; j < _layers[i].faces.size(); ++j) {
			if (_layers[i].faces[j].vertices.size() < 3) continue;

			for (int k = 0; k < _layers[i].faces[j].vertices.size(); k += 3) {
				if (glutils::rayTriangleIntersection(p, v, _layers[i].faces[j].vertices[k].position, _layers[i].faces[j].vertices[k+1].position, _layers[i].faces[j].vertices[k+2].position, intPt)) {
					float dist = glm::length(intPt - p);

					if (dist < min_dist) {
						min_dist = dist;
						selected = true;
						newSelectedFace = &_layers[i].faces[j];
					}
				}

			}
		}
	}

	if (selected) {
		if (_selectedFace != NULL) {
			_selectedFace->unselect();
		}
		_selectedFace = newSelectedFace;
		*selectedFace = _selectedFace;
		_selectedFace->select();
		return true;
	}
	else return false;
}

/**
* Select a top face by hit testing.
*
* @param p			the point that emits ray.
* @param v			the ray vector
*/
bool BuildingMass::selectTopFace(const glm::vec3& p, const glm::vec3& v, glutils::Face** selectedFace) {
	glm::vec3 intPt;
	float min_dist = (std::numeric_limits<float>::max)();
	bool selected = false;

	glutils::Face* newSelectedFace = NULL;

	for (int i = 0; i < _layers.size(); ++i) {
		if (i == _currentLayer) continue;

		for (int j = 0; j < _layers[i].faces.size(); ++j) {
			if (_layers[i].faces[j].vertices.size() < 3) continue;

			for (int k = 0; k < _layers[i].faces[j].vertices.size(); k += 3) {
				if (glm::dot(_layers[i].faces[j].vertices[0].normal, glm::vec3(0, 1, 0)) < 0.9) continue;

				if (glutils::rayTriangleIntersection(p, v, _layers[i].faces[j].vertices[k].position, _layers[i].faces[j].vertices[k + 1].position, _layers[i].faces[j].vertices[k + 2].position, intPt)) {
					float dist = glm::length(intPt - p);

					if (dist < min_dist) {
						min_dist = dist;
						selected = true;
						newSelectedFace = &_layers[i].faces[j];
					}
				}

			}
		}
	}

	if (selected) {
		if (_selectedFace != NULL) {
			_selectedFace->unselect();
		}
		_selectedFace = newSelectedFace;
		*selectedFace = _selectedFace;
		_selectedFace->select();

		return true;
	}
	else return false;
}

/**
* Select a side face by hit testing.
*
* @param p			the point that emits ray.
* @param v			the ray vector
*/
bool BuildingMass::selectSideFace(const glm::vec3& p, const glm::vec3& v, glutils::Face** selectedFace) {
	glm::vec3 intPt;
	float min_dist = (std::numeric_limits<float>::max)();
	bool selected = false;

	glutils::Face* newSelectedFace = NULL;

	for (int i = 0; i < _layers.size(); ++i) {
		if (i == _currentLayer) continue;

		for (int j = 0; j < _layers[i].faces.size(); ++j) {
			if (_layers[i].faces[j].vertices.size() < 3) continue;

			for (int k = 0; k < _layers[i].faces[j].vertices.size(); k += 3) {
				if (fabs(glm::dot(_layers[i].faces[j].vertices[0].normal, glm::vec3(0, 1, 0))) > 0.1) continue;

				if (glutils::rayTriangleIntersection(p, v, _layers[i].faces[j].vertices[k].position, _layers[i].faces[j].vertices[k + 1].position, _layers[i].faces[j].vertices[k + 2].position, intPt)) {
					float dist = glm::length(intPt - p);

					if (dist < min_dist) {
						min_dist = dist;
						selected = true;
						newSelectedFace = &_layers[i].faces[j];
					}
				}

			}
		}
	}

	if (selected) {
		if (_selectedFace != NULL) {
			_selectedFace->unselect();
		}
		_selectedFace = newSelectedFace;
		*selectedFace = _selectedFace;
		_selectedFace->select();

		return true;
	}
	else return false;
}

void BuildingMass::generateGeometry(cga::CGA* system, RenderManager* renderManager) {
	for (int i = 0; i < _layers.size(); ++i) {
		_layers[i].generateGeometry(system, renderManager);
	}

}

void BuildingMass::updateGeometry(RenderManager* renderManager) {
	for (int i = 0; i < _layers.size(); ++i) {
		_layers[i].updateGeometry(renderManager);
	}
}

Roof::Roof() {

}

void Roof::clear() {

}

Scene::Scene() {
	system.modelMat = glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0));

}

void Scene::clear() {
	building.clear();
	roof.clear();

}

/**
 * Generate geometry by the grammars, and send the geometry to GPU memory.
 */
void Scene::generateGeometry(RenderManager* renderManager) {
	renderManager->removeObjects();

	building.generateGeometry(&system, renderManager);
}

/**
 * Send the geometry to GPU memory. 
 * Note that the re-derivation by the grammars is not performed. Instead, already generated faces are used.
 */
void Scene::updateGeometry(RenderManager* renderManager) {
	renderManager->removeObjects();

	building.updateGeometry(renderManager);
}

}