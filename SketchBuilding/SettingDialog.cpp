#include "SettingDialog.h"
#include "MainWindow.h"
#include <QFileDialog>

SettingDialog::SettingDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(ui.pushButtonDefaultGrammar, SIGNAL(clicked()), this, SLOT(onDefaultGrammar()));

	//hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void SettingDialog::onOK() {
	accept();
}

void SettingDialog::onCancel() {
	reject();
}

void SettingDialog::onDefaultGrammar() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open xml file..."), "", tr("xml files (*.xml)"));
	if (!filename.isEmpty()) {
		ui.lineEditDefaultGrammar->setText(filename);
	}
}
