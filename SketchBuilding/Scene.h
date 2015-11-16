#pragma once

#include <vector>
#include "CGA.h"

class RenderManager;

class ShapeLayer {
public:
	float offset_x;
	float offset_y;
	float object_width;
	float object_depth;
	float height;
	cga::Grammar _grammar;

public:
	ShapeLayer() : offset_x(0), offset_y(0), object_width(0), object_depth(0), height(0) {}
	ShapeLayer(float offset_x, float offset_y, float offset_width, float offset_depth, float height, const cga::Grammar& grammar);
	void setFootprint(float offset_x, float offset_y, float object_width, float object_depth);
	void setHeight(float height);
	void setGrammar(const cga::Grammar& grammar, const std::vector<float>& params);
	cga::Grammar& grammar() { return _grammar; }
};

class BuildingMass {
private:
	std::vector<ShapeLayer> _layers;
	int _currentLayer;

public:
	BuildingMass();

	void clear();
	void newLayer();
	void alignLayers();
	ShapeLayer& currentLayer() { return _layers[_currentLayer]; }
	void generateGeometry(cga::CGA* system, RenderManager* renderManager);
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
};

