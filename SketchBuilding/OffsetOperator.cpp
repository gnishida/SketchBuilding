#include "OffsetOperator.h"
#include "CGA.h"
#include "Shape.h"

namespace cga {

OffsetOperator::OffsetOperator(const std::string& offsetDistance, int offsetSelector) {
	this->name = "offset";
	this->offsetDistance = offsetDistance;
	this->offsetSelector = offsetSelector;
}

boost::shared_ptr<Shape> OffsetOperator::apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar,  std::list<boost::shared_ptr<Shape> >& stack) {
	float actual_offsetDistancet = grammar.evalFloat(offsetDistance, shape);

	return shape->offset(shape->_name, actual_offsetDistancet, offsetSelector);
}

}
