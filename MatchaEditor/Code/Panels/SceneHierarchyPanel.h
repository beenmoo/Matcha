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

class SceneHierarchyPanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit SceneHierarchyPanel(ads::CDockManager* dockManager, EngineContext& context, QWidget* parent = nullptr);

signals:
    void SelectionChanged(std::vector<Entity> entities);

private:
    SceneHierarchyWidget* m_TreeWidget;
};
}  // namespace MatchaEditor
