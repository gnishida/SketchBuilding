#include "GrammarDialog.h"
#include "MainWindow.h"
#include <QLayout>
#include <QScrollArea>
#include <boost/lexical_cast.hpp>

GrammarDialog::GrammarDialog(MainWindow *parent) : QDockWidget(parent) {
	this->mainWin = parent;

	ui.setupUi(this);
	
	listWidget = new QListWidget(this);
	listWidget->setMinimumHeight(200);
	
	QVBoxLayout* layout = new QVBoxLayout();
	this->widget()->setLayout(layout);
	layout->addWidget(listWidget);

	listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void GrammarDialog::updateGrammar() {
	listWidget->clear();

	for (auto it = mainWin->glWidget->scene.currentObject().grammars.begin(); it != mainWin->glWidget->scene.currentObject().grammars.end(); ++it) {
		// Hack: skip the grammar for the border
		if (it->first == "Border") continue;

		listWidget->addItem((it->first + ": " + boost::lexical_cast<std::string>(it->second.rules.size()) + " rules").c_str());
	}
}