#include "mainwindow.h"
#include "GFX\gfx.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	HedgeEdit::GFX::InitGFX();

	HedgeEdit::UI::MainWindow w;
    w.show();

    return a.exec();
}