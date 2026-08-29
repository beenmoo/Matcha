#pragma once

#include <QDockWidget>

namespace Matcha
{
class Scene;
}  // namespace Matcha

namespace MatchaEditor
{
class SceneHierarchyWidget;

class SceneHierarchyPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit SceneHierarchyPanel(Matcha::Scene& scene, QWidget* parent = nullptr);
    ~SceneHierarchyPanel() = default;

private:
    SceneHierarchyWidget* m_TreeWidget;
};
}  // namespace MatchaEditor
