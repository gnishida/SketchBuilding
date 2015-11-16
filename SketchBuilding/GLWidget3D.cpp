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
#include <time.h>

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

	// load grammar
	grammars["building"].resize(2);
	grammars["facade"].resize(1);
	try {
		cga::parseGrammar("../cga/simple_shapes/shape_01.xml", grammars["building"][0]);
		cga::parseGrammar("../cga/simple_shapes/shape_02.xml", grammars["building"][1]);
		cga::parseGrammar("../cga/facade/facade_01.xml", grammars["facade"][0]);
	}
	catch (const std::string& ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
	catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}

	// initialize deep learning network
	regressions.resize(2);
	regressions[0] = new Regression("../models/cuboid_43/deploy.prototxt", "../models/cuboid_43/train_iter_64000.caffemodel");
	regressions[1] = new Regression("../models/lshape_44/deploy.prototxt", "../models/lshape_44/train_iter_64000.caffemodel");

	changeStage(STAGE_BUILDING);
	shapeType = 0;
}

void GLWidget3D::drawLineTo(const QPoint &endPoint) {
	QPoint pt1(lastPos.x(), lastPos.y());
	QPoint pt2(endPoint.x(), endPoint.y());

	QPainter painter(&sketch);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.drawLine(pt1, pt2);

	lastPos = endPoint;
}

/**
 * Clear the canvas.
 */
void GLWidget3D::clearSketch() {
	sketch.fill(qRgba(255, 255, 255, 255));

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
/*
void GLWidget3D::loadCGA(char* filename) {
	renderManager.removeObjects();

	float object_width = 8.0f;
	float object_height = 8.0f;

	{ // for parthenon
		cga::Rectangle* start = new cga::Rectangle("Start", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(-object_width*0.5f, -object_height*0.5f, 0)), glm::mat4(), object_width, object_height, glm::vec3(1, 1, 1));
		system.stack.push_back(boost::shared_ptr<cga::Shape>(start));
	}

	try {
		cga::Grammar grammar;
		cga::parseGrammar(filename, grammar);
		system.randomParamValues(grammar);
		system.derive(grammar);
		system.generateGeometry(&renderManager);
	}
	catch (const std::string& ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
	catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}

	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);

	updateGL();
}
*/

/**
 * Use the sketch as an input to the pretrained network, and obtain the probabilities as output.
 * Then, display the options ordered by the probabilities.
 */
void GLWidget3D::predictBuilding() {
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
	std::vector<float> params = regressions[shapeType]->Predict(grayMat);
	/*
	for (int i = 0; i < params.size(); ++i) {
		std::cout << params[i] << ",";
	}
	std::cout << std::endl;
	*/

	float offset_x = params[0] * 12 - 6;
	float offset_y = params[1] * 12 - 6;
	float object_width = params[2] * 8 + 8;
	float object_depth = params[3] * 8 + 8;
	offset_x -= object_width * 0.5f;
	offset_y -= object_depth * 0.5f;

	scene.building.currentLayer().setFootprint(offset_x, offset_y, current_z, object_width, object_depth);

	std::cout << offset_x << "," << offset_y << "," << object_width << "," << object_depth << std::endl;

	// remove the first four parameters because they are not included in the grammar
	params.erase(params.begin(), params.begin() + 4);

	// set parameter values
	scene.building.currentLayer().setGrammar(grammars["building"][shapeType], params);

	// set height
	std::vector<std::pair<float, float> > ranges = cga::CGA::getParamRanges(grammars["building"][shapeType]);
	scene.building.currentLayer().setHeight((ranges[0].second - ranges[0].first) * params[0] + ranges[0].first);
	
	scene.generateGeometry(&renderManager);

	time_t end = clock();
	std::cout << "Duration: " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << std::endl;

	update();
}

void GLWidget3D::predictFacade() {

}

void GLWidget3D::fixGeometry() {
	if (stage == STAGE_BUILDING) {
		clearSketch();
		scene.building.newLayer();
		update();
	}
	else if (stage == STAGE_ROOF) {

	}
	else {

	}
}

void GLWidget3D::newLayer() {
	clearSketch();
	scene.building.newLayer();
	scene.generateGeometry(&renderManager);
	update();
}

glm::vec3 GLWidget3D::viewVector(const glm::vec2& point, const glm::mat4& mvMatrix, float focalLength, float aspect) {
	glm::vec3 dir((point.x - width() * 0.5f) * 2.0f / width() * aspect, (height() * 0.5f - point.y) * 2.0f / height(), -focalLength);
	return glm::vec3(glm::inverse(mvMatrix) * glm::vec4(dir, 0));
}

void GLWidget3D::changeStage(int stage) {
	this->stage = stage;
	clearSketch();

	switch (stage) {
	case STAGE_BUILDING:
		camera.pos = glm::vec3(0, 0, 40);
		camera.xrot = 30.0f;
		camera.yrot = -45.0f;
		camera.zrot = 0.0f;
		current_z = 0.0f;
		break;
	case STAGE_FACADE:
		break;
	}

	camera.updateMVPMatrix();
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

		// view vector
		glm::vec3 view_v1 = viewVector(glm::vec2(e->x(), e->y()), camera.mvMatrix, camera.f(), camera.aspect());

		if (stage == STAGE_BUILDING) {
			glutils::Face* selectedFace;
			if (scene.building.selectTopFace(cameraPos, view_v1, &selectedFace)) {
				// shift the camera such that the selected face becomes a ground plane.
				camera.pos.y = selectedFace->vertices[0].position.y;
				current_z = selectedFace->vertices[0].position.y;
			}
			else {
				// shift the camera such that the ground plane becomes really a ground plane.
				camera.pos.y = 0;
				current_z = 0;
			}
			camera.updateMVPMatrix();
		}
		else if (stage == STAGE_FACADE) {
			glutils::Face* selectedFace;
			if (scene.building.selectSideFace(cameraPos, view_v1, &selectedFace)) {
				// shift the camera such that the selected face becomes parallel to the image plane.
				camera.pos.y = selectedFace->bbox.center().y;
				camera.xrot = 0.0f;
				camera.yrot = -atan2f(selectedFace->vertices[0].normal.x, selectedFace->vertices[0].normal.z) / 3.141592653 * 180;
				camera.zrot = 0.0f;
			}
			camera.updateMVPMatrix();
		}
		else {

		}
		scene.updateGeometry(&renderManager);
		update();
	}
	else { // start drawing a stroke
		lastPos = e->pos();
		dragging = true;
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

		if (stage == STAGE_BUILDING) {
			predictBuilding();
		}
		else if (stage == STAGE_ROOF) {
			// To Do
		}
		else if (stage == STAGE_FACADE) {
			predictFacade();
		}
		else if (stage == STAGE_FLOOR) {
			// To Do
		}
		else if (stage == STAGE_WINDOW) {
			// To Do
		}
		else if (stage == STAGE_LEDGE) {
			// To Do
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



