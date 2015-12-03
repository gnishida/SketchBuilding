#include "FaceSelector.h"
#include "Scene.h"
#include <iostream>

namespace sc {

FaceSelector::FaceSelector() {
	_selected = false;
	_scene = NULL;
}

bool FaceSelector::selected() {
	return _selected;
}

glutils::Face FaceSelector::selectedFace() {
	return _selectedFace;
}

bool FaceSelector::selectFace(sc::Scene* scene, const glm::vec3& cameraPos, const glm::vec3& viewDir, const std::string& stage, const glm::vec3& normal) {
	this->_scene = scene;
	this->_cameraPos = cameraPos;
	this->_viewDir = viewDir;
	this->_stage = stage;
	this->_normal = normal;

	_selected = scene->selectFace(cameraPos, viewDir, stage, normal);
	if (_selected) {
		_selectedFace = *(scene->selectedFace());
		_selectedFaceName = scene->selectedFace()->name;
		return true;
	}
	else {
		return false;
	}
}

std::string FaceSelector::selectedFaceName() {
	return _selectedFaceName;
}

bool FaceSelector::reselectFace() {
	if (!_selected) return false;

	_selected = _scene->selectFace(_cameraPos, _viewDir, _stage, _normal);
	if (_selected) {
		_selectedFace = *(_scene->selectedFace());
		_selectedFaceName = _scene->selectedFace()->name;
		return true;
	}
	else {
		return false;
	}
}

void FaceSelector::unselect() {
	if (_selected && _scene != NULL) {
		_scene->unselectFace();
	}

	_selected = false;
}

}