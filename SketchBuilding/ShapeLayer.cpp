#include "ShapeLayer.h"

ShapeLayer::ShapeLayer(float offset_x, float offset_y, float offset_width, float offset_depth, float height, const cga::Grammar& grammar) {
	this->offset_x = offset_x;
	this->offset_y = offset_y;
	this->object_width = object_width;
	this->object_depth = object_depth;
	this->height = height;
	this->grammar = grammar;
}

