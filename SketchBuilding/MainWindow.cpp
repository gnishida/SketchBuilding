#include "MainWindow.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include "LeftWindowItemWidget.h"
#include <QIcon>
#include "SettingDialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	ui.setupUi(this);

	QActionGroup* renderingModeGroup = new QActionGroup(this);
	renderingModeGroup->addAction(ui.actionViewBasicRendering);
	renderingModeGroup->addAction(ui.actionViewSSAO);
	renderingModeGroup->addAction(ui.actionViewLineRendering);
	renderingModeGroup->addAction(ui.actionViewHatching);
	renderingModeGroup->addAction(ui.actionViewSketchyRendering);

	ui.actionViewShadow->setChecked(true);
	ui.actionViewBasicRendering->setChecked(true);
	ui.actionViewGroundPlane->setChecked(true);

	// add menu handlers
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionClearSketch, SIGNAL(triggered()), this, SLOT(onClearSketch()));
	connect(ui.actionOpenCGA, SIGNAL(triggered()), this, SLOT(onOpenCGA()));
	connect(ui.actionSaveGeometry, SIGNAL(triggered()), this, SLOT(onSaveGeometry()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionAddBuildingMass, SIGNAL(triggered()), this, SLOT(onAddBuildingMass()));
	connect(ui.actionCopyBuildingMass, SIGNAL(triggered()), this, SLOT(onCopyBuildingMass()));
	connect(ui.actionDeleteBuildingMass, SIGNAL(triggered()), this, SLOT(onDeleteBuildingMass()));
	connect(ui.actionUndo, SIGNAL(triggered()), this, SLOT(onUndo()));
	connect(ui.actionViewShadow, SIGNAL(triggered()), this, SLOT(onViewShadow()));
	connect(ui.actionViewBasicRendering, SIGNAL(triggered()), this, SLOT(onViewRendering()));
	connect(ui.actionViewSSAO, SIGNAL(triggered()), this, SLOT(onViewRendering()));
	connect(ui.actionViewLineRendering, SIGNAL(triggered()), this, SLOT(onViewRendering()));
	connect(ui.actionViewHatching, SIGNAL(triggered()), this, SLOT(onViewRendering()));
	connect(ui.actionViewSketchyRendering, SIGNAL(triggered()), this, SLOT(onViewRendering()));
	connect(ui.actionViewGroundPlane, SIGNAL(triggered()), this, SLOT(onViewGroundPlane()));
	connect(ui.actionSetting, SIGNAL(triggered()), this, SLOT(onSetting()));
	connect(ui.actionGrammarWindow, SIGNAL(triggered()), this, SLOT(onGrammarWindow()));

	// create tool bar for stages
	QActionGroup* stageGroup = new QActionGroup(this);
	std::string stage_names[7] = { "building", "roof", "facade", "floor", "window", "ledge", "final" };
	for (int i = 0; i < 7; ++i) {
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
	std::string mode_names[6] = { "sketch", "select", "select_building", "copy", "eraser", "camera" };
	for (int i = 0; i < 6; ++i) {
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

	grammarDialog = new GrammarDialog(this);
	grammarDialog->show();
	addDockWidget(Qt::RightDockWidgetArea, grammarDialog);
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
	glWidget->scene.buildingSelector->unselectBuilding();
	glWidget->clearGeometry();
	glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_HATCHING;
	glWidget->update();
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

void MainWindow::onSaveGeometry() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save OBJ file..."), "", tr("OBJ Files (*.obj)"));
	if (filename.isEmpty()) return;

	glWidget->scene.saveGeometry(filename.toUtf8().data());
}

void MainWindow::onAddBuildingMass() {
	glWidget->addBuildingMass();
}

void MainWindow::onCopyBuildingMass() {
	if (glWidget->scene.buildingSelector->isBuildingSelected()) {
		// the selected building will be copied, so the history should be updated before copy
		glWidget->scene.updateHistory();

		glWidget->scene.buildingSelector->copy();
		glWidget->generateGeometry();
		glWidget->update();
	}
}

void MainWindow::onDeleteBuildingMass() {
	if (glWidget->scene.buildingSelector->isBuildingSelected()) {
		// the selected building will be removed, so the history should be updated before copy
		glWidget->scene.updateHistory();

		glWidget->scene.buildingSelector->remove();
		glWidget->generateGeometry();
		glWidget->update();
	}
}

void MainWindow::onUndo() {
	glWidget->scene.undo();
	glWidget->generateGeometry();
	glWidget->update();
}

void MainWindow::onViewShadow() {
	glWidget->renderManager.useShadow = ui.actionViewShadow->isChecked();
	glWidget->update();
}

void MainWindow::onViewRendering() {
	if (ui.actionViewBasicRendering->isChecked()) {
		glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_BASIC;
	}
	else if (ui.actionViewSSAO->isChecked()) {
		glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_SSAO;
	}
	else if (ui.actionViewLineRendering->isChecked()) {
		glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_LINE;
	}
	else if (ui.actionViewHatching->isChecked()) {
		glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_HATCHING;
	}
	else {
		glWidget->renderManager.renderingMode = RenderManager::RENDERING_MODE_SKETCHY;
	}
	glWidget->update();
}

void MainWindow::onViewGroundPlane() {
	if (ui.actionViewGroundPlane->isChecked()) {
		glWidget->showGroundPlane = true;
	}
	else {
		glWidget->showGroundPlane = false;
	}
	glWidget->updateGeometry();
	glWidget->update();
}

void MainWindow::onSetting() {
	SettingDialog dlg(this);
	dlg.ui.lineEditDefaultGrammar->setText(glWidget->scene.default_grammar_file.c_str());

	if (dlg.exec() == QDialog::Accepted) {
		glWidget->scene.loadDefaultGrammar(dlg.ui.lineEditDefaultGrammar->text().toUtf8().constData());
	}
}

void MainWindow::onGrammarWindow() {
	grammarDialog->show();
}

void MainWindow::onStageChanged() {
	if (actionStages["final"]->isChecked()) {
		glWidget->changeStage("final");

		//actionModes["sketch"]->setChecked(false);
		actionModes["camera"]->setChecked(true);
		glWidget->mode = GLWidget3D::MODE_CAMERA;
	}
	else {
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
}

void MainWindow::onModeChanged() {
	if (actionModes["sketch"]->isChecked()) {
		glWidget->changeMode(GLWidget3D::MODE_SKETCH);
	}
	else if (actionModes["select"]->isChecked()) {
		glWidget->changeMode(GLWidget3D::MODE_SELECT);
	}
	else if (actionModes["select_building"]->isChecked()) {
		glWidget->changeMode(GLWidget3D::MODE_SELECT_BUILDING);
	}
	else if (actionModes["copy"]->isChecked()) {
		glWidget->changeMode(GLWidget3D::MODE_COPY);

		actionModes["select_building"]->setChecked(true);
	}
	else if (actionModes["eraser"]->isChecked()) {
		glWidget->changeMode(GLWidget3D::MODE_ERASER);

		actionModes["sketch"]->setChecked(true);
	}
	else if (actionModes["camera"]->isChecked()) {
		glWidget->changeMode(GLWidget3D::MODE_CAMERA);
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