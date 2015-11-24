#include "CGA.h"
#include "GLUtils.h"
#include "OBJLoader.h"
#include <map>
#include <iostream>
#include <random>
#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace cga {

CGA::CGA() {
}

/**
 * Randomly select parameter values if the range is specified for the parameter
 */
std::vector<float> CGA::randomParamValues(Grammar& grammar) {
	std::vector<float> param_values;

	for (auto it = grammar.attrs.begin(); it != grammar.attrs.end(); ++it) {
		if (it->second.hasRange) {
			float r = (float)rand() / RAND_MAX;
			float v = r * (it->second.range_end - it->second.range_start) + it->second.range_start;
			it->second.value = boost::lexical_cast<std::string>(v);
			//param_values.push_back(v);
			param_values.push_back(r);
		}
	}

	return param_values;
}

std::vector<std::pair<float, float> > CGA::getParamRanges(const Grammar& grammar) {
	std::vector<std::pair<float, float> > ranges;

	for (auto it = grammar.attrs.begin(); it != grammar.attrs.end(); ++it) {
		if (it->second.hasRange) {
			ranges.push_back(std::make_pair(it->second.range_start, it->second.range_end));
		}
	}

	return ranges;

}

/**
* Set parameter values.
* Each value is normalized to [0, 1], so it has to be populated based on the range.
* If the parameter value is out of [0, 1], it is forced to be between [0, 1].
*/
void CGA::setParamValues(Grammar& grammar, const std::vector<float>& params) {
	int count = 0;
	for (auto it = grammar.attrs.begin(); it != grammar.attrs.end(); ++it, ++count) {
		if (it->second.hasRange) {
			float param = std::min(1.0f, std::max(0.0f, params[count]));

			grammar.attrs[it->first].value = boost::lexical_cast<std::string>((it->second.range_end - it->second.range_start) * param + it->second.range_start);
		}
	}
}

/**
 * Execute a derivation of the grammar
 */
void CGA::derive(const Grammar& grammar, bool suppressWarning) {
	shapes.clear();

	while (!stack.empty()) {
		boost::shared_ptr<Shape> shape = stack.front();
		stack.pop_front();

		if (grammar.contain(shape->_name)) {
			grammar.getRule(shape->_name).apply(shape, grammar, stack, shapes);
		} else {
			if (!suppressWarning && shape->_name.back() != '!' && shape->_name.back() != '.') {
				std::cout << "Warning: " << "no rule is found for " << shape->_name << "." << std::endl;
			}
			shapes.push_back(shape);
		}
	}
}

void CGA::derive(const std::map<std::string, Grammar>& grammars, bool suppressWarning) {
	shapes.clear();

	std::vector<boost::shared_ptr<Shape> > inactive_shapes;

	while (!stack.empty()) {
		boost::shared_ptr<Shape> shape = stack.front();
		stack.pop_front();

		bool found = false;
		std::string name;
		for (auto it = grammars.begin(); it != grammars.end(); ++it) {
			if (it->second.contain(shape->_name)) {
				found = true;
				name = it->first;
				break;
			}
		}
		
		if (found) {
			// if the shape's grammar is different from the grammar that is selected for this shape,
			// this shape is marked as axiom, and is put into the shape list.
			// This shape will be used when the user select a face,on which she will work.
			if (shape->_grammar_type != grammars.at(name).type) {
				boost::shared_ptr<Shape> copiedShape = shape->clone(shape->_name);
				copiedShape->translate(MODE_RELATIVE, COORD_SYSTEM_OBJECT, 0, 0, -0.03);
				copiedShape->_axiom = true;
				shapes.push_back(copiedShape);
			}

			shape->_grammar_type = grammars.at(name).type;
			grammars.at(name).getRule(shape->_name).apply(shape, grammars.at(name), stack, shapes);
		} else {
			if (!suppressWarning && shape->_name.back() != '!' && shape->_name.back() != '.') {
				std::cout << "Warning: " << "no rule is found for " << shape->_name << "." << std::endl;
			}
			shapes.push_back(shape);
		}
	}
}

/**
 * Generate a geometry and add it to the render manager.
 */
void CGA::generateGeometry(std::vector<boost::shared_ptr<glutils::Face> >& faces) {
	for (int i = 0; i < shapes.size(); ++i) {
		shapes[i]->generateGeometry(faces, 1.0f);
	}
}

}
