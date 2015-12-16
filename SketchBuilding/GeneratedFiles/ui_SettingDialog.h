/********************************************************************************
** Form generated from reading UI file 'SettingDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGDIALOG_H
#define UI_SETTINGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_SettingDialog
{
public:
    QLabel *label;
    QLineEdit *lineEditDefaultGrammar;
    QPushButton *pushButtonDefaultGrammar;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *SettingDialog)
    {
        if (SettingDialog->objectName().isEmpty())
            SettingDialog->setObjectName(QStringLiteral("SettingDialog"));
        SettingDialog->resize(372, 101);
        label = new QLabel(SettingDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(30, 20, 81, 16));
        lineEditDefaultGrammar = new QLineEdit(SettingDialog);
        lineEditDefaultGrammar->setObjectName(QStringLiteral("lineEditDefaultGrammar"));
        lineEditDefaultGrammar->setGeometry(QRect(120, 20, 191, 20));
        pushButtonDefaultGrammar = new QPushButton(SettingDialog);
        pushButtonDefaultGrammar->setObjectName(QStringLiteral("pushButtonDefaultGrammar"));
        pushButtonDefaultGrammar->setGeometry(QRect(320, 19, 31, 23));
        okButton = new QPushButton(SettingDialog);
        okButton->setObjectName(QStringLiteral("okButton"));
        okButton->setGeometry(QRect(200, 60, 75, 23));
        cancelButton = new QPushButton(SettingDialog);
        cancelButton->setObjectName(QStringLiteral("cancelButton"));
        cancelButton->setGeometry(QRect(280, 60, 75, 23));

        retranslateUi(SettingDialog);

        QMetaObject::connectSlotsByName(SettingDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingDialog)
    {
        SettingDialog->setWindowTitle(QApplication::translate("SettingDialog", "Setting Dialog", 0));
        label->setText(QApplication::translate("SettingDialog", "Default Grammar:", 0));
        pushButtonDefaultGrammar->setText(QApplication::translate("SettingDialog", "...", 0));
        okButton->setText(QApplication::translate("SettingDialog", "OK", 0));
        cancelButton->setText(QApplication::translate("SettingDialog", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class SettingDialog: public Ui_SettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGDIALOG_H
