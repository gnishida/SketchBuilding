#include "MainWindow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include "LeftWindowItemWidget.h"
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	QActionGroup* shapeGroup = new QActionGroup(this);
	shapeGroup->addAction(ui.actionCuboid);
	shapeGroup->addAction(ui.actionLShape);
	ui.actionCuboid->setChecked(true);

	// add menu handlers
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionFixGeometry, SIGNAL(triggered()), this, SLOT(onFixGeometry()));
	connect(ui.actionCuboid, SIGNAL(triggered()), this, SLOT(onSelectShape()));
	connect(ui.actionLShape, SIGNAL(triggered()), this, SLOT(onSelectShape()));
	connect(ui.actionNewLayer, SIGNAL(triggered()), this, SLOT(onNewLayer()));

	// create tool bar
	QActionGroup* stageGroup = new QActionGroup(this);
	std::string stage_names[6] = { "building", "roof", "facade", "floor", "window", "ledge" };
	for (int i = 0; i < 6; ++i) {
		QIcon ic("resources/building.png");

		actionStages[stage_names[i]] = new QAction(QIcon(std::string("resources/" + stage_names[i] + ".png").c_str()), std::string("&" + stage_names[i]).c_str(), this);
		actionStages[stage_names[i]]->setCheckable(true);
		stageGroup->addAction(actionStages[stage_names[i]]);
		ui.mainToolBar->addAction(actionStages[stage_names[i]]);

		connect(actionStages[stage_names[i]], SIGNAL(triggered()), this, SLOT(onStageChanged()));

		if (i == 0) {
			actionStages[stage_names[i]]->setChecked(true);
		}
	}
	
	// setup layouts
	glWidget = new GLWidget3D(this);

	thumbsList = new QListWidget(this);
	thumbsList->setFlow(QListView::TopToBottom);
	thumbsList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	thumbsList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	thumbsList->setFixedWidth(LeftWindowItemWidget::WIDGET_WIDTH + thumbsList->frameWidth() * 2 + 18);
	
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(thumbsList);
	layout->addWidget(glWidget);

	centralWidget()->setLayout(layout);
}

/**
 * This is called when the user clicks [File] -> [New].
 */
void MainWindow::onNew() {
	glWidget->clearSketch();
	glWidget->clearGeometry();
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