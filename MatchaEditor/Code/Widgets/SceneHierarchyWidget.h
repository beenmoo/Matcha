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
