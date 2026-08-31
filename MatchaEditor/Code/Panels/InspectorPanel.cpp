#include "InspectorPanel.h"
#include "Widgets/ComponentBoxWidget.h"
#include "Widgets/Vec3ControlWidget.h"
#include "Widgets/Vec4ControlWidget.h"
#include "Widgets/StringFieldWidget.h"
#include "Widgets/BoolFieldWidget.h"
#include "Widgets/FloatFieldWidget.h"
#include "Widgets/EnumFieldWidget.h"
#include "Utility/EntityUtils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QTimer>

#include <algorithm>

namespace MatchaEditor
{
namespace
{
QPushButton* CreateAddButton(const QString& text, QWidget* parent)
{
    QPushButton* button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(
        "QPushButton {"
        "   background-color: #383838;"
        "   color: #dcdcdc;"
        "   border: 1px solid #242424;"
        "   border-radius: 2px;"
        "   font-size: 11px;"
        "   min-height: 22px;"
        "   max-height: 22px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #404040;"
        "}");
    return button;
}
}  // namespace

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

    RegisterComponentInspectors();
    RegisterScripts();

    m_Scene.AddOnSceneChanged([this] { OnSceneChanged(); });

    // Matches the editor's own render-tick cadence (see Editor::m_TickTimer) - frequent enough
    // that a script-driven value (e.g. CameraController moving the camera) visibly updates in
    // real time instead of only refreshing on reselect.
    m_SyncTimer = new QTimer(this);
    connect(m_SyncTimer, &QTimer::timeout, this, &InspectorPanel::SyncLiveValues);
    m_SyncTimer->start(16);
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

void InspectorPanel::SyncLiveValues()
{
    if (m_SelectedEntities.empty())
        return;

    for (const auto& sync : m_LiveSyncCallbacks)
        sync();
}

void InspectorPanel::Refresh()
{
    m_LiveSyncCallbacks.clear();

    // Clear layout items safely...
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

    // Initial values shown in every field come from the first selected entity - controls don't
    // have a "mixed values" display, so a multi-selection with differing values just starts from
    // whatever the first entity has until edited.
    for (const ComponentInspectorEntry& entry : m_ComponentInspectors)
    {
        if (!entry.allHave())
            continue;

        ComponentBoxWidget* box = CreateComponentBox(entry.name);
        entry.draw(box);
        m_MainLayout->addWidget(box);
    }

    QPushButton* addButton = CreateAddButton("+ Add Component", m_ContentWidget);
    connect(addButton, &QPushButton::clicked, this, [this, addButton] { ShowAddComponentMenu(addButton); });
    m_MainLayout->addWidget(addButton);
}

void InspectorPanel::RegisterComponentInspectors()
{
    RegisterComponentInspector<TagComponent>("Entity Properties", false, [this](ComponentBoxWidget* box) {
        TagComponent& tagComponent = m_SelectedEntities.front().GetComponent<TagComponent>();
        AddBoolField<TagComponent>(box, "Active", tagComponent.isActive, &TagComponent::isActive);
        AddStringField<TagComponent>(box, "Name", QString::fromStdString(tagComponent.name), &TagComponent::name);
    });

    RegisterComponentInspector<TransformComponent>("Transform", false, [this](ComponentBoxWidget* box) {
        TransformComponent& transformComponent = m_SelectedEntities.front().GetComponent<TransformComponent>();
        AddVec3Field(
            box, "Position", transformComponent.transform.GetPosition(),
            [this] { return m_SelectedEntities.front().GetComponent<TransformComponent>().transform.GetPosition(); },
            &Transform::SetPosition);
        // Edited as Euler angles - the transform stores rotation as a quaternion.
        AddVec3Field(
            box, "Rotation", transformComponent.transform.GetRotationEuler(),
            [this] { return m_SelectedEntities.front().GetComponent<TransformComponent>().transform.GetRotationEuler(); },
            &Transform::SetRotationEuler);
        AddVec3Field(
            box, "Scale", transformComponent.transform.GetScale(),
            [this] { return m_SelectedEntities.front().GetComponent<TransformComponent>().transform.GetScale(); },
            &Transform::SetScale);
    });

    RegisterComponentInspector<LightComponent>("Light", true, [this](ComponentBoxWidget* box) {
        LightComponent& light = m_SelectedEntities.front().GetComponent<LightComponent>();
        AddEnumField<LightComponent>(box, "Type", {"Directional", "Point", "Spot"}, light.type, &LightComponent::type);
        AddVec3Field<LightComponent>(box, "Color", light.color, &LightComponent::color);
        AddFloatField<LightComponent>(box, "Intensity", light.intensity, &LightComponent::intensity);
        AddFloatField<LightComponent>(box, "Range", light.range, &LightComponent::range);
        AddFloatField<LightComponent>(box, "Inner Cone", light.innerConeAngle, &LightComponent::innerConeAngle);
        AddFloatField<LightComponent>(box, "Outer Cone", light.outerConeAngle, &LightComponent::outerConeAngle);
        AddFloatField<LightComponent>(box, "Ambient Str.", light.ambientStrength, &LightComponent::ambientStrength);
        AddVec3Field<LightComponent>(box, "Ambient Color", light.ambientColor, &LightComponent::ambientColor);
        AddBoolField<LightComponent>(box, "Cast Shadows", light.castShadows, &LightComponent::castShadows);
    });

    RegisterComponentInspector<CameraComponent>("Camera", true, [this](ComponentBoxWidget* box) {
        CameraComponent& camera = m_SelectedEntities.front().GetComponent<CameraComponent>();
        AddEnumField<CameraComponent>(box, "Projection", {"Perspective", "Orthographic"}, camera.projectionType,
                                      &CameraComponent::projectionType);
        AddFloatField<CameraComponent>(box, "FOV", camera.perspectiveFOV, &CameraComponent::perspectiveFOV);
        AddFloatField<CameraComponent>(box, "Near", camera.perspectiveNear, &CameraComponent::perspectiveNear);
        AddFloatField<CameraComponent>(box, "Far", camera.perspectiveFar, &CameraComponent::perspectiveFar);
        AddFloatField<CameraComponent>(box, "Ortho Size", camera.orthographicSize, &CameraComponent::orthographicSize);
        AddFloatField<CameraComponent>(box, "Ortho Near", camera.orthographicNear, &CameraComponent::orthographicNear);
        AddFloatField<CameraComponent>(box, "Ortho Far", camera.orthographicFar, &CameraComponent::orthographicFar);
        AddBoolField<CameraComponent>(box, "Primary", camera.primary, &CameraComponent::primary);
        AddBoolField<CameraComponent>(box, "Fixed Aspect", camera.fixedAspectRatio, &CameraComponent::fixedAspectRatio);
    });

    RegisterComponentInspector<MaterialComponent>("Material", true, [this](ComponentBoxWidget* box) {
        MaterialComponent& material = m_SelectedEntities.front().GetComponent<MaterialComponent>();
        AddVec4Field<MaterialComponent>(box, "Albedo", material.albedoColor, &MaterialComponent::albedoColor);
        AddFloatField<MaterialComponent>(box, "Specular", material.specularStrength, &MaterialComponent::specularStrength);
        AddFloatField<MaterialComponent>(box, "Shininess", material.shininess, &MaterialComponent::shininess);
        // Shader/texture are resource handles - no asset picker UI exists yet, so they aren't editable here.
    });

    RegisterComponentInspector<MeshComponent>("Mesh", true, [this](ComponentBoxWidget* box) {
        MeshComponent& meshComponent = m_SelectedEntities.front().GetComponent<MeshComponent>();
        // No asset picker UI exists yet, so the mesh handle is shown read-only rather than editable.
        QString info = meshComponent.mesh.IsValid() ? QString("Handle #%1").arg(meshComponent.mesh.GetID()) : "None";
        QLabel* label = new QLabel(info, box);
        label->setStyleSheet("color: #b0b0b0; font-size: 11px; padding: 2px;");
        box->SetContent(label);
    });

    RegisterComponentInspector<NativeScriptComponent>("Native Script", true, [this](ComponentBoxWidget* box) {
        NativeScriptComponent& script = m_SelectedEntities.front().GetComponent<NativeScriptComponent>();

        QLabel* countLabel = new QLabel(QString("%1 script(s) bound").arg(script.bindings.size()), box);
        countLabel->setStyleSheet("color: #b0b0b0; font-size: 11px; padding: 2px;");
        box->SetContent(countLabel);

        QPushButton* addScriptButton = CreateAddButton("+ Add Script", box);
        connect(addScriptButton, &QPushButton::clicked, this, [this, addScriptButton] { ShowAddScriptMenu(addScriptButton); });
        box->SetContent(addScriptButton);
    });
}

template <typename Component>
void InspectorPanel::RegisterComponentInspector(const std::string& name, bool addable,
                                                std::function<void(ComponentBoxWidget*)> draw)
{
    ComponentInspectorEntry entry;
    entry.name = name;
    entry.addable = addable;
    entry.allHave = [this] { return AllEntitiesHaveComponent<Component>(m_SelectedEntities); };
    entry.draw = std::move(draw);
    entry.addToSelection = [this] {
        for (Entity entity : m_SelectedEntities)
            if (!entity.HasComponent<Component>())
                entity.AddComponent<Component>();
        Refresh();
    };
    m_ComponentInspectors.push_back(std::move(entry));
}

void InspectorPanel::ShowAddComponentMenu(QPushButton* anchor)
{
    QMenu menu(this);
    bool anyAddable = false;

    for (ComponentInspectorEntry& entry : m_ComponentInspectors)
    {
        if (!entry.addable || entry.allHave())
            continue;

        anyAddable = true;
        QAction* action = menu.addAction(QString::fromStdString(entry.name));
        connect(action, &QAction::triggered, this, [&entry] { entry.addToSelection(); });
    }

    if (!anyAddable)
        menu.addAction("All components added")->setEnabled(false);

    menu.exec(anchor->mapToGlobal(QPoint(0, anchor->height())));
}

void InspectorPanel::RegisterScripts()
{
    RegisterScript<CameraController>("Camera Controller");
}

template <typename Script>
void InspectorPanel::RegisterScript(const std::string& name)
{
    ScriptInspectorEntry entry;
    entry.name = name;
    entry.bind = [](Entity entity) { entity.GetComponent<NativeScriptComponent>().Bind<Script>(); };
    m_ScriptInspectors.push_back(std::move(entry));
}

void InspectorPanel::ShowAddScriptMenu(QPushButton* anchor)
{
    // Every selected entity already has a NativeScriptComponent here - this box only draws when
    // entry.allHave() (NativeScriptComponent) is true for the whole selection.
    std::vector<Entity> entities = m_SelectedEntities;

    QMenu menu(this);

    for (ScriptInspectorEntry& entry : m_ScriptInspectors)
    {
        QAction* action = menu.addAction(QString::fromStdString(entry.name));
        connect(action, &QAction::triggered, this, [this, entities, &entry] {
            for (Entity entity : entities)
                entry.bind(entity);
            Refresh();
        });
    }

    menu.exec(anchor->mapToGlobal(QPoint(0, anchor->height())));
}

void InspectorPanel::AddVec3Field(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue,
                                  std::function<Vector3()> getter, void (Transform::*setter)(const Vector3&))
{
    // Captured by value (not a reference into any entity's TransformComponent, which entt can
    // relocate) and re-fetched fresh each time the control emits ValueChanged. Editing it applies
    // the new value to every selected entity.
    std::vector<Entity> entities = m_SelectedEntities;

    Vec3ControlWidget* control = new Vec3ControlWidget(label, initialValue);
    connect(control, &Vec3ControlWidget::ValueChanged, this,
            [entities, setter](const Vector3& value) mutable {
                for (Entity entity : entities)
                    (entity.GetComponent<TransformComponent>().transform.*setter)(value);
            });
    box->SetContent(control);

    m_LiveSyncCallbacks.push_back([control, getter] { control->SetValue(getter()); });
}

template <typename Component>
void InspectorPanel::AddStringField(ComponentBoxWidget* box, const QString& label, const QString& initialValue,
                                    std::string Component::*member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    StringFieldWidget* field = new StringFieldWidget(label, initialValue);
    connect(field, &StringFieldWidget::ValueChanged, this, [this, entities, member](const QString& value) {
        std::string stdValue = value.toStdString();
        for (Entity entity : entities)
            entity.GetComponent<Component>().*member = stdValue;
        m_Scene.NotifyChanged();
    });
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, member] {
        field->SetValue(QString::fromStdString(m_SelectedEntities.front().GetComponent<Component>().*member));
    });
}

template <typename Component>
void InspectorPanel::AddBoolField(ComponentBoxWidget* box, const QString& label, bool initialValue,
                                  bool Component::*member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    BoolFieldWidget* field = new BoolFieldWidget(label, initialValue);
    connect(field, &BoolFieldWidget::ValueChanged, this, [this, entities, member](bool value) {
        for (Entity entity : entities)
            entity.GetComponent<Component>().*member = value;
        m_Scene.NotifyChanged();
    });
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, member] { field->SetValue(m_SelectedEntities.front().GetComponent<Component>().*member); });
}

template <typename Component>
void InspectorPanel::AddFloatField(ComponentBoxWidget* box, const QString& label, float initialValue,
                                   float Component::*member)
{
    // Unlike AddStringField/AddBoolField above (TagComponent's name/active, which the Scene
    // Hierarchy tree displays), none of these numeric fields are reflected anywhere outside the
    // Inspector, so editing them doesn't call m_Scene.NotifyChanged() - same reasoning as
    // AddVec3Field's TransformComponent fields.
    std::vector<Entity> entities = m_SelectedEntities;

    FloatFieldWidget* field = new FloatFieldWidget(label, initialValue);
    connect(field, &FloatFieldWidget::ValueChanged, this, [entities, member](float value) {
        for (Entity entity : entities)
            entity.GetComponent<Component>().*member = value;
    });
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, member] { field->SetValue(m_SelectedEntities.front().GetComponent<Component>().*member); });
}

template <typename Component>
void InspectorPanel::AddVec3Field(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue,
                                  Vector3 Component::*member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    Vec3ControlWidget* control = new Vec3ControlWidget(label, initialValue);
    connect(control, &Vec3ControlWidget::ValueChanged, this, [entities, member](const Vector3& value) {
        for (Entity entity : entities)
            entity.GetComponent<Component>().*member = value;
    });
    box->SetContent(control);

    m_LiveSyncCallbacks.push_back([this, control, member] { control->SetValue(m_SelectedEntities.front().GetComponent<Component>().*member); });
}

template <typename Component>
void InspectorPanel::AddVec4Field(ComponentBoxWidget* box, const QString& label, const Vector4& initialValue,
                                  Vector4 Component::*member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    Vec4ControlWidget* control = new Vec4ControlWidget(label, initialValue);
    connect(control, &Vec4ControlWidget::ValueChanged, this, [entities, member](const Vector4& value) {
        for (Entity entity : entities)
            entity.GetComponent<Component>().*member = value;
    });
    box->SetContent(control);

    m_LiveSyncCallbacks.push_back([this, control, member] { control->SetValue(m_SelectedEntities.front().GetComponent<Component>().*member); });
}

template <typename Component, typename Enum>
void InspectorPanel::AddEnumField(ComponentBoxWidget* box, const QString& label, const QStringList& options,
                                  Enum initialValue, Enum Component::*member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    EnumFieldWidget* field = new EnumFieldWidget(label, options, static_cast<int>(initialValue));
    connect(field, &EnumFieldWidget::ValueChanged, this, [entities, member](int index) {
        Enum value = static_cast<Enum>(index);
        for (Entity entity : entities)
            entity.GetComponent<Component>().*member = value;
    });
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back(
        [this, field, member] { field->SetValue(static_cast<int>(m_SelectedEntities.front().GetComponent<Component>().*member)); });
}

ComponentBoxWidget* InspectorPanel::CreateComponentBox(const std::string& name)
{
    bool isCollapsed = m_ComponentCollapseStates[name];

    ComponentBoxWidget* box = new ComponentBoxWidget(QString::fromStdString(name), isCollapsed, m_ContentWidget);
    connect(box, &ComponentBoxWidget::CollapseStateChanged, this, [this, name](bool collapsed) {
        m_ComponentCollapseStates[name] = collapsed;
    });

    return box;
}
}  // namespace MatchaEditor
