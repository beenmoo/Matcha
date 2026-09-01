#pragma once

#include <DockWidget.h>

namespace Matcha
{
class QtViewportWidget;
}  // namespace Matcha

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
// Thin CDockWidget wrapper around the engine's QtViewportWidget - lets the viewport be moved,
// floated, or resized alongside every other panel instead of being pinned as QMainWindow's fixed
// central widget.
class ViewportPanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit ViewportPanel(ads::CDockManager* dockManager, Matcha::QtViewportWidget* viewport, QWidget* parent = nullptr);
};
}  // namespace MatchaEditor
