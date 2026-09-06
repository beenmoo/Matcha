#include "SceneHierarchyWidget.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Core/CommandManager.h"
#include "Core/Commands/CreateEntityCommand.h"
#include "Core/Commands/DeleteEntitiesCommand.h"
#include "Core/Commands/PasteEntitiesCommand.h"
#include "Core/Commands/RenameEntitiesCommand.h"
#include "Core/Commands/ReparentEntitiesCommand.h"

#include <Matcha.h>

#include <entt/entt.hpp>

#include <QAction>
#include <QDropEvent>
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
    setDragDropMode(QAbstractItemView::InternalMove);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

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

    // Expansion is restored alongside selection: this rebuild throws away every QTreeWidgetItem,
    // so without capturing it, any scene change (creating an entity, renaming one, an undo)
    // would collapse the whole tree and lose the user's place in it.
    QSet<quint32> previouslyExpanded;
    for (auto it = m_ItemsByHandle.constBegin(); it != m_ItemsByHandle.constEnd(); ++it)
        if (it.value()->isExpanded())
            previouslyExpanded.insert(it.key());

    // Rebuilding programmatically sets item text, which would otherwise fire itemChanged and
    // route through RenameItem as if the user had typed it.
    const QSignalBlocker blocker(this);

    clear();
    m_ItemsByHandle.clear();

    for (Entity root : m_Context.GetScene().GetRootEntities())
        AddEntityItem(nullptr, root.GetHandle());

    for (quint32 handle : previouslyExpanded)
        if (m_ItemsByHandle.contains(handle))
            m_ItemsByHandle[handle]->setExpanded(true);

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
        entt::entity childHandle = entity.GetComponent<HierarchyComponent>().GetFirstChild();
        while (childHandle != entt::null)
        {
            AddEntityItem(item, childHandle);
            childHandle = entity.WithHandle(childHandle).GetComponent<HierarchyComponent>().GetNextSibling();
        }
    }
}

const std::vector<SceneHierarchyWidget::EntityTemplate>& SceneHierarchyWidget::EntityTemplates()
{
    // `populate` runs inside CreateEntityCommand::Execute(), i.e. on redo as well as the first
    // creation, so it must only touch the entity it's given - anything needing the editor's own
    // state (a GL resource, the viewport's aspect ratio) is resolved once in CreateFromTemplate
    // and captured, not read from here.
    static const std::vector<EntityTemplate> templates = {
        {"Create Cube", "Create Cube", "Cube",
         [](SceneHierarchyWidget& widget) -> std::function<void(Entity)> {
             // Resolved here, once per action, rather than inside the returned callback: both need
             // the GL context made current, which only this menu-driven path arranges for - an
             // undo/redo replay runs the callback with no such guarantee.
             ShaderHandle shader = widget.EnsureStandardMeshShader();
             MeshHandle mesh = widget.m_Context.GetResourceManager().GetOrCreatePrimitiveMesh("Cube");

             return [mesh, shader](Entity entity) {
                 entity.AddComponent<MeshComponent>().mesh = mesh;

                 MaterialComponent& material = entity.AddComponent<MaterialComponent>();
                 material.shader = shader;
                 material.albedoColor = Vector4(0.8f, 0.8f, 0.8f, 1.0f);
             };
         }},
        {"Create Camera", "Create Camera", "Camera",
         [](SceneHierarchyWidget& widget) -> std::function<void(Entity)> {
             float aspectRatio = widget.m_Context.GetWindow().GetAspectRatio();
             return [aspectRatio](Entity entity) { entity.AddComponent<CameraComponent>().aspectRatio = aspectRatio; };
         }},
        {"Create Light", "Create Light", "Light",
         [](SceneHierarchyWidget&) -> std::function<void(Entity)> {
             return [](Entity entity) { entity.AddComponent<LightComponent>(); };
         }},
    };

    return templates;
}

void SceneHierarchyWidget::CreateFromTemplate(const EntityTemplate& entityTemplate, std::optional<UUID> parentId)
{
    m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(
        m_Context, entityTemplate.commandDescription, entityTemplate.entityName, parentId, entityTemplate.makePopulate(*this)));
}

ShaderHandle SceneHierarchyWidget::EnsureStandardMeshShader()
{
    if (m_StandardMeshShader.IsValid())
        return m_StandardMeshShader;

    // Shader compilation issues GL calls, only guaranteed to have the viewport's context current
    // inside the render loop - so make it current explicitly here, since this runs from a menu
    // action instead. Cached, so this only compiles once per editor session.
    m_Context.GetWindow().MakeContextCurrent();

    m_StandardMeshShader = m_Context.GetResourceManager().CreateShader(
        "StandardMesh", {"Assets/Shaders/StandardMesh.vert", "Assets/Shaders/StandardMesh.frag"});

    return m_StandardMeshShader;
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

bool SceneHierarchyWidget::IsAncestorOrSelf(Entity ancestor, Entity entity)
{
    while (entity.IsValid())
    {
        if (entity == ancestor)
            return true;

        entity = entity.HasComponent<HierarchyComponent>() ? entity.WithHandle(entity.GetComponent<HierarchyComponent>().GetParent()) : Entity();
    }

    return false;
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

    // Menu entry and creation behavior come from one table (see EntityTemplates()), so adding a
    // new creatable entity kind means adding one entry there rather than editing both the menu
    // built here and a matching branch in the dispatch below.
    QAction* createAction = menu.addAction(item ? "Create Child Entity" : "Create Empty Entity");

    std::vector<QAction*> templateActions;
    for (const EntityTemplate& entityTemplate : EntityTemplates())
        templateActions.push_back(menu.addAction(QString::fromStdString(entityTemplate.menuLabel)));

    QAction* deleteAction = !selected.isEmpty() ? menu.addAction(selected.size() > 1 ? "Delete Selected" : "Delete") : nullptr;
    QAction* renameAction = item ? menu.addAction("Rename") : nullptr;

    menu.addSeparator();
    QAction* copyAction = !selected.isEmpty() ? menu.addAction("Copy") : nullptr;
    QAction* pasteAction = menu.addAction("Paste");
    pasteAction->setEnabled(!m_Clipboard.empty());

    QAction* chosen = menu.exec(viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == createAction)
    {
        m_CommandManager.ExecuteCommand(std::make_unique<CreateEntityCommand>(m_Context, "Create Entity", "Entity", parentId));
        return;
    }

    for (size_t i = 0; i < templateActions.size(); ++i)
    {
        if (chosen != templateActions[i])
            continue;

        CreateFromTemplate(EntityTemplates()[i], parentId);
        return;
    }

    if (chosen == deleteAction)
    {
        DeleteSelectedEntities();
    }
    else if (chosen == renameAction)
    {
        RenameCurrentSelection();
    }
    else if (chosen == copyAction)
    {
        CopySelectedEntities();
    }
    else if (chosen == pasteAction)
    {
        PasteEntitiesAtSelection();
    }
}

void SceneHierarchyWidget::DeleteSelectedEntities()
{
    std::vector<Entity> subtreeRoots = CollectSelectedSubtreeRoots();
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

std::vector<Entity> SceneHierarchyWidget::CollectSelectedSubtreeRoots() const
{
    Scene& scene = m_Context.GetScene();

    QList<QTreeWidgetItem*> selected = selectedItems();
    std::vector<entt::entity> handles;
    handles.reserve(selected.size());
    for (QTreeWidgetItem* item : selected)
        handles.push_back(VariantToHandle(item->data(0, Qt::UserRole)));

    std::vector<Entity> roots;
    for (entt::entity handle : handles)
    {
        Entity entity(handle, &scene);
        if (!entity.IsValid())
            continue;

        bool hasSelectedAncestor = false;
        if (entity.HasComponent<HierarchyComponent>())
        {
            entt::entity ancestor = entity.GetComponent<HierarchyComponent>().GetParent();
            while (ancestor != entt::null)
            {
                if (std::find(handles.begin(), handles.end(), ancestor) != handles.end())
                {
                    hasSelectedAncestor = true;
                    break;
                }
                ancestor = entity.WithHandle(ancestor).GetComponent<HierarchyComponent>().GetParent();
            }
        }

        if (!hasSelectedAncestor)
            roots.push_back(entity);
    }

    return roots;
}

void SceneHierarchyWidget::CopySelectedEntities()
{
    std::vector<Entity> roots = CollectSelectedSubtreeRoots();
    if (roots.empty())
        return;

    // Whole subtrees, not just the selected rows: copying a parent is expected to copy what's
    // under it, and a partial copy would deserialize children whose parent isn't in the set.
    std::vector<Entity> entities;
    for (Entity root : roots)
        CollectSubtree(root, entities);

    m_Clipboard = SceneSerializer::SerializeEntities(entities, m_Context.GetResourceManager());
}

void SceneHierarchyWidget::PasteEntitiesAtSelection()
{
    QList<QTreeWidgetItem*> selected = selectedItems();
    std::optional<UUID> parentId = ParentIdFor(selected.isEmpty() ? nullptr : selected.front());

    auto command = std::make_unique<PasteEntitiesCommand>(m_Context, "Paste Entities", m_Clipboard, parentId);
    if (command->IsEmpty())
        return;

    m_CommandManager.ExecuteCommand(std::move(command));
}

void SceneHierarchyWidget::dropEvent(QDropEvent* event)
{
    // Only two outcomes make sense for a model with no sibling-order concept to preserve:
    // dropping squarely onto a row re-parents under that row (OnItem); dropping in the empty area
    // below every row detaches to the scene root (OnViewport). Dropping *between* two rows reads
    // as a reorder request this model has no way to honor, so it's ignored - and the base
    // QTreeWidget::dropEvent() is never called, since it would move QTreeWidgetItems directly;
    // this tree is always rebuilt from Scene state instead (see Refresh()).
    QAbstractItemView::DropIndicatorPosition indicatorPos = dropIndicatorPosition();
    if (indicatorPos != QAbstractItemView::OnItem && indicatorPos != QAbstractItemView::OnViewport)
    {
        event->ignore();
        return;
    }

    QTreeWidgetItem* targetItem = itemAt(event->position().toPoint());
    std::optional<UUID> newParentId = ParentIdFor(targetItem);

    Scene& scene = m_Context.GetScene();
    Entity newParent = newParentId ? scene.FindEntityByUUID(*newParentId) : Entity();

    // Selected rows are exactly the rows being dragged - selection doesn't change over the course
    // of a drag-and-drop within the same view. A dragged entity whose ancestor is also dragged is
    // filtered out by CollectSelectedSubtreeRoots(): it moves implicitly along with that ancestor,
    // so an explicit SetParent for it would be redundant.
    std::vector<ReparentEntitiesCommand::Reparent> reparents;
    for (Entity entity : CollectSelectedSubtreeRoots())
    {
        // Dropping onto itself or one of its own descendants would create a cycle.
        if (newParent.IsValid() && IsAncestorOrSelf(entity, newParent))
            continue;

        std::optional<UUID> currentParentId;
        if (entity.HasComponent<HierarchyComponent>())
        {
            entt::entity parentHandle = entity.GetComponent<HierarchyComponent>().GetParent();
            if (parentHandle != entt::null)
                currentParentId = entity.WithHandle(parentHandle).GetComponent<TagComponent>().id;
        }

        // Already parented there - a no-op drop shouldn't push an empty undo entry.
        if (currentParentId == newParentId)
            continue;

        reparents.push_back({entity.GetComponent<TagComponent>().id, currentParentId});
    }

    if (reparents.empty())
    {
        event->ignore();
        return;
    }

    QString description = reparents.size() > 1 ? QString("Reparent %1 Entities").arg(reparents.size()) : QString("Reparent Entity");
    m_CommandManager.ExecuteCommand(
        std::make_unique<ReparentEntitiesCommand>(m_Context, description.toStdString(), std::move(reparents), newParentId));

    // Not acceptProposedAction(): for QAbstractItemView::InternalMove, the proposed action is
    // Qt::MoveAction, and accepting a drop with MoveAction makes QAbstractItemView::startDrag()
    // (on the drag source side - the same widget here) run its own post-drop cleanup afterward,
    // removing whatever QTreeWidgetItem currently sits at the dragged row's old index. That runs
    // *after* this handler returns - by which point ExecuteCommand()'s NotifyChanged() has
    // already triggered Refresh() and rebuilt the whole tree from scratch, so that "old index"
    // now belongs to an unrelated freshly-created item, which Qt then deletes out from under us
    // (the entity just correctly reparented visibly disappearing until something else happens to
    // Refresh() again). Accepting with IgnoreAction still marks the event handled (so Qt doesn't
    // fall back to any default behavior) without telling startDrag() a model "move" occurred, so
    // that cleanup is skipped - correct here regardless, since this tree never lets Qt manage its
    // own items in the first place.
    event->setDropAction(Qt::IgnoreAction);
    event->accept();
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

    if (event->matches(QKeySequence::Copy))
    {
        CopySelectedEntities();
        event->accept();
        return;
    }

    if (event->matches(QKeySequence::Paste))
    {
        PasteEntitiesAtSelection();
        event->accept();
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
