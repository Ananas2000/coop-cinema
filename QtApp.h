#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QtApp.h"

class QtApp : public QMainWindow
{
    Q_OBJECT

public:
    QtApp(QWidget *parent = nullptr);
    ~QtApp();

private:
    Ui::QtAppClass ui;
};

