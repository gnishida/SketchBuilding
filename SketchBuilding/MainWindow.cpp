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
	connect(ui.actionOpenCGA, SIGNAL(triggered()), this, SLOT(onOpenCGA()));
	connect(ui.actionAddBuildingMass, SIGNAL(triggered()), this, SLOT(onAddBuildingMass()));

	// create tool bar
	QActionGroup* stageGroup = new QActionGroup(this);
	std::string stage_names[5] = { "building", "roof", "facade", "window", "ledge" };
	for (int i = 0; i < 5; ++i) {
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
	else if (actionStages["window"]->isChecked()) {
		glWidget->changeStage("window");
	}
	else if (actionStages["ledge"]->isChecked()) {
		glWidget->changeStage("ledge");
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