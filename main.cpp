#include "mainwindow.h"
#include "iclientinterface.h"
#include <QApplication>

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //std::cout << w.configWindow->gameWindow->per << std::endl;
    return a.exec();
}
