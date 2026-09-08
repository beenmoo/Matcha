#include "SceneHierarchyPanel.h"
#include "Widgets/SceneHierarchyWidget.h"

#include <DockManager.h>

namespace MatchaEditor
{
SceneHierarchyPanel::SceneHierarchyPanel(ads::CDockManager* dockManager, SceneManager& sceneManager, ResourceManager& resourceManager,
                                         Window& window, CommandManager& commandManager, QWidget* parent)
    : ads::CDockWidget(dockManager, "Scene Hierarchy Panel", parent)
{
    setObjectName("SceneHierarchyPanel");

    m_TreeWidget = new SceneHierarchyWidget(sceneManager, resourceManager, window, commandManager, this);
    connect(m_TreeWidget, &SceneHierarchyWidget::SelectionChanged, this, &SceneHierarchyPanel::SelectionChanged);

    setWidget(m_TreeWidget);
}

void SceneHierarchyPanel::SelectEntities(const std::vector<Entity>& entities)
{
    m_TreeWidget->SelectEntities(entities);
}
}  // namespace MatchaEditor
