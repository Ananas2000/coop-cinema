/********************************************************************************
** Form generated from reading UI file 'QtApp.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTAPP_H
#define UI_QTAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_QtAppClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *QtAppClass)
    {
        if (QtAppClass->objectName().isEmpty())
            QtAppClass->setObjectName("QtAppClass");
        QtAppClass->resize(600, 400);
        menuBar = new QMenuBar(QtAppClass);
        menuBar->setObjectName("menuBar");
        QtAppClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(QtAppClass);
        mainToolBar->setObjectName("mainToolBar");
        QtAppClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(QtAppClass);
        centralWidget->setObjectName("centralWidget");
        QtAppClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(QtAppClass);
        statusBar->setObjectName("statusBar");
        QtAppClass->setStatusBar(statusBar);

        retranslateUi(QtAppClass);

        QMetaObject::connectSlotsByName(QtAppClass);
    } // setupUi

    void retranslateUi(QMainWindow *QtAppClass)
    {
        QtAppClass->setWindowTitle(QCoreApplication::translate("QtAppClass", "QtApp", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QtAppClass: public Ui_QtAppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QTAPP_H
