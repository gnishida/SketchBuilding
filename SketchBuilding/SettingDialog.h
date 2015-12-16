#pragma once

#include "glew.h"

#include <QDialog>
#include "ui_SettingDialog.h"

class MainWindow;

class SettingDialog : public QDialog {
Q_OBJECT

public:
	Ui::SettingDialog ui;
	MainWindow* mainWin;

public:
	SettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
	void onDefaultGrammar();
};

