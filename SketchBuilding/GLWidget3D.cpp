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
#include "LayoutExtractor.h"

#ifndef M_PI
#define M_PI	3.141592653
#endif

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
	mainWin = (MainWindow*)parent;
	dragging = false;
	ctrlPressed = false;
	shiftPressed = false;
	altPressed = false;
	align_threshold = 0.5;
	pen_pressure = 0.5;
	showGroundPlane = true;
	camera_timer = NULL;

	// this flag is for workaround.
	// Qt should not call TabletEvent and MouseEvent at the same time, but it actually calls both events.
	tableEventUsed = false;

	// This is necessary to prevent the screen overdrawn by OpenGL
	setAutoFillBackground(false);

	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));
	//light_dir = glm::normalize(glm::vec3(-1, -3, -2));

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
	classifiers["building"] = new Classifier("models/building/deploy.prototxt", "models/building/train_iter_20000.caffemodel", "models/building/buildings_mean.binaryproto");
	regressions["building"].resize(6);
	regressions["building"][0] = new Regression("models/building/deploy_1.prototxt", "models/building/building1_iter_80000.caffemodel");
	regressions["building"][1] = new Regression("models/building/deploy_2.prototxt", "models/building/building2_iter_80000.caffemodel");
	regressions["building"][2] = new Regression("models/building/deploy_3.prototxt", "models/building/building3_iter_80000.caffemodel");
	regressions["building"][3] = new Regression("models/building/deploy_4.prototxt", "models/building/building4_iter_80000.caffemodel");
	regressions["building"][4] = new Regression("models/building/deploy_5.prototxt", "models/building/building5_iter_80000.caffemodel");
	regressions["building"][5] = new Regression("models/building/deploy_6.prototxt", "models/building/building6_iter_80000.caffemodel");

	classifiers["roof"] = new Classifier("models/roof/deploy.prototxt", "models/roof/train_iter_10000.caffemodel", "models/roof/roofs_mean.binaryproto");
	regressions["roof"].resize(7);
	regressions["roof"][0] = new Regression("models/roof/deploy_1.prototxt", "models/roof/roof1_iter_80000.caffemodel");
	regressions["roof"][1] = new Regression("models/roof/deploy_2.prototxt", "models/roof/roof2_iter_80000.caffemodel");
	regressions["roof"][2] = new Regression("models/roof/deploy_3.prototxt", "models/roof/roof3_iter_80000.caffemodel");
	regressions["roof"][3] = new Regression("models/roof/deploy_4.prototxt", "models/roof/roof4_iter_80000.caffemodel");
	regressions["roof"][4] = new Regression("models/roof/deploy_5.prototxt", "models/roof/roof5_iter_80000.caffemodel");
	regressions["roof"][5] = new Regression("models/roof/deploy_6.prototxt", "models/roof/roof6_iter_80000.caffemodel");
	regressions["roof"][6] = new Regression("models/roof/deploy_7.prototxt", "models/roof/roof7_iter_80000.caffemodel");

	classifiers["window"] = new Classifier("models/window/deploy.prototxt", "models/window/train_iter_10000.caffemodel", "models/window/windows_mean.binaryproto");
	regressions["window"].resize(9);
	regressions["window"][0] = new Regression("models/window/deploy_1.prototxt", "models/window/window1_iter_80000.caffemodel");
	regressions["window"][1] = new Regression("models/window/deploy_2.prototxt", "models/window/window2_iter_80000.caffemodel");
	regressions["window"][2] = new Regression("models/window/deploy_3.prototxt", "models/window/window3_iter_80000.caffemodel");
	regressions["window"][3] = new Regression("models/window/deploy_4.prototxt", "models/window/window4_iter_80000.caffemodel");
	regressions["window"][4] = new Regression("models/window/deploy_5.prototxt", "models/window/window5_iter_80000.caffemodel");
	regressions["window"][5] = new Regression("models/window/deploy_6.prototxt", "models/window/window6_iter_80000.caffemodel");
	regressions["window"][6] = new Regression("models/window/deploy_7.prototxt", "models/window/window7_iter_80000.caffemodel");
	regressions["window"][7] = new Regression("models/window/deploy_8.prototxt", "models/window/window8_iter_80000.caffemodel");
	regressions["window"][8] = new Regression("models/window/deploy_9.prototxt", "models/window/window9_iter_80000.caffemodel");

	classifiers["ledge"] = new Classifier("models/ledge/deploy.prototxt", "models/ledge/train_iter_20000.caffemodel", "models/ledge/ledges_mean.binaryproto");
	regressions["ledge"].resize(4);
	regressions["ledge"][0] = new Regression("models/ledge/deploy_1.prototxt", "models/ledge/ledge1_iter_80000.caffemodel");
	regressions["ledge"][1] = new Regression("models/ledge/deploy_2.prototxt", "models/ledge/ledge2_iter_80000.caffemodel");
	regressions["ledge"][2] = new Regression("models/ledge/deploy_3.prototxt", "models/ledge/ledge3_iter_80000.caffemodel");
	regressions["ledge"][3] = new Regression("models/ledge/deploy_4.prototxt", "models/ledge/ledge4_iter_80000.caffemodel");

	mcmc = new MCMC(this);
}

void GLWidget3D::drawLineTo(const QPoint &endPoint, float width) {
	QPoint pt1(lastPos.x(), lastPos.y());
	QPoint pt2(endPoint.x(), endPoint.y());

	QPainter painter(&sketch);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.setBrush(QBrush(Qt::black));
	painter.drawLine(pt1, pt2);

	strokes.back().push_back(glm::vec2(endPoint.x(), height() - endPoint.y()));
	stroke_widths.back().push_back(width);

	lastPos = endPoint;
}

void GLWidget3D::drawLassoLineTo(const QPoint &endPoint, float width) {
	QPoint pt1(lastPos.x(), lastPos.y());
	QPoint pt2(endPoint.x(), endPoint.y());

	lasso.push_back(glm::vec2(endPoint.x(), height() - endPoint.y()));
	lasso_widths.push_back(width);

	lastPos = endPoint;
}

/**
 * Clear the canvas.
 */
void GLWidget3D::clearSketch() {
	sketch.fill(qRgba(255, 255, 255, 255));
	strokes.clear();
	stroke_widths.clear();
}

void GLWidget3D::clearGeometry() {
	scene.clear();
	generateGeometry();
}

/**
* Draw the scene.
*/
void GLWidget3D::drawScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(true);

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

		sc::SceneObject so(&scene);
		so.setFootprint(0, 0, 0, 16, 12);

		std::vector<float> params(10);
		for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;
		so.setGrammar("Start", grammar, params, true);
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

	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);

	update();
}

void GLWidget3D::generateGeometry() {
	scene.generateGeometry(&renderManager, stage);

	// add a ground plane
	if (showGroundPlane) {
		std::vector<Vertex> vertices;
		glutils::drawGrid(50, 50, 2.5, glm::vec4(0.521, 0.815, 0.917, 1), glm::vec4(0.898, 0.933, 0.941, 1), scene.system.modelMat, vertices);
		renderManager.addObject("grid", "", vertices, false);
	}

	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
}

void GLWidget3D::updateGeometry() {
	scene.updateGeometry(&renderManager, stage);

	// add a ground plane
	if (showGroundPlane) {
		std::vector<Vertex> vertices;
		glutils::drawGrid(50, 50, 2.5, glm::vec4(0.521, 0.815, 0.917, 1), glm::vec4(0.898, 0.933, 0.941, 1), scene.system.modelMat, vertices);
		renderManager.addObject("grid", "", vertices, false);
	}
}

void GLWidget3D::selectOption(int option_index) {
	if (stage == "building") {
		predictBuilding(option_index);
	} 
	else if (stage == "roof") {
		predictRoof(option_index);
	} 
	else if (stage == "facade") {
		if (scene.faceSelector->selected()) {
			scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["facade"][option_index]);
			generateGeometry();
		}
	} 
	else if (stage == "floor") {
		if (scene.faceSelector->selected()) {
			scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["floor"][option_index]);
			generateGeometry();
		}
	} 
	else if (stage == "window") {
		predictWindow(option_index);
	} 
	else if (stage == "ledge") {
		predictLedge(option_index);
	}

	update();
}

void GLWidget3D::updateBuildingOptions() {
	mainWin->thumbsList->clear();

	time_t start = clock();

	cv::Mat img;
	convertSketch(false, img);
	std::vector<Prediction> predictions = classifiers["building"]->Classify(img, 10);
	
	time_t end = clock();
	std::cout << "Duration of classification: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	QPainter painter(&sketch);
	int bestIndex;
	for (int i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		mainWin->addListItem(QString::number(p.second, 'f', 3), grammarImages["building"][p.first], p.first);

		if (i == 0) {
			bestIndex = p.first;
		}
	}

	predictBuilding(bestIndex);

	update();
}

void GLWidget3D::updateRoofOptions() {
	mainWin->thumbsList->clear();

	time_t start = clock();

	cv::Mat img;
	convertSketch(false, img);
	std::vector<Prediction> predictions = classifiers["roof"]->Classify(img, 10);

	time_t end = clock();
	std::cout << "Duration of classification: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	QPainter painter(&sketch);
	int bestIndex;
	for (int i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		mainWin->addListItem(QString::number(p.second, 'f', 3), grammarImages["roof"][p.first], p.first);

		if (i == 0) {
			bestIndex = p.first;
		}
	}

	predictRoof(bestIndex);

	update();
}

void GLWidget3D::updateFacadeOptions() {
	mainWin->thumbsList->clear();

	std::pair<int, std::vector<float> > result = LayoutExtractor::extractFacadePattern(width(), height(), strokes, scene.faceSelector->selectedFaceCopy(), camera.mvpMatrix);

	QPainter painter(&sketch);
	mainWin->addListItem("1.00", grammarImages["facade"][result.first], result.first);
	for (int i = 0; i < grammarImages["facade"].size(); ++i) {
		if (i == result.first) continue;

		mainWin->addListItem("0.00", grammarImages["facade"][i], i);
	}

	predictFacade(result.first, result.second);

	update();
}

void GLWidget3D::updateFloorOptions() {
	mainWin->thumbsList->clear();

	std::pair<int, std::vector<float> > result = LayoutExtractor::extractFloorPattern(width(), height(), strokes, scene.faceSelector->selectedFaceCopy(), camera.mvpMatrix);

	if (result.first >= 0) {
		QPainter painter(&sketch);
		mainWin->addListItem("1.00", grammarImages["floor"][result.first], result.first);
		for (int i = 0; i < grammarImages["floor"].size(); ++i) {
			if (i == result.first) continue;

			mainWin->addListItem("0.00", grammarImages["floor"][i], i);
		}

		predictFloor(result.first, result.second);

		update();
	}
}

void GLWidget3D::updateWindowOptions() {
	mainWin->thumbsList->clear();

	time_t start = clock();

	cv::Mat img;
	convertSketch(false, img);
	std::vector<Prediction> predictions = classifiers["window"]->Classify(img, 10);
	
	time_t end = clock();
	std::cout << "Duration of classification: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	QPainter painter(&sketch);
	int bestIndex;
	for (int i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		mainWin->addListItem(QString::number(p.second, 'f', 3), grammarImages["window"][p.first], p.first);

		if (i == 0) {
			bestIndex = p.first;
		}
	}

	predictWindow(bestIndex);

	update();
}

void GLWidget3D::updateLedgeOptions() {
	mainWin->thumbsList->clear();

	time_t start = clock();

	cv::Mat img;
	convertSketch(false, img);
	std::vector<Prediction> predictions = classifiers["ledge"]->Classify(img, 10);

	time_t end = clock();
	std::cout << "Duration of classification: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	QPainter painter(&sketch);
	int bestIndex;
	for (int i = 0; i < predictions.size(); ++i) {
		Prediction p = predictions[i];
		mainWin->addListItem(QString::number(p.second, 'f', 3), grammarImages["ledge"][p.first], p.first);

		if (i == 0) {
			bestIndex = p.first;
		}
	}

	predictLedge(bestIndex);

	update();
}

/**
 * Use the sketch as an input to the pretrained network, and obtain the probabilities as output.
 * Then, display the options ordered by the probabilities.
 */
void GLWidget3D::predictBuilding(int grammar_id) {
	renderManager.removeObjects();

	// predict parameter values by deep learning
	time_t start = clock();
	cv::Mat img;
	convertSketch(true, img);
	std::vector<float> params = regressions["building"][grammar_id]->Predict(img);
	debug("Building regression: ", params);
	time_t end = clock();
	std::cout << "Duration of regression: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	// optimize the parameter values by MCMC
	start = clock();
	mcmc->optimize(grammars["building"][grammar_id], img, 10.0f, 10, current_z, params);
	debug("Building MCMC: ", params);
	end = clock();
	std::cout << "Duration of MCMC: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;
	
	float offset_x = params[0] * (BUILDING_MASS_MAX_X - BUILDING_MASS_MIN_X) + BUILDING_MASS_MIN_X;
	float offset_y = params[1] * (BUILDING_MASS_MAX_Y - BUILDING_MASS_MIN_Y) + BUILDING_MASS_MIN_Y;
	float object_width = params[2] * (BUILDING_MASS_MAX_WIDTH - BUILDING_MASS_MIN_WIDTH) + BUILDING_MASS_MIN_WIDTH;
	float object_depth = params[3] * (BUILDING_MASS_MAX_DEPTH - BUILDING_MASS_MIN_DEPTH) + BUILDING_MASS_MIN_DEPTH;

	// HACK: for observatory
	if (grammar_id == 3) {
		float avg_width_depth = (object_width + object_depth) * 0.5f;
		object_width = avg_width_depth;
		object_depth = avg_width_depth;
	}

	offset_x -= object_width * 0.5f;
	offset_y -= object_depth * 0.5f;

	scene.currentObject().setFootprint(offset_x, offset_y, current_z, object_width, object_depth);
	if (scene.faceSelector->selected()) {
		scene.alignObjects(scene.faceSelector->selectedFaceCopy(), align_threshold);
	}
	else {
		scene.alignObjects(align_threshold);
	}
	
	// remove the first four parameters because they are not included in the grammar
	params.erase(params.begin(), params.begin() + 4);
	std::cout << "Height: " << params[0] << std::endl;
	
	// set parameter values
	scene.currentObject().setGrammar("Start", grammars["building"][grammar_id], params, true);
	
	// set height
	std::vector<std::pair<float, float> > ranges = cga::CGA::getParamRanges(grammars["building"][grammar_id]);
	scene.currentObject().setHeight((ranges[0].second - ranges[0].first) * params[0] + ranges[0].first);
	generateGeometry();

	// updte the grammar window
	mainWin->grammarDialog->updateGrammar();

	update();
}

void GLWidget3D::predictRoof(int grammar_id) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	std::cout << "Roof: grammar " << (grammar_id + 1) << " was selected." << std::endl;

	renderManager.removeObjects();

	time_t start = clock();

	// predict parameter values by deep learning
	cv::Mat img;
	convertSketch(true, img);
	std::vector<float> params = regressions["roof"][grammar_id]->Predict(img);
	debug("Roof regression", params);

	time_t end = clock();
	std::cout << "Duration of regression: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;
		
	// set parameter values
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["roof"][grammar_id], params, true);
	generateGeometry();

	// updte the grammar window
	mainWin->grammarDialog->updateGrammar();

	update();
}

void GLWidget3D::predictFacade(int grammar_id, const std::vector<float>& params) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	// set grammar
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["facade"][grammar_id], params, false);
	generateGeometry();

	// updte the grammar window
	mainWin->grammarDialog->updateGrammar();

	update();
}

void GLWidget3D::predictFloor(int grammar_id, const std::vector<float>& params) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	// set grammar
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["floor"][grammar_id], params, false);
	generateGeometry();

	// updte the grammar window
	mainWin->grammarDialog->updateGrammar();

	update();
}

void GLWidget3D::predictWindow(int grammar_id) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	renderManager.removeObjects();

	time_t start = clock();
	
	// predict parameter values by deep learning
	cv::Mat img;
	convertSketch(true, img);
	std::vector<float> params = regressions["window"][grammar_id]->Predict(img);
	debug("Window regression", params);

	time_t end = clock();
	std::cout << "Duration of regression: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	// set parameter values
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["window"][grammar_id], params, true);
	generateGeometry();

	// updte the grammar window
	mainWin->grammarDialog->updateGrammar();

	update();
}

void GLWidget3D::predictLedge(int grammar_id) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	renderManager.removeObjects();

	time_t start = clock();

	// predict parameter values by deep learning
	cv::Mat img;
	convertSketch(true, img);
	std::vector<float> params = regressions["ledge"][grammar_id]->Predict(img);
	debug("Ledge regression", params);

	time_t end = clock();
	std::cout << "Duration of regression: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	// set parameter values
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["ledge"][grammar_id], params, true);
	generateGeometry();

	// updte the grammar window
	mainWin->grammarDialog->updateGrammar();

	update();
}

bool GLWidget3D::selectFace(const glm::vec2& mouse_pos) {
	// camera position in the world coordinates
	glm::vec3 cameraPos = camera.cameraPosInWorld();

	clearSketch();

	// view direction
	glm::vec3 view_dir = viewVector(mouse_pos, camera.mvMatrix, camera.f(), camera.aspect());

	if (stage == "building") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(0, 1, 0))) {
			scene.newObject();

			selectFaceForBuilding();
		}
		else {
			scene.newObject();

			// shift the camera such that the ground plane becomes really a ground plane.
			intCamera = InterpolationCamera(camera, 30, -45, 0, computeDownwardedCameraPos(CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH, intCamera.camera_end.xrot));
			current_z = 0;
		}

		return true;
	}
	else if (stage == "roof") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(0, 1, 0))) {
			selectFaceForRoof();
			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "facade") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			selectFaceForFacade();
			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "floor") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			selectFaceForFloor();
			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "window") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			selectFaceForWindow();
			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "ledge") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			selectFaceForLedge();
			return true;
		}
		else {
			return false;
		}
	}
}

/**
* infer stage and face
*/
bool GLWidget3D::selectStageAndFace(const glm::vec2& mouse_pos) {
	clearSketch();

	// the selected building should be unselected.
	scene.buildingSelector->unselectBuilding();
	scene.faceSelector->unselect();

	glm::vec3 camera_view = viewVector(mouse_pos, camera.mvMatrix, camera.f(), camera.aspect());
	std::pair<int, boost::shared_ptr<glutils::Face> > result = scene.findFace(lasso, camera.mvpMatrix, camera_view, width(), height());
	int object_id = result.first;
	boost::shared_ptr<glutils::Face> face = result.second;

	if (face != NULL) {
		if (face->name.compare(0, 4, "Roof") == 0) {
			stage = "roof";
			scene.faceSelector->selectFace(object_id, face);

			selectFaceForRoof();
			return true;
		}
		else if (face->name.compare(0, 6, "Facade") == 0) {
			stage = "facade";
			scene.faceSelector->selectFace(object_id, face);

			selectFaceForFacade();
			return true;
		}
		else if (face->name.compare(0, 5, "Floor") == 0) {
			stage = "floor";
			scene.faceSelector->selectFace(object_id, face);

			selectFaceForFloor();
			return true;
		}
		else if (face->name.compare(0, 6, "Window") == 0) {
			stage = "window";
			scene.faceSelector->selectFace(object_id, face);

			selectFaceForWindow();
			return true;
		}
		else if (face->name.compare(0, 5, "Ledge") == 0) {
			stage = "ledge";
			scene.faceSelector->selectFace(object_id, face);

			selectFaceForLedge();
			return true;
		}
	}

	return false;
}

void GLWidget3D::selectFaceForBuilding() {
	// shift the camera such that the selected face becomes a ground plane.
	intCamera = InterpolationCamera(camera, 30, -45, 0, computeDownwardedCameraPos(scene.faceSelector->selectedFace()->vertices[0].position.y + CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH, intCamera.camera_end.xrot));
	current_z = scene.faceSelector->selectedFace()->vertices[0].position.y;

	scene.faceSelector->selectedFace()->select();
}

void GLWidget3D::selectFaceForRoof() {
	// make the yrot in the rage [-180,179]
	camera.yrot = (int)(camera.yrot + 360 * 10) % 360;
	if (camera.yrot > 180) camera.yrot -= 360;

	// find the nearest quadrant
	float yrot = 0.0f;
	if (camera.yrot >= -90 && camera.yrot <= 0) {
		yrot = -45.0f;
	}
	else if (camera.yrot > 0 && camera.yrot <= 90) {
		yrot = 45.0f;
	}
	else if (camera.yrot > 90) {
		yrot = 135.0f;
	}
	else {
		yrot = -135.0f;
	}

	// shift the camera such that the selected face lies at the center of the ground plane.
	glm::vec3 center = scene.faceSelector->selectedFace()->bbox.center();
	intCamera = InterpolationCamera(camera, 30, yrot, 0.0, glm::vec3(center.x, center.y, center.z + CAMERA_DEFAULT_DEPTH));
	current_z = scene.faceSelector->selectedFace()->vertices[0].position.y;

	scene.faceSelector->selectedFace()->select();
}

void GLWidget3D::selectFaceForFacade() {
	// compute appropriate camera distance for the selected face
	float rot_y = atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
	glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
	float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
	float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
	float d = std::max(d1, d2) * 1.5f;

	// turn the camera such that the selected face becomes parallel to the image plane.
	intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

	scene.faceSelector->selectedFace()->select();
}

void GLWidget3D::selectFaceForFloor() {
	// compute appropriate camera distance for the selected face
	float rot_y = atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
	glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
	float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
	float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
	float d = std::max(d1, d2) * 1.5f;

	// turn the camera such that the selected face becomes parallel to the image plane.
	intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

	scene.faceSelector->selectedFace()->select();
}

void GLWidget3D::selectFaceForWindow() {
	// compute appropriate camera distance for the selected face
	float rot_y = atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
	glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
	//float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
	float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
	//float d = std::max(d1, d2) * 1.5f;
	float d = d2 * 3.5f;

	// turn the camera such that the selected face becomes parallel to the image plane.
	intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

	scene.faceSelector->selectedFace()->select();
}

void GLWidget3D::selectFaceForLedge() {
	// compute appropriate camera distance for the selected face
	float rot_y = -M_PI * 0.4 + atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
	glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
	float d = 6;

	// turn the camera such that the selected face becomes parallel to the image plane.
	intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.center().z + d));

	scene.faceSelector->selectedFace()->select();
}

bool GLWidget3D::selectBuilding(const glm::vec2& mouse_pos) {
	// camera position in the world coordinates
	glm::vec3 cameraPos = camera.cameraPosInWorld();

	clearSketch();

	// view direction
	glm::vec3 view_dir = viewVector(mouse_pos, camera.mvMatrix, camera.f(), camera.aspect());

	// select a building
	int current_object = scene.buildingSelector->selectBuilding(cameraPos, view_dir);
	if (current_object >= 0) {
		scene._currentObject = current_object;
		return true;
	}
	else {
		return false;
	}
}

bool GLWidget3D::selectBuildingControlPoint(const glm::vec2& mouse_pos) {
	// camera position in the world coordinates
	glm::vec3 cameraPos = camera.cameraPosInWorld();

	// view direction
	glm::vec3 view_dir = viewVector(mouse_pos, camera.mvMatrix, camera.f(), camera.aspect());

	// select a control point
	int object_id = scene.buildingSelector->selectBuildingControlPoint(cameraPos, view_dir, mouse_pos, camera.mvpMatrix, width(), height());
	if (object_id >= 0) {
		scene._currentObject = object_id;
		return true;
	}
	else {
		return false;
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

void GLWidget3D::convertSketch(bool regression, cv::Mat& result) {
	if (regression) {
		cv::Mat mat = cv::Mat(sketch.height(), sketch.width(), CV_8UC3, sketch.bits(), sketch.bytesPerLine());
		cv::Mat grayMat;
		cv::cvtColor(mat, grayMat, CV_BGR2GRAY);

		// resize the sketch
		int min_size = std::min(grayMat.cols, grayMat.rows);
		result = grayMat(cv::Rect((grayMat.cols - min_size) * 0.5, (grayMat.rows - min_size) * 0.5, min_size, min_size));

		if (min_size > 512) {
			cv::resize(result, result, cv::Size(512, 512));
			cv::threshold(result, result, 250, 255, CV_THRESH_BINARY);

		}
		cv::resize(result, result, cv::Size(256, 256));
		cv::threshold(result, result, 250, 255, CV_THRESH_BINARY);
		cv::resize(result, result, cv::Size(128, 128));
		cv::threshold(result, result, 250, 255, CV_THRESH_BINARY);
	}
	else {
		QImage swapped = sketch.rgbSwapped();
		cv::Mat mat = cv::Mat(swapped.height(), swapped.width(), CV_8UC3, swapped.bits(), swapped.bytesPerLine()).clone();

		// resize the sketch
		int min_size = std::min(mat.cols, mat.rows);
		result = mat(cv::Rect((mat.cols - min_size) * 0.5, (mat.rows - min_size) * 0.5, min_size, min_size));

		if (min_size > 512) {
			cv::resize(result, result, cv::Size(512, 512));
			cv::threshold(result, result, 250, 255, CV_THRESH_BINARY);

		}
		cv::resize(result, result, cv::Size(256, 256));
		cv::threshold(result, result, 250, 255, CV_THRESH_BINARY);
	}
}

void GLWidget3D::changeStage(const std::string& stage) {
	this->stage = stage;
	clearSketch();

	// the selected building should be unselected.
	scene.buildingSelector->unselectBuilding();
	scene.faceSelector->unselect();

	if (stage == "building") {
		intCamera = InterpolationCamera(camera, 30, -45, 0, glm::vec3(0, CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH));
		current_z = 0.0f;

		camera_timer = new QTimer(this);
		connect(camera_timer, SIGNAL(timeout()), mainWin, SLOT(camera_update()));
		camera_timer->start(20);
	}
	else if (stage == "roof") {
	}
	else if (stage == "facade") {
	}
	else if (stage == "floor") {
	}
	else if (stage == "window") {
	}
	else if (stage == "ledge") {
	}
	else if (stage == "final") {
		renderManager.renderingMode = RenderManager::RENDERING_MODE_SSAO;
		generateGeometry();
	}

	updateGeometry();
	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);

	update();
}

void GLWidget3D::changeMode(int new_mode) {
	if (new_mode == MODE_COPY) {
		clearSketch();

		if (scene.buildingSelector->isBuildingSelected()) {
			scene.buildingSelector->copy();
			generateGeometry();
		}
	}
	else if (new_mode == MODE_ERASER) {
		clearSketch();

		if (stage == "building") {
			scene.clearCurrentObject();
		}

	}
	else {
		clearSketch();

		if (scene.buildingSelector->isBuildingSelected()) {
			scene.buildingSelector->unselectBuilding();
			updateGeometry();
		}

		mode = new_mode;
	}

	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
	update();
}

void GLWidget3D::camera_update() {
	if (intCamera.forward()) {
		camera_timer->stop();
		delete camera_timer;
		camera_timer = NULL;
	}

	camera = intCamera.currentCamera();
	camera.updateMVPMatrix();
	update();
}

glm::vec3 GLWidget3D::computeDownwardedCameraPos(float downward, float distToCamera, float camera_xrot) {
	return glm::vec3(0,
		downward * cosf(camera_xrot / 180.0f * M_PI),
		downward * sinf(camera_xrot / 180.0f * M_PI) + distToCamera);
}

void GLWidget3D::keyPressEvent(QKeyEvent *e) {
	ctrlPressed = false;
	shiftPressed = false;
	altPressed = false;

	switch (e->key()) {
	case Qt::Key_Control:
		ctrlPressed = true;
		break;
	case Qt::Key_Shift:
		shiftPressed = true;
		break;
	case Qt::Key_Alt:
		altPressed = true;
		break;
	default:
		break;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	ctrlPressed = false;
	shiftPressed = false;
	altPressed = false;
}

void GLWidget3D::tabletEvent(QTabletEvent *e) {
	switch (e->type()) {
	case QEvent::TabletMove:
		pen_pressure = e->pressure();
		break;
	default:
		break;
	}
}

/**
* This event handler is called when the mouse button is pressed.
*/
void GLWidget3D::mousePressEvent(QMouseEvent* e) {
	dragging = true;
	mouse_pressed_time = clock();

	if (mode == MODE_CAMERA) { // move camera
		camera.mousePress(e->x(), e->y());
	}
	else if (mode == MODE_SELECT) {
		// do nothing
	}
	else if (mode == MODE_SELECT_BUILDING) {
		if (selectBuildingControlPoint(glm::vec2(e->x(), e->y()))) {
			// a building is selected, so update the history to prepare for undo in the future
			scene.updateHistory();

			updateGeometry();
		}
		renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
		update();
	}
	else {
		if (e->buttons() & Qt::RightButton) {
			// start drawing a lasso
			lastPos = e->pos();
		}
		else if (e->buttons() & Qt::LeftButton) {
			// start drawing a stroke
			lastPos = e->pos();
			strokes.resize(strokes.size() + 1);
			stroke_widths.resize(stroke_widths.size() + 1);
		}
	}
}

/**
* This event handler is called when the mouse button is released.
*/
void GLWidget3D::mouseReleaseEvent(QMouseEvent* e) {
	dragging = false;

	if (mode == MODE_CAMERA) {
		// do nothing
	}
	else if (mode == MODE_SELECT) { // select a face
		if (selectFace(glm::vec2(e->x(), e->y()))) {
			updateGeometry();

			// When a face is selected, the user should start drawing.
			mode = MODE_SKETCH;
			mainWin->actionModes["sketch"]->setChecked(true);

			camera_timer = new QTimer(this);
			connect(camera_timer, SIGNAL(timeout()), mainWin, SLOT(camera_update()));
			camera_timer->start(20);

			update();
		}
	}
	else if (mode == MODE_SELECT_BUILDING) { // select a building
		if (scene.buildingSelector->isBuildingControlPointSelected()) {
			if (shiftPressed) {
				scene.buildingSelector->alignObjects(align_threshold);
			}
			scene.buildingSelector->unselectBuildingControlPoint();
			generateGeometry();
		}
		else {
			if (selectBuilding(glm::vec2(e->x(), e->y()))) {
				std::cout << "A building is selected." << std::endl;
			}
		}

		updateGeometry();

		// updte the grammar window
		mainWin->grammarDialog->updateGrammar();

		update();
	}
	else {
		if (e->button() == Qt::RightButton) {
			if (selectStageAndFace(glm::vec2(e->x(), e->y()))) {
				camera_timer = new QTimer(this);
				connect(camera_timer, SIGNAL(timeout()), mainWin, SLOT(camera_update()));
				camera_timer->start(20);

				mainWin->actionStages[stage]->setChecked(true);
			}

			lasso.clear();
			lasso_widths.clear();
			update();
		}
		else if (e->button() == Qt::LeftButton) {
			if (stage == "building") {
				if (strokes.size() >= 3) updateBuildingOptions();
			}
			else if (stage == "roof") {
				if (strokes.size() >= 2) updateRoofOptions();
			}
			else if (stage == "facade") {
				updateFacadeOptions();
			}
			else if (stage == "floor") {
				updateFloorOptions();
			}
			else if (stage == "window") {
				if (strokes.size() >= 2) updateWindowOptions();
			}
			else if (stage == "ledge") {
				updateLedgeOptions();
			}
		}
	}
}

/**
* This event handler is called when the mouse is dragged.
*/
void GLWidget3D::mouseMoveEvent(QMouseEvent* e) {
	// Workaround:
	// Tablet emits the mouseMoveEvent even if the pen touches the screen for a very short time.
	// Thus, if the elapsed time is too short, skip this event.
	time_t time = clock();
	if (time - mouse_pressed_time < 50) return;

	if (dragging) {
		if (mode == MODE_CAMERA) {
			if (e->buttons() & Qt::RightButton) { // Zoom
				camera.zoom(e->x(), e->y());
			}
			else if (e->buttons() & Qt::MidButton) { // Move
				camera.move(e->x(), e->y());
			}
			else if (e->buttons() & Qt::LeftButton) { // Rotate
				camera.rotate(e->x(), e->y());
			}
			clearSketch();
		}
		else if (mode == MODE_SELECT) {
			// do nothing
		}
		else if (mode == MODE_SELECT_BUILDING) {
			if (scene.buildingSelector->isBuildingControlPointSelected()) {
				// resize the building
				scene.buildingSelector->resize(glm::vec2(e->x(), e->y()), !ctrlPressed, altPressed);
				if (shiftPressed) {
					scene.buildingSelector->alignObjects(align_threshold);
				}
				generateGeometry();
			}
		}
		else {
			if (e->buttons() & Qt::RightButton) {
				// keep drawing a lasso
				drawLassoLineTo(e->pos(), pen_pressure * 10 + 1);
			}
			else if (e->buttons() & Qt::LeftButton) {
				// keep drawing a stroke
				drawLineTo(e->pos(), pen_pressure * 10 + 1);
			}
		}

		update();
	}
	else {
		if (e->y() > height() - BOTTOM_AREA_HEIGHT) {
			if (stage != "peek_final") {
				preStage = stage;
				preRenderingMode = renderManager.renderingMode;
				stage = "peek_final";
				renderManager.renderingMode = RenderManager::RENDERING_MODE_SSAO;

				// generate final geometry
				generateGeometry();

				update();
			}
		}
		else {
			if (stage == "peek_final") {
				stage = preStage;
				renderManager.renderingMode = preRenderingMode;

				// generate geometry
				generateGeometry();

				update();
			}
		}
	}
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	// init glew
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
	}

	if (glewIsSupported("GL_VERSION_4_2"))
		printf("Ready for OpenGL 4.2\n");
	else {
		printf("OpenGL 4.2 not supported\n");
		exit(1);
	}
	const GLubyte* text = glGetString(GL_VERSION);
	printf("VERSION: %s\n", text);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexGenf(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGenf(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_TEXTURE_3D);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDisable(GL_TEXTURE_3D);

	glEnable(GL_TEXTURE_2D_ARRAY);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glDisable(GL_TEXTURE_2D_ARRAY);

	////////////////////////////////
	renderManager.init("", "", "", true, 8192);
	renderManager.resize(this->width(), this->height());
	renderManager.renderingMode = RenderManager::RENDERING_MODE_HATCHING;

	glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex0"), 0);//tex0: 0
		
	sketch = QImage(this->width(), this->height(), QImage::Format_RGB888);
	sketch.fill(qRgba(255, 255, 255, 255));

	mode = MODE_SKETCH;
	
	camera.xrot = 30.0f;
	camera.yrot = -45.0f;
	camera.zrot = 0.0f;
	camera.pos = computeDownwardedCameraPos(CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH, camera.xrot);
	current_z = 0.0f;

	//changeStage("building");
	stage = "building";

	generateGeometry();

	setMouseTracking(true);
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height ? height : 1;

	glViewport(0, 0, (GLint)width, (GLint)height);
	camera.updatePMatrix(width, height);
	renderManager.resize(width, height);

	QImage newImage(width, height, QImage::Format_RGB888);
	newImage.fill(qRgba(255, 255, 255, 255));
	sketch = newImage;

	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
}

void GLWidget3D::paintEvent(QPaintEvent* e) {
	// OpenGLで描画
	makeCurrent();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PASS 1: Render to texture
	glUseProgram(renderManager.programs["pass1"]);

	glBindFramebuffer(GL_FRAMEBUFFER, renderManager.fragDataFB);
	glClearColor(0.95, 0.95, 0.95, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderManager.fragDataTex[0], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderManager.fragDataTex[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderManager.fragDataTex[2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderManager.fragDataTex[3], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderManager.fragDepthTex, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, DrawBuffers); // "3" is the size of DrawBuffers
	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("+ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
		exit(0);
	}

	glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["pass1"], "mvpMatrix"), 1, false, &camera.mvpMatrix[0][0]);
	glUniform3f(glGetUniformLocation(renderManager.programs["pass1"], "lightDir"), light_dir.x, light_dir.y, light_dir.z);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["pass1"], "light_mvpMatrix"), 1, false, &light_mvpMatrix[0][0]);

	glUniform1i(glGetUniformLocation(renderManager.programs["pass1"], "shadowMap"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, renderManager.shadow.textureDepth);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	drawScene();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PASS 2: Create AO
	if (renderManager.renderingMode == RenderManager::RENDERING_MODE_SSAO) {
		glUseProgram(renderManager.programs["ssao"]);
		glBindFramebuffer(GL_FRAMEBUFFER, renderManager.fragDataFB_AO);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderManager.fragAOTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderManager.fragDepthTex_AO, 0);
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Always check that our framebuffer is ok
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			printf("++ERROR: GL_FRAMEBUFFER_COMPLETE false\n");
			exit(0);
		}

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUniform2f(glGetUniformLocation(renderManager.programs["ssao"], "pixelSize"), 2.0f / this->width(), 2.0f / this->height());

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex0"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex1"), 2);
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[1]);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[2]);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "noiseTex"), 7);
		glActiveTexture(GL_TEXTURE7);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragNoiseTex);

		{
			glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["ssao"], "mvpMatrix"), 1, false, &camera.mvpMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["ssao"], "pMatrix"), 1, false, &camera.pMatrix[0][0]);
		}

		glUniform1i(glGetUniformLocation(renderManager.programs["ssao"], "uKernelSize"), renderManager.uKernelSize);
		glUniform3fv(glGetUniformLocation(renderManager.programs["ssao"], "uKernelOffsets"), renderManager.uKernelOffsets.size(), (const GLfloat*)renderManager.uKernelOffsets.data());

		glUniform1f(glGetUniformLocation(renderManager.programs["ssao"], "uPower"), renderManager.uPower);
		glUniform1f(glGetUniformLocation(renderManager.programs["ssao"], "uRadius"), renderManager.uRadius);

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
	}
	else if (renderManager.renderingMode == RenderManager::RENDERING_MODE_LINE || renderManager.renderingMode == RenderManager::RENDERING_MODE_HATCHING) {
		glUseProgram(renderManager.programs["line"]);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUniform2f(glGetUniformLocation(renderManager.programs["line"], "pixelSize"), 1.0f / this->width(), 1.0f / this->height());
		glUniformMatrix4fv(glGetUniformLocation(renderManager.programs["line"], "pMatrix"), 1, false, &camera.pMatrix[0][0]);
		if (renderManager.renderingMode == RenderManager::RENDERING_MODE_LINE) {
			glUniform1i(glGetUniformLocation(renderManager.programs["line"], "useHatching"), 0);
		}
		else {
			glUniform1i(glGetUniformLocation(renderManager.programs["line"], "useHatching"), 1);
		}

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex0"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex1"), 2);
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[1]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[2]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "tex3"), 4);
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[3]);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(renderManager.programs["line"], "hatchingTexture"), 5);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, renderManager.hatchingTextures);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Blur

	if (renderManager.renderingMode != RenderManager::RENDERING_MODE_LINE && renderManager.renderingMode != RenderManager::RENDERING_MODE_HATCHING) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		qglClearColor(QColor(0xFF, 0xFF, 0xFF));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		glUseProgram(renderManager.programs["blur"]);
		glUniform2f(glGetUniformLocation(renderManager.programs["blur"], "pixelSize"), 2.0f / this->width(), 2.0f / this->height());
		//printf("pixelSize loc %d\n", glGetUniformLocation(vboRenderManager.programs["blur"], "pixelSize"));

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex0"), 1);//COLOR
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[0]);

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex1"), 2);//NORMAL
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[1]);

		/*glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex2"), 3);
		glActiveTexture(GL_TEXTURE3);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDataTex[2]);*/

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "depthTex"), 8);
		glActiveTexture(GL_TEXTURE8);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragDepthTex);

		glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "tex3"), 4);//AO
		glActiveTexture(GL_TEXTURE4);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, renderManager.fragAOTex);

		if (renderManager.renderingMode == RenderManager::RENDERING_MODE_SSAO) {
			glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "ssao_used"), 1); // ssao used
		}
		else {
			glUniform1i(glGetUniformLocation(renderManager.programs["blur"], "ssao_used"), 0); // no ssao
		}

		glBindVertexArray(renderManager.secondPassVAO);

		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);
		glDepthFunc(GL_LEQUAL);

	}

	// REMOVE
	glActiveTexture(GL_TEXTURE0);


	//printf("<<\n");
	//VBOUtil::disaplay_memory_usage();




	// OpenGLの設定を元に戻す
	glShadeModel(GL_FLAT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// draw sketch
	QPainter painter(this);
	QPen pen(Qt::blue, 3);
	painter.setPen(pen);
	for (int k = 0; k < strokes.size(); ++k) {
		for (int i = 0; i < (int)strokes[k].size() - 1; ++i) {
			pen.setWidthF(stroke_widths[k][i]);
			painter.setPen(pen);
			painter.drawLine(strokes[k][i].x, height() - strokes[k][i].y, strokes[k][i + 1].x, height() - strokes[k][i + 1].y);
		}
	}

	// draw lasso
	QPen pen2(Qt::red, 3);
	painter.setPen(pen2);
	for (int i = 0; i < (int)lasso.size() - 1; ++i) {
		pen2.setWidthF(lasso_widths[i]);
		painter.setPen(pen2);
		painter.drawLine(lasso[i].x, height() - lasso[i].y, lasso[i + 1].x, height() - lasso[i + 1].y);
	}

	// draw the bottom area
	painter.setOpacity(0.6);
	QRect bottomArea(0, height() - BOTTOM_AREA_HEIGHT, width(), BOTTOM_AREA_HEIGHT);
	painter.fillRect(bottomArea, Qt::white);
	QFont font = painter.font();
	font.setPointSize(font.pointSize() * 3);
	painter.setFont(font);
	painter.drawText(bottomArea, Qt::AlignCenter | Qt::AlignVCenter, tr("Peek a final view"));
	painter.end();

	glEnable(GL_DEPTH_TEST);
}

void GLWidget3D::debug(const std::string& message, const std::vector<float>& values) {
	std::cout << message << ": ";
	for (int i = 0; i < values.size(); ++i) {
		if (i > 0) std::cout << ", ";
		std::cout << values[i];
	}
	std::cout << std::endl;
}