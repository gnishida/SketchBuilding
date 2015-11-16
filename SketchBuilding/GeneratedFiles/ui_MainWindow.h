/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QAction *actionOpen;
    QAction *actionExit;
    QAction *actionPredict;
    QAction *actionNew;
    QAction *actionOpenCGA;
    QAction *actionCuboid;
    QAction *actionLShape;
    QAction *actionNewLayer;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuTool;
    QMenu *menuShape;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(512, 565);
        actionOpen = new QAction(MainWindowClass);
        actionOpen->setObjectName(QStringLiteral("actionOpen"));
        actionExit = new QAction(MainWindowClass);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        actionPredict = new QAction(MainWindowClass);
        actionPredict->setObjectName(QStringLiteral("actionPredict"));
        actionNew = new QAction(MainWindowClass);
        actionNew->setObjectName(QStringLiteral("actionNew"));
        actionOpenCGA = new QAction(MainWindowClass);
        actionOpenCGA->setObjectName(QStringLiteral("actionOpenCGA"));
        actionCuboid = new QAction(MainWindowClass);
        actionCuboid->setObjectName(QStringLiteral("actionCuboid"));
        actionCuboid->setCheckable(true);
        actionLShape = new QAction(MainWindowClass);
        actionLShape->setObjectName(QStringLiteral("actionLShape"));
        actionLShape->setCheckable(true);
        actionNewLayer = new QAction(MainWindowClass);
        actionNewLayer->setObjectName(QStringLiteral("actionNewLayer"));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 512, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuTool = new QMenu(menuBar);
        menuTool->setObjectName(QStringLiteral("menuTool"));
        menuShape = new QMenu(menuBar);
        menuShape->setObjectName(QStringLiteral("menuShape"));
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuTool->menuAction());
        menuBar->addAction(menuShape->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionOpenCGA);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuTool->addAction(actionPredict);
        menuShape->addAction(actionCuboid);
        menuShape->addAction(actionLShape);
        menuShape->addSeparator();
        menuShape->addAction(actionNewLayer);

        retranslateUi(MainWindowClass);

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QApplication::translate("MainWindowClass", "Sketching Application", 0));
        actionOpen->setText(QApplication::translate("MainWindowClass", "Open", 0));
        actionOpen->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+O", 0));
        actionExit->setText(QApplication::translate("MainWindowClass", "Exit", 0));
        actionPredict->setText(QApplication::translate("MainWindowClass", "Predict", 0));
        actionPredict->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+P", 0));
        actionNew->setText(QApplication::translate("MainWindowClass", "New", 0));
        actionNew->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+N", 0));
        actionOpenCGA->setText(QApplication::translate("MainWindowClass", "Open CGA", 0));
        actionCuboid->setText(QApplication::translate("MainWindowClass", "Cuboid", 0));
        actionLShape->setText(QApplication::translate("MainWindowClass", "L-Shape", 0));
        actionNewLayer->setText(QApplication::translate("MainWindowClass", "New Layer", 0));
        menuFile->setTitle(QApplication::translate("MainWindowClass", "File", 0));
        menuTool->setTitle(QApplication::translate("MainWindowClass", "Tool", 0));
        menuShape->setTitle(QApplication::translate("MainWindowClass", "Shape", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
