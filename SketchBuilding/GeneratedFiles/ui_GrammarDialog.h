/********************************************************************************
** Form generated from reading UI file 'GrammarDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GRAMMARDIALOG_H
#define UI_GRAMMARDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GrammarDialog
{
public:
    QWidget *widget;

    void setupUi(QDockWidget *GrammarDialog)
    {
        if (GrammarDialog->objectName().isEmpty())
            GrammarDialog->setObjectName(QStringLiteral("GrammarDialog"));
        GrammarDialog->resize(855, 242);
        widget = new QWidget();
        widget->setObjectName(QStringLiteral("widget"));
        GrammarDialog->setWidget(widget);

        retranslateUi(GrammarDialog);

        QMetaObject::connectSlotsByName(GrammarDialog);
    } // setupUi

    void retranslateUi(QDockWidget *GrammarDialog)
    {
        GrammarDialog->setWindowTitle(QApplication::translate("GrammarDialog", "Grammar", 0));
    } // retranslateUi

};

namespace Ui {
    class GrammarDialog: public Ui_GrammarDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GRAMMARDIALOG_H
