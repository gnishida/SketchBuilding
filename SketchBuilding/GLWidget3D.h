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
#include <QTimer>
#include "InterpolationCamera.h"
#include "FaceSelector.h"

using namespace std;

class Classifier;
class MainWindow;
class Regression;
class MCMC;

class GLWidget3D : public QGLWidget {
public:
	static enum { STAGE_BUILDING = 0, STAGE_ROOF, STAGE_FACADE, STAGE_WINDOW, STAGE_LEDGE };
	static enum { MODE_SKETCH = 0, MODE_SELECT, MODE_SELECT_BUILDING, MODE_COPY, MODE_ERASER, MODE_CAMERA };
	
	const int BOTTOM_AREA_HEIGHT = 40;
	const float CAMERA_DEFAULT_HEIGHT = 5.0f;
	const float CAMERA_DEFAULT_DEPTH = 80.0f;
	
	MainWindow* mainWin;
	QImage sketch;
	std::vector<std::vector<glm::vec2> > strokes;
	std::vector<std::vector<float> > stroke_widths;
	std::vector<glm::vec2> lasso;
	std::vector<float> lasso_widths;
	float pen_pressure;
	QPoint lastPos;
	bool dragging;
	bool ctrlPressed;
	bool shiftPressed;

	bool tableEventUsed;

	std::string stage;
	std::string preStage;
	int preRenderingMode;
	int mode;
	int demo_mode;
	std::map<std::string, std::vector<Regression*> > regressions;
	std::map<std::string, Classifier*> classifiers;
	MCMC* mcmc;
	std::map<std::string, std::vector<QImage> > grammarImages;
	std::map<std::string, std::vector<cga::Grammar> > grammars;
	sc::Scene scene;
	float current_z;

	Camera camera;
	glm::vec3 light_dir;
	glm::mat4 light_mvpMatrix;
	RenderManager renderManager;

	InterpolationCamera intCamera;
	QTimer* camera_timer;

public:
	GLWidget3D(QWidget* parent);
	void drawLineTo(const QPoint& endPoint, float width);
	void drawLassoLineTo(const QPoint& endPoint, float width);
	void clearSketch();
	void clearGeometry();
	void drawScene();
	void loadCGA(char* filename);
	void generateGeometry();
	void updateGeometry();
	void selectOption(int option_index);

	void updateBuildingOptions();
	void updateRoofOptions();
	void updateFacadeOptions();
	void updateFloorOptions();
	void updateWindowOptions();
	void updateLedgeOptions();

	void predictBuilding(int grammar_id);
	void predictRoof(int grammar_id);
	void predictFacade(int grammar_id, const std::vector<float>& params);
	void predictFloor(int grammar_id, const std::vector<float>& params);
	void predictWindow(int grammar_id);
	void predictLedge(int grammar_id);

	bool selectFace(const glm::vec2& mouse_pos);
	bool selectStageAndFace(const glm::vec2& mouse_pos);
	bool selectBuilding(const glm::vec2& mouse_pos);
	bool selectBuildingControlPoint(const glm::vec2& mouse_pos);
	void addBuildingMass();
	glm::vec3 viewVector(const glm::vec2& point, const glm::mat4& mvMatrix, float focalLength, float aspect);
	void convertSketch(bool regression, cv::Mat& result);
	void changeStage(const std::string& stage);
	void changeMode(int new_mode);
	glm::vec3 computeDownwardedCameraPos(float downward, float distToCamera, float camera_xrot);
	void camera_update();
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

protected:
	void tabletEvent(QTabletEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void initializeGL();
	void resizeGL(int width, int height);
	void paintEvent(QPaintEvent* e);

	void debug(const std::string& message, const std::vector<float>& values);
};

