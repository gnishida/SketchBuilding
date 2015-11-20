#pragma once

#include <vector>
#include "CGA.h"
#include "GLUtils.h"

class RenderManager;

namespace sc {

class ShapeLayer {
public:
	float offset_x;
	float offset_y;
	float offset_z;
	float object_width;
	float object_depth;
	float height;
	//cga::Grammar grammar;
	std::map<std::string, cga::Grammar> grammars;
	std::vector<boost::shared_ptr<glutils::Face> > faces;

public:
	ShapeLayer();
	ShapeLayer(float offset_x, float offset_y, float offset_width, float offset_depth, float height, const cga::Grammar& grammar);
	void setFootprint(float offset_x, float offset_y, float offset_z, float object_width, float object_depth);
	void setHeight(float height);
	void setGrammar(const std::string& name, const cga::Grammar& grammar);
	void setGrammar(const std::string& name, const cga::Grammar& grammar, const std::vector<float>& params);

	void generateGeometry(cga::CGA* system, RenderManager* renderManager);
	void updateGeometry(RenderManager* renderManager);
};

class BuildingMass {
public:
	std::vector<ShapeLayer> _layers;
	int _currentLayer;
	boost::shared_ptr<glutils::Face> _selectedFace;

public:
	BuildingMass();

	void clear();
	void newLayer();
	void alignLayers();
	ShapeLayer& currentLayer() { return _layers[_currentLayer]; }
	boost::shared_ptr<glutils::Face> selectedFace() { return _selectedFace; }
	bool selectFace(const glm::vec3& p, const glm::vec3& v, const glm::vec3& normal = glm::vec3(1, 1, 1));
	//bool selectTopFace(const glm::vec3& p, const glm::vec3& v);
	//bool selectSideFace(const glm::vec3& p, const glm::vec3& v);
	void unselectFace();

	void generateGeometry(cga::CGA* system, RenderManager* renderManager);
	void updateGeometry(RenderManager* renderManager);
};

class Roof {
public:
	Roof();

	void clear();
};

class Scene {
public:
	cga::CGA system;

	BuildingMass building;
	Roof roof;

public:
	Scene();

	void clear();
	void generateGeometry(RenderManager* renderManager);
	void updateGeometry(RenderManager* renderManager);
};

}
