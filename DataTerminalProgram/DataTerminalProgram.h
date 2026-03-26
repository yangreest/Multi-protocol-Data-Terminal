#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DataTerminalProgram.h"

class DataTerminalProgram : public QMainWindow
{
    Q_OBJECT

public:
    DataTerminalProgram(QWidget *parent = nullptr);
    ~DataTerminalProgram();

private:
    Ui::DataTerminalProgramClass ui;
};

