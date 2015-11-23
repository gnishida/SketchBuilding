#pragma once

#include <vector>
#include "CGA.h"
#include "GLUtils.h"

class RenderManager;

namespace sc {

class SceneObject {
public:
	float offset_x;
	float offset_y;
	float offset_z;
	float object_width;
	float object_depth;
	float height;
	
	std::map<std::string, cga::Grammar> grammars;
	std::vector<boost::shared_ptr<glutils::Face> > faces;

public:
	SceneObject();
	SceneObject(float offset_x, float offset_y, float offset_width, float offset_depth, float height, const cga::Grammar& grammar);
	void setFootprint(float offset_x, float offset_y, float offset_z, float object_width, float object_depth);
	void setHeight(float height);
	void setGrammar(const std::string& name, const cga::Grammar& grammar);
	void setGrammar(const std::string& name, const cga::Grammar& grammar, const std::vector<float>& params);

	void generateGeometry(cga::CGA* system, RenderManager* renderManager, const std::string& stage);
	void updateGeometry(RenderManager* renderManager, const std::string& stage);
};

class Scene {
public:
	cga::CGA system;
	std::vector<SceneObject> _objects;
	int _currentObject;
	boost::shared_ptr<glutils::Face> _selectedFace;
	std::string _selectedFaceName;

public:
	Scene();

	void clear();
	void newLayer();
	void alignLayers();
	SceneObject& currentObject() { return _objects[_currentObject]; }
	boost::shared_ptr<glutils::Face> selectedFace() { return _selectedFace; }
	bool selectFace(const glm::vec3& p, const glm::vec3& v, const glm::vec3& normal = glm::vec3(1, 1, 1));
	void unselectFace();

	void generateGeometry(RenderManager* renderManager, const std::string& stage);
	void updateGeometry(RenderManager* renderManager, const std::string& stage);
};

}
