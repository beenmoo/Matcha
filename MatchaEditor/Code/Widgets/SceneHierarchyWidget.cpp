#include "SceneHierarchyWidget.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include <Matcha.h>

#include <entt/entt.hpp>

#include <QAction>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalBlocker>

#include <algorithm>
#include <vector>

namespace MatchaEditor
{
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

SceneHierarchyWidget::SceneHierarchyWidget(EngineContext& context, QWidget* parent)
    : QTreeWidget(parent),
      m_Context(context),
      m_Scene(context.GetScene())
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, &QTreeWidget::customContextMenuRequested, this, &SceneHierarchyWidget::ShowContextMenu);
    connect(this, &QTreeWidget::itemChanged, this, &SceneHierarchyWidget::RenameItem);

    // Not fired by Refresh()'s own reselection - that happens under a QSignalBlocker, so
    // rebuilding the tree around the same still-selected set doesn't spuriously renotify.
    connect(this, &QTreeWidget::itemSelectionChanged, this, [this] { emit SelectionChanged(GetSelectedEntities()); });

    m_Scene.AddOnSceneChanged([this] { Refresh(); });

    Refresh();
}

std::vector<Entity> SceneHierarchyWidget::GetSelectedEntities() const
{
    std::vector<Entity> entities;

    for (QTreeWidgetItem* item : selectedItems())
        entities.emplace_back(VariantToHandle(item->data(0, Qt::UserRole)), &m_Scene);

    return entities;
}

void SceneHierarchyWidget::Refresh()
{
    QList<quint32> previouslySelected;
    for (QTreeWidgetItem* item : selectedItems())
        previouslySelected.push_back(item->data(0, Qt::UserRole).toUInt());

    // Rebuilding programmatically sets item text, which would otherwise fire itemChanged and
    // route through RenameItem as if the user had typed it.
    const QSignalBlocker blocker(this);

    clear();
    m_ItemsByHandle.clear();

    for (Entity root : m_Scene.GetRootEntities())
        AddEntityItem(nullptr, root.GetHandle());

    for (quint32 handle : previouslySelected)
        if (m_ItemsByHandle.contains(handle))
            m_ItemsByHandle[handle]->setSelected(true);
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

Entity SceneHierarchyWidget::CreateCubeEntity()
{
    ResourceManager& resourceManager = m_Context.GetResourceManager();

    if (!m_CubeShader.IsValid() || !m_CubeMesh.IsValid())
    {
        // Resource creation below issues GL calls (shader compilation, buffer uploads) - only
        // guaranteed to have the viewport's context current inside the render loop, so make it
        // current explicitly here. Cached in m_CubeShader/m_CubeMesh above so this only runs once
        // per editor session - every subsequent cube reuses the same shader/mesh handles.
        m_Context.GetWindow().MakeContextCurrent();

        m_CubeShader = resourceManager.CreateShader(
            "StandardMesh", {"Assets/Shaders/StandardMesh.vert", "Assets/Shaders/StandardMesh.frag"});

        CubePrimitive cubePrimitive;
        m_CubeMesh = resourceManager.CreateMesh(cubePrimitive.vertices,
                                                {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2},
                                                cubePrimitive.indices);
    }

    Entity entity = m_Scene.CreateEntity("Cube");
    entity.AddComponent<MeshComponent>().mesh = m_CubeMesh;

    MaterialComponent& material = entity.AddComponent<MaterialComponent>();
    material.shader = m_CubeShader;
    material.albedoColor = Vector4(0.8f, 0.8f, 0.8f, 1.0f);

    return entity;
}

Entity SceneHierarchyWidget::CreateCameraEntity()
{
    Entity entity = m_Scene.CreateEntity("Camera");
    entity.AddComponent<CameraComponent>().aspectRatio = m_Context.GetWindow().GetAspectRatio();
    entity.AddComponent<NativeScriptComponent>().Bind<CameraController>();

    return entity;
}

Entity SceneHierarchyWidget::CreateLightEntity()
{
    Entity entity = m_Scene.CreateEntity("Light");
    entity.AddComponent<LightComponent>();

    return entity;
}

void SceneHierarchyWidget::ShowContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);
    QList<QTreeWidgetItem*> selected = selectedItems();

    QMenu menu(this);
    QAction* createAction = menu.addAction(item ? "Create Child Entity" : "Create Empty Entity");
    QAction* createCubeAction = menu.addAction("Create Cube");
    QAction* createCameraAction = menu.addAction("Create Camera");
    QAction* createLightAction = menu.addAction("Create Light");
    QAction* deleteAction = !selected.isEmpty() ? menu.addAction(selected.size() > 1 ? "Delete Selected" : "Delete") : nullptr;
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
    else if (chosen == createCubeAction)
    {
        // Same reasoning as createAction above: capture the parent's handle before creating the
        // entity, since that notifies (and thus Refresh()es, deleting `item`) synchronously.
        entt::entity parentHandle = item ? VariantToHandle(item->data(0, Qt::UserRole)) : entt::null;

        Entity created = CreateCubeEntity();
        if (parentHandle != entt::null)
            SetParent(created, Entity(parentHandle, &m_Scene));
    }
    else if (chosen == createCameraAction)
    {
        // Same reasoning as createAction above: capture the parent's handle before creating the
        // entity, since that notifies (and thus Refresh()es, deleting `item`) synchronously.
        entt::entity parentHandle = item ? VariantToHandle(item->data(0, Qt::UserRole)) : entt::null;

        Entity created = CreateCameraEntity();
        if (parentHandle != entt::null)
            SetParent(created, Entity(parentHandle, &m_Scene));
    }
    else if (chosen == createLightAction)
    {
        // Same reasoning as createAction above: capture the parent's handle before creating the
        // entity, since that notifies (and thus Refresh()es, deleting `item`) synchronously.
        entt::entity parentHandle = item ? VariantToHandle(item->data(0, Qt::UserRole)) : entt::null;

        Entity created = CreateLightEntity();
        if (parentHandle != entt::null)
            SetParent(created, Entity(parentHandle, &m_Scene));
    }
    else if (chosen == deleteAction)
    {
        // Collected up front: DestroyEntityRecursive() notifies (Refresh()ing this tree, which
        // deletes every QTreeWidgetItem) after each top-level deletion, so nothing below this
        // point can keep reading from `selected`/`item`.
        std::vector<entt::entity> handles;
        handles.reserve(selected.size());
        for (QTreeWidgetItem* selectedItem : selected)
            handles.push_back(VariantToHandle(selectedItem->data(0, Qt::UserRole)));

        for (entt::entity handle : handles)
        {
            Entity entity(handle, &m_Scene);
            if (!entity.IsValid())
                continue;  // already gone - destroyed below as part of a selected ancestor's subtree

            // Skip if a selected ancestor will destroy this one anyway as part of its own
            // subtree - deleting it here too would double-destroy an already-dead handle.
            bool hasSelectedAncestor = false;
            if (entity.HasComponent<HierarchyComponent>())
            {
                entt::entity ancestor = entity.GetComponent<HierarchyComponent>().parent;
                while (ancestor != entt::null)
                {
                    if (std::find(handles.begin(), handles.end(), ancestor) != handles.end())
                    {
                        hasSelectedAncestor = true;
                        break;
                    }
                    ancestor = entity.WithHandle(ancestor).GetComponent<HierarchyComponent>().parent;
                }
            }

            if (!hasSelectedAncestor)
                DestroyEntityRecursive(m_Scene, entity);
        }
    }
    else if (chosen == renameAction)
    {
        if (selected.size() > 1)
            RenameSelected(selected);
        else
            editItem(item, 0);
    }
}

void SceneHierarchyWidget::mousePressEvent(QMouseEvent* event)
{
    // Clicking empty space (no item under the cursor) deselects everything, rather than leaving
    // whatever was selected before untouched - matches the behavior of most tree/list editors.
    if (!itemAt(event->pos()))
        clearSelection();

    QTreeWidget::mousePressEvent(event);
}

void SceneHierarchyWidget::RenameItem(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    Entity entity(VariantToHandle(item->data(0, Qt::UserRole)), &m_Scene);
    if (entity.IsValid() && entity.HasComponent<TagComponent>())
        entity.GetComponent<TagComponent>().name = item->text(0).toStdString();
}

// editItem() only ever drives one native inline editor at a time, and per-row inline editors
// turned out fiddly to make read correctly against the tree's own selection styling - so renaming
// several entities at once just asks for one name up front and applies it to all of them.
void SceneHierarchyWidget::RenameSelected(const QList<QTreeWidgetItem*>& items)
{
    bool ok = false;
    QString name = QInputDialog::getText(this, "Rename Entities", "Name:", QLineEdit::Normal, items.front()->text(0), &ok);
    if (!ok || name.isEmpty())
        return;

    for (QTreeWidgetItem* item : items)
        item->setText(0, name);  // fires itemChanged -> RenameItem() applies it to the entity
}
}  // namespace MatchaEditor
