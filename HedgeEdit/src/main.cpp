#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    using namespace HedgeEdit;
    QApplication a(argc, argv);

    UI::MainWindow w;
    w.show();

    return a.exec();
}
