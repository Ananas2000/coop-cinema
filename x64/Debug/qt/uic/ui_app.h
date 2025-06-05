/********************************************************************************
** Form generated from reading UI file 'app.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_APP_H
#define UI_APP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_appClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *appClass)
    {
        if (appClass->objectName().isEmpty())
            appClass->setObjectName("appClass");
        appClass->resize(600, 400);
        menuBar = new QMenuBar(appClass);
        menuBar->setObjectName("menuBar");
        appClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(appClass);
        mainToolBar->setObjectName("mainToolBar");
        appClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(appClass);
        centralWidget->setObjectName("centralWidget");
        appClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(appClass);
        statusBar->setObjectName("statusBar");
        appClass->setStatusBar(statusBar);

        retranslateUi(appClass);

        QMetaObject::connectSlotsByName(appClass);
    } // setupUi

    void retranslateUi(QMainWindow *appClass)
    {
        appClass->setWindowTitle(QCoreApplication::translate("appClass", "app", nullptr));
    } // retranslateUi

};

namespace Ui {
    class appClass: public Ui_appClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APP_H
