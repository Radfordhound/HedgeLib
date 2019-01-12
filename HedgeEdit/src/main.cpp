#include "mainwindow.h"
#include "GFX\gfx.h"
#include <QApplication>
#include <QStyleFactory> // TODO
#include <QMessageBox> // TODO

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QApplication::setStyle(QStyleFactory::create("WindowsVista"));

	QMessageBox Msgbox;
	Msgbox.setText(QApplication::style()->metaObject()->className());
	Msgbox.exec();

	HedgeEdit::GFX::InitGFX();

	HedgeEdit::UI::MainWindow w;
    w.show();

    return a.exec();
}