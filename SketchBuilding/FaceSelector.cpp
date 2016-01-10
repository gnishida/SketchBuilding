#include "FaceSelector.h"
#include "Scene.h"
#include <iostream>

namespace sc {

FaceSelector::FaceSelector(sc::Scene* scene) {
	_scene = scene;
	_selected = false;
}

bool FaceSelector::selected() {
	return _selected;
}

boost::shared_ptr<glutils::Face> FaceSelector::selectedFace() {
	return _selectedFace;
}

glutils::Face FaceSelector::selectedFaceCopy() {
	return _selectedFaceCopy;
}

bool FaceSelector::selectFace(const glm::vec3& cameraPos, const glm::vec3& viewDir, const std::string& stage, const glm::vec3& normal) {
	glm::vec3 intPt;
	float min_dist = (std::numeric_limits<float>::max)();

	unselect();

	for (int i = 0; i < _scene->_objects.size(); ++i) {
		for (int j = 0; j < _scene->_objects[i].faces.size(); ++j) {
			if (_scene->_objects[i].faces[j]->vertices.size() < 3) continue;

			// check the face's type
			if (stage == "building") {
				if (_scene->_objects[i].faces[j]->grammar_type != "building") continue;
			}
			else if (stage == "roof") {
				if (_scene->_objects[i].faces[j]->grammar_type != "building") continue;
			}
			else if (stage == "facade") {
				if (_scene->_objects[i].faces[j]->grammar_type != "building") continue;
			}
			else if (stage == "floor") {
				if (_scene->_objects[i].faces[j]->grammar_type != "facade") continue;
			}
			else if (stage == "window") {
				if (_scene->_objects[i].faces[j]->grammar_type != "floor") continue;
			}
			else if (stage == "ledge") {
				if (_scene->_objects[i].faces[j]->grammar_type != "facade") continue;
			}

			for (int k = 0; k < _scene->_objects[i].faces[j]->vertices.size(); k += 3) {
				if (fabs(glm::dot(_scene->_objects[i].faces[j]->vertices[0].normal, normal)) < 0.99f) continue;

				if (glutils::rayTriangleIntersection(cameraPos, viewDir, _scene->_objects[i].faces[j]->vertices[k].position, _scene->_objects[i].faces[j]->vertices[k + 1].position, _scene->_objects[i].faces[j]->vertices[k + 2].position, intPt)) {
					float dist = glm::length(intPt - cameraPos);

					if (dist < min_dist) {
						min_dist = dist;
						_selectedFace = _scene->_objects[i].faces[j];
						_selectedFaceShape = _scene->_objects[i].faces[j]->shape->clone("Start");
						_scene->_currentObject = i;
					}
				}

			}
		}
	}

	if (_selectedFace) {
		_selectedFaceCopy = *_selectedFace;
		_selectedFaceName = _selectedFace->name;
		_selected = true;
		return true;
	}
	else {
		_selectedFaceName = "";
		_selected = false;
		return false;
	}
}

void FaceSelector::selectFace(int object_id, const boost::shared_ptr<glutils::Face>& face) {
	unselect();

	_scene->_currentObject = object_id;
	_selectedFace = face;
	_selectedFaceShape = face->shape->clone("Start");
	_selectedFaceCopy = *_selectedFace;
	_selectedFaceName = _selectedFace->name;
	_selected = true;
}

std::string FaceSelector::selectedFaceName() {
	return _selectedFaceName;
}

void FaceSelector::unselect() {
	if (_selected) {
		_selectedFace->unselect();
		_selectedFace.reset();
	}

	_selected = false;
}

}