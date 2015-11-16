#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "GLWidget3D.h"
#include <QListWidget>

class MainWindow : public QMainWindow {
	Q_OBJECT

private:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;
	QListWidget *thumbsList;

public:
	MainWindow(QWidget *parent = 0);

public slots:
	void onNew();
	void onPredict();
	void onSelectShape();
	void onNewLayer();
};

#endif // MAINWINDOW_H
