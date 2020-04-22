#pragma once
#include <QTreeView>
#include <QStandardItemModel>
#include <memory>

namespace HedgeEdit
{
    struct TerrainGroup;
}

namespace HedgeEdit::UI
{
    class SceneViewWidget : public QTreeView
    {
        std::unique_ptr<QStandardItemModel> sceneViewModel;

    public:
        explicit SceneViewWidget(QWidget* parent = nullptr);
        void AddTerrainInstances(const TerrainGroup& trrGroup, QStandardItem* parent);
        void UpdateUI();

    protected:
        void selectionChanged(const QItemSelection& selected,
            const QItemSelection& deselected) override;
    };
}
