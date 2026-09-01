#pragma once

#include <entt/entt.hpp>

#include <Matcha.h>
#include <QHash>
#include <QTreeWidget>
#include <vector>

namespace Matcha
{
class Entity;
}  // namespace Matcha

namespace MatchaEditor
{
// Builds and maintains a tree view of a Scene's entities, driven by Scene::SetOnSceneChanged
// rather than rebuilding every tick - see Scene::GetRootEntities()/HierarchyComponent for the
// traversal this mirrors.
class SceneHierarchyWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit SceneHierarchyWidget(EngineContext& context, QWidget* parent = nullptr);

    [[nodiscard]] std::vector<Entity> GetSelectedEntities() const;

signals:
    void SelectionChanged(std::vector<Entity> entities);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

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

    // Lazily creates (once) and reuses a single cube mesh/shader pair for every "Create Cube" -
    // resource creation issues GL calls, which need the viewport's context current. That's only
    // guaranteed automatically inside the render loop, so this makes it current itself first.
    Entity CreateCubeEntity();
    Entity CreateCameraEntity();
    Entity CreateLightEntity();

private:
    EngineContext& m_Context;

    ShaderHandle m_CubeShader;
    MeshHandle m_CubeMesh;

    // Rebuilt every Refresh() - lets a structural change elsewhere (e.g. a new mesh import)
    // reselect whatever was selected before, without the panel having to track it itself.
    QHash<quint32, QTreeWidgetItem*> m_ItemsByHandle;
};
}  // namespace MatchaEditor
