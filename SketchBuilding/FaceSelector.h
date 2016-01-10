#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include "GLUtils.h"
#include <boost/shared_ptr.hpp>
#include "Shape.h"

namespace sc {

class Scene;

class FaceSelector {
public:
	sc::Scene* _scene;
	bool _selected;
	boost::shared_ptr<glutils::Face> _selectedFace;
	glutils::Face _selectedFaceCopy;
	std::string _selectedFaceName;

	boost::shared_ptr<cga::Shape> _selectedFaceShape;

public:
	FaceSelector(sc::Scene* scene);

	bool selected();
	boost::shared_ptr<glutils::Face> selectedFace();
	glutils::Face selectedFaceCopy();
	std::string selectedFaceName();
	bool selectFace(const glm::vec3& cameraPos, const glm::vec3& viewDir, const std::string& stage, const glm::vec3& normal);
	void selectFace(int object_id, const boost::shared_ptr<glutils::Face>& face);
	void unselect();
};

}