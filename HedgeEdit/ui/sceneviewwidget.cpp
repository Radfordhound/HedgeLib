#include "sceneviewwidget.h"
#include "mainwindow.h"
#include "../src/gfx/instance.h"
#include <cstring>

namespace HedgeEdit::UI
{
    SceneViewWidget::SceneViewWidget(QWidget* parent)
    {
        // Create scene view model
        sceneViewModel = std::unique_ptr<QStandardItemModel>(new QStandardItemModel());
        connect(sceneViewModel.get(), SIGNAL(itemChanged(QStandardItem*)), this,
            SLOT(SceneViewItemChanged(QStandardItem*)));

        // Setup hierarchy tree
        connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(SceneViewCustomContextMenuRequested(QPoint)));

        setIndentation(15);
        setHeaderHidden(true);
        setModel(sceneViewModel.get());
        setContextMenuPolicy(Qt::CustomContextMenu);
        //hierarchyTree->setSelectionMode(QTreeView::MultiSelection);
    }

    void SceneViewWidget::AddTerrainInstances(
        const TerrainGroup& trrGroup, QStandardItem* parent)
    {
        for (const auto& trrInst : trrGroup.Terrain)
        {
            QStandardItem* trrInstItem = new QStandardItem(
                QString::fromUtf8(trrInst.ModelName.c_str()));

            //item->setData(QVariant::fromValue(&proj));
            trrInstItem->setEditable(false);
            parent->appendRow(trrInstItem);
        }
    }

    void SceneViewWidget::UpdateUI()
    {
        // Reset scene view
        sceneViewModel->clear();

        // Add objects to scene view
        QStandardItem* objsItem = new QStandardItem("Object Layers");

        //item->setData(QVariant::fromValue(&proj));
        objsItem->setEditable(false);

        // TODO

        //UpdateTerrain(proj.RootNode, item);
        /*child->setEditable(false);
        item->appendRow(child);*/
        sceneViewModel->setItem(0, 0, objsItem);

        // Add default terrain group to scene view
        // TODO: Add icon
        QStandardItem* terrainItem = new QStandardItem("Terrain Groups");
        TerrainGroup* trrGroups = GFX::MainInstance.GetTerrainGroups();

        //item->setData(QVariant::fromValue(&proj));
        terrainItem->setEditable(false);

        AddTerrainInstances(trrGroups[0], terrainItem);

        // Add other terrain groups to scene view
        for (std::size_t i = 1; i < GFX::MainInstance.GetTerrainGroupCount(); ++i)
        {
            QStandardItem* trrGroupItem = new QStandardItem(
                QString::fromUtf8(trrGroups[i].Name.c_str()));

            AddTerrainInstances(trrGroups[i], trrGroupItem);

            //item->setData(QVariant::fromValue(&proj));
            trrGroupItem->setEditable(false);
            terrainItem->appendRow(trrGroupItem);
        }

        //UpdateTerrain(proj.RootNode, item);
        /*child->setEditable(false);
        item->appendRow(child);*/
        sceneViewModel->setItem(1, 0, terrainItem);
    }

    template<bool isSelected>
    void INSelectionChanged(const QItemSelection& selection)
    {
        // TODO

        // Set IsSelected appropriately on all casts that were just selected/deselected
        //for (const auto& range : selection)
        //{
        //    for (const auto& item : range.indexes())
        //    {
        //        // Skip nodes that don't contain a cast
        //        QVariant v = item.data(Qt::UserRole + 1);
        //        const char* typeName = v.typeName();

        //        if (std::strcmp(typeName, "hl::NCPCast*")) continue;

        //        // Set IsSelected on cast
        //        hl::NCPCast* cast = v.value<hl::NCPCast*>();
        //        cast->IsSelected = isSelected;

        //        // Increment SelectedCastsCount if necessary
        //        if constexpr (isSelected)
        //        {
        //            ++GFX::MainInstance.SelectedCastsCount;
        //        }
        //        else
        //        {
        //            --GFX::MainInstance.SelectedCastsCount;
        //        }

        //        //v.userType(); .value<hl::NCPCast>();
        //    }
        //}
    }

    void SceneViewWidget::selectionChanged(const QItemSelection& selected,
        const QItemSelection& deselected)
    {
        if (!MainInstance) return;

        // Update scene view
        INSelectionChanged<false>(deselected);
        INSelectionChanged<true>(selected);

        // Update properties panel
        // TODO
        //MainInstance->UpdateProperties();
    }
}
