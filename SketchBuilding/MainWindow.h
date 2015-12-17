#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "GLWidget3D.h"
#include <QListWidget>
#include <map>
#include <QAction>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;
	QListWidget *thumbsList;
	std::map<std::string, QAction*> actionStages;
	std::map<std::string, QAction*> actionModes;

public:
	MainWindow(QWidget *parent = 0);

protected:
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

public slots:
	void onNew();
	void onClearSketch();
	void onOpenCGA();
	void onSaveGeometry();
	void onAddBuildingMass();
	void onCopyBuildingMass();
	void onDeleteBuildingMass();
	void onViewShadow();
	void onViewRendering();
	void onSetting();
	void onStageChanged();
	void onModeChanged();
	void addListItem(const QString& text, const QImage& image, int option_index);
	void camera_update();
};

#endif // MAINWINDOW_H
