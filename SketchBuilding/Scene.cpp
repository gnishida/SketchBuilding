#include "Scene.h"
#include "RenderManager.h"
#include "Rectangle.h"
#include <iostream>
#include "GrammarParser.h"
#include <boost/lexical_cast.hpp>

namespace sc {

SceneObject::SceneObject() : offset_x(0), offset_y(0), object_width(0), object_depth(0), height(0) {
	// set the default grammar for Window, Ledge, and Wall
	try {
		cga::parseGrammar("cga/default_border.xml", grammars["Border"]);
		//cga::parseGrammar("cga/default_window.xml", grammars["Window"]);
		cga::parseGrammar("cga/default_ledge.xml", grammars["Ledge"]);
		//cga::parseGrammar("cga/default_wall.xml", grammars["Wall"]);
		//cga::parseGrammar("cga/default_roof_ledge.xml", grammars["RoofLedge"]);
		//cga::parseGrammar("cga/default_roof_top.xml", grammars["RoofTop"]);
		//cga::parseGrammar("cga/default_ledge_face.xml", grammars["LedgeFace"]);
		//cga::parseGrammar("cga/default_window_sill.xml", grammars["WindowSill"]);
		//cga::parseGrammar("cga/default_window_frame.xml", grammars["WindowFrame"]);
		//cga::parseGrammar("cga/default_window_glass.xml", grammars["WindowGlass"]);
		//cga::parseGrammar("cga/default_window_shutter_frame.xml", grammars["WindowShutterFrame"]);
	}
	catch (const std::string& ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
	catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
}

void SceneObject::setFootprint(float offset_x, float offset_y, float offset_z, float object_width, float object_depth) {
	this->offset_x = offset_x;
	this->offset_y = offset_y;
	this->offset_z = offset_z;
	this->object_width = object_width;
	this->object_depth = object_depth;
}

void SceneObject::setHeight(float height) {
	this->height = height;

	// HACK: 高さパラメータを設定する
	grammars["building"].attrs["height"].value = boost::lexical_cast<std::string>(height);
}

void SceneObject::setGrammar(const std::string& name, const cga::Grammar& grammar) {
	grammars[name] = grammar;

	// rewrite the axiom to the name
	for (auto it = grammars[name].rules.begin(); it != grammars[name].rules.end(); ++it) {
		if (it->first == "Start") {
			cga::Rule rule = it->second;
			grammars[name].rules.erase(it->first);
			grammars[name].rules[name] = rule;
			break;
		}
	}
}

void SceneObject::setGrammar(const std::string& name, const cga::Grammar& grammar, const std::vector<float>& params, bool normalized) {
	grammars[name] = grammar;
	cga::CGA::setParamValues(grammars[name], params, normalized);

	// rewrite the axiom to the name
	for (auto it = grammars[name].rules.begin(); it != grammars[name].rules.end(); ++it) {
		if (it->first == "Start") {
			cga::Rule rule = it->second;
			grammars[name].rules.erase(it->first);
			grammars[name].rules[name] = rule;
			break;
		}
	}
}

void SceneObject::generateGeometry(cga::CGA* system, RenderManager* renderManager, const std::string& stage) {
	faces.clear();

	if (height == 0.0f) return;

	if (stage == "final") {
		// facadeのfloor border sizeを0にする
		if (grammars.find("Facade") != grammars.end()) {
			if (grammars["Facade"].attrs.find("z_floor_border_size") != grammars["Facade"].attrs.end()) {
				grammars["Facade"].attrs["z_floor_border_size"].value = "0";
			}
		}

		// floorのwindowのborder sizeを0にする
		if (grammars.find("Floor") != grammars.end()) {
			if (grammars["Floor"].attrs.find("z_window_border_size") != grammars["Floor"].attrs.end()) {
				grammars["Floor"].attrs["z_window_border_size"].value = "0";
			}
		}

		// apply the color/texture
		cga::parseGrammar("cga/default_wall.xml", grammars["Wall"]);
		cga::parseGrammar("cga/default_roof_ledge.xml", grammars["RoofLedge"]);
		cga::parseGrammar("cga/default_roof_top.xml", grammars["RoofTop"]);
		cga::parseGrammar("cga/default_ledge_face.xml", grammars["LedgeFace"]);
		cga::parseGrammar("cga/default_window_sill.xml", grammars["WindowSill"]);
		cga::parseGrammar("cga/default_window_frame.xml", grammars["WindowFrame"]);
		cga::parseGrammar("cga/default_window_glass.xml", grammars["WindowGlass"]);
		cga::parseGrammar("cga/default_window_shutter_frame.xml", grammars["WindowShutterFrame"]);
	}

	// footprint
	cga::Rectangle* footprint = new cga::Rectangle("Start", "building", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(offset_x, offset_y, offset_z)), glm::mat4(), object_width, object_depth, glm::vec3(1, 1, 1));
	system->stack.push_back(boost::shared_ptr<cga::Shape>(footprint));

	//system->derive(grammar, true);
	if (stage == "final") {
		system->derive(grammars, false, true);
	}
	else {
		system->derive(grammars, true, true);
	}
	
	system->generateGeometry(faces);

	//renderManager->addFaces(faces);
	updateGeometry(renderManager, stage);
}

void SceneObject::updateGeometry(RenderManager* renderManager, const std::string& stage) {
	for (int i = 0; i < faces.size(); ++i) {
		bool transparent = false;

		if (stage == "") {
			transparent = false;
		}
		else if (stage == "building") {
			if (faces[i]->grammar_type != "building") {
				transparent = true;
			}
		}
		else if (stage == "roof") {
			if (faces[i]->grammar_type == "roof") {
				transparent = true;
			}
		}
		else if (stage == "facade") {
			if (faces[i]->grammar_type != "building" && faces[i]->grammar_type != "roof") {
				transparent = true;
			}
		}
		else if (stage == "floor") {
			if (faces[i]->grammar_type != "building" && faces[i]->grammar_type != "roof" && faces[i]->grammar_type != "facade") {
				transparent = true;
			}
		}
		else if (stage == "window") {
			if (faces[i]->grammar_type == "window") {
				transparent = true;
			}
		}
		else if (stage == "ledge") {
			if (faces[i]->grammar_type == "ledge") {
				transparent = true;
			}
		}

		if (transparent) {
			for (int j = 0; j < faces[i]->vertices.size(); ++j) {
				faces[i]->vertices[j].color.a = 0.5f;
			}
		} else {
			for (int j = 0; j < faces[i]->vertices.size(); ++j) {
				faces[i]->vertices[j].color.a = 1.0f;
			}
		}
	}

	renderManager->addFaces(faces);
}

Scene::Scene() {
	system.modelMat = glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0));
	_objects.resize(1);
	_currentObject = 0;
}

void Scene::clear() {
	_objects.clear();
	newObject();
}

void Scene::clearCurrentObject() {
	_objects.erase(_objects.begin() + _currentObject);
	newObject();
}

void Scene::newObject() {
	_objects.push_back(SceneObject());
	_currentObject = _objects.size() - 1;
}

void Scene::alignObjects() {
	for (int i = 0; i < _objects.size(); ++i) {
		if (i == _currentObject) continue;

		if (fabs(_objects[_currentObject].offset_x - _objects[i].offset_x) < 2.0f) {
			_objects[_currentObject].offset_x = _objects[i].offset_x;
		}
		if (fabs(_objects[_currentObject].offset_y - _objects[i].offset_y) < 2.0f) {
			_objects[_currentObject].offset_y = _objects[i].offset_y;
		}
		if (fabs(_objects[_currentObject].offset_x + _objects[_currentObject].object_width - _objects[i].offset_x - _objects[i].object_width) < 2.0f) {
			_objects[_currentObject].object_width = _objects[i].offset_x + _objects[i].object_width - _objects[_currentObject].offset_x;
		}
		if (fabs(_objects[_currentObject].offset_y + _objects[_currentObject].object_depth - _objects[i].offset_y - _objects[i].object_depth) < 2.0f) {
			_objects[_currentObject].object_depth = _objects[i].offset_y + _objects[i].object_depth - _objects[_currentObject].offset_y;
		}

		if (fabs(_objects[_currentObject].offset_x - _objects[i].offset_x - _objects[i].object_width) < 2.0f) {
			_objects[_currentObject].offset_x = _objects[i].offset_x + _objects[i].object_width;
		}
		if (fabs(_objects[_currentObject].offset_y - _objects[i].offset_y - _objects[i].object_depth) < 2.0f) {
			_objects[_currentObject].offset_y = _objects[i].offset_y + _objects[i].object_depth;
		}
		if (fabs(_objects[_currentObject].offset_x + _objects[_currentObject].object_width - _objects[i].offset_x) < 2.0f) {
			_objects[_currentObject].object_width = _objects[i].offset_x - _objects[_currentObject].offset_x;
		}
		if (fabs(_objects[_currentObject].offset_y + _objects[_currentObject].object_depth - _objects[i].offset_y) < 2.0f) {
			_objects[_currentObject].object_depth = _objects[i].offset_y - _objects[_currentObject].offset_y;
		}
	}
}

void Scene::alignObjectsForWillisTower() {
	// 最下層
	if (_currentObject == 0) {
		float wh = (_objects[_currentObject].object_width + _objects[_currentObject].object_depth) * 0.5f;
		_objects[_currentObject].object_width = wh;
		_objects[_currentObject].object_depth = wh;
	}
	
	// １つ目の層
	if (_currentObject == 1) {
		_objects[_currentObject].offset_x = _objects[0].offset_x;
		_objects[_currentObject].offset_y = _objects[0].offset_y + _objects[0].object_depth / 2;
		_objects[_currentObject].object_width = _objects[0].object_width / 2;
		_objects[_currentObject].object_depth = _objects[0].object_depth / 2;
	}
	else if (_currentObject == 2) {
		_objects[_currentObject].offset_x = _objects[0].offset_x;
		_objects[_currentObject].offset_y = _objects[0].offset_y;
		_objects[_currentObject].object_width = _objects[0].object_width / 2;
		_objects[_currentObject].object_depth = _objects[0].object_depth / 2;
	}
	else if (_currentObject == 3) {
		_objects[_currentObject].offset_x = _objects[0].offset_x + _objects[0].object_width / 2;
		_objects[_currentObject].offset_y = _objects[0].offset_y + _objects[0].object_depth / 2;
		_objects[_currentObject].object_width = _objects[0].object_width / 2;
		_objects[_currentObject].object_depth = _objects[0].object_depth / 2;
		_objects[_currentObject].setHeight(_objects[1].height);
	}
	else if (_currentObject == 4) {
		_objects[_currentObject].offset_x = _objects[0].offset_x + _objects[0].object_width / 2;
		_objects[_currentObject].offset_y = _objects[0].offset_y;
		_objects[_currentObject].object_width = _objects[0].object_width / 2;
		_objects[_currentObject].object_depth = _objects[0].object_depth / 2;
		_objects[_currentObject].setHeight(_objects[1].height);
	}

	// 2つめの層
	if (_currentObject == 5) {
		_objects[_currentObject].offset_x = _objects[0].offset_x;
		_objects[_currentObject].offset_y = _objects[0].offset_y + _objects[0].object_depth / 2;
		_objects[_currentObject].object_width = _objects[0].object_width / 2;
		_objects[_currentObject].object_depth = _objects[0].object_depth / 2;
	}

	// 3つめの層
	if (_currentObject == 6) {
		_objects[_currentObject].offset_x = _objects[0].offset_x;
		_objects[_currentObject].offset_y = _objects[0].offset_y + _objects[0].object_depth / 2;
		_objects[_currentObject].object_width = _objects[0].object_width / 2;
		_objects[_currentObject].object_depth = _objects[0].object_depth / 2;
	}
}

/**
* Select a face by hit testing.
*
* @param p			the point that emits ray.
* @param v			the ray vector
*/
bool Scene::selectFace(const glm::vec3& p, const glm::vec3& v, const std::string& stage, const glm::vec3& normal) {
	glm::vec3 intPt;
	float min_dist = (std::numeric_limits<float>::max)();

	unselectFace();

	for (int i = 0; i < _objects.size(); ++i) {
		for (int j = 0; j < _objects[i].faces.size(); ++j) {
			if (_objects[i].faces[j]->vertices.size() < 3) continue;

			// check the face's type
			if (stage == "building") {
				if (_objects[i].faces[j]->grammar_type != "building") continue;
			}
			else if (stage == "roof") {
				if (_objects[i].faces[j]->grammar_type != "building") continue;
			}
			else if (stage == "facade") {
				if (_objects[i].faces[j]->grammar_type != "building") continue;
			}
			else if (stage == "floor") {
				if (_objects[i].faces[j]->grammar_type != "facade") continue;
			}
			else if (stage == "window") {
				if (_objects[i].faces[j]->grammar_type != "floor") continue;
			}
			else if (stage == "ledge") {
				if (_objects[i].faces[j]->grammar_type != "facade") continue;
			}

			for (int k = 0; k < _objects[i].faces[j]->vertices.size(); k += 3) {
				if (fabs(glm::dot(_objects[i].faces[j]->vertices[0].normal, normal)) < 0.99f) continue;

				if (glutils::rayTriangleIntersection(p, v, _objects[i].faces[j]->vertices[k].position, _objects[i].faces[j]->vertices[k + 1].position, _objects[i].faces[j]->vertices[k + 2].position, intPt)) {
					float dist = glm::length(intPt - p);

					if (dist < min_dist) {
						min_dist = dist;
						_selectedFace = _objects[i].faces[j];
						_currentObject = i;
					}
				}

			}
		}
	}

	if (_selectedFace) {
		//_selectedFace->select();
		_selectedFaceName = _selectedFace->name;
		return true;
	}
	else {
		_selectedFaceName = "";
		return false;
	}
}

void Scene::unselectFace() {
	if (_selectedFace) {
		_selectedFace->unselect();
		_selectedFace.reset();
	}
}

/**
 * Generate geometry by the grammars, and send the geometry to GPU memory.
 */
void Scene::generateGeometry(RenderManager* renderManager, const std::string& stage) {
	renderManager->removeObjects();

	// Since the geometry will be updated, the pointer to a face will not be valid any more.
	unselectFace();

	for (int i = 0; i < _objects.size(); ++i) {
		_objects[i].generateGeometry(&system, renderManager, stage);
	}

	std::vector<Vertex> vertices;
	glutils::drawGrid(50, 50, 2.5, glm::vec4(0, 0, 1, 1), glm::vec4(1, 1, 1, 1), system.modelMat, vertices);
	renderManager->addObject("grid", "", vertices);
}

/**
 * Send the geometry to GPU memory. 
 * Note that the re-derivation by the grammars is not performed. Instead, already generated faces are used.
 */
void Scene::updateGeometry(RenderManager* renderManager, const std::string& stage) {
	renderManager->removeObjects();

	for (int i = 0; i < _objects.size(); ++i) {
		_objects[i].updateGeometry(renderManager, stage);
	}

	std::vector<Vertex> vertices;
	glutils::drawGrid(50, 50, 2.5, glm::vec4(0, 0, 1, 1), glm::vec4(1, 1, 1, 1), system.modelMat, vertices);
	renderManager->addObject("grid", "", vertices);

}

}