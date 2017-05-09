/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
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
    QAction *actionAddBuildingMass;
    QAction *actionFixGeometry;
    QAction *actionClearSketch;
    QAction *actionSaveGeometry;
    QAction *actionSketchyRendering;
    QAction *actionViewShadow;
    QAction *actionViewBasicRendering;
    QAction *actionViewSSAO;
    QAction *actionViewLineRendering;
    QAction *actionViewHatching;
    QAction *actionViewSketchyRendering;
    QAction *actionSetting;
    QAction *actionCopyBuildingMass;
    QAction *actionDeleteBuildingMass;
    QAction *actionGrammarWindow;
    QAction *actionViewGroundPlane;
    QAction *actionUndo;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuShape;
    QMenu *menuView;
    QMenu *menuOptions;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName(QStringLiteral("MainWindowClass"));
        MainWindowClass->resize(1057, 800);
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
        actionAddBuildingMass = new QAction(MainWindowClass);
        actionAddBuildingMass->setObjectName(QStringLiteral("actionAddBuildingMass"));
        actionFixGeometry = new QAction(MainWindowClass);
        actionFixGeometry->setObjectName(QStringLiteral("actionFixGeometry"));
        actionClearSketch = new QAction(MainWindowClass);
        actionClearSketch->setObjectName(QStringLiteral("actionClearSketch"));
        actionSaveGeometry = new QAction(MainWindowClass);
        actionSaveGeometry->setObjectName(QStringLiteral("actionSaveGeometry"));
        actionSketchyRendering = new QAction(MainWindowClass);
        actionSketchyRendering->setObjectName(QStringLiteral("actionSketchyRendering"));
        actionViewShadow = new QAction(MainWindowClass);
        actionViewShadow->setObjectName(QStringLiteral("actionViewShadow"));
        actionViewShadow->setCheckable(true);
        actionViewBasicRendering = new QAction(MainWindowClass);
        actionViewBasicRendering->setObjectName(QStringLiteral("actionViewBasicRendering"));
        actionViewBasicRendering->setCheckable(true);
        actionViewSSAO = new QAction(MainWindowClass);
        actionViewSSAO->setObjectName(QStringLiteral("actionViewSSAO"));
        actionViewSSAO->setCheckable(true);
        actionViewLineRendering = new QAction(MainWindowClass);
        actionViewLineRendering->setObjectName(QStringLiteral("actionViewLineRendering"));
        actionViewLineRendering->setCheckable(true);
        actionViewHatching = new QAction(MainWindowClass);
        actionViewHatching->setObjectName(QStringLiteral("actionViewHatching"));
        actionViewHatching->setCheckable(true);
        actionViewSketchyRendering = new QAction(MainWindowClass);
        actionViewSketchyRendering->setObjectName(QStringLiteral("actionViewSketchyRendering"));
        actionViewSketchyRendering->setCheckable(true);
        actionSetting = new QAction(MainWindowClass);
        actionSetting->setObjectName(QStringLiteral("actionSetting"));
        actionCopyBuildingMass = new QAction(MainWindowClass);
        actionCopyBuildingMass->setObjectName(QStringLiteral("actionCopyBuildingMass"));
        actionDeleteBuildingMass = new QAction(MainWindowClass);
        actionDeleteBuildingMass->setObjectName(QStringLiteral("actionDeleteBuildingMass"));
        actionGrammarWindow = new QAction(MainWindowClass);
        actionGrammarWindow->setObjectName(QStringLiteral("actionGrammarWindow"));
        actionViewGroundPlane = new QAction(MainWindowClass);
        actionViewGroundPlane->setObjectName(QStringLiteral("actionViewGroundPlane"));
        actionViewGroundPlane->setCheckable(true);
        actionUndo = new QAction(MainWindowClass);
        actionUndo->setObjectName(QStringLiteral("actionUndo"));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindowClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1057, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuShape = new QMenu(menuBar);
        menuShape->setObjectName(QStringLiteral("menuShape"));
        menuView = new QMenu(menuBar);
        menuView->setObjectName(QStringLiteral("menuView"));
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QStringLiteral("menuOptions"));
        MainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindowClass->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuShape->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addAction(actionClearSketch);
        menuFile->addAction(actionOpenCGA);
        menuFile->addAction(actionSaveGeometry);
        menuFile->addAction(actionSketchyRendering);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuShape->addAction(actionAddBuildingMass);
        menuShape->addAction(actionCopyBuildingMass);
        menuShape->addAction(actionDeleteBuildingMass);
        menuShape->addSeparator();
        menuShape->addAction(actionUndo);
        menuView->addAction(actionViewShadow);
        menuView->addSeparator();
        menuView->addAction(actionViewBasicRendering);
        menuView->addAction(actionViewSSAO);
        menuView->addAction(actionViewLineRendering);
        menuView->addAction(actionViewHatching);
        menuView->addAction(actionViewSketchyRendering);
        menuView->addSeparator();
        menuView->addAction(actionViewGroundPlane);
        menuOptions->addAction(actionSetting);
        menuOptions->addAction(actionGrammarWindow);

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
        actionAddBuildingMass->setText(QApplication::translate("MainWindowClass", "Add Building Mass", 0));
        actionFixGeometry->setText(QApplication::translate("MainWindowClass", "Fix", 0));
        actionFixGeometry->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+F", 0));
        actionClearSketch->setText(QApplication::translate("MainWindowClass", "Clear Sketch", 0));
        actionSaveGeometry->setText(QApplication::translate("MainWindowClass", "Save Geometry", 0));
        actionSaveGeometry->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+S", 0));
        actionSketchyRendering->setText(QApplication::translate("MainWindowClass", "Sketchy Rendering", 0));
        actionViewShadow->setText(QApplication::translate("MainWindowClass", "Shadow", 0));
        actionViewBasicRendering->setText(QApplication::translate("MainWindowClass", "Basic Rendering", 0));
        actionViewSSAO->setText(QApplication::translate("MainWindowClass", "SSAO", 0));
        actionViewLineRendering->setText(QApplication::translate("MainWindowClass", "Line Rendering", 0));
        actionViewHatching->setText(QApplication::translate("MainWindowClass", "Hatching", 0));
        actionViewSketchyRendering->setText(QApplication::translate("MainWindowClass", "Sketchy Rendering", 0));
        actionSetting->setText(QApplication::translate("MainWindowClass", "Setting", 0));
        actionCopyBuildingMass->setText(QApplication::translate("MainWindowClass", "Copy Building Mass", 0));
        actionCopyBuildingMass->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+C", 0));
        actionDeleteBuildingMass->setText(QApplication::translate("MainWindowClass", "Delete Building Mass", 0));
        actionDeleteBuildingMass->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+X", 0));
        actionGrammarWindow->setText(QApplication::translate("MainWindowClass", "Grammar Window", 0));
        actionViewGroundPlane->setText(QApplication::translate("MainWindowClass", "Ground Plane", 0));
        actionUndo->setText(QApplication::translate("MainWindowClass", "Undo", 0));
        actionUndo->setShortcut(QApplication::translate("MainWindowClass", "Ctrl+Z", 0));
        menuFile->setTitle(QApplication::translate("MainWindowClass", "File", 0));
        menuShape->setTitle(QApplication::translate("MainWindowClass", "Edit", 0));
        menuView->setTitle(QApplication::translate("MainWindowClass", "View", 0));
        menuOptions->setTitle(QApplication::translate("MainWindowClass", "Options", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
