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

private:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;
	QListWidget *thumbsList;
	std::map<std::string, QAction*> actionStages;

public:
	MainWindow(QWidget *parent = 0);

public slots:
	void onNew();
	void onFixGeometry();
	void onSelectShape();
	void onNewLayer();
	void onStageChanged();
};

#endif // MAINWINDOW_H
