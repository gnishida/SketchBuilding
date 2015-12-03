#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GLUtils.h"

namespace sc {

class Scene;

class FaceSelector {
private:
	bool _selected;
	sc::Scene* _scene;
	glm::vec3 _cameraPos;
	glm::vec3 _viewDir;
	std::string _stage;
	glm::vec3 _normal;
	glutils::Face _selectedFace;
	std::string _selectedFaceName;

public:
	FaceSelector();

	bool selected();
	glutils::Face selectedFace();
	std::string selectedFaceName();
	bool selectFace(sc::Scene* scene, const glm::vec3& cameraPos, const glm::vec3& viewDir, const std::string& stage, const glm::vec3& normal);
	bool reselectFace();
	void unselect();
};

}