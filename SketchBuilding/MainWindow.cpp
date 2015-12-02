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
	connect(ui.actionClearSketch, SIGNAL(triggered()), this, SLOT(onClearSketch()));
	connect(ui.actionOpenCGA, SIGNAL(triggered()), this, SLOT(onOpenCGA()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionAddBuildingMass, SIGNAL(triggered()), this, SLOT(onAddBuildingMass()));

	// create tool bar for stages
	QActionGroup* stageGroup = new QActionGroup(this);
	std::string stage_names[6] = { "building", "roof", "facade", "floor", "window", "ledge" };
	for (int i = 0; i < 6; ++i) {
		actionStages[stage_names[i]] = new QAction(QIcon(std::string("resources/" + stage_names[i] + ".png").c_str()), std::string("&" + stage_names[i]).c_str(), this);
		actionStages[stage_names[i]]->setCheckable(true);
		stageGroup->addAction(actionStages[stage_names[i]]);
		ui.mainToolBar->addAction(actionStages[stage_names[i]]);

		connect(actionStages[stage_names[i]], SIGNAL(triggered()), this, SLOT(onStageChanged()));

		if (i == 0) {
			actionStages[stage_names[i]]->setChecked(true);
		}
	}

	ui.mainToolBar->addSeparator();

	// create tool bar for modes
	QActionGroup* modeGroup = new QActionGroup(this);
	std::string mode_names[2] = { "sketch", "select" };
	for (int i = 0; i < 2; ++i) {
		actionModes[mode_names[i]] = new QAction(QIcon(std::string("resources/" + mode_names[i] + ".png").c_str()), std::string("&" + mode_names[i]).c_str(), this);
		actionModes[mode_names[i]]->setCheckable(true);
		modeGroup->addAction(actionModes[mode_names[i]]);
		ui.mainToolBar->addAction(actionModes[mode_names[i]]);

		connect(actionModes[mode_names[i]], SIGNAL(triggered()), this, SLOT(onModeChanged()));

		if (i == 0) {
			actionModes[mode_names[i]]->setChecked(true);
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

void MainWindow::keyPressEvent(QKeyEvent* e) {
	glWidget->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
	glWidget->keyReleaseEvent(e);
}

/**
 * This is called when the user clicks [File] -> [New].
 */
void MainWindow::onNew() {
	glWidget->clearSketch();
	glWidget->clearGeometry();
}

void MainWindow::onClearSketch() {
	glWidget->clearSketch();

	if (glWidget->stage == "building") {
		glWidget->scene.clearCurrentObject();
	}
}

void MainWindow::onOpenCGA() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open CGA file..."), "", tr("CGA Files (*.xml)"));
	if (filename.isEmpty()) return;

	glWidget->loadCGA(filename.toUtf8().data());
}

void MainWindow::onAddBuildingMass() {
	glWidget->addBuildingMass();
}

void MainWindow::onStageChanged() {
	if (actionStages["building"]->isChecked()) {
		glWidget->changeStage("building");
	}
	else if (actionStages["roof"]->isChecked()) {
		glWidget->changeStage("roof");
	}
	else if (actionStages["facade"]->isChecked()) {
		glWidget->changeStage("facade");
	}
	else if (actionStages["floor"]->isChecked()) {
		glWidget->changeStage("floor");
	}
	else if (actionStages["window"]->isChecked()) {
		glWidget->changeStage("window");
	}
	else if (actionStages["ledge"]->isChecked()) {
		glWidget->changeStage("ledge");
	}

	// When the stage is changed, the user should to select a face.
	actionModes["select"]->setChecked(true);
	glWidget->mode = GLWidget3D::MODE_SELECT;

}

void MainWindow::onModeChanged() {
	if (actionModes["sketch"]->isChecked()) {
		glWidget->mode = GLWidget3D::MODE_SKETCH;
	}
	else if (actionModes["select"]->isChecked()) {
		glWidget->mode = GLWidget3D::MODE_SELECT;
	}
}

/**
* Add an option to the list widget.
*
* @param text			probability that is shown on this item widget
* @param image			image that is shown on this item widget
* @param option_index	the index of this option (This index is not the index of the ordered options, but the index of the options in the original order.)
*/
void MainWindow::addListItem(const QString& text, const QImage& image, int option_index) {
	LeftWindowItemWidget* widget = new LeftWindowItemWidget(this, text, image, option_index);
	QListWidgetItem *item = new QListWidgetItem();
	item->setSizeHint(widget->size());
	thumbsList->addItem(item);
	thumbsList->setItemWidget(item, widget);
}

void MainWindow::camera_update() {
	glWidget->camera_update();
}