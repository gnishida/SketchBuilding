#include "SetupProjectionOperator.h"
#include "CGA.h"
#include "Shape.h"

namespace cga {

SetupProjectionOperator::SetupProjectionOperator(int axesSelector, const Value& texWidth, const Value& texHeight) {
	this->name = "setupProjection";
	this->axesSelector = axesSelector;
	this->texWidth = texWidth;
	this->texHeight = texHeight;
}

boost::shared_ptr<Shape> SetupProjectionOperator::apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack) {
	float actual_texWidth;
	float actual_texHeight;

	if (texWidth.type == Value::TYPE_RELATIVE) {
		actual_texWidth = shape->_scope.x * grammar.evalFloat(texWidth.value, shape);
	} else {
		actual_texWidth = grammar.evalFloat(texWidth.value, shape);
	}
	if (texHeight.type == Value::TYPE_RELATIVE) {
		actual_texHeight = shape->_scope.y * grammar.evalFloat(texHeight.value, shape);
	} else {
		actual_texHeight = grammar.evalFloat(texHeight.value, shape);
	}


	shape->setupProjection(axesSelector, actual_texWidth, actual_texHeight);

	return shape;
}

std::string SetupProjectionOperator::to_string() {
	std::string ret = "setupProjection(0, ";

	switch (axesSelector) {
	case AXES_SCOPE_XY:
		ret += "scope.xy";
		break;
	case AXES_SCOPE_XZ:
		ret += "scope.xz";
		break;
	default:
		ret += "?";
		break;
	}
	ret += ", ";

	if (texWidth.type == Value::TYPE_ABSOLUTE) {
		ret += texWidth.value;
	}
	else if (texWidth.type == Value::TYPE_RELATIVE) {
		ret += "'" + texWidth.value;
	}
	else {
		ret += "~" + texWidth.value;
	}
	ret += ", ";

	if (texHeight.type == Value::TYPE_ABSOLUTE) {
		ret += texHeight.value;
	}
	else if (texHeight.type == Value::TYPE_RELATIVE) {
		ret += "'" + texHeight.value;
	}
	else {
		ret += "~" + texHeight.value;
	}
	ret += ")";

	return ret;
}

}
