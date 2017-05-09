#include "Scene.h"
#include "RenderManager.h"
#include "Rectangle.h"
#include <iostream>
#include "GrammarParser.h"
#include <boost/lexical_cast.hpp>
#include "OBJWriter.h"

namespace sc {

SceneObject::SceneObject(Scene* scene) : scene(scene), offset_x(0), offset_y(0), offset_z(0), object_width(0), object_depth(0), height(0) {
	system.modelMat = glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0));

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

void SceneObject::generateGeometry(RenderManager* renderManager, const std::string& stage) {
	faces.clear();

	// if no building mass is created, don't generate geometry.
	if (offset_x == 0 && offset_y == 0 && offset_z == 0) return;

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
	system.stack.push_back(boost::shared_ptr<cga::Shape>(footprint));

	//system->derive(grammar, true);
	if (stage == "final" || stage == "peek_final") {
		system.derive(grammars, scene->default_grammars, true, true);
	}
	else {
		system.derive(grammars, scene->default_grammars, false, true);
	}
	
	system.generateGeometry(faces);
}

/**
* Select only those visible faces and send them to GPU
*/
void SceneObject::updateGeometry(RenderManager* renderManager, const std::string& stage) {
	std::vector<boost::shared_ptr<glutils::Face> > visible_faces;
	for (int i = 0; i < faces.size(); ++i) {
		if (faces[i]->vertices[0].color.a == 1.0f) {
			visible_faces.push_back(faces[i]);
		}
	}

	renderManager->addFaces(visible_faces);
}

Scene::Scene() {
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

void Scene::updateHistory() {
	_history.push_back(_objects);
	if (_history.size() > 10) {
		_history.erase(_history.begin());
	}
}

void Scene::undo() {
	if (_history.size() > 0) {
		_objects = _history.back();
		_history.pop_back();
	}
}

void Scene::alignObjects(float threshold) {
	alignObjects(_currentObject, 0, threshold);
}

void Scene::alignObjects(int currentObject, int controlPoint, float threshold) {
	float min_threshold = 0.0001f;

	// This flag is to check whether the face is already snapped.
	// Note that the face that is already snaped should not be snapped to other faces again at the same time.
	std::vector<bool> snapped(5, false);

	for (int loop = 0; loop < 5; ++loop) {
		float min_dist = std::numeric_limits<float>::max();
		int snap_to_object = -1;
		int snap_type = -1;
		for (int i = 0; i < _objects.size(); ++i) {
			if (i == currentObject) continue;

			float dist;
			if (!snapped[0]) {
				dist = fabs(_objects[currentObject].offset_x - _objects[i].offset_x);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 0;
				}
				dist = fabs(_objects[currentObject].offset_x - _objects[i].offset_x - _objects[i].object_width);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 1;
				}
			}
			if (!snapped[2]) {
				dist = fabs(_objects[currentObject].offset_y - _objects[i].offset_y);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 2;
				}
				dist = fabs(_objects[currentObject].offset_y - _objects[i].offset_y - _objects[i].object_depth);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 3;
				}
			}
			if (!snapped[1]) {
				dist = fabs(_objects[currentObject].offset_x + _objects[currentObject].object_width - _objects[i].offset_x - _objects[i].object_width);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 4;
				}
				dist = fabs(_objects[currentObject].offset_x + _objects[currentObject].object_width - _objects[i].offset_x);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 5;
				}
			}
			if (!snapped[3]) {
				dist = fabs(_objects[currentObject].offset_y + _objects[currentObject].object_depth - _objects[i].offset_y - _objects[i].object_depth);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 6;
				}
				dist = fabs(_objects[currentObject].offset_y + _objects[currentObject].object_depth - _objects[i].offset_y);
				if (dist < min_dist && dist > min_threshold) {
					min_dist = dist;
					snap_to_object = i;
					snap_type = 7;
				}
			}
		}

		if (min_dist >= threshold) break;

		if (fabs(_objects[currentObject].offset_x - _objects[snap_to_object].offset_x) < threshold) {
			float diff = _objects[snap_to_object].offset_x - _objects[currentObject].offset_x;
			if (controlPoint == 0 || controlPoint == 1) {
				_objects[currentObject].offset_x = _objects[snap_to_object].offset_x;
			}
			if (controlPoint == 1) {
				_objects[currentObject].object_width -= diff;
			}
			snapped[0] = true;
		}
		if (fabs(_objects[currentObject].offset_x - _objects[snap_to_object].offset_x - _objects[snap_to_object].object_width) < threshold) {
			float diff = _objects[snap_to_object].offset_x + _objects[snap_to_object].object_width - _objects[currentObject].offset_x;
			if (controlPoint == 0 || controlPoint == 1) {
				_objects[currentObject].offset_x = _objects[snap_to_object].offset_x + _objects[snap_to_object].object_width;
			}
			if (controlPoint == 1) {
				_objects[currentObject].object_width -= diff;
			}
			snapped[0] = true;
		}

		if (fabs(_objects[currentObject].offset_y - _objects[snap_to_object].offset_y) < threshold) {
			float diff = _objects[snap_to_object].offset_y - _objects[currentObject].offset_y;
			if (controlPoint == 0 || controlPoint == 3) {
				_objects[currentObject].offset_y = _objects[snap_to_object].offset_y;
			}
			if (controlPoint == 3) {
				_objects[currentObject].object_depth -= diff;
			}
			snapped[2] = true;
		}
		if (fabs(_objects[currentObject].offset_y - _objects[snap_to_object].offset_y - _objects[snap_to_object].object_depth) < threshold) {
			float diff = _objects[snap_to_object].offset_y + _objects[snap_to_object].object_depth - _objects[currentObject].offset_y;
			if (controlPoint == 0 || controlPoint == 3) {
				_objects[currentObject].offset_y = _objects[snap_to_object].offset_y + _objects[snap_to_object].object_depth;
			}
			if (controlPoint == 3) {
				_objects[currentObject].object_depth -= diff;
			}
			snapped[2] = true;
		}


		if (fabs(_objects[currentObject].offset_x + _objects[currentObject].object_width - _objects[snap_to_object].offset_x - _objects[snap_to_object].object_width) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_x = _objects[snap_to_object].offset_x + _objects[snap_to_object].object_width - _objects[currentObject].object_width;
			}
			if (controlPoint == 2) {
				_objects[currentObject].object_width = _objects[snap_to_object].offset_x + _objects[snap_to_object].object_width - _objects[currentObject].offset_x;
			}
			snapped[1] = true;
		}
		if (fabs(_objects[currentObject].offset_x + _objects[currentObject].object_width - _objects[snap_to_object].offset_x) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_x = _objects[snap_to_object].offset_x - _objects[currentObject].object_width;
			}
			if (controlPoint == 2) {
				_objects[currentObject].object_width = _objects[snap_to_object].offset_x - _objects[currentObject].offset_x;
			}
			snapped[1] = true;
		}

		if (fabs(_objects[currentObject].offset_y + _objects[currentObject].object_depth - _objects[snap_to_object].offset_y - _objects[snap_to_object].object_depth) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_y = _objects[snap_to_object].offset_y + _objects[snap_to_object].object_depth - _objects[currentObject].object_depth;
			}
			if (controlPoint == 4) {
				_objects[currentObject].object_depth = _objects[snap_to_object].offset_y + _objects[snap_to_object].object_depth - _objects[currentObject].offset_y;
			}
			snapped[3] = true;
		}
		if (fabs(_objects[currentObject].offset_y + _objects[currentObject].object_depth - _objects[snap_to_object].offset_y) < threshold) {
			if (controlPoint == 0) {
				_objects[currentObject].offset_y = _objects[snap_to_object].offset_y - _objects[currentObject].object_depth;
			}
			if (controlPoint == 4) {
				_objects[currentObject].object_depth = _objects[snap_to_object].offset_y - _objects[currentObject].offset_y;
			}
			snapped[3] = true;
		}
	}
}

void Scene::alignObjects(const glutils::Face& baseFace, float threshold) {
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


	alignObjects(threshold);
}

/**
 * 各faceについて、screen 座標を計算し、lassoに入る頂点のみで構成される面の面積を計算し、最も面積が大きい面を返却する。
 */
std::pair<int, boost::shared_ptr<glutils::Face> > Scene::findFace(const std::vector<glm::vec2>& lasso, const glm::mat4& mvpMatrix, const glm::vec3& camera_view, int screen_width, int screen_height) {
	float max_area = 0;
	int object_id;
	boost::shared_ptr<glutils::Face> face;

	for (int i = 0; i < _objects.size(); ++i) {
		for (int j = 0; j < _objects[i].faces.size(); ++j) {
			// skip the back face
			if (glm::dot(camera_view, _objects[i].faces[j]->vertices[0].normal) >= 0) continue;

			std::vector<glm::vec2> insideFacePoints;

			for (int k = 0; k < _objects[i].faces[j]->vertices.size() / 3; ++k) {
				if (k == 0) {
					glm::vec4 pp = mvpMatrix * glm::vec4(_objects[i].faces[j]->vertices[k * 3].position, 1);
					glm::vec2 p((pp.x / pp.w * 0.5 + 0.5) * screen_width, (pp.y / pp.w * 0.5 + 0.5) * screen_height);

					// 点pが、lasso内にあるか？
					if (glutils::isWithinPolygon(p, lasso)) {
						insideFacePoints.push_back(p);
					}

					glm::vec4 pp2 = mvpMatrix * glm::vec4(_objects[i].faces[j]->vertices[k * 3 + 1].position, 1);
					glm::vec2 p2((pp2.x / pp2.w * 0.5 + 0.5) * screen_width, (pp2.y / pp2.w * 0.5 + 0.5) * screen_height);

					// 点p2が、lasso内にあるか？
					if (glutils::isWithinPolygon(p2, lasso)) {
						insideFacePoints.push_back(p2);
					}

					glm::vec4 pp3 = mvpMatrix * glm::vec4(_objects[i].faces[j]->vertices[k * 3 + 2].position, 1);
					glm::vec2 p3((pp3.x / pp3.w * 0.5 + 0.5) * screen_width, (pp3.y / pp3.w * 0.5 + 0.5) * screen_height);

					// 点p3が、lasso内にあるか？
					if (glutils::isWithinPolygon(p3, lasso)) {
						insideFacePoints.push_back(p3);
					}
				}
				else {
					glm::vec4 pp = mvpMatrix * glm::vec4(_objects[i].faces[j]->vertices[k * 3 + 2].position, 1);
					glm::vec2 p((pp.x / pp.w * 0.5 + 0.5) * screen_width, (pp.y / pp.w * 0.5 + 0.5) * screen_height);

					// 点pが、lasso内にあるか？
					if (glutils::isWithinPolygon(p, lasso)) {
						insideFacePoints.push_back(p);
					}
				}
			}

			// lasso内の頂点のみで構成される面の面積を計算する
			float area = 0;
			if (insideFacePoints.size() >= 3) {
				area = glutils::area(insideFacePoints);
			}

			if (area > max_area) {
				max_area = area;

				object_id = i;
				face = _objects[i].faces[j];
			}
		}
	}

	return std::make_pair(object_id, face);
}

/**
 * Generate geometry by the grammars, and send the geometry to GPU memory.
 */
void Scene::generateGeometry(RenderManager* renderManager, const std::string& stage) {
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
		_objects[i].generateGeometry(renderManager, stage);
	}

	renderManager->removeObjects();
	for (int i = 0; i < _objects.size(); ++i) {
		_objects[i].updateGeometry(renderManager, stage);
	}

	// if a building is selected, add its control spheres
	if (buildingSelector->isBuildingSelected()) {
		buildingSelector->generateGeometry(renderManager);
	}
}

/**
* Generate geometry by the grammars, and send the geometry to GPU memory.
*/
void Scene::generateGeometry(RenderManager* renderManager, const std::string& stage, int currentObject) {
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
	
	_objects[currentObject].generateGeometry(renderManager, stage);

	renderManager->removeObjects();
	for (int i = 0; i < _objects.size(); ++i) {
		_objects[i].updateGeometry(renderManager, stage);
	}

	// if a building is selected, add its control spheres
	if (buildingSelector->isBuildingSelected()) {
		buildingSelector->generateGeometry(renderManager);
	}
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
}

void Scene::saveGeometry(const std::string& filename) {
	OBJWriter::write(_objects, filename);
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