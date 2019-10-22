#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory> // TODO

int main(int argc, char *argv[])
{
    using namespace HedgeEdit;
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("WindowsVista"));

    UI::MainWindow w;
    w.show();

    return a.exec();
}
