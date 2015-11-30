#include <iostream>
#include "GLWidget3D.h"
#include "MainWindow.h"
#include <GL/GLU.h>
#include <QDir>
#include <QFileInfoList>
#include "OBJLoader.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "GrammarParser.h"
#include "Rectangle.h"
#include "GLUtils.h"
#include "Regression.h"
#include "Classifier.h"
#include "MCMC.h"
#include <time.h>
#include "LeftWindowItemWidget.h"
#include "Scene.h"

#ifndef M_PI
#define M_PI	3.141592653
#endif

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
	mainWin = (MainWindow*)parent;
	dragging = false;
	ctrlPressed = false;
	demo_mode = 0;

	// This is necessary to prevent the screen overdrawn by OpenGL
	setAutoFillBackground(false);

	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-100, 100, -100, 100, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;

	std::string stage_names[6] = { "building", "roof", "facade", "floor", "window", "ledge" };

	// load grammars and their thumbnail images
	for (int i = 0; i < 6; ++i) {
		{
			QStringList filters;
			filters << "*.xml";
			QFileInfoList fileInfoList = QDir(std::string("cga/" + stage_names[i] + "/").c_str()).entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
			for (auto fileInfo : fileInfoList) {
				cga::Grammar grammar;
				try {
					std::cout << "Load gramar: " << fileInfo.absoluteFilePath().toUtf8().constData() << std::endl;
					cga::parseGrammar(fileInfo.absoluteFilePath().toUtf8().constData(), grammar);
					grammars[stage_names[i]].push_back(grammar);
				}
				catch (const std::string& ex) {
					std::cout << "ERROR:" << std::endl << ex << std::endl;
				}
				catch (const char* ex) {
					std::cout << "ERROR:" << std::endl << ex << std::endl;
				}
			}
		}

		// load thumbnail for each grammar
		{
			QStringList filters;
			filters << "*.png" << "*.jpg" << "*.bmp";
			QFileInfoList fileInfoList = QDir(std::string("cga/" + stage_names[i] + "/").c_str()).entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
			for (auto fileInfo : fileInfoList) {
				grammarImages[stage_names[i]].push_back(QImage(fileInfo.absoluteFilePath()).scaled(LeftWindowItemWidget::IMAGE_WIDTH, LeftWindowItemWidget::IMAGE_HEIGHT));
			}
		}
	}

	// initialize deep learning network
	classifiers["building"] = new Classifier("models/building/building.prototxt", "models/building/building.caffemodel", "models/building/mean.binaryproto");

	regressions["building"].resize(4);
	regressions["building"][0] = new Regression("models/building/building_01.prototxt", "models/building/building_01.caffemodel");
	regressions["building"][1] = new Regression("models/building/building_02.prototxt", "models/building/building_02.caffemodel");
	regressions["building"][2] = new Regression("models/building/building_03.prototxt", "models/building/building_03.caffemodel");
	regressions["building"][3] = new Regression("models/building/building_04.prototxt", "models/building/building_04.caffemodel");

	mcmc = new MCMC(this);
}

void GLWidget3D::drawLineTo(const QPoint &endPoint) {
	QPoint pt1(lastPos.x(), lastPos.y());
	QPoint pt2(endPoint.x(), endPoint.y());

	QPainter painter(&sketch);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.drawLine(pt1, pt2);

	strokes.back().push_back(glm::vec2(endPoint.x(), endPoint.y()));

	lastPos = endPoint;
}

/**
 * Clear the canvas.
 */
void GLWidget3D::clearSketch() {
	sketch.fill(qRgba(255, 255, 255, 255));
	strokes.clear();

	update();
}

void GLWidget3D::clearGeometry() {
	scene.clear();
	scene.generateGeometry(&renderManager, stage);
	update();
}

/**
* Draw the scene.
*/
void GLWidget3D::drawScene(int drawMode) {
	if (drawMode == 0) {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);
	}
	else {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 1);
	}

	renderManager.renderAll();
}

/**
 * Load a grammar from a file and generate a 3d geometry.
 * This is only for test usage.
 */
void GLWidget3D::loadCGA(char* filename) {
	renderManager.removeObjects();
	
	try {
		cga::Grammar grammar;
		cga::parseGrammar(filename, grammar);
		cga::CGA::randomParamValues(grammar);

		sc::SceneObject so;
		so.setFootprint(0, 0, 0, 16, 12);

		std::vector<float> params(10);
		for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;
		so.setGrammar("Start", grammar, params);
		//so.setGrammar(grammar, params);

		cga::CGA system;
		so.generateGeometry(&system, &renderManager, "");
	}
	catch (const std::string& ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
	catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}

	update();
}

void GLWidget3D::selectOption(int option_index) {
	if (stage == "building") {
		predictBuilding(option_index);
	} else if (stage == "roof") {
		predictRoof(option_index);
	} else if (stage == "facade") {
		if (!scene._selectedFaceName.empty()) {
			scene.currentObject().setGrammar(scene._selectedFaceName, grammars["facade"][option_index]);
			scene.generateGeometry(&renderManager, "facade");
		}
	} else if (stage == "floor") {
		if (!scene._selectedFaceName.empty()) {
			scene.currentObject().setGrammar(scene._selectedFaceName, grammars["floor"][option_index]);
			scene.generateGeometry(&renderManager, "floor");
		}
	} else if (stage == "window") {
		predictWindow(option_index);
	} else if (stage == "ledge") {
		predictLedge(option_index);
	}

	update();
}

void GLWidget3D::updateBuildingOptions() {
	mainWin->thumbsList->clear();

	QImage swapped = sketch.scaled(256, 256).rgbSwapped();
	cv::Mat img = cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();

	std::vector<Prediction> predictions = classifiers["building"]->Classify(img, 10);
	
	QPainter painter(&sketch);
	int bestIndex;
	for (int i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		mainWin->addListItem(QString::number(p.second, 'f', 3), grammarImages["building"][p.first], p.first);

		if (i == 0) {
			bestIndex = p.first;
		}
	}

	/*
	//////////////////////// DEBUG ////////////////////////
	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["building"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());

	QPainter painter(&sketch);
	for (int i = 0; i < grammarImages["building"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["building"][indexes[i]], indexes[i]);
	}
	int bestIndex = 0;
	//////////////////////// DEBUG ////////////////////////
	*/

	predictBuilding(bestIndex);

	update();
}

void GLWidget3D::updateRoofOptions() {
	mainWin->thumbsList->clear();

	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["roof"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());

	QPainter painter(&sketch);
	for (int i = 0; i < grammarImages["roof"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["roof"][indexes[i]], indexes[i]);
	}

	predictRoof(0);

	update();
}

void GLWidget3D::updateFacadeOptions() {
	mainWin->thumbsList->clear();

	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["facade"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());

	QPainter painter(&sketch);
	for (int i = 0; i < grammarImages["facade"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["facade"][indexes[i]], indexes[i]);
	}

	predictFacade(0);

	update();
}

void GLWidget3D::updateFloorOptions() {
	mainWin->thumbsList->clear();

	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["floor"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());

	QPainter painter(&sketch);
	for (int i = 0; i < grammarImages["floor"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["floor"][indexes[i]], indexes[i]);
	}

	predictFloor(0);

	update();
}

void GLWidget3D::updateWindowOptions() {
	mainWin->thumbsList->clear();

	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["window"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());

	QPainter painter(&sketch);
	for (int i = 0; i < grammarImages["window"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["window"][indexes[i]], indexes[i]);
	}

	predictFacade(0);

	update();
}

void GLWidget3D::updateLedgeOptions() {
	mainWin->thumbsList->clear();

	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["ledge"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());

	QPainter painter(&sketch);
	for (int i = 0; i < grammarImages["ledge"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["ledge"][indexes[i]], indexes[i]);
	}

	predictFacade(0);

	update();
}

/**
 * Use the sketch as an input to the pretrained network, and obtain the probabilities as output.
 * Then, display the options ordered by the probabilities.
 */
void GLWidget3D::predictBuilding(int grammar_id) {
	time_t start = clock();
	
	renderManager.removeObjects();

	// convert the sketch to grayscale
	cv::Mat mat(sketch.height(), sketch.width(), CV_8UC3, sketch.bits(), sketch.bytesPerLine());
	cv::Mat grayMat;
	cv::cvtColor(mat, grayMat, CV_BGR2GRAY);

	// resize the sketch
	cv::Mat resizedGrayMat;
	cv::resize(grayMat, resizedGrayMat, cv::Size(256, 256));
	cv::threshold(resizedGrayMat, resizedGrayMat, 250, 255, CV_THRESH_BINARY);
	cv::resize(resizedGrayMat, resizedGrayMat, cv::Size(128, 128));
	cv::threshold(resizedGrayMat, resizedGrayMat, 250, 255, CV_THRESH_BINARY);

	// predict parameter values by deep learning
	std::vector<float> params = regressions["building"][grammar_id]->Predict(resizedGrayMat);
	/*
	for (int i = 0; i < params.size(); ++i) {
		std::cout << params[i] << ",";
	}
	std::cout << std::endl;
	*/

	//std::vector<float> params(7);
	//for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;

	// optimize the parameter values by MCMC
	mcmc->optimize(grammars["building"][grammar_id], grayMat, 10.0f, current_z, params);
	/*
	for (int i = 0; i < params.size(); ++i) {
		std::cout << params[i] << ",";
	}
	std::cout << std::endl;
	*/

	renderManager.renderingMode = RenderManager::RENDERING_MODE_REGULAR;

	
	//////////////////////// DEBUG ////////////////////////
	/*
	std::vector<float> params(7);
	for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;
	std::cout << demo_mode << endl;
	if (demo_mode == 0) {
		if (scene._currentObject == 0) {
			params[2] = 1.0f;
			params[4] = 0.5f;
		}
		else if (scene._currentObject == 1) {
			params[0] = 0.25f;
			params[2] = 2.0f / 3.0f;
			params[4] = 0.0f;
		}
		else if (scene._currentObject > 1) {
			params[0] = 0.0;
			params[2] = 1.0f / 3.0f;
			params[4] = 0.0f;
		}
	}
	*/

	float offset_x = params[0] * 16 - 8;
	float offset_y = params[1] * 16 - 8;
	float object_width = params[2] * 24 + 4;
	float object_depth = params[3] * 24 + 4;
	offset_x -= object_width * 0.5f;
	offset_y -= object_depth * 0.5f;

	scene.currentObject().setFootprint(offset_x, offset_y, current_z, object_width, object_depth);
	scene.alignObjects();
	
	//std::cout << offset_x << "," << offset_y << "," << object_width << "," << object_depth << std::endl;

	// remove the first four parameters because they are not included in the grammar
	params.erase(params.begin(), params.begin() + 4);
	
	// set parameter values
	scene.currentObject().setGrammar("Start", grammars["building"][grammar_id], params);
	
	// set height
	std::vector<std::pair<float, float> > ranges = cga::CGA::getParamRanges(grammars["building"][grammar_id]);
	scene.currentObject().setHeight((ranges[0].second - ranges[0].first) * params[0] + ranges[0].first);
	
	scene.generateGeometry(&renderManager, "building");

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictRoof(int grammar_id) {
	if (scene._selectedFaceName.empty()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	time_t start = clock();

	renderManager.removeObjects();

	//////////////////////// DEBUG ////////////////////////
	scene.currentObject().setGrammar(scene._selectedFaceName, grammars["roof"][grammar_id]);

	scene.generateGeometry(&renderManager, "roof");

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictFacade(int grammar_id) {
	if (scene._selectedFaceName.empty()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	// list up y coordinates
	std::vector<float> y_coordinates;
	for (auto stroke : strokes) {
		if (stroke.size() <= 1) continue;

		float y = height() - (stroke[0].y + stroke.back().y) * 0.5f;

		// check if there is any other y coordinate that is close to this y
		bool tooClose = false;
		for (auto y_coord : y_coordinates) {
			if (fabs(y_coord - y) < 1) {
				tooClose = true;
				break;
			}
		}
		if (tooClose) continue;

		y_coordinates.push_back(y);

	}

	// order the lines by Y
	std::sort(y_coordinates.begin(), y_coordinates.end());

	// project the face to the image plane, and find the y coordinates of the bottom and top lines
	float bottom_y = height();
	float top_y = 0.0f;
	for (int i = 0; i < scene.selectedFace()->vertices.size(); ++i) {
		glm::vec4 projectedPt = camera.mvpMatrix * glm::vec4(scene.selectedFace()->vertices[i].position, 1);
		float y = (projectedPt.y / projectedPt.w + 1.0) * 0.5 * height();
		if (y < bottom_y) {
			bottom_y = y;
		}
		if (y > top_y) {
			top_y = y;
		}
	}

	std::cout << bottom_y << std::endl;
	for (auto y_coord : y_coordinates) {
		std::cout << y_coord << std::endl;
	}
	std::cout << top_y << std::endl;

	//////////////////////// DEBUG ////////////////////////
	//scene.building.currentLayer().setGrammar("Facade", grammars["facade"][grammar_id]);

	//scene.generateGeometry(&renderManager);

	update();
}

void GLWidget3D::predictFloor(int grammar_id) {
	if (scene._selectedFaceName.empty()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}



	update();
}

void GLWidget3D::predictWindow(int grammar_id) {
	if (scene._selectedFaceName.empty()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	time_t start = clock();

	renderManager.removeObjects();

	//////////////////////// DEBUG ////////////////////////
	scene.currentObject().setGrammar(scene._selectedFaceName, grammars["window"][grammar_id]);

	scene.generateGeometry(&renderManager, "window");
	
	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictLedge(int grammar_id) {
	if (scene._selectedFaceName.empty()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	time_t start = clock();

	renderManager.removeObjects();

	//////////////////////// DEBUG ////////////////////////
	scene.currentObject().setGrammar(scene._selectedFaceName, grammars["ledge"][grammar_id]);

	scene.generateGeometry(&renderManager, "ledge");

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

bool GLWidget3D::selectFace(const glm::vec2& mouse_pos) {
	// camera position in the world coordinates
	glm::vec3 cameraPos = camera.cameraPosInWorld();

	clearSketch();

	// view direction
	glm::vec3 view_dir = viewVector(mouse_pos, camera.mvMatrix, camera.f(), camera.aspect());

	if (stage == "building") {
		if (scene.selectFace(cameraPos, view_dir, stage, glm::vec3(0, 1, 0))) {
			scene.newObject();

			// shift the camera such that the selected face becomes a ground plane.
			intCamera = InterpolationCamera(camera, camera);
			intCamera.camera_end.pos = glm::vec3(0, scene.selectedFace()->vertices[0].position.y + CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH);
			intCamera.camera_end.xrot = 30.0f;
			intCamera.camera_end.yrot = -45.0f;
			intCamera.camera_end.zrot = 0.0f;
			current_z = scene.selectedFace()->vertices[0].position.y;
		}
		else {
			// shift the camera such that the ground plane becomes really a ground plane.
			intCamera = InterpolationCamera(camera, camera);
			intCamera.camera_end.pos = glm::vec3(0, CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH);
			intCamera.camera_end.xrot = 30.0f;
			intCamera.camera_end.yrot = -45.0f;
			intCamera.camera_end.zrot = 0.0f;
			current_z = 0;
		}

		return true;
	}
	else if (stage == "roof") {
		if (scene.selectFace(cameraPos, view_dir, stage, glm::vec3(0, 1, 0))) {
			// shift the camera such that the selected face becomes a ground plane.
			intCamera = InterpolationCamera(camera, camera);
			intCamera.camera_end.pos = glm::vec3(0, scene.selectedFace()->vertices[0].position.y, CAMERA_DEFAULT_DEPTH);
			intCamera.camera_end.xrot = 30.0f;
			intCamera.camera_end.yrot = -45.0f;
			intCamera.camera_end.zrot = 0.0f;
			current_z = scene.selectedFace()->vertices[0].position.y;

			return true;
		} else {
			return false;
		}
	}
	else if (stage == "facade") {
		if (scene.selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, camera);

			float rot_y = atan2f(scene.selectedFace()->vertices[0].normal.x, scene.selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));

			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			intCamera.camera_end.pos.x = rotatedFace.bbox.center().x;
			intCamera.camera_end.pos.y = rotatedFace.bbox.center().y;
			intCamera.camera_end.pos.z = rotatedFace.bbox.maxPt.z + d;

			intCamera.camera_end.xrot = 0.0f;
			intCamera.camera_end.yrot = -rot_y / M_PI * 180;
			intCamera.camera_end.zrot = 0.0f;

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "floor") {
		if (scene.selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, camera);

			float rot_y = atan2f(scene.selectedFace()->vertices[0].normal.x, scene.selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));

			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			intCamera.camera_end.pos.x = rotatedFace.bbox.center().x;
			intCamera.camera_end.pos.y = rotatedFace.bbox.center().y;
			intCamera.camera_end.pos.z = rotatedFace.bbox.maxPt.z + d;

			intCamera.camera_end.xrot = 0.0f;
			intCamera.camera_end.yrot = -rot_y / M_PI * 180;
			intCamera.camera_end.zrot = 0.0f;

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "window") {
		if (scene.selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, camera);

			float rot_y = atan2f(scene.selectedFace()->vertices[0].normal.x, scene.selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));

			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			intCamera.camera_end.pos.x = rotatedFace.bbox.center().x;
			intCamera.camera_end.pos.y = rotatedFace.bbox.center().y;
			intCamera.camera_end.pos.z = rotatedFace.bbox.maxPt.z + d;

			intCamera.camera_end.xrot = 0.0f;
			intCamera.camera_end.yrot = -rot_y / M_PI * 180;
			intCamera.camera_end.zrot = 0.0f;

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "ledge") {
		if (scene.selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, camera);

			float rot_y = atan2f(scene.selectedFace()->vertices[0].normal.x, scene.selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));

			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			intCamera.camera_end.pos.x = rotatedFace.bbox.center().x;
			intCamera.camera_end.pos.y = rotatedFace.bbox.center().y;
			intCamera.camera_end.pos.z = rotatedFace.bbox.maxPt.z + d;

			intCamera.camera_end.xrot = 0.0f;
			intCamera.camera_end.yrot = -rot_y / M_PI * 180;
			intCamera.camera_end.zrot = 0.0f;

			return true;
		}
		else {
			return false;
		}
	}
}

void GLWidget3D::addBuildingMass() {
	if (stage != "building") return;

	clearSketch();
	scene.newObject();
	update();
}

glm::vec3 GLWidget3D::viewVector(const glm::vec2& point, const glm::mat4& mvMatrix, float focalLength, float aspect) {
	glm::vec3 dir((point.x - width() * 0.5f) * 2.0f / width() * aspect, (height() * 0.5f - point.y) * 2.0f / height(), -focalLength);
	return glm::vec3(glm::inverse(mvMatrix) * glm::vec4(dir, 0));
}

void GLWidget3D::changeStage(const std::string& stage) {
	this->stage = stage;
	clearSketch();

	scene.unselectFace();

	if (stage == "building") {
		intCamera = InterpolationCamera(camera, camera);
		intCamera.camera_end.pos = glm::vec3(0, CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH);
		intCamera.camera_end.xrot = 30.0f;
		intCamera.camera_end.yrot = -45.0f;
		intCamera.camera_end.zrot = 0.0f;
		current_z = 0.0f;

		camera_timer = new QTimer(this);
		connect(camera_timer, SIGNAL(timeout()), mainWin, SLOT(camera_update()));
		camera_timer->start(20);
	} else if (stage == "roof") {
	} else if (stage == "facade") {
	} else if (stage == "floor") {
	} else if (stage == "window") {
	} else if (stage == "ledge") {
	}

	scene.updateGeometry(&renderManager, stage);

	update();
}

void GLWidget3D::camera_update() {
	if (intCamera.forward()) {
		camera_timer->stop();
	}

	camera = intCamera.currentCamera();
	camera.updateMVPMatrix();
	update();
}

void GLWidget3D::keyPressEvent(QKeyEvent *e) {
	ctrlPressed = false;

	switch (e->key()) {
	case Qt::Key_Control:
		ctrlPressed = true;
		break;
	case Qt::Key_0:
		demo_mode = 0;
		break;
	case Qt::Key_1:
		demo_mode = 1;
		break;
	default:
		break;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	ctrlPressed = false;
}

/**
 * This event handler is called when the mouse button is pressed.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	if (ctrlPressed) { // move camera
		camera.mousePress(e->x(), e->y());
	}
	else if (mode == MODE_SELECT) {
		// do nothing
	}
	else { // start drawing a stroke
		lastPos = e->pos();
		dragging = true;
		strokes.resize(strokes.size() + 1);
	}
}

/**
 * This event handler is called when the mouse button is released.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
	if (ctrlPressed) {
		// do nothing
	}
	else if (mode == MODE_SELECT) { // select a face
		if (selectFace(glm::vec2(e->x(), e->y()))) {
			scene.updateGeometry(&renderManager, stage);

			// When a face is selected, the user should start drawing.
			mode = MODE_SKETCH;
			mainWin->actionModes["sketch"]->setChecked(true);

			camera_timer = new QTimer(this);
			connect(camera_timer, SIGNAL(timeout()), mainWin, SLOT(camera_update()));
			camera_timer->start(20);

			update();
		}
	}
	else {
		dragging = false;

		if (stage == "building") {
			updateBuildingOptions();
		}
		else if (stage == "roof") {
			updateRoofOptions();
		}
		else if (stage == "facade") {
			updateFacadeOptions();
		}
		else if (stage == "floor") {
			updateFloorOptions();
		}
		else if (stage == "window") {
			updateWindowOptions();
		}
		else if (stage == "ledge") {
			updateLedgeOptions();
		}
	}
}

/**
 * This event handler is called when the mouse is dragged.
 */
void GLWidget3D::mouseMoveEvent(QMouseEvent *e) {
	if (ctrlPressed) { // moving a camera
		if (e->buttons() & Qt::LeftButton) { // Rotate
			camera.rotate(e->x(), e->y());
		}
		else if (e->buttons() & Qt::MidButton) { // Move
			camera.move(e->x(), e->y());
		}
		else if (e->buttons() & Qt::RightButton) { // Zoom
			camera.zoom(e->x(), e->y());
		}
		clearSketch();
	}
	else if (mode == MODE_SELECT) {
		// do nothing
	}
	else { // keep drawing a stroke
		drawLineTo(e->pos());
	}

	update();
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	renderManager.init("../shaders/vertex.glsl", "../shaders/geometry.glsl", "../shaders/fragment.glsl", false);
	renderManager.renderingMode = RenderManager::RENDERING_MODE_REGULAR;
	//renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;

	//glClearColor(1, 1, 1, 0.0);
	glClearColor(0.9, 0.9, 0.9, 0.0);

	sketch = QImage(this->width(), this->height(), QImage::Format_RGB888);
	sketch.fill(qRgba(255, 255, 255, 255));

	mode = MODE_SKETCH;
	
	camera.pos = glm::vec3(0, CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH);
	camera.xrot = 30.0f;
	camera.yrot = -45.0f;
	camera.zrot = 0.0f;
	current_z = 0.0f;
	scene.updateGeometry(&renderManager, "building");

	//changeStage("building");
	stage = "building";
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height?height:1;

	glViewport(0, 0, (GLint)width, (GLint)height );
	camera.updatePMatrix(width, height);
	renderManager.resize(width, height);

	QImage newImage(width, height, QImage::Format_RGB888);
	newImage.fill(qRgba(255, 255, 255, 255));
	QPainter painter(&newImage);

	painter.drawImage(0, 0, sketch);
	sketch = newImage;
}

void GLWidget3D::paintEvent(QPaintEvent *event) {
	// OpenGLで描画
	makeCurrent();

	glUseProgram(renderManager.program);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	// for transparacy
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"), 1, GL_FALSE, &camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"), 1, GL_FALSE, &camera.mvMatrix[0][0]);

	// pass the light direction to the shader
	//glUniform1fv(glGetUniformLocation(renderManager.program, "lightDir"), 3, &light_dir[0]);
	glUniform3f(glGetUniformLocation(renderManager.program, "lightDir"), light_dir.x, light_dir.y, light_dir.z);

	drawScene(0);





	// OpenGLの設定を元に戻す
	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// QPainterで描画
	QPainter painter(this);
	painter.setOpacity(0.5);
	painter.drawImage(0, 0, sketch);
	painter.end();

	glEnable(GL_DEPTH_TEST);
}

