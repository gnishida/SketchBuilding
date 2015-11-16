#pragma once

#include "Grammar.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cga {

class SizeOperator : public Operator {
private:
	Value xSize;
	Value ySize;
	Value zSize;

public:
	SizeOperator(const Value& xSize, const Value& ySize, const Value& zSize);

	boost::shared_ptr<Shape> apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack);
};

}