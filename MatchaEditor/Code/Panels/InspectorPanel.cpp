#include "InspectorPanel.h"
#include "Widgets/ComponentBoxWidget.h"
#include "Widgets/Vec3ControlWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QScrollArea>

#include <algorithm>

namespace MatchaEditor
{
InspectorPanel::InspectorPanel(Scene& scene, QWidget* parent)
    : QDockWidget("Inspector Panel", parent),
      m_Scene(scene)
{
    setObjectName("InspectorPanel");

    // Use a scroll area because entities can have many components
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    m_ContentWidget = new QWidget();
    m_MainLayout = new QVBoxLayout(m_ContentWidget);
    m_MainLayout->setAlignment(Qt::AlignTop);

    m_ContentWidget->setLayout(m_MainLayout);
    scrollArea->setWidget(m_ContentWidget);
    setWidget(scrollArea);

    m_Scene.AddOnSceneChanged([this] { OnSceneChanged(); });
}
void InspectorPanel::SetSelectedEntities(std::vector<Entity> entities)
{
    m_SelectedEntities = std::move(entities);
    Refresh();
}

void InspectorPanel::OnSceneChanged()
{
    // Not a general "refresh on every scene change" - that would fight in-progress edits in
    // the spin boxes below. This only catches a selected entity having been deleted out from
    // under the panel (e.g. via the Scene Hierarchy panel), pruning it so the panel falls back
    // to "No entity selected" (or keeps editing whatever in the selection is still alive)
    // instead of showing controls for a dead entity.
    size_t countBefore = m_SelectedEntities.size();
    std::erase_if(m_SelectedEntities, [](const Entity& entity) { return !entity.IsValid(); });

    if (m_SelectedEntities.size() != countBefore)
        Refresh();
}

void InspectorPanel::Refresh()
{  // Clear layout items safely...
    QLayoutItem* item;
    while ((item = m_MainLayout->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    if (m_SelectedEntities.empty())
    {
        m_MainLayout->addWidget(new QLabel("No entity selected.", m_ContentWidget));
        return;
    }

    // 1. Transform Component UI (Clean and encapsulated!)
    // Only shown when every selected entity has one - a control here edits all of them at once,
    // which wouldn't make sense (and would crash on GetComponent) for a mixed selection.
    bool allHaveTransform = std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(),
                                         [](Entity entity) { return entity.HasComponent<TransformComponent>(); });

    if (allHaveTransform)
    {
        // Initial values shown come from the first selected entity - controls don't have a
        // "mixed values" display, so a multi-selection with differing values just starts from
        // whatever the first entity has until edited.
        auto& transformComponent = m_SelectedEntities.front().GetComponent<TransformComponent>();

        auto* box = new ComponentBoxWidget("Transform", m_ContentWidget);

        // Captured by value (not references into any entity's TransformComponent, which entt
        // can relocate) and re-fetched fresh each time a control emits ValueChanged. Editing a
        // control applies that value to every selected entity.
        std::vector<Entity> entities = m_SelectedEntities;

        // Add Position control
        auto* posControl = new Vec3ControlWidget("Position", transformComponent.transform.GetPosition());
        connect(posControl, &Vec3ControlWidget::ValueChanged, this,
                [entities](const Vector3& value) mutable
                {
                    for (Entity entity : entities)
                        entity.GetComponent<TransformComponent>().transform.SetPosition(value);
                });
        box->SetContent(posControl);

        // Add Rotation control (edited as Euler angles - the transform stores rotation as a quaternion)
        auto* rotControl = new Vec3ControlWidget("Rotation", transformComponent.transform.GetRotationEuler());
        connect(rotControl, &Vec3ControlWidget::ValueChanged, this,
                [entities](const Vector3& value) mutable
                {
                    for (Entity entity : entities)
                        entity.GetComponent<TransformComponent>().transform.SetRotationEuler(value);
                });
        box->SetContent(rotControl);

        // Add Scale control
        auto* scaleControl = new Vec3ControlWidget("Scale", transformComponent.transform.GetScale());
        connect(scaleControl, &Vec3ControlWidget::ValueChanged, this,
                [entities](const Vector3& value) mutable
                {
                    for (Entity entity : entities)
                        entity.GetComponent<TransformComponent>().transform.SetScale(value);
                });
        box->SetContent(scaleControl);

        m_MainLayout->addWidget(box);
    }
}
}  // namespace MatchaEditor