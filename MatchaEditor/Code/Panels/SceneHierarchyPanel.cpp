#include "SceneHierarchyPanel.h"
#include "Widgets/SceneHierarchyWidget.h"

namespace MatchaEditor
{
SceneHierarchyPanel::SceneHierarchyPanel(EngineContext& context, QWidget* parent)
    : QDockWidget("Scene Hierarchy Panel", parent)
{
    setObjectName("SceneHierarchyPanel");

    m_TreeWidget = new SceneHierarchyWidget(context, this);
    connect(m_TreeWidget, &SceneHierarchyWidget::SelectionChanged, this, &SceneHierarchyPanel::SelectionChanged);

    setWidget(m_TreeWidget);
}
}  // namespace MatchaEditor
