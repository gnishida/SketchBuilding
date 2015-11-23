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
//class Regression;

class GLWidget3D : public QGLWidget {
public:
	static enum { STAGE_BUILDING = 0, STAGE_ROOF, STAGE_FACADE, STAGE_WINDOW, STAGE_LEDGE };
	
	const float CAMERA_DEFAULT_DEPTH = 50.0f;

	MainWindow* mainWin;
	QImage sketch;
	std::vector<std::vector<glm::vec2> > strokes;
	QPoint lastPos;
	bool dragging;
	bool ctrlPressed;
	bool shiftPressed;
		
	std::string stage;
	//std::vector<Regression*> regressions;
	std::map<std::string, std::vector<QImage> > grammarImages;
	std::map<std::string, std::vector<cga::Grammar> > grammars;
	sc::Scene scene;
	float current_z;

	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;
	RenderManager renderManager;

public:
	GLWidget3D(QWidget *parent);
	void drawLineTo(const QPoint &endPoint);
	void clearSketch();
	void clearGeometry();
	void drawScene(int drawMode);
	void loadCGA(char* filename);
	void selectOption(int option_index);

	void updateBuildingOptions();
	void updateRoofOptions();
	void updateFacadeOptions();
	void updateFloorOptions();
	void updateWindowOptions();
	void updateLedgeOptions();

	void predictBuilding(int grammar_id);
	void predictRoof(int grammar_id);
	void predictFacade(int grammar_id);
	void predictFloor(int grammar_id);
	void predictWindow(int grammar_id);
	void predictLedge(int grammar_id);

	void selectFace(const glm::vec2& mouse_pos);
	void addBuildingMass();
	glm::vec3 viewVector(const glm::vec2& point, const glm::mat4& mvMatrix, float focalLength, float aspect);
	void changeStage(const std::string& stage);
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

