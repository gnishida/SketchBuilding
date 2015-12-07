#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GLUtils.h"

class RenderManager;

namespace sc {

class Scene;

class BuildingSelector {
private:
	int _selectedBuilding;
	int _selectedBuildingControlPoint;
	glm::vec2 _mouseStartPt;
	glm::vec2 _controlPointDir;

public:
	BuildingSelector();

	bool isBuildingSelected();
	bool isBuildingControlPointSelected();
	bool selectBuilding(sc::Scene* scene, const glm::vec3& cameraPos, const glm::vec3& viewDir, const std::string& stage);
	bool selectBuildingControlPoint(sc::Scene* scene, const glm::vec3& cameraPos, const glm::vec3& viewDir, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height);
	void unselectBuilding();
	void unselectBuildingControlPoint();

	void copy(Scene* scene);
	void resize(Scene* scene, const glm::vec2& mousePt);

	void generateGeometry(RenderManager* renderManager, Scene* scene);
};

}