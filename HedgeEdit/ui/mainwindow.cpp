#include "mainwindow.h"
#include <QStyleFactory> // TODO

namespace HedgeEdit::UI
{
    MainWindow* MainInstance = nullptr;

    MainWindow::MainWindow(QWidget* parent) :
        QMainWindow(parent)
    {
        setupUi(this);
        
        connect(actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
        connect(propertiesPanel, SIGNAL(destroyed()), this, SLOT(PropertiesClosed()));

        MainInstance = this;
    }

    MainWindow::~MainWindow()
    {
        MainInstance = nullptr;
    }

    void MainWindow::PropertiesClosed()
    {
        actionPropertiesPanel->setChecked(false);
    }

    void MainWindow::DebugEnableDarkTheme()
    {
        // Adapted from https://gist.github.com/QuantumCD/6245215
        // TODO: Maybe use this instead: https://github.com/ColinDuquesnoy/QDarkStyleSheet
        // TODO: Move this elsewhere, possibly create custom XML files for theming?
        qApp->setStyle(QStyleFactory::create("Fusion"));

        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        qApp->setPalette(darkPalette);

        qApp->setStyleSheet(
            "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    }
}
