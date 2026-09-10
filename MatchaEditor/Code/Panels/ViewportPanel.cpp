#include "ViewportPanel.h"

#include <DockManager.h>

namespace MatchaEditor
{
ViewportPanel::ViewportPanel(ads::CDockManager* dockManager, QWidget* viewportContainer, QWidget* parent)
    : ads::CDockWidget(dockManager, "Viewport Panel", parent)
{
    setObjectName("ViewportPanel");

    setWidget(viewportContainer);
}
}  // namespace MatchaEditor
