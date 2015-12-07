﻿#include <iostream>
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
	demo_mode = 0;

	// This is necessary to prevent the screen overdrawn by OpenGL
	setAutoFillBackground(false);

	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	//light_dir = glm::normalize(glm::vec3(-4, -5, -8));
	light_dir = glm::normalize(glm::vec3(-1, -3, -2));

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
	classifiers["building"] = new Classifier("../models/building/building.prototxt", "../models/building/building.caffemodel", "../models/building/mean.binaryproto");

	regressions["building"].resize(4);
	regressions["building"][0] = new Regression("../models/building/building_01.prototxt", "../models/building/building_01.caffemodel");
	regressions["building"][1] = new Regression("../models/building/building_02.prototxt", "../models/building/building_02.caffemodel");
	regressions["building"][2] = new Regression("../models/building/building_03.prototxt", "../models/building/building_03.caffemodel");
	regressions["building"][3] = new Regression("../models/building/building_04.prototxt", "../models/building/building_04.caffemodel");

	classifiers["roof"] = new Classifier("../models/roof/roof.prototxt", "../models/roof/roof.caffemodel", "../models/roof/mean.binaryproto");
	classifiers["window"] = new Classifier("../models/window/window.prototxt", "../models/window/window.caffemodel", "../models/window/mean.binaryproto");
	classifiers["ledge"] = new Classifier("../models/ledge/ledge.prototxt", "../models/ledge/ledge.caffemodel", "../models/ledge/mean.binaryproto");

	mcmc = new MCMC(this);
}

void GLWidget3D::drawLineTo(const QPoint &endPoint) {
	QPoint pt1(lastPos.x(), lastPos.y());
	QPoint pt2(endPoint.x(), endPoint.y());

	QPainter painter(&sketch);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.drawLine(pt1, pt2);

	strokes.back().push_back(glm::vec2(endPoint.x(), height() - endPoint.y()));

	lastPos = endPoint;
}

/**
 * Clear the canvas.
 */
void GLWidget3D::clearSketch() {
	sketch.fill(qRgba(255, 255, 255, 255));
	strokes.clear();
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
		if (stage == "final") {
			glUniform1i(glGetUniformLocation(renderManager.program, "useShadow"), 1);
		}
		else {
			glUniform1i(glGetUniformLocation(renderManager.program, "useShadow"), 0);
		}
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

	update();
}

void GLWidget3D::generateGeometry() {
	scene.generateGeometry(&renderManager, stage);

	if (stage == "final" || stage == "peek_final") {
		renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);
	}
}

void GLWidget3D::updateGeometry() {
	scene.updateGeometry(&renderManager, stage);
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

	// スケッチ画像を256x256に縮小
	QImage swapped = sketch.rgbSwapped();
	cv::Mat img = cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
	int size = std::min(img.rows, img.cols);
	cv::Mat img2 = img(cv::Rect((img.cols - size) / 2, (img.rows - size) / 2, size, size));
	if (size > 512) {
		cv::resize(img2, img2, cv::Size(512, 512));
		cv::threshold(img2, img2, 250, 255, CV_THRESH_BINARY);
	}
	cv::resize(img2, img2, cv::Size(256, 256));
	cv::threshold(img2, img, 250, 255, CV_THRESH_BINARY);

	std::vector<Prediction> predictions = classifiers["roof"]->Classify(img, 10);

	/*
	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["roof"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());
	*/

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

	QPainter painter(&sketch);
	mainWin->addListItem("1.00", grammarImages["floor"][result.first], result.first);
	for (int i = 0; i < grammarImages["floor"].size(); ++i) {
		if (i == result.first) continue;

		mainWin->addListItem("0.00", grammarImages["floor"][i], i);
	}

	predictFloor(result.first, result.second);

	update();
}

void GLWidget3D::updateWindowOptions() {
	mainWin->thumbsList->clear();

	// スケッチ画像を256x256に縮小
	QImage swapped = sketch.rgbSwapped();
	cv::Mat img = cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
	int size = std::min(img.rows, img.cols);
	cv::Mat img2 = img(cv::Rect((img.cols - size) / 2, (img.rows - size) / 2, size, size));
	if (size > 512) {
		cv::resize(img2, img2, cv::Size(512, 512));
		cv::threshold(img2, img2, 250, 255, CV_THRESH_BINARY);
	}
	cv::resize(img2, img2, cv::Size(256, 256));
	cv::threshold(img2, img, 250, 255, CV_THRESH_BINARY);

	std::vector<Prediction> predictions = classifiers["window"]->Classify(img, 10);
	
	/*
	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["window"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());
	*/

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

	// スケッチ画像を256x256に縮小
	QImage swapped = sketch.rgbSwapped();
	cv::Mat img = cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
	int size = std::min(img.rows, img.cols);
	cv::Mat img2 = img(cv::Rect((img.cols - size) / 2, (img.rows - size) / 2, size, size));
	if (size > 512) {
		cv::resize(img2, img2, cv::Size(512, 512));
		cv::threshold(img2, img2, 250, 255, CV_THRESH_BINARY);
	}
	cv::resize(img2, img2, cv::Size(256, 256));
	cv::threshold(img2, img, 250, 255, CV_THRESH_BINARY);

	std::vector<Prediction> predictions = classifiers["ledge"]->Classify(img, 10);

	/*
	std::vector<int> indexes;
	for (int i = 0; i < grammarImages["ledge"].size(); ++i) {
		indexes.push_back(i);
	}
	std::random_shuffle(indexes.begin(), indexes.end());
	*/

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
	time_t start = clock();
	
	renderManager.removeObjects();

	// convert the sketch to grayscale
	cv::Mat mat(sketch.height(), sketch.width(), CV_8UC3, sketch.bits(), sketch.bytesPerLine());
	cv::Mat grayMat;
	cv::cvtColor(mat, grayMat, CV_BGR2GRAY);

	// resize the sketch
	int size = std::min(grayMat.cols, grayMat.rows);
	cv::Mat resizedGrayMat = grayMat(cv::Rect((grayMat.cols - size) * 0.5, (grayMat.rows - size) * 0.5, size, size));
	cv::resize(resizedGrayMat, resizedGrayMat, cv::Size(256, 256));
	cv::threshold(resizedGrayMat, resizedGrayMat, 250, 255, CV_THRESH_BINARY);
	cv::resize(resizedGrayMat, resizedGrayMat, cv::Size(128, 128));
	cv::threshold(resizedGrayMat, resizedGrayMat, 250, 255, CV_THRESH_BINARY);

	// predict parameter values by deep learning
	std::vector<float> params = regressions["building"][grammar_id]->Predict(resizedGrayMat);
	for (int i = 0; i < params.size(); ++i) {
		std::cout << params[i] << ",";
	}
	std::cout << std::endl;

	//std::vector<float> params(7);
	//for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;

	// optimize the parameter values by MCMC
	if (strokes.size() > 2) {
		mcmc->optimize(grammars["building"][grammar_id], grayMat, 10.0f, 25, current_z, params);
	}
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

	std::cout << current_z << std::endl;
	scene.currentObject().setFootprint(offset_x, offset_y, current_z, object_width, object_depth);
	if (scene.faceSelector->selected()) {
		scene.alignObjects(scene.faceSelector->selectedFaceCopy());
	}
	else {
		scene.alignObjects();
	}
	//scene.alignObjectsForWillisTower();
	
	//std::cout << offset_x << "," << offset_y << "," << object_width << "," << object_depth << std::endl;

	// remove the first four parameters because they are not included in the grammar
	params.erase(params.begin(), params.begin() + 4);
	

	// HACK
	if (params[0] < 0) params[0] = 0;
	if (params[0] > 1) params[0] = 1;
	params[0] = (int)(params[0] * 10 + 0.5) * 0.1f;

	std::cout << "Height: " << params[0] << std::endl;


	// set parameter values
	scene.currentObject().setGrammar("Start", grammars["building"][grammar_id], params, true);
	
	// set height
	std::vector<std::pair<float, float> > ranges = cga::CGA::getParamRanges(grammars["building"][grammar_id]);
	scene.currentObject().setHeight((ranges[0].second - ranges[0].first) * params[0] + ranges[0].first);
	
	generateGeometry();

	// select the face again
	//faceSelector.reselectFace();
	//scene.updateGeometry(&renderManager, stage);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictRoof(int grammar_id) {
	//if (scene._selectedFaceName.empty()) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	time_t start = clock();

	renderManager.removeObjects();

	//////////////////////// DEBUG ////////////////////////
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["roof"][grammar_id]);

	generateGeometry();

	// select the face again
	//faceSelector.reselectFace();
	//scene.updateGeometry(&renderManager, stage);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictFacade(int grammar_id, const std::vector<float>& params) {
	//if (scene._selectedFaceName.empty()) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	// set grammar
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["facade"][grammar_id], params, false);
	generateGeometry();

	// select the face again
	//faceSelector.reselectFace();
	//scene.updateGeometry(&renderManager, stage);

	update();
}

void GLWidget3D::predictFloor(int grammar_id, const std::vector<float>& params) {
	//if (scene._selectedFaceName.empty()) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	// set grammar
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["floor"][grammar_id], params, false);
	generateGeometry();

	// select the face again
	//faceSelector.reselectFace();
	//scene.updateGeometry(&renderManager, stage);

	update();
}

void GLWidget3D::predictWindow(int grammar_id) {
	//if (scene._selectedFaceName.empty()) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	time_t start = clock();

	renderManager.removeObjects();

	//////////////////////// DEBUG ////////////////////////
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["window"][grammar_id]);

	generateGeometry();

	// select the face again
	//faceSelector.reselectFace();
	//scene.updateGeometry(&renderManager, stage);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictLedge(int grammar_id) {
	//if (scene._selectedFaceName.empty()) {
	if (!scene.faceSelector->selected()) {
		std::cout << "Warning: face is not selected." << std::endl;
		return;
	}

	time_t start = clock();

	renderManager.removeObjects();

	//////////////////////// DEBUG ////////////////////////
	scene.currentObject().setGrammar(scene.faceSelector->selectedFaceName(), grammars["ledge"][grammar_id]);

	generateGeometry();

	// select the face again
	//faceSelector.reselectFace();
	//scene.updateGeometry(&renderManager, stage);

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
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(0, 1, 0))) {
			scene.newObject();

			// shift the camera such that the selected face becomes a ground plane.
			intCamera = InterpolationCamera(camera, 30, -45, 0, computeDownwardedCameraPos(scene.faceSelector->selectedFace()->vertices[0].position.y + CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH, intCamera.camera_end.xrot));
			current_z = scene.faceSelector->selectedFace()->vertices[0].position.y;

			scene.faceSelector->selectedFace()->select();
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

			// shift the camera such that the selected face becomes a ground plane.
			intCamera = InterpolationCamera(camera, 30, yrot, 0.0, glm::vec3(0, scene.faceSelector->selectedFace()->vertices[0].position.y, CAMERA_DEFAULT_DEPTH));
			current_z = scene.faceSelector->selectedFace()->vertices[0].position.y;

			scene.faceSelector->selectedFace()->select();

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "facade") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// compute appropriate camera distance for the selected face
			float rot_y = atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

			scene.faceSelector->selectedFace()->select();

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "floor") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// compute appropriate camera distance for the selected face
			float rot_y = atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

			scene.faceSelector->selectedFace()->select();

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "window") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// compute appropriate camera distance for the selected face
			float rot_y = atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

			scene.faceSelector->selectedFace()->select();

			return true;
		}
		else {
			return false;
		}
	}
	else if (stage == "ledge") {
		if (scene.faceSelector->selectFace(cameraPos, view_dir, stage, glm::vec3(1, 0, 1))) {
			// compute appropriate camera distance for the selected face
			float rot_y = -M_PI * 0.4 + atan2f(scene.faceSelector->selectedFace()->vertices[0].normal.x, scene.faceSelector->selectedFace()->vertices[0].normal.z);
			glutils::Face rotatedFace = scene.faceSelector->selectedFace()->rotate(-rot_y, glm::vec3(0, 1, 0));
			float d1 = rotatedFace.bbox.sx() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d2 = rotatedFace.bbox.sy() * 0.5f / tanf(camera.fovy * M_PI / 180.0f * 0.5f);
			float d = std::max(d1, d2) * 1.5f;

			// turn the camera such that the selected face becomes parallel to the image plane.
			intCamera = InterpolationCamera(camera, 0, -rot_y / M_PI * 180, 0, glm::vec3(rotatedFace.bbox.center().x, rotatedFace.bbox.center().y, rotatedFace.bbox.maxPt.z + d));

			scene.faceSelector->selectedFace()->select();

			return true;
		}
		else {
			return false;
		}
	}
}

bool GLWidget3D::selectBuilding(const glm::vec2& mouse_pos) {
	// camera position in the world coordinates
	glm::vec3 cameraPos = camera.cameraPosInWorld();

	clearSketch();

	// view direction
	glm::vec3 view_dir = viewVector(mouse_pos, camera.mvMatrix, camera.f(), camera.aspect());

	if (scene.buildingSelector->selectBuilding(cameraPos, view_dir)) {
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
	if (scene.buildingSelector->selectBuildingControlPoint(cameraPos, view_dir, mouse_pos, camera.mvpMatrix, width(), height())) {
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
	} else if (stage == "roof") {
	} else if (stage == "facade") {
	} else if (stage == "floor") {
	} else if (stage == "window") {
	} else if (stage == "ledge") {
	} else if (stage == "final") {
		generateGeometry();
	}

	updateGeometry();

	update();
}

void GLWidget3D::changeMode(int new_mode) {
	if (new_mode == MODE_COPY) {
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
		if (scene.buildingSelector->isBuildingSelected()) {
			scene.buildingSelector->unselectBuilding();
			updateGeometry();
		}

		mode = new_mode;
	}

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

glm::vec3 GLWidget3D::computeDownwardedCameraPos(float downward, float distToCamera, float camera_xrot) {
	return glm::vec3(0,
		downward * cosf(camera_xrot / 180.0f * M_PI),
		downward * sinf(camera_xrot / 180.0f * M_PI) + distToCamera);
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
	dragging = true;

	if (mode == MODE_CAMERA) { // move camera
		camera.mousePress(e->x(), e->y());
	}
	else if (mode == MODE_SELECT) {
		// do nothing
	}
	else if (mode == MODE_SELECT_BUILDING) {
		if (scene.buildingSelector->isBuildingSelected()) {
			selectBuildingControlPoint(glm::vec2(e->x(), e->y()));

			updateGeometry();
			update();
		}
	}
	else { // start drawing a stroke
		lastPos = e->pos();
		strokes.resize(strokes.size() + 1);
	}
}

/**
 * This event handler is called when the mouse button is released.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
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
			scene.buildingSelector->alignObjects();
			scene.buildingSelector->unselectBuildingControlPoint();
			generateGeometry();
		}
		else {
			if (selectBuilding(glm::vec2(e->x(), e->y()))) {
				std::cout << "A building is selected." << std::endl;
			}
		}

		updateGeometry();

		update();
	}
	else {
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
	if (dragging) {
		if (mode == MODE_CAMERA) {
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
		else if (mode == MODE_SELECT_BUILDING) {
			if (scene.buildingSelector->isBuildingControlPointSelected()) {
				// resize the building
				scene.buildingSelector->resize(glm::vec2(e->x(), e->y()));
				generateGeometry();
			}
		}
		else { // keep drawing a stroke
			drawLineTo(e->pos());
		}

		update();
	}
	else {
		if (e->y() > height() - BOTTOM_AREA_HEIGHT) {
			if (stage != "peek_final") {
				preStage = stage;
				stage = "peek_final";

				// generate final geometry
				generateGeometry();

				update();
			}
		}
		else {
			if (stage == "peek_final") {
				stage = preStage;

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
	renderManager.init("../shaders/vertex.glsl", "../shaders/geometry.glsl", "../shaders/fragment.glsl", false);
	renderManager.renderingMode = RenderManager::RENDERING_MODE_REGULAR;

	glClearColor(0.9, 0.9, 0.9, 0.0);

	sketch = QImage(this->width(), this->height(), QImage::Format_RGB888);
	sketch.fill(qRgba(255, 255, 255, 255));

	mode = MODE_SKETCH;
	
	camera.xrot = 30.0f;
	camera.yrot = -45.0f;
	camera.zrot = 0.0f;
	camera.pos = computeDownwardedCameraPos(CAMERA_DEFAULT_HEIGHT, CAMERA_DEFAULT_DEPTH, camera.xrot);
	current_z = 0.0f;
	scene.updateGeometry(&renderManager, "building");

	//changeStage("building");
	stage = "building";

	setMouseTracking(true);
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

	// draw sketch
	QPainter painter(this);
	//painter.setOpacity(0.5);
	//painter.drawImage(0, 0, sketch);
	for (auto stroke : strokes) {
		for (int i = 0; i < (int)stroke.size() - 1; ++i) {
			painter.drawLine(stroke[i].x, height() - stroke[i].y, stroke[i + 1].x, height() - stroke[i + 1].y);
		}
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

