#include "SceneHierarchyWidget.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <entt/entt.hpp>

#include <QAction>
#include <QMenu>
#include <QSignalBlocker>

namespace MatchaEditor
{
using Matcha::Entity;
using Matcha::HierarchyComponent;
using Matcha::Scene;
using Matcha::TagComponent;

namespace
{
quint32 HandleToVariant(entt::entity handle)
{
    return static_cast<quint32>(entt::to_integral(handle));
}

entt::entity VariantToHandle(const QVariant& variant)
{
    return static_cast<entt::entity>(variant.toUInt());
}
}  // namespace

SceneHierarchyWidget::SceneHierarchyWidget(Scene& scene, QWidget* parent)
    : QTreeWidget(parent),
      m_Scene(scene)
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::SingleSelection);

    connect(this, &QTreeWidget::customContextMenuRequested, this, &SceneHierarchyWidget::ShowContextMenu);
    connect(this, &QTreeWidget::itemChanged, this, &SceneHierarchyWidget::RenameItem);

    m_Scene.SetOnSceneChanged([this] { Refresh(); });

    Refresh();
}

Entity SceneHierarchyWidget::GetSelectedEntity() const
{
    QTreeWidgetItem* item = currentItem();
    if (!item)
        return Entity();

    return Entity(VariantToHandle(item->data(0, Qt::UserRole)), &m_Scene);
}

void SceneHierarchyWidget::Refresh()
{
    QTreeWidgetItem* current = currentItem();
    quint32 selectedHandle = current ? current->data(0, Qt::UserRole).toUInt() : 0;

    // Rebuilding programmatically sets item text, which would otherwise fire itemChanged and
    // route through RenameItem as if the user had typed it.
    const QSignalBlocker blocker(this);

    clear();
    m_ItemsByHandle.clear();

    for (Entity root : m_Scene.GetRootEntities())
        AddEntityItem(nullptr, root.GetHandle());

    if (current && m_ItemsByHandle.contains(selectedHandle))
        setCurrentItem(m_ItemsByHandle[selectedHandle]);
}

void SceneHierarchyWidget::AddEntityItem(QTreeWidgetItem* parentItem, entt::entity handle)
{
    Entity entity(handle, &m_Scene);

    QString label = entity.HasComponent<TagComponent>() ? QString::fromStdString(entity.GetComponent<TagComponent>().name) : "Entity";

    QTreeWidgetItem* item = parentItem ? new QTreeWidgetItem(parentItem) : new QTreeWidgetItem(this);
    item->setText(0, label);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setData(0, Qt::UserRole, HandleToVariant(handle));

    m_ItemsByHandle.insert(HandleToVariant(handle), item);

    if (entity.HasComponent<HierarchyComponent>())
    {
        entt::entity childHandle = entity.GetComponent<HierarchyComponent>().firstChild;
        while (childHandle != entt::null)
        {
            AddEntityItem(item, childHandle);
            childHandle = entity.WithHandle(childHandle).GetComponent<HierarchyComponent>().nextSibling;
        }
    }
}

void SceneHierarchyWidget::ShowContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);

    QMenu menu(this);
    QAction* createAction = menu.addAction(item ? "Create Child Entity" : "Create Empty Entity");
    QAction* deleteAction = item ? menu.addAction("Delete") : nullptr;
    QAction* renameAction = item ? menu.addAction("Rename") : nullptr;

    QAction* chosen = menu.exec(viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == createAction)
    {
        // Capture the parent's handle before CreateEntity(): it notifies the scene-changed
        // callback synchronously, which Refresh()es this tree and deletes every QTreeWidgetItem
        // - including `item` - so it can't be touched again afterward.
        entt::entity parentHandle = item ? VariantToHandle(item->data(0, Qt::UserRole)) : entt::null;

        Entity created = m_Scene.CreateEntity("Entity");
        if (parentHandle != entt::null)
            SetParent(created, Entity(parentHandle, &m_Scene));
    }
    else if (chosen == deleteAction)
    {
        DestroyEntityRecursive(m_Scene, Entity(VariantToHandle(item->data(0, Qt::UserRole)), &m_Scene));
    }
    else if (chosen == renameAction)
    {
        editItem(item, 0);
    }
}

void SceneHierarchyWidget::RenameItem(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    Entity entity(VariantToHandle(item->data(0, Qt::UserRole)), &m_Scene);
    if (entity.IsValid() && entity.HasComponent<TagComponent>())
        entity.GetComponent<TagComponent>().name = item->text(0).toStdString();
}
}  // namespace MatchaEditor
