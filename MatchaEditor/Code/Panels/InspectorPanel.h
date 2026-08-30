#pragma once

#include "Scene/Entity.h"

#include <Matcha.h>
#include <QDockWidget>
#include <vector>

class QVBoxLayout;

namespace MatchaEditor
{
class ComponentBoxWidget;

class InspectorPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit InspectorPanel(Scene& scene, QWidget* parent = nullptr);

    void SetSelectedEntities(std::vector<Entity> entities);

private:
    void Refresh();
    void OnSceneChanged();

    // Adds one Vec3ControlWidget to `box`, wired to apply edits (via `setter`) to every
    // currently-selected entity's TransformComponent.
    void AddVec3Control(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue,
                        void (Transform::*setter)(const Vector3&));

    Scene& m_Scene;
    std::vector<Entity> m_SelectedEntities;
    QWidget* m_ContentWidget;
    QVBoxLayout* m_MainLayout;
};
}  // namespace MatchaEditor