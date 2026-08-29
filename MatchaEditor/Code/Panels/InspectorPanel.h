#pragma once

#include "Scene/Entity.h"

#include <Matcha.h>
#include <QDockWidget>
#include <vector>

class QVBoxLayout;

namespace MatchaEditor
{
class InspectorPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit InspectorPanel(Scene& scene, QWidget* parent = nullptr);

    void SetSelectedEntities(std::vector<Entity> entities);

private:
    void Refresh();
    void OnSceneChanged();

    Scene& m_Scene;
    std::vector<Entity> m_SelectedEntities;
    QWidget* m_ContentWidget;
    QVBoxLayout* m_MainLayout;
};
}  // namespace MatchaEditor