#include "SettingDialog.h"
#include "MainWindow.h"
#include <QFileDialog>

SettingDialog::SettingDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	/*ui.checkBoxHDImage->setChecked(false);
	ui.lineEditStart->setText("0.0");
	ui.lineEditEnd->setText("1.0");
	ui.lineEditStep->setText("0.01");
	*/

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));
	connect(ui.pushButtonMaterial, SIGNAL(clicked()), this, SLOT(onMaterial()));

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

void SettingDialog::onMaterial() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open CGA file..."), "", tr("CGA Files (*.xml)"));
	if (!filename.isEmpty()) {
		ui.lineEditMaterial->setText(filename);
	}
}
