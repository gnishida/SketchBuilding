#include "GrammarDialog.h"
#include "MainWindow.h"
#include <QLayout>
#include <QScrollArea>
#include <boost/lexical_cast.hpp>
#include <iostream>

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

	connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(updateCheckState(QTreeWidgetItem*, int)));
}

void GrammarDialog::updateGrammar() {
	treeWidget->clear();

	for (auto it = mainWin->glWidget->scene.currentObject().grammars.begin(); it != mainWin->glWidget->scene.currentObject().grammars.end(); ++it) {
		// Hack: skip the grammar for the border
		if (it->first == "Border") continue;

		QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget);
		rootItem->setText(0, it->first.c_str());
		rootItem->setExpanded(true);

		for (auto it2 = it->second.attrs.begin(); it2 != it->second.attrs.end(); ++it2) {
			// Hack: skip the grammar for the border
			if (it2->first == "z_floor_border_size" || it2->first == "z_window_border_size") continue;

			QTreeWidgetItem* childItem = new QTreeWidgetItem();
			childItem->setText(0, it2->first.c_str());
			QFont font = childItem->font(0);
			font.setBold(true);
			childItem->setFont(0, font);
			rootItem->addChild(childItem);
			childItem->setExpanded(true);

			QTreeWidgetItem* groundChildItem = new QTreeWidgetItem();
			groundChildItem->setText(0, it2->second.value.c_str());
			groundChildItem->setData(1, Qt::UserRole, it->first.c_str());
			groundChildItem->setData(2, Qt::UserRole, it2->first.c_str());
			groundChildItem->setFlags(groundChildItem->flags() | Qt::ItemIsEditable);
			childItem->addChild(groundChildItem);
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

	//treeWidget->expandAll();
}

void GrammarDialog::updateCheckState(QTreeWidgetItem* item, int a) {
	std::string type = item->data(1, Qt::UserRole).toString().toUtf8().constData();
	std::string paramter = item->data(2, Qt::UserRole).toString().toUtf8().constData();
	std::string value = item->text(0).toUtf8().constData();
	
	if (type.empty() || paramter.empty() || value.empty()) return;

	mainWin->glWidget->scene.currentObject().grammars[type].attrs[paramter].value = value;
	mainWin->glWidget->generateGeometry();
	mainWin->glWidget->update();
}