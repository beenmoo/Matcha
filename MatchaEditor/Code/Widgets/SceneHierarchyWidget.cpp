#include "SceneHierarchyWidget.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Core/CommandManager.h"
#include "Core/Commands/CreateEntityCommand.h"
#include "Core/Commands/DeleteEntitiesCommand.h"
#include "Core/Commands/RenameEntitiesCommand.h"

#include <Matcha.h>

#include <entt/entt.hpp>

#include <QAction>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalBlocker>

#include <algorithm>
#include <memory>
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

SceneHierarchyWidget::SceneHierarchyWidget(EngineContext& context, CommandManager& commandManager, QWidget* parent)
    : QTreeWidget(parent),
      m_Context(context),
      m_CommandManager(commandManager)
{
    setHeaderHidden(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, &QTreeWidget::customContextMenuRequested, this, &SceneHierarchyWidget::ShowContextMenu);
    connect(this, &QTreeWidget::itemChanged, this, &SceneHierarchyWidget::RenameItem);

    // Not fired by Refresh()'s own reselection - that happens under a QSignalBlocker, so
    // rebuilding the tree around the same still-selected set doesn't spuriously renotify.
    connect(this, &QTreeWidget::itemSelectionChanged, this, [this] { emit SelectionChanged(GetSelectedEntities()); });

    // Re-subscribes to whatever Scene is current whenever SceneManager swaps it out (New/Open) -
    // a swapped-out Scene's own AddOnSceneChanged subscriber list is destroyed along with it.
    context.GetSceneManager().AddOnSceneReplaced([this] { BindScene(); });
    BindScene();
}

void SceneHierarchyWidget::BindScene()
{
    m_Context.GetScene().AddOnSceneChanged([this] { Refresh(); });
    Refresh();
}

std::vector<Entity> SceneHierarchyWidget::GetSelectedEntities() const
{
    Scene& scene = m_Context.GetScene();
    std::vector<Entity> entities;

    for (QTreeWidgetItem* item : selectedItems())
        entities.emplace_back(VariantToHandle(item->data(0, Qt::UserRole)), &scene);

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

    for (Entity root : m_Context.GetScene().GetRootEntities())
        AddEntityItem(nullptr, root.GetHandle());

    for (quint32 handle : previouslySelected)
        if (m_ItemsByHandle.contains(handle))
            m_ItemsByHandle[handle]->setSelected(true);
}

void SceneHierarchyWidget::AddEntityItem(QTreeWidgetItem* parentItem, entt::entity handle)
{
    Entity entity(handle, &m_Context.GetScene());

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

void SceneHierarchyWidget::EnsureCubeResources()
{
    if (m_CubeShader.IsValid() && m_CubeMesh.IsValid())
        return;

    // Resource creation below issues GL calls (shader compilation, buffer uploads) - only
    // guaranteed to have the viewport's context current inside the render loop, so make it
    // current explicitly here. Cached in m_CubeShader/m_CubeMesh so this only runs once per
    // editor session - every subsequent cube reuses the same shader/mesh handles.
    m_Context.GetWindow().MakeContextCurrent();

    ResourceManager& resourceManager = m_Context.GetResourceManager();
    m_CubeShader = resourceManager.CreateShader(
        "StandardMesh", {"Assets/Shaders/StandardMesh.vert", "Assets/Shaders/StandardMesh.frag"});

    // "Cube" tags this handle as a regeneratable primitive - see ResourceManager::CreateMesh
    // and SceneSerializer, which needs to be able to rebuild this exact geometry from scratch
    // on scene load rather than saving/resolving a source file that doesn't exist for it.
    CubePrimitive cubePrimitive;
    m_CubeMesh = resourceManager.CreateMesh(cubePrimitive.vertices,
                                            {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2},
                                            cubePrimitive.indices, "Cube");
}

std::optional<UUID> SceneHierarchyWidget::ParentIdFor(QTreeWidgetItem* item) const
{
    if (!item)
        return std::nullopt;

    Entity entity(VariantToHandle(item->data(0, Qt::UserRole)), &m_Context.GetScene());
    if (!entity.IsValid() || !entity.HasComponent<TagComponent>())
        return std::nullopt;

    return entity.GetComponent<TagComponent>().id;
}

void SceneHierarchyWidget::ShowContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);
    QList<QTreeWidgetItem*> selected = selectedItems();

    // Resolved before the menu runs any action below: CreateEntityCommand::Execute() notifies the
    // scene-changed callback synchronously, which Refresh()es this tree and deletes every
    // QTreeWidgetItem - including `item` - so it can't be touched again afterward.
    std::optional<UUID> parentId = ParentIdFor(item);

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
        m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(m_Context, "Create Entity", "Entity", parentId));
    }
    else if (chosen == createCubeAction)
    {
        EnsureCubeResources();
        ShaderHandle cubeShader = m_CubeShader;
        MeshHandle cubeMesh = m_CubeMesh;

        m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(
            m_Context, "Create Cube", "Cube", parentId, [cubeMesh, cubeShader](Entity entity) {
                entity.AddComponent<MeshComponent>().mesh = cubeMesh;

                MaterialComponent& material = entity.AddComponent<MaterialComponent>();
                material.shader = cubeShader;
                material.albedoColor = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
            }));
    }
    else if (chosen == createCameraAction)
    {
        float aspectRatio = m_Context.GetWindow().GetAspectRatio();

        m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(
            m_Context, "Create Camera", "Camera", parentId,
            [aspectRatio](Entity entity) { entity.AddComponent<CameraComponent>().aspectRatio = aspectRatio; }));
    }
    else if (chosen == createLightAction)
    {
        m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(
            m_Context, "Create Light", "Light", parentId, [](Entity entity) { entity.AddComponent<LightComponent>(); }));
    }
    else if (chosen == deleteAction)
    {
        DeleteSelectedEntities();
    }
    else if (chosen == renameAction)
    {
        RenameCurrentSelection();
    }
}

void SceneHierarchyWidget::DeleteSelectedEntities()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.isEmpty())
        return;

    Scene& scene = m_Context.GetScene();

    // Collected up front, same as before this routed through a Command: nothing here destroys
    // anything itself anymore (that happens once, inside DeleteEntitiesCommand::Execute()), but
    // `selected`'s items are still only valid for this synchronous call.
    std::vector<entt::entity> handles;
    handles.reserve(selected.size());
    for (QTreeWidgetItem* selectedItem : selected)
        handles.push_back(VariantToHandle(selectedItem->data(0, Qt::UserRole)));

    std::vector<Entity> subtreeRoots;
    for (entt::entity handle : handles)
    {
        Entity entity(handle, &scene);
        if (!entity.IsValid())
            continue;

        // Skip if a selected ancestor already covers this one as part of its own subtree -
        // otherwise the same descendant would end up in the snapshot twice.
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
            subtreeRoots.push_back(entity);
    }

    if (subtreeRoots.empty())
        return;

    QString description = subtreeRoots.size() > 1 ? QString("Delete %1 Entities").arg(subtreeRoots.size()) : QString("Delete Entity");
    m_CommandManager.ExecuteCommand(
        std::make_unique<DeleteEntitiesCommand>(m_Context, description.toStdString(), subtreeRoots));
}

void SceneHierarchyWidget::RenameCurrentSelection()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.isEmpty())
        return;

    if (selected.size() > 1)
        RenameSelected(selected);
    else
        editItem(selected.front(), 0);
}

void SceneHierarchyWidget::CreateEntityAtSelection()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    std::optional<UUID> parentId = ParentIdFor(selected.isEmpty() ? nullptr : selected.front());

    m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(m_Context, "Create Entity", "Entity", parentId));
}

void SceneHierarchyWidget::mousePressEvent(QMouseEvent* event)
{
    // Clicking empty space (no item under the cursor) deselects everything, rather than leaving
    // whatever was selected before untouched - matches the behavior of most tree/list editors.
    if (!itemAt(event->pos()))
        clearSelection();

    QTreeWidget::mousePressEvent(event);
}

void SceneHierarchyWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        DeleteSelectedEntities();
        return;
    }

    if (event->key() == Qt::Key_F2)
    {
        RenameCurrentSelection();
        return;
    }

    if (event->key() == Qt::Key_N && event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
    {
        CreateEntityAtSelection();
        return;
    }

    QTreeWidget::keyPressEvent(event);
}

void SceneHierarchyWidget::RenameItem(QTreeWidgetItem* item, int column)
{
    if (column != 0)
        return;

    Entity entity(VariantToHandle(item->data(0, Qt::UserRole)), &m_Context.GetScene());
    if (!entity.IsValid() || !entity.HasComponent<TagComponent>())
        return;

    std::string oldName = entity.GetComponent<TagComponent>().name;
    std::string newName = item->text(0).toStdString();
    if (oldName == newName)
        return;

    std::vector<RenameEntitiesCommand::Rename> renames{{entity.GetComponent<TagComponent>().id, oldName, newName}};
    m_CommandManager.ExecuteCommand(std::make_unique<RenameEntitiesCommand>(m_Context, "Rename Entity", std::move(renames)));
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

    Scene& scene = m_Context.GetScene();
    std::vector<RenameEntitiesCommand::Rename> renames;
    renames.reserve(items.size());

    {
        // Sets item text directly rather than relying on itemChanged -> RenameItem() (which would
        // push one Command per item) - blocked the same way Refresh() blocks its own bulk update,
        // so the batch below becomes a single undo entry instead of N.
        const QSignalBlocker blocker(this);
        for (QTreeWidgetItem* item : items)
        {
            Entity entity(VariantToHandle(item->data(0, Qt::UserRole)), &scene);
            if (!entity.IsValid() || !entity.HasComponent<TagComponent>())
                continue;

            renames.push_back({entity.GetComponent<TagComponent>().id, entity.GetComponent<TagComponent>().name, name.toStdString()});
            item->setText(0, name);
        }
    }

    if (!renames.empty())
        m_CommandManager.ExecuteCommand(std::make_unique<RenameEntitiesCommand>(m_Context, "Rename Entities", std::move(renames)));
}
}  // namespace MatchaEditor
