#include "Scene.h"
#include "RenderManager.h"
#include "Rectangle.h"
#include <iostream>
#include "GrammarParser.h"
#include <boost/lexical_cast.hpp>

namespace sc {

SceneObject::SceneObject(Scene* scene) : scene(scene), offset_x(0), offset_y(0), object_width(0), object_depth(0), height(0) {
	// set the default grammar for Window, Ledge, and Wall
	try {
		cga::parseGrammar("cga/default_border.xml", grammars["Border"]);
		//cga::parseGrammar("cga/default_ledge.xml", grammars["Ledge"]);
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
	grammars["Start"].attrs["height"].value = boost::lexical_cast<std::string>(height);
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

	//if (height == 0.0f) return;

	if (stage == "final" || stage == "peek_final") {
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
	}
	else {
		// facadeのfloor border sizeを0.08にする
		if (grammars.find("Facade") != grammars.end()) {
			if (grammars["Facade"].attrs.find("z_floor_border_size") != grammars["Facade"].attrs.end()) {
				grammars["Facade"].attrs["z_floor_border_size"].value = "0.08";
			}
		}

		// floorのwindowのborder sizeを0.03にする
		if (grammars.find("Floor") != grammars.end()) {
			if (grammars["Floor"].attrs.find("z_window_border_size") != grammars["Floor"].attrs.end()) {
				grammars["Floor"].attrs["z_window_border_size"].value = "0.03";
			}
		}
	}

	// footprint
	cga::Rectangle* footprint = new cga::Rectangle("Start", "building", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(offset_x, offset_y, offset_z)), glm::mat4(), object_width, object_depth, glm::vec3(1, 1, 1));
	system->stack.push_back(boost::shared_ptr<cga::Shape>(footprint));

	//system->derive(grammar, true);
	if (stage == "final" || stage == "peek_final") {
		system->derive(grammars, scene->default_grammars, true, true);
	}
	else {
		system->derive(grammars, scene->default_grammars, false, true);
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
	_objects.push_back(SceneObject(this));
	_currentObject = 0;

	faceSelector = new FaceSelector(this);
	buildingSelector = new BuildingSelector(this);

	loadDefaultGrammar("cga/paris.xml");
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
	_objects.push_back(SceneObject(this));
	_currentObject = _objects.size() - 1;
}

void Scene::removeObject(int objectId) {
	if (objectId >= _objects.size()) return;

	_objects.erase(_objects.begin() + objectId);
	if (_objects.size() == 0) {
		newObject();
	}
	else {
		if (objectId < _currentObject) {
			_currentObject--;
		}
	}
}

void Scene::alignObjects() {
	alignObjects(_currentObject, 0);
}

void Scene::alignObjects(int currentObject, int controlPoint) {
	float threshold = 0.5f;

	for (int i = 0; i < _objects.size(); ++i) {
		if (i == currentObject) continue;

		if (fabs(_objects[currentObject].offset_x - _objects[i].offset_x) < threshold) {
			float diff = _objects[i].offset_x - _objects[currentObject].offset_x;
			if (controlPoint == 0 || controlPoint == 1) {
				_objects[currentObject].offset_x = _objects[i].offset_x;
			}
			if (controlPoint == 1) {
				_objects[currentObject].object_width -= diff;
			}
		}
		if (fabs(_objects[currentObject].offset_x - _objects[i].offset_x - _objects[i].object_width) < threshold) {
			float diff = _objects[i].offset_x + _objects[i].object_width - _objects[currentObject].offset_x;
			if (controlPoint == 0 || controlPoint == 1) {
				_objects[currentObject].offset_x = _objects[i].offset_x + _objects[i].object_width;
			}
			if (controlPoint == 1) {
				_objects[currentObject].object_width -= diff;
			}
		}

		if (fabs(_objects[currentObject].offset_y - _objects[i].offset_y) < threshold) {
			float diff = _objects[i].offset_y - _objects[currentObject].offset_y;
			if (controlPoint == 0 || controlPoint == 3) {
				_objects[currentObject].offset_y = _objects[i].offset_y;
			}
			if (controlPoint == 3) {
				_objects[currentObject].object_depth -= diff;
			}
		}
		if (fabs(_objects[currentObject].offset_y - _objects[i].offset_y - _objects[i].object_depth) < threshold) {
			float diff = _objects[i].offset_y + _objects[i].object_depth - _objects[currentObject].offset_y;
			if (controlPoint == 0 || controlPoint == 3) {
				_objects[currentObject].offset_y = _objects[i].offset_y + _objects[i].object_depth;
			}
			if (controlPoint == 3) {
				_objects[currentObject].object_depth -= diff;
			}
		}


		if (fabs(_objects[currentObject].offset_x + _objects[currentObject].object_width - _objects[i].offset_x - _objects[i].object_width) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_x = _objects[i].offset_x + _objects[i].object_width - _objects[currentObject].object_width;
			}
			if (controlPoint == 2) {
				_objects[currentObject].object_width = _objects[i].offset_x + _objects[i].object_width - _objects[currentObject].offset_x;
			}
		}
		if (fabs(_objects[currentObject].offset_x + _objects[currentObject].object_width - _objects[i].offset_x) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_x = _objects[i].offset_x - _objects[currentObject].object_width;
			}
			if (controlPoint == 2) {
				_objects[currentObject].object_width = _objects[i].offset_x - _objects[currentObject].offset_x;
			}
		}

		if (fabs(_objects[currentObject].offset_y + _objects[currentObject].object_depth - _objects[i].offset_y - _objects[i].object_depth) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_y = _objects[i].offset_y + _objects[i].object_depth - _objects[currentObject].object_depth;
			}
			if (controlPoint == 4) {
				_objects[currentObject].object_depth = _objects[i].offset_y + _objects[i].object_depth - _objects[currentObject].offset_y;
			}
		}
		if (fabs(_objects[currentObject].offset_y + _objects[currentObject].object_depth - _objects[i].offset_y) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_y = _objects[i].offset_y - _objects[currentObject].object_depth;
			}
			if (controlPoint == 4) {
				_objects[currentObject].object_depth = _objects[i].offset_y - _objects[currentObject].offset_y;
			}
		}
	}
}

void Scene::alignObjects(const glutils::Face& baseFace) {
	if (_objects[_currentObject].offset_x < baseFace.bbox.minPt.x) {
		float diff = baseFace.bbox.minPt.x - _objects[_currentObject].offset_x;
		_objects[_currentObject].offset_x = baseFace.bbox.minPt.x;
		_objects[_currentObject].object_width -= diff;
	}
	if (_objects[_currentObject].offset_x > baseFace.bbox.maxPt.x) {
		_objects[_currentObject].offset_x = baseFace.bbox.maxPt.x - 1.0f;
		_objects[_currentObject].object_width = 1.0f;
	}

	if (_objects[_currentObject].offset_y < -baseFace.bbox.maxPt.z) {
		float diff = -baseFace.bbox.maxPt.z - _objects[_currentObject].offset_y;
		_objects[_currentObject].offset_y = -baseFace.bbox.maxPt.z;
		_objects[_currentObject].object_depth -= diff;
	}
	if (_objects[_currentObject].offset_y > -baseFace.bbox.minPt.z) {
		_objects[_currentObject].offset_y = -baseFace.bbox.minPt.z - 1.0f;
		_objects[_currentObject].object_depth = 1.0f;
	}

	if (_objects[_currentObject].offset_x + _objects[_currentObject].object_width > baseFace.bbox.maxPt.x) {
		_objects[_currentObject].object_width = baseFace.bbox.maxPt.x - _objects[_currentObject].offset_x;
	}

	if (_objects[_currentObject].offset_y + _objects[_currentObject].object_depth > -baseFace.bbox.minPt.z) {
		_objects[_currentObject].object_depth = -baseFace.bbox.minPt.z - _objects[_currentObject].offset_y;
	}


	alignObjects();
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
/*
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
*/

/*void Scene::unselectFace() {
	if (faceSelector.selected()) {
		_selectedFace->unselect();
		_selectedFace.reset();
	}
}*/

/**
 * Generate geometry by the grammars, and send the geometry to GPU memory.
 */
void Scene::generateGeometry(RenderManager* renderManager, const std::string& stage) {
	renderManager->removeObjects();

	// Since the geometry will be updated, the pointer to a face will not be valid any more.
	//faceSelector->unselect();

	if (stage == "final" || stage == "peek_final") {
		// facadeのfloor border sizeを0にする
		if (default_grammars.find("Facade") != default_grammars.end()) {
			if (default_grammars["Facade"].attrs.find("z_floor_border_size") != default_grammars["Facade"].attrs.end()) {
				default_grammars["Facade"].attrs["z_floor_border_size"].value = "0";
			}
		}

		// floorのwindowのborder sizeを0にする
		if (default_grammars.find("Floor") != default_grammars.end()) {
			if (default_grammars["Floor"].attrs.find("z_window_border_size") != default_grammars["Floor"].attrs.end()) {
				default_grammars["Floor"].attrs["z_window_border_size"].value = "0";
			}
		}
	}
	else {
		// facadeのfloor border sizeを0.08にする
		if (default_grammars.find("Facade") != default_grammars.end()) {
			if (default_grammars["Facade"].attrs.find("z_floor_border_size") != default_grammars["Facade"].attrs.end()) {
				default_grammars["Facade"].attrs["z_floor_border_size"].value = "0.08";
			}
		}

		// floorのwindowのborder sizeを0.03にする
		if (default_grammars.find("Floor") != default_grammars.end()) {
			if (default_grammars["Floor"].attrs.find("z_window_border_size") != default_grammars["Floor"].attrs.end()) {
				default_grammars["Floor"].attrs["z_window_border_size"].value = "0.03";
			}
		}
	}


	for (int i = 0; i < _objects.size(); ++i) {
		_objects[i].generateGeometry(&system, renderManager, stage);
	}

	// if a building is selected, add its control spheres
	if (buildingSelector->isBuildingSelected()) {
		buildingSelector->generateGeometry(renderManager);
	}

	// add a ground plane
	std::vector<Vertex> vertices;
	glutils::drawGrid(50, 50, 2.5, glm::vec4(0, 0, 0, 1), glm::vec4(1, 1, 1, 1), system.modelMat, vertices);
	renderManager->addObject("grid", "", vertices, false);
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

	// if a building is selected, add its control spheres
	if (buildingSelector->isBuildingSelected()) {
		buildingSelector->generateGeometry(renderManager);
	}

	// add a ground plane
	std::vector<Vertex> vertices;
	glutils::drawGrid(50, 50, 2.5, glm::vec4(0, 0, 0, 1), glm::vec4(1, 1, 1, 1), system.modelMat, vertices);
	renderManager->addObject("grid", "", vertices, false);
}

void Scene::saveGeometry(const std::string& filename) {
	FILE* fp = fopen(filename.c_str(), "w");

	for (int i = 0; i < _objects.size(); ++i) {
		for (int j = 0; j < _objects[i].faces.size(); ++j) {
			for (int k = 0; k < _objects[i].faces[j]->vertices.size(); ++k) {
				fprintf(fp, "v %lf %lf %lf\n", _objects[i].faces[j]->vertices[k].position.x, _objects[i].faces[j]->vertices[k].position.y, _objects[i].faces[j]->vertices[k].position.z);
			}
		}
	}
	fprintf(fp, "\n");

	int vertexId = 1;
	for (int i = 0; i < _objects.size(); ++i) {
		for (int j = 0; j < _objects[i].faces.size(); ++j) {
			for (int k = 0; k < _objects[i].faces[j]->vertices.size() / 3; ++k) {
				fprintf(fp, "f ");
				for (int l = 0; l < 3; ++l) {
					if (l > 0) {
						fprintf(fp, " ");
					}
					fprintf(fp, "%d", vertexId++);
				}
				fprintf(fp, "\n");
			}
		}
	}

	fclose(fp);
}

void Scene::loadDefaultGrammar(const std::string& default_grammar_file) {
	this->default_grammar_file = default_grammar_file;

	// parse the file
	QFile file(default_grammar_file.c_str());

	QDomDocument doc;
	doc.setContent(&file, true);
	QDomElement root = doc.documentElement();

	QDomNode child_node = root.firstChild();
	while (!child_node.isNull()) {
		if (child_node.toElement().tagName() == "default_grammar") {
			if (!child_node.toElement().hasAttribute("type")) {
				throw "<default_grammar> tag must contain type attribute.";
			}
			if (!child_node.toElement().hasAttribute("file")) {
				throw "<default_grammar> tag must contain file attribute.";
			}

			QString type = child_node.toElement().attribute("type");
			QString filename = child_node.toElement().attribute("file");

			if (type == "material") {
				cga::parseGrammar(filename.toUtf8().constData(), default_grammars["Material"]);
			}
			else {
				cga::Grammar defaultGrammar;
				cga::parseGrammar(filename.toUtf8().constData(), defaultGrammar);
				setDefaultGrammar(type.toUtf8().constData(), defaultGrammar);
			}
		}

		child_node = child_node.nextSibling();
	}
}

void Scene::setDefaultGrammar(const std::string& name, const cga::Grammar& grammar) {
	default_grammars[name] = grammar;

	// rewrite the axiom to the name
	for (auto it = default_grammars[name].rules.begin(); it != default_grammars[name].rules.end(); ++it) {
		if (it->first == "Start") {
			cga::Rule rule = it->second;
			default_grammars[name].rules.erase(it->first);
			default_grammars[name].rules[name] = rule;
			break;
		}
	}
}

}