#include "CenterOperator.h"
#include "CGA.h"
#include "Shape.h"

namespace cga {

CenterOperator::CenterOperator(int axesSelector) {
	this->name = "center";
	this->axesSelector = axesSelector;
}

boost::shared_ptr<Shape> CenterOperator::apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack) {
	shape->center(axesSelector);

	return shape;
}

std::string CenterOperator::to_string() {
	switch (axesSelector) {
	case AXES_SELECTOR_XYZ:
		return "center(xyz)";
	case AXES_SELECTOR_X:
		return "center(x)";
	case AXES_SELECTOR_Y:
		return "center(y)";
	case AXES_SELECTOR_Z:
		return "center(z)";
	case AXES_SELECTOR_XY:
		return "center(xy)";
	case AXES_SELECTOR_XZ:
		return "center(xz)";
	case AXES_SELECTOR_YZ:
		return "center(yz)";
	default:
		return "center(?)";
	}
}

}
