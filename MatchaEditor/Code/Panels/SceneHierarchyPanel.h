#pragma once

#include <Matcha.h>
#include <QDockWidget>
#include <vector>

class Matcha::Scene;

namespace MatchaEditor
{
class SceneHierarchyWidget;

class SceneHierarchyPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit SceneHierarchyPanel(Scene& scene, QWidget* parent = nullptr);

signals:
    void SelectionChanged(std::vector<Entity> entities);

private:
    SceneHierarchyWidget* m_TreeWidget;
};
}  // namespace MatchaEditor
