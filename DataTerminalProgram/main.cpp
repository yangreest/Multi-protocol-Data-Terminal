#include "DataTerminalProgram.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    DataTerminalProgram window;
    window.show();
    return app.exec();
}
