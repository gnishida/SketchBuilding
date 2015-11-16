#pragma once

#include <glew.h>
#include "Shader.h"
#include "Vertex.h"
#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <vector>
#include <QImage>
#include "Camera.h"
#include "ShadowMapping.h"
#include "RenderManager.h"
#include "CGA.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Scene.h"

using namespace std;
class Classifier;
class MainWindow;
class Regression;

class GLWidget3D : public QGLWidget {
public:
	static enum { STAGE_BUILDING, STAGE_ROOF, STAGE_FACADE, STAGE_FLOOR, STAGE_WINDOW, STAGE_LEDGE };

	MainWindow* mainWin;
	QImage sketch;
	QPoint lastPos;
	bool dragging;
	bool ctrlPressed;
	
	int stage;
	std::vector<Regression*> regressions;
	std::vector<cga::Grammar> grammars;
	int shapeType;
	Scene scene;

	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;
	RenderManager renderManager;
	//cga::CGA system;

public:
	GLWidget3D(QWidget *parent);
	void drawLineTo(const QPoint &endPoint);
	void clearSketch();
	void clearGeometry();
	void drawScene(int drawMode);
	//void loadCGA(char* filename);
	void predict();
	void fixGeometry();
	void newLayer();
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void initializeGL();
	void resizeGL(int width, int height);
	void paintEvent(QPaintEvent *event);

};

