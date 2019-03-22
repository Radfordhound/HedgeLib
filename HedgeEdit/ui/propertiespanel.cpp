#include "propertiespanel.h"
#include "mainwindow.h"
#include <QSize>

namespace HedgeEdit::UI
{
    PropertiesPanel::PropertiesPanel(QWidget* parent)
    {
        setupUi(this);

        setAttribute(Qt::WA_DeleteOnClose); // TODO: UNCHECK THE BOX SOMEHOW
        splitterProperties->setStretchFactor(0, 0);
        splitterProperties->setStretchFactor(1, 1);
    }

    QSize PropertiesPanel::sizeHint() const
    {
        return QSize(minimumWidth(),
            minimumHeight());
    }

    void PropertiesPanel::closeEvent(QCloseEvent* e)
    {
        if (!MainInstance)
            return;

        MainInstance->actionPropertiesPanel->setChecked(false);
    }
}
