#pragma once

#include <entt/entt.hpp>

#include <Matcha.h>
#include <QHash>
#include <QTreeWidget>
#include <vector>

namespace Matcha
{
class Scene;
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
    explicit SceneHierarchyWidget(Scene& scene, QWidget* parent = nullptr);

    [[nodiscard]] std::vector<Entity> GetSelectedEntities() const;

signals:
    void SelectionChanged(std::vector<Entity> entities);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    void Refresh();
    void AddEntityItem(QTreeWidgetItem* parentItem, entt::entity handle);
    void ShowContextMenu(const QPoint& pos);
    void RenameItem(QTreeWidgetItem* item, int column);
    void RenameSelected(const QList<QTreeWidgetItem*>& items);

    Scene& m_Scene;

    // Rebuilt every Refresh() - lets a structural change elsewhere (e.g. a new mesh import)
    // reselect whatever was selected before, without the panel having to track it itself.
    QHash<quint32, QTreeWidgetItem*> m_ItemsByHandle;
};
}  // namespace MatchaEditor
