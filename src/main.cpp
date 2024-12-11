#include <iostream>
#include <QApplication>
#include "appwindow.hpp"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    AppWindow appWindow;

    appWindow.show();

    return app.exec();
}
