#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GLUtils.h"
#include <boost/shared_ptr.hpp>

namespace sc {

class Scene;

class FaceSelector {
private:
	sc::Scene* _scene;
	bool _selected;
	boost::shared_ptr<glutils::Face> _selectedFace;
	glutils::Face _selectedFaceCopy;
	std::string _selectedFaceName;

public:
	FaceSelector(sc::Scene* scene);

	bool selected();
	boost::shared_ptr<glutils::Face> selectedFace();
	glutils::Face selectedFaceCopy();
	std::string selectedFaceName();
	bool selectFace(const glm::vec3& cameraPos, const glm::vec3& viewDir, const std::string& stage, const glm::vec3& normal);
	void unselect();
};

}