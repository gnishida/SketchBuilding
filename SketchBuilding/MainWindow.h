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

public:
	MainWindow(QWidget *parent = 0);

public slots:
	void onNew();
	void onFixGeometry();
	void onSelectShape();
	void onNewLayer();
	void onStageChanged();
	void addListItem(const QString& text, const QImage& image, int option_index);
};

#endif // MAINWINDOW_H
