#ifndef GRAMMARDIALOG_H
#define GRAMMARDIALOG_H

#include <QDockWidget>
#include "ui_GrammarDialog.h"
#include <QTreeWidget>

class MainWindow;

class GrammarDialog : public QDockWidget {
	Q_OBJECT

private:
	Ui::GrammarDialog ui;
	MainWindow* mainWin;
	QTreeWidget* treeWidget;

public:
	GrammarDialog(MainWindow* parent = 0);

	void updateGrammar();
};

#endif // GRAMMARDIALOG_H
