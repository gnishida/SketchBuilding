#include "GrammarDialog.h"
#include "MainWindow.h"
#include <QLayout>
#include <QScrollArea>
#include <boost/lexical_cast.hpp>

GrammarDialog::GrammarDialog(MainWindow *parent) : QDockWidget(parent) {
	this->mainWin = parent;

	ui.setupUi(this);
		
	treeWidget = new QTreeWidget(this);
	treeWidget->setMinimumHeight(200);
	treeWidget->setColumnCount(1);
	treeWidget->header()->close();

	QVBoxLayout* layout = new QVBoxLayout();
	this->widget()->setLayout(layout);
	layout->addWidget(treeWidget);

	treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	treeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void GrammarDialog::updateGrammar() {
	treeWidget->clear();

	for (auto it = mainWin->glWidget->scene.currentObject().grammars.begin(); it != mainWin->glWidget->scene.currentObject().grammars.end(); ++it) {
		// Hack: skip the grammar for the border
		if (it->first == "Border") continue;

		QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget);
		rootItem->setText(0, it->first.c_str());

		for (auto it2 = it->second.attrs.begin(); it2 != it->second.attrs.end(); ++it2) {
			QTreeWidgetItem* childItem = new QTreeWidgetItem();
			childItem->setText(0, (it2->first + ": " + it2->second.value).c_str());
			rootItem->addChild(childItem);
		}

		for (auto it2 = it->second.rules.begin(); it2 != it->second.rules.end(); ++it2) {
			QTreeWidgetItem* childItem = new QTreeWidgetItem();
			childItem->setText(0, it2->first.c_str());
			rootItem->addChild(childItem);

			for (int k = 0; k < it2->second.operators.size(); ++k) {
				QTreeWidgetItem* groundChildItem = new QTreeWidgetItem();
				groundChildItem->setText(0, it2->second.operators[k]->to_string().c_str());
				childItem->addChild(groundChildItem);
			}
		}
	}

	treeWidget->expandAll();
}