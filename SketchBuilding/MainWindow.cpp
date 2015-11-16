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

	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionPredict, SIGNAL(triggered()), this, SLOT(onPredict()));
	connect(ui.actionCuboid, SIGNAL(triggered()), this, SLOT(onSelectShape()));
	connect(ui.actionLShape, SIGNAL(triggered()), this, SLOT(onSelectShape()));
	connect(ui.actionNewLayer, SIGNAL(triggered()), this, SLOT(onNewLayer()));

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