#pragma once

#include <entt/entt.hpp>

#include <Matcha.h>
#include <QHash>
#include <QTreeWidget>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <vector>

namespace Matcha
{
class Entity;
}  // namespace Matcha

namespace MatchaEditor
{
class CommandManager;

// Builds and maintains a tree view of a Scene's entities, driven by Scene::SetOnSceneChanged
// rather than rebuilding every tick - see Scene::GetRootEntities()/HierarchyComponent for the
// traversal this mirrors.
class SceneHierarchyWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit SceneHierarchyWidget(EngineContext& context, CommandManager& commandManager, QWidget* parent = nullptr);

    [[nodiscard]] std::vector<Entity> GetSelectedEntities() const;

signals:
    void SelectionChanged(std::vector<Entity> entities);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    // (Re)subscribes to the *current* Scene's AddOnSceneChanged and refreshes - called both at
    // construction and every time SceneManager::AddOnSceneReplaced fires, since a swapped-out
    // Scene destroys its own subscriber list along with it.
    void BindScene();
    void Refresh();
    void AddEntityItem(QTreeWidgetItem* parentItem, entt::entity handle);
    void ShowContextMenu(const QPoint& pos);
    void RenameItem(QTreeWidgetItem* item, int column);
    void RenameSelected(const QList<QTreeWidgetItem*>& items);

    // Shared by the context menu's Delete/Delete Selected action and the Delete key
    // (keyPressEvent) - both just act on whatever's currently selected.
    void DeleteSelectedEntities();

    // Shared by the context menu's Rename action and the F2 key - single selection opens the
    // tree's own inline editor, multiple goes through RenameSelected()'s one-name-for-all dialog.
    void RenameCurrentSelection();

    // Ctrl+Shift+N (Unity's own binding for this) - creates as a child of the current selection
    // if there is one, else at the scene root. Deliberately separate from the context menu's own
    // Create Empty Entity/Create Child Entity handling in ShowContextMenu(), which targets
    // whatever was right-clicked rather than the current selection - the two aren't always the
    // same item, so sharing one implementation would change one path's existing behavior.
    void CreateEntityAtSelection();

    // Ctrl+C / Ctrl+V. Copy snapshots the selection (whole subtrees, not just the selected rows)
    // into m_Clipboard as serialized data rather than as entity references, so it survives the
    // originals being deleted, renamed, or reparented afterwards. Paste re-creates it under the
    // current selection with fresh UUIDs - see PasteEntitiesCommand.
    void CopySelectedEntities();
    void PasteEntitiesAtSelection();

    // The selected entities with any whose ancestor is also selected filtered out - those are
    // already covered by that ancestor's own subtree, so acting on both would double-handle them
    // (destroying an already-destroyed entity, copying a descendant twice, reparenting something
    // that moves anyway). Shared by delete, copy, and drag-and-drop reparenting, all of which need
    // exactly this set.
    [[nodiscard]] std::vector<Entity> CollectSelectedSubtreeRoots() const;

    // One creatable entity kind: its context-menu entry, its undo description, the name the entity
    // gets, and a factory producing the callback that adds its components. Kept as data so adding
    // a kind is one table entry (see EntityTemplates()) rather than an edit to both the menu and a
    // matching dispatch branch.
    struct EntityTemplate
    {
        std::string menuLabel;
        std::string commandDescription;
        std::string entityName;

        // Takes the widget so it can resolve editor-side state (a GL resource, the viewport's
        // aspect ratio) once at action time, then returns the callback CreateEntityCommand runs on
        // execute and on every redo.
        std::function<std::function<void(Entity)>(SceneHierarchyWidget&)> makePopulate;
    };

    [[nodiscard]] static const std::vector<EntityTemplate>& EntityTemplates();
    void CreateFromTemplate(const EntityTemplate& entityTemplate, std::optional<UUID> parentId);

    // Lazily compiles (once) the shader every editor-created mesh entity uses. Shader compilation
    // issues GL calls needing the viewport's context current, which is only guaranteed inside the
    // render loop - so this makes it current itself first. Mesh geometry is no longer created
    // here: ResourceManager::GetOrCreatePrimitiveMesh owns that, shared with SceneSerializer.
    ShaderHandle EnsureStandardMeshShader();

    // Reads the right-clicked/selected item's entity id, if any - used as CreateEntityCommand's
    // parentId. Captured as a UUID (not the raw entt::entity handle) because CreateEntityCommand
    // resolves its parent fresh at Execute()/Redo() time, by which point the original handle may
    // have been recycled by an intervening destroy+recreate.
    std::optional<UUID> ParentIdFor(QTreeWidgetItem* item) const;

    // Walks up from `entity` via HierarchyComponent::parent (same traversal shape as
    // IsActiveInHierarchy/ParentIdFor) checking whether `ancestor` is reached - true if `entity`
    // *is* `ancestor` too, not just a strict descendant. Used to reject a drag-and-drop reparent
    // that would drop an entity onto itself or one of its own descendants, which would otherwise
    // create a cycle in the hierarchy.
    [[nodiscard]] static bool IsAncestorOrSelf(Entity ancestor, Entity entity);

private:
    EngineContext& m_Context;
    CommandManager& m_CommandManager;

    ShaderHandle m_StandardMeshShader;

    // Rebuilt every Refresh() - lets a structural change elsewhere (e.g. a new mesh import)
    // reselect whatever was selected before, without the panel having to track it itself.
    QHash<quint32, QTreeWidgetItem*> m_ItemsByHandle;

    // Serialized entity data, not entity references: a copy stays pasteable after its originals
    // are deleted, and pastes correctly into a different scene than it was copied from.
    nlohmann::json m_Clipboard;
};
}  // namespace MatchaEditor
