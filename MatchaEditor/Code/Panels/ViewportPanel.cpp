#include "ViewportPanel.h"
#include "Core/Qt/QtViewportWidget.h"

#include <DockManager.h>

namespace MatchaEditor
{
ViewportPanel::ViewportPanel(ads::CDockManager* dockManager, Matcha::QtViewportWidget* viewport, QWidget* parent)
    : ads::CDockWidget(dockManager, "Viewport Panel", parent)
{
    setObjectName("ViewportPanel");

    setWidget(viewport);
}
}  // namespace MatchaEditor
