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
	/*std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution(0.0, 1.0);*/

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

/**
 * Execute a derivation of the grammar
 */
void CGA::derive(const Grammar& grammar, bool suppressWarning) {
	shapes.clear();

	while (!stack.empty()) {
		boost::shared_ptr<Shape> shape = stack.front();
		stack.pop_front();

		if (grammar.contain(shape->_name)) {
			grammar.getRule(shape->_name).apply(shape, grammar, stack);
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
void CGA::generateGeometry(RenderManager* renderManager) {
	for (int i = 0; i < shapes.size(); ++i) {
		shapes[i]->generateGeometry(renderManager, 1.0f);
	}
}

}
