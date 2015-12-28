#include "CornerCutOperator.h"
#include "CGA.h"
#include "Shape.h"
#include <sstream>

namespace cga {

CornerCutOperator::CornerCutOperator(int type, const std::string& length) {
	this->name = "cornerCut";
	this->type = type;
	this->length = length;
}


boost::shared_ptr<Shape> CornerCutOperator::apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack) {
	float actual_length = grammar.evalFloat(length, shape);
	return shape->cornerCut(shape->_name, type, actual_length);
}

std::string CornerCutOperator::to_string() {
	switch (type) {
	case CORNER_CUT_STRAIGHT:
		return "cornerCut(straight, " + length + ")";
	case CORNER_CUT_CURVE:
		return "cornerCut(curve, " + length + ")";
	case CORNER_CUT_NEGATIVE_CURVE:
		return "cornerCut(negative_curve, " + length + ")";
	default:
		return "cornerCut(?, " + length + ")";
	}
}

}
