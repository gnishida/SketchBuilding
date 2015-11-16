#include "ShapeLOperator.h"
#include "CGA.h"
#include "Shape.h"

namespace cga {

ShapeLOperator::ShapeLOperator(float frontWidth, float leftWidth) {
	this->name = "shapeL";
	this->frontWidth = frontWidth;
	this->leftWidth = leftWidth;
}

boost::shared_ptr<Shape> ShapeLOperator::apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack) {
	return shape->shapeL(shape->_name, frontWidth, leftWidth);
}

}
