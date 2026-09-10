#pragma once

#include <DockWidget.h>

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
// Thin CDockWidget wrapper around the window-container QWidget that embeds MatchaEditor's
// EngineViewportWidget (a QWindow) - lets the viewport be moved, floated, or resized alongside
// every other panel instead of being pinned as QMainWindow's fixed central widget.
class ViewportPanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit ViewportPanel(ads::CDockManager* dockManager, QWidget* viewportContainer, QWidget* parent = nullptr);
};
}  // namespace MatchaEditor
