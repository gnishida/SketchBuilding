#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "GLUtils.h"

class LayoutExtractor {
public:
	LayoutExtractor() {}

	static std::pair<int, std::vector<float> > extractFacadePattern(int width, int height, const std::vector<std::vector<glm::vec2> >& strokes, const glutils::Face& face, const glm::mat4& mvpMatrix);
	static std::pair<int, std::vector<float> > extractFloorPattern(int width, int height, const std::vector<std::vector<glm::vec2> >& strokes, const glutils::Face& face, const glm::mat4& mvpMatrix);
};

