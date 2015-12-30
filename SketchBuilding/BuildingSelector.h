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
	static float CONTROL_POINT_SIZE;

private:
	sc::Scene* _scene;
	int _selectedBuilding;
	int _selectedBuildingControlPoint;
	glm::vec2 _mouseStartPt;
	glm::vec2 _controlPointDir;
	glm::vec2 _xDir;
	glm::vec2 _yDir;

public:
	BuildingSelector(sc::Scene* scene);

	bool isBuildingSelected();
	bool isBuildingControlPointSelected();
	int selectBuilding(const glm::vec3& cameraPos, const glm::vec3& viewDir);
	int selectBuildingControlPoint(const glm::vec3& cameraPos, const glm::vec3& viewDir, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height);
	void unselectBuilding();
	void unselectBuildingControlPoint();

	void copy();
	void remove();
	void resize(const glm::vec2& mousePt, bool conflictAllowed, bool sameWidthDepth);
	void avoidBuildingConflict(int currentBuilding);
	void alignObjects(float threshold);

	void generateGeometry(RenderManager* renderManager);

private:
	bool hitTestForControlPoint(const glm::vec3& point, const glm::vec2& mousePt, const glm::mat4& mvpMatrix, int screen_width, int screen_height);
};

}