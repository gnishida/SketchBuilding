#include "CompOperator.h"
#include "CGA.h"
#include "Rectangle.h"
#include "Polygon.h"

namespace cga {

CompOperator::CompOperator(const std::map<std::string, std::string>& name_map) {
	this->name = "comp";
	this->name_map = name_map;
}

boost::shared_ptr<Shape> CompOperator::apply(boost::shared_ptr<Shape>& shape, const Grammar& grammar, std::list<boost::shared_ptr<Shape> >& stack) {
	std::vector<boost::shared_ptr<Shape> > shapes;
	
	shape->comp(name_map, shapes);
	stack.insert(stack.end(), shapes.begin(), shapes.end());

	return boost::shared_ptr<Shape>();
}

std::string CompOperator::to_string() {
	std::string ret = "comp(f) { ";

	int count = 0;
	for (auto it = name_map.begin(); it != name_map.end(); ++it, ++count) {
		if (count > 0) {
			ret += " | ";
		}
		ret += it->first + " : " + it->second;
	}

	ret += " }";

	return ret;
}

}
