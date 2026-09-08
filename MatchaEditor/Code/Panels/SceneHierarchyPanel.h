#pragma once

#include <Matcha.h>
#include <DockWidget.h>
#include <vector>

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
class SceneHierarchyWidget;
class CommandManager;

class SceneHierarchyPanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit SceneHierarchyPanel(ads::CDockManager* dockManager, SceneManager& sceneManager, ResourceManager& resourceManager,
                                 Window& window, CommandManager& commandManager, QWidget* parent = nullptr);

    void SelectEntities(const std::vector<Entity>& entities);

signals:
    void SelectionChanged(std::vector<Entity> entities);

private:
    SceneHierarchyWidget* m_TreeWidget;
};
}  // namespace MatchaEditor
