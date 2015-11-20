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
//#include "Regression.h"
#include <time.h>
#include "LeftWindowItemWidget.h"

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {
	mainWin = (MainWindow*)parent;
	dragging = false;
	ctrlPressed = false;
	shiftPressed = false;

	// これがないと、QPainterによって、OpenGLによる描画がクリアされてしまう
	setAutoFillBackground(false);

	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-100, 100, -100, 100, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;

	std::string stage_names[6] = { "building", "roof", "facade", "window", "ledge" };

	// load grammar
	for (int i = 0; i < 6; ++i) {
		// load grammar
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
	/*
	regressions.resize(2);
	regressions[0] = new Regression("../models/cuboid_43/deploy.prototxt", "../models/cuboid_43/train_iter_64000.caffemodel");
	regressions[1] = new Regression("../models/lshape_44/deploy.prototxt", "../models/lshape_44/train_iter_64000.caffemodel");
	*/
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
	scene.generateGeometry(&renderManager);
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

		sc::ShapeLayer sl;
		sl.setFootprint(0, 0, 0, 16, 12);

		std::vector<float> params(10);
		for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;
		sl.setGrammar("Start", grammar, params);
		//sl.setGrammar(grammar, params);

		cga::CGA system;
		sl.generateGeometry(&system, &renderManager);
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
		scene.building.currentLayer().setGrammar("Facade", grammars["facade"][option_index]);
		scene.generateGeometry(&renderManager);
	} else if (stage == "window") {
		predictWindow(option_index);
	} else if (stage == "ledge") {
		predictLedge(option_index);
	}

	update();
}

void GLWidget3D::updateBuildingOptions() {
	mainWin->thumbsList->clear();

	QPainter painter(&sketch);
	for (size_t i = 0; i < grammarImages["building"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["building"][i], i);
	}

	predictBuilding(0);

	update();
}

void GLWidget3D::updateRoofOptions() {
	mainWin->thumbsList->clear();

	QPainter painter(&sketch);
	for (size_t i = 0; i < grammarImages["roof"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["roof"][i], i);
	}

	predictRoof(0);

	update();
}

void GLWidget3D::updateFacadeOptions() {
	mainWin->thumbsList->clear();

	QPainter painter(&sketch);
	for (size_t i = 0; i < grammarImages["facade"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["facade"][i], i);
	}

	predictFacade(0);

	update();
}

void GLWidget3D::updateWindowOptions() {
	mainWin->thumbsList->clear();

	QPainter painter(&sketch);
	for (size_t i = 0; i < grammarImages["window"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["window"][i], i);
	}

	predictFacade(0);

	update();
}

void GLWidget3D::updateLedgeOptions() {
	mainWin->thumbsList->clear();

	QPainter painter(&sketch);
	for (size_t i = 0; i < grammarImages["ledge"].size(); ++i) {
		mainWin->addListItem("???", grammarImages["ledge"][i], i);
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
	cv::resize(grayMat, grayMat, cv::Size(256, 256));
	cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);
	cv::resize(grayMat, grayMat, cv::Size(128, 128));
	cv::threshold(grayMat, grayMat, 250, 255, CV_THRESH_BINARY);

	// predict parameter values by deep learning
	//std::vector<float> params = regressions[grammar_id]->Predict(grayMat);
	
	// DEBUG用に、Deeplearningを使わないで、固定のパラメータ値を使用する
	std::vector<float> params(7);
	for (int i = 0; i < params.size(); ++i) params[i] = 0.5f;

	float offset_x = params[0] * 12 - 6;
	float offset_y = params[1] * 12 - 6;
	float object_width = params[2] * 12 + 8;
	float object_depth = params[3] * 12 + 8;
	offset_x -= object_width * 0.5f;
	offset_y -= object_depth * 0.5f;

	scene.building.currentLayer().setFootprint(offset_x, offset_y, current_z, object_width, object_depth);
	
	//std::cout << offset_x << "," << offset_y << "," << object_width << "," << object_depth << std::endl;

	// remove the first four parameters because they are not included in the grammar
	params.erase(params.begin(), params.begin() + 4);
	
	// set parameter values
	scene.building.currentLayer().setGrammar("Footprint", grammars["building"][grammar_id], params);
	
	// set height
	std::vector<std::pair<float, float> > ranges = cga::CGA::getParamRanges(grammars["building"][grammar_id]);
	scene.building.currentLayer().setHeight((ranges[0].second - ranges[0].first) * params[0] + ranges[0].first);
	
	scene.generateGeometry(&renderManager);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictRoof(int grammar_id) {
	time_t start = clock();

	renderManager.removeObjects();

	// DEBUG用に、roof grammarを固定で選択
	scene.building.currentLayer().setGrammar("Roof", grammars["roof"][grammar_id]);

	scene.generateGeometry(&renderManager);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictFacade(int grammar_id) {
	if (scene.building.selectedFace() == NULL) return;

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
	for (int i = 0; i < scene.building.selectedFace()->vertices.size(); ++i) {
		glm::vec4 projectedPt = camera.mvpMatrix * glm::vec4(scene.building.selectedFace()->vertices[i].position, 1);
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

	// DEBUG用に、facade grammarを固定で選択
	//scene.building.currentLayer().setGrammar("Facade", grammars["facade"][grammar_id]);

	//scene.generateGeometry(&renderManager);

	update();
}

void GLWidget3D::predictWindow(int grammar_id) {
	time_t start = clock();

	renderManager.removeObjects();

	// DEBUG用に、window grammarを固定で選択
	scene.building.currentLayer().setGrammar("Window", grammars["window"][grammar_id]);

	scene.generateGeometry(&renderManager);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictLedge(int grammar_id) {
	time_t start = clock();

	renderManager.removeObjects();

	// DEBUG用に、ledge grammarを固定で選択
	scene.building.currentLayer().setGrammar("Ledge", grammars["ledge"][grammar_id]);

	scene.generateGeometry(&renderManager);

	time_t end = clock();
	//std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::fixGeometry() {
	if (stage == "building") {
		clearSketch();
		scene.building.newLayer();
		update();
	} else if (stage == "roof") {
		clearSketch();
		update();
	} else if (stage == "facade") {
		clearSketch();
		update();
	} else if (stage == "window") {
		clearSketch();
		update();
	} else if (stage == "ledge") {
		clearSketch();
		update();
	}
}

glm::vec3 GLWidget3D::viewVector(const glm::vec2& point, const glm::mat4& mvMatrix, float focalLength, float aspect) {
	glm::vec3 dir((point.x - width() * 0.5f) * 2.0f / width() * aspect, (height() * 0.5f - point.y) * 2.0f / height(), -focalLength);
	return glm::vec3(glm::inverse(mvMatrix) * glm::vec4(dir, 0));
}

void GLWidget3D::changeStage(const std::string& stage) {
	this->stage = stage;
	clearSketch();

	if (scene.building.selectedFace() != NULL) {
		scene.building.selectedFace()->unselect();
		scene.building.unselectFace();
	}

	if (stage == "building") {
		camera.pos = glm::vec3(0, 0, CAMERA_DEFAULT_DEPTH);
		camera.xrot = 30.0f;
		camera.yrot = -45.0f;
		camera.zrot = 0.0f;
		current_z = 0.0f;
		camera.updateMVPMatrix();
	} else if (stage == "roof") {
	} else if (stage == "facade") {
	} else if (stage == "window") {
	} else if (stage == "ledge") {
	}

	update();
}


void GLWidget3D::keyPressEvent(QKeyEvent *e) {
	ctrlPressed = false;

	switch (e->key()) {
	case Qt::Key_Control:
		ctrlPressed = true;
		break;
	case Qt::Key_Shift:
		shiftPressed = true;
		break;
	default:
		break;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	ctrlPressed = false;
	shiftPressed = false;
}

/**
 * This event handler is called when the mouse button is pressed.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	if (ctrlPressed) { // move camera
		camera.mousePress(e->x(), e->y());
	}
	else if (shiftPressed) { // select a face
		std::cout << "shift clicked" << std::endl;
		// camera position in the world coordinates
		glm::vec3 cameraPos = camera.cameraPosInWorld();

		// view direction
		glm::vec3 view_dir = viewVector(glm::vec2(e->x(), e->y()), camera.mvMatrix, camera.f(), camera.aspect());

		if (stage == "building") {
			if (scene.building.selectTopFace(cameraPos, view_dir)) {
				// shift the camera such that the selected face becomes a ground plane.
				camera.pos = glm::vec3(0, scene.building.selectedFace()->vertices[0].position.y, CAMERA_DEFAULT_DEPTH);
				camera.xrot = 30.0f;
				camera.yrot = -45.0f;
				camera.zrot = 0.0f;
				current_z = scene.building.selectedFace()->vertices[0].position.y;
			}
			else {
				// shift the camera such that the ground plane becomes really a ground plane.
				camera.pos = glm::vec3(0, 0, CAMERA_DEFAULT_DEPTH);
				camera.xrot = 30.0f;
				camera.yrot = -45.0f;
				camera.zrot = 0.0f;
				current_z = 0;
			}
			camera.updateMVPMatrix();
		}
		else if (stage == "roof") {
			if (scene.building.selectTopFace(cameraPos, view_dir)) {
				// shift the camera such that the selected face becomes a ground plane.
				camera.pos = glm::vec3(0, scene.building.selectedFace()->vertices[0].position.y, CAMERA_DEFAULT_DEPTH);
				camera.xrot = 30.0f;
				camera.yrot = -45.0f;
				camera.zrot = 0.0f;
				current_z = scene.building.selectedFace()->vertices[0].position.y;
			}
			camera.updateMVPMatrix();
			updateRoofOptions();
		}
		else if (stage == "facade") {
			if (scene.building.selectSideFace(cameraPos, view_dir)) {
				// turn the camera such that the selected face becomes parallel to the image plane.
				camera.pos.y = scene.building.selectedFace()->bbox.center().y;
				camera.xrot = 0.0f;
				camera.yrot = -atan2f(scene.building.selectedFace()->vertices[0].normal.x, scene.building.selectedFace()->vertices[0].normal.z) / 3.141592653 * 180;
				camera.zrot = 0.0f;
			}
			camera.updateMVPMatrix();
			updateFacadeOptions();
		}
		else if (stage == "window") {
			if (scene.building.selectSideFace(cameraPos, view_dir)) {
				// turn the camera such that the selected face becomes parallel to the image plane.
				camera.pos.y = scene.building.selectedFace()->bbox.center().y;
				camera.xrot = 0.0f;
				camera.yrot = -atan2f(scene.building.selectedFace()->vertices[0].normal.x, scene.building.selectedFace()->vertices[0].normal.z) / 3.141592653 * 180;
				camera.zrot = 0.0f;
			}
			camera.updateMVPMatrix();
			updateWindowOptions();
		}
		else if (stage == "ledge") {
			if (scene.building.selectSideFace(cameraPos, view_dir)) {
				// turn the camera such that the selected face becomes perpendicular to the image plane.
				camera.pos.y = scene.building.selectedFace()->bbox.center().y;
				camera.xrot = 0.0f;
				camera.yrot = -atan2f(scene.building.selectedFace()->vertices[0].normal.x, scene.building.selectedFace()->vertices[0].normal.z) / 3.141592653 * 180;
				//camera.yrot = atan2f(scene.building.selectedFace()->vertices[0].normal.z, scene.building.selectedFace()->vertices[0].normal.x) / 3.141592653 * 180;
				camera.zrot = 0.0f;
			}
			camera.updateMVPMatrix();
			updateLedgeOptions();
		}
		scene.updateGeometry(&renderManager);
		update();
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
	else if (shiftPressed) {
		// do nothing
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
	if (ctrlPressed) { // movign a camera
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
	else if (shiftPressed) {
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

	changeStage("building");
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



