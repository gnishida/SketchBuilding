#pragma once

#include "Grammar.h"

namespace cga {

class OffsetOperator : public Operator {
private:
	std::string offsetDistance;
	int offsetSelector;

public:
	OffsetOperator(const std::string& offsetDistance, int offsetSelector);

	boost::shared_ptr<Shape> apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack);
};

}