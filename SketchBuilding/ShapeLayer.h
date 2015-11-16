#pragma once

#include "CGA.h"

class ShapeLayer {
public:
	float offset_x;
	float offset_y;
	float object_width;
	float object_depth;
	float height;
	cga::Grammar grammar;

public:
	ShapeLayer() : offset_x(0), offset_y(0), object_width(0), object_depth(0), height(0) {}
	ShapeLayer(float offset_x, float offset_y, float offset_width, float offset_depth, float height, const cga::Grammar& grammar);
};

