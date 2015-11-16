#include "Scene.h"
#include "RenderManager.h"
#include "Rectangle.h"

void ShapeLayer::setFootprint(float offset_x, float offset_y, float object_width, float object_depth) {
	this->offset_x = offset_x;
	this->offset_y = offset_y;
	this->object_width = object_width;
	this->object_depth = object_depth;
}

void ShapeLayer::setHeight(float height) {
	this->height = height;
}

void ShapeLayer::setGrammar(const cga::Grammar& grammar, const std::vector<float>& params) {
	this->_grammar = grammar;
	cga::CGA::setParamValues(_grammar, params);
}

BuildingMass::BuildingMass() {
	_layers.resize(1);
	_currentLayer = 0;

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

void BuildingMass::generateGeometry(cga::CGA* system, RenderManager* renderManager) {
	float currentHeight = 0.0f;
	for (int i = 0; i < _currentLayer; ++i) {
		if (_layers[i].height == 0.0f) continue;
		currentHeight += _layers[i].height;
	}

	float height = 0.0f;
	for (int i = 0; i < _layers.size(); ++i) {
		if (_layers[i].height == 0.0f) continue;

		// footprint
		cga::Rectangle* footprint = new cga::Rectangle("Start", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(_layers[i].offset_x, _layers[i].offset_y, height - currentHeight)), glm::mat4(), _layers[i].object_width, _layers[i].object_depth, glm::vec3(1, 1, 1));
		system->stack.push_back(boost::shared_ptr<cga::Shape>(footprint));

		system->derive(_layers[i]._grammar, true);
		system->generateGeometry(renderManager);

		height += _layers[i].height;
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

void Scene::generateGeometry(RenderManager* renderManager) {
	renderManager->removeObjects();

	building.generateGeometry(&system, renderManager);
}