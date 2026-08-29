#include "InspectorPanel.h"

namespace MatchaEditor
{
InspectorPanel::InspectorPanel(QWidget* parent)
    : QDockWidget("Inspector Panel", parent)
{
    setObjectName("InspectorPanel");
}
}  // namespace MatchaEditor