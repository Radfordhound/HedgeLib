#include "mainwindow.h"
#include "GFX/device.h"
#include <QApplication>
#include <QStyleFactory> // TODO
#include <QMessageBox> // TODO
#include <memory>
#include <stdexcept>

using namespace HedgeEdit::GFX;

namespace HedgeEdit
{
    static std::unique_ptr<Device> device = nullptr;

    Device& GetDevice()
    {
        if (!device)
        {
            throw std::runtime_error(
                "D3D has not been initialized! Please call InitD3D() first.");
        }

        return *device.get();
    }

    void InitD3D()
    {
        if (device != nullptr)
            return;

        device = std::unique_ptr<Device>(new Device());
    }
}

int main(int argc, char *argv[])
{
    using namespace HedgeEdit;
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("WindowsVista"));

    QMessageBox Msgbox;
    Msgbox.setText(QApplication::style()->metaObject()->className());
    Msgbox.exec();

    InitD3D();

    UI::MainWindow w;
    w.show();

    return a.exec();
}
