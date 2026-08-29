#include "SceneHierarchyPanel.h"
#include "Widgets/SceneHierarchyWidget.h"

namespace MatchaEditor
{
SceneHierarchyPanel::SceneHierarchyPanel(Matcha::Scene& scene, QWidget* parent)
    : QDockWidget("Scene Hierarchy Panel", parent)
{
    setObjectName("SceneHierarchyPanel");

    m_TreeWidget = new SceneHierarchyWidget(scene, this);

    setWidget(m_TreeWidget);
}
}  // namespace MatchaEditor
