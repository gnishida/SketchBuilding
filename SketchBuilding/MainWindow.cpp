#include "MainWindow.h"
#include "Regression.h"
#include <QFileDialog>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	QActionGroup* shapeGroup = new QActionGroup(this);
	shapeGroup->addAction(ui.actionCuboid);
	shapeGroup->addAction(ui.actionLShape);
	ui.actionCuboid->setChecked(true);

	QActionGroup* stageGroup = new QActionGroup(this);
	actionStages["building"] = ui.mainToolBar->addAction("Building");
	actionStages["building"]->setCheckable(true);
	actionStages["building"]->setChecked(true);
	stageGroup->addAction(actionStages["building"]);
	actionStages["roof"] = ui.mainToolBar->addAction("Roof");
	actionStages["roof"]->setCheckable(true);
	stageGroup->addAction(actionStages["roof"]);
	actionStages["facade"] = ui.mainToolBar->addAction("Facade");
	actionStages["facade"]->setCheckable(true);
	stageGroup->addAction(actionStages["facade"]);
	actionStages["floor"] = ui.mainToolBar->addAction("Floor");
	actionStages["floor"]->setCheckable(true);
	stageGroup->addAction(actionStages["floor"]);
	actionStages["window"] = ui.mainToolBar->addAction("Window");
	actionStages["window"]->setCheckable(true);
	stageGroup->addAction(actionStages["window"]);
	actionStages["ledge"] = ui.mainToolBar->addAction("Ledge");
	actionStages["ledge"]->setCheckable(true);
	stageGroup->addAction(actionStages["ledge"]);

	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionPredict, SIGNAL(triggered()), this, SLOT(onPredict()));
	connect(ui.actionFixGeometry, SIGNAL(triggered()), this, SLOT(onFixGeometry()));
	connect(ui.actionCuboid, SIGNAL(triggered()), this, SLOT(onSelectShape()));
	connect(ui.actionLShape, SIGNAL(triggered()), this, SLOT(onSelectShape()));
	connect(ui.actionNewLayer, SIGNAL(triggered()), this, SLOT(onNewLayer()));

	connect(actionStages["building"], SIGNAL(triggered()), this, SLOT(onStageChanged()));
	connect(actionStages["roof"], SIGNAL(triggered()), this, SLOT(onStageChanged()));
	connect(actionStages["facade"], SIGNAL(triggered()), this, SLOT(onStageChanged()));
	connect(actionStages["floor"], SIGNAL(triggered()), this, SLOT(onStageChanged()));
	connect(actionStages["window"], SIGNAL(triggered()), this, SLOT(onStageChanged()));
	connect(actionStages["ledge"], SIGNAL(triggered()), this, SLOT(onStageChanged()));

	glWidget = new GLWidget3D(this);

	this->setCentralWidget(glWidget);
}

/**
 * This is called when the user clicks [File] -> [New].
 */
void MainWindow::onNew() {
	glWidget->clearSketch();
	glWidget->clearGeometry();
}

/**
* This is called when the user clickes [Tool] -> [Predict]
*/
void MainWindow::onPredict() {
	glWidget->predict();
}

void MainWindow::onFixGeometry() {
	glWidget->fixGeometry();
}

void MainWindow::onSelectShape() {
	if (ui.actionCuboid->isChecked()) {
		glWidget->shapeType = 0;
	}
	else if (ui.actionLShape->isChecked()) {
		glWidget->shapeType = 1;
	}
}

void MainWindow::onNewLayer() {
	glWidget->newLayer();
}

void MainWindow::onStageChanged() {
	if (actionStages["building"]->isChecked()) {
		glWidget->changeStage(GLWidget3D::STAGE_BUILDING);
	}
	else if (actionStages["roof"]->isChecked()) {
		glWidget->changeStage(GLWidget3D::STAGE_ROOF);
	}
	else if (actionStages["facade"]->isChecked()) {
		glWidget->changeStage(GLWidget3D::STAGE_FACADE);
	}
	else if (actionStages["floor"]->isChecked()) {
		glWidget->changeStage(GLWidget3D::STAGE_FLOOR);
	}
	else if (actionStages["window"]->isChecked()) {
		glWidget->changeStage(GLWidget3D::STAGE_WINDOW);
	}
	else if (actionStages["ledge"]->isChecked()) {
		glWidget->changeStage(GLWidget3D::STAGE_LEDGE);
	}
}