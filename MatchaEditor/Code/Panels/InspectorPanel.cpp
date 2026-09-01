#include "InspectorPanel.h"
#include "Widgets/ComponentBoxWidget.h"
#include "Widgets/Vec3ControlWidget.h"
#include "Widgets/Vec4ControlWidget.h"
#include "Widgets/StringFieldWidget.h"
#include "Widgets/BoolFieldWidget.h"
#include "Widgets/FloatFieldWidget.h"
#include "Widgets/EnumFieldWidget.h"
#include "Utility/EntityUtils.h"
#include "Core/CommandManager.h"
#include "Core/Commands/PropertyEditCommand.h"
#include "Scene/Component/TagComponent.h"

#include <DockManager.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QTimer>

#include <algorithm>
#include <memory>

namespace MatchaEditor
{
namespace
{
QPushButton* CreateAddButton(const QString& text, QWidget* parent)
{
    QPushButton* button = new QPushButton(text, parent);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

// Thin horizontal rule for splitting a component box's fields into visually distinct groups
// (e.g. CameraComponent's Perspective-only vs Orthographic-only fields, both always present on
// the struct regardless of which projectionType is active). Fusion's own QFrame::HLine painting
// is enough on its own - no stylesheet rule needed.
QFrame* CreateSeparator(QWidget* parent)
{
    QFrame* separator = new QFrame(parent);
    separator->setFrameShape(QFrame::HLine);
    return separator;
}

// Names the group of fields that follow it (e.g. "Perspective" above FOV/Near/Far), for a
// component box like CameraComponent's whose fields aren't self-explanatory as a flat list -
// distinct from a normal field's own label, which names one value rather than a group of them.
// Bold weight (Editor.qss's QLabel#SectionLabel rule) is the semantic hierarchy cue vs a plain
// field label.
QLabel* CreateSectionLabel(const QString& text, QWidget* parent)
{
    QLabel* label = new QLabel(text, parent);
    label->setObjectName("SectionLabel");
    return label;
}
}  // namespace

InspectorPanel::InspectorPanel(ads::CDockManager* dockManager, EngineContext& context, CommandManager& commandManager,
                               QWidget* parent)
    : ads::CDockWidget(dockManager, "Inspector Panel", parent),
      m_Context(context),
      m_CommandManager(commandManager)
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

    m_Context.GetSceneManager().AddOnSceneReplaced([this] { OnSceneReplaced(); });
    BindScene();

    // Matches the editor's own render-tick cadence (see Editor::m_TickTimer) - frequent enough
    // that a script-driven value (e.g. CameraController moving the camera) visibly updates in
    // real time instead of only refreshing on reselect.
    m_SyncTimer = new QTimer(this);
    connect(m_SyncTimer, &QTimer::timeout, this, &InspectorPanel::SyncLiveValues);
    m_SyncTimer->start(16);
}

void InspectorPanel::BindScene()
{
    m_Context.GetScene().AddOnSceneChanged([this] { OnSceneChanged(); });
}

void InspectorPanel::OnSceneReplaced()
{
    m_SelectedEntities.clear();
    BindScene();
    Refresh();
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
            [](Entity entity) { return entity.GetComponent<TransformComponent>().transform.GetPosition(); },
            &Transform::SetPosition);
        // Edited as Euler angles - the transform stores rotation as a quaternion.
        AddVec3Field(
            box, "Rotation", transformComponent.transform.GetRotationEuler(),
            [](Entity entity) { return entity.GetComponent<TransformComponent>().transform.GetRotationEuler(); },
            &Transform::SetRotationEuler);
        AddVec3Field(
            box, "Scale", transformComponent.transform.GetScale(),
            [](Entity entity) { return entity.GetComponent<TransformComponent>().transform.GetScale(); },
            &Transform::SetScale);
    });

    RegisterComponentInspector<LightComponent>("Light", true, [this](ComponentBoxWidget* box) {
        LightComponent& light = m_SelectedEntities.front().GetComponent<LightComponent>();
        AddEnumField<LightComponent>(box, "Type", {"Directional", "Point", "Spot"}, light.type, &LightComponent::type);
        box->SetContent(CreateSectionLabel("General", box));
        AddVec3Field<LightComponent>(box, "Color", light.color, &LightComponent::color);
        AddFloatField<LightComponent>(box, "Intensity", light.intensity, &LightComponent::intensity);
        box->SetContent(CreateSeparator(box));
        box->SetContent(CreateSectionLabel("Point / Spot", box));
        AddFloatField<LightComponent>(box, "Range", light.range, &LightComponent::range);
        box->SetContent(CreateSeparator(box));
        box->SetContent(CreateSectionLabel("Spot Only", box));
        AddFloatField<LightComponent>(box, "Inner Cone", light.innerConeAngle, &LightComponent::innerConeAngle);
        AddFloatField<LightComponent>(box, "Outer Cone", light.outerConeAngle, &LightComponent::outerConeAngle);
        box->SetContent(CreateSeparator(box));
        box->SetContent(CreateSectionLabel("Ambient", box));
        AddFloatField<LightComponent>(box, "Ambient Strength", light.ambientStrength, &LightComponent::ambientStrength);
        AddVec3Field<LightComponent>(box, "Ambient Color", light.ambientColor, &LightComponent::ambientColor);
        AddBoolField<LightComponent>(box, "Cast Shadows", light.castShadows, &LightComponent::castShadows);
    });

    RegisterComponentInspector<CameraComponent>("Camera", true, [this](ComponentBoxWidget* box) {
        CameraComponent& camera = m_SelectedEntities.front().GetComponent<CameraComponent>();
        AddEnumField<CameraComponent>(box, "Projection", {"Perspective", "Orthographic"}, camera.projectionType,
                                      &CameraComponent::projectionType);
        box->SetContent(CreateSectionLabel("Perspective", box));
        AddFloatField<CameraComponent>(box, "FOV", camera.perspectiveFOV, &CameraComponent::perspectiveFOV);
        AddFloatField<CameraComponent>(box, "Near", camera.perspectiveNear, &CameraComponent::perspectiveNear);
        AddFloatField<CameraComponent>(box, "Far", camera.perspectiveFar, &CameraComponent::perspectiveFar);
        box->SetContent(CreateSeparator(box));
        box->SetContent(CreateSectionLabel("Orthographic", box));
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
        label->setObjectName("ReadOnlyInfoLabel");
        box->SetContent(label);
    });

    RegisterComponentInspector<NativeScriptComponent>("Native Script", true, [this](ComponentBoxWidget* box) {
        NativeScriptComponent& script = m_SelectedEntities.front().GetComponent<NativeScriptComponent>();

        QLabel* countLabel = new QLabel(QString("%1 script(s) bound").arg(script.bindings.size()), box);
        countLabel->setObjectName("ReadOnlyInfoLabel");
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

template <typename ValueType>
std::function<void()> InspectorPanel::MakeCommitHandler(const QString& description, std::vector<Entity> entities,
                                                         std::function<ValueType(Entity)> getter,
                                                         std::function<void(Entity, ValueType)> setter)
{
    std::vector<typename PropertyEditCommand<ValueType>::Edit> edits;
    edits.reserve(entities.size());
    for (Entity entity : entities)
        edits.push_back({entity.GetComponent<TagComponent>().id, getter(entity)});

    // Mutable: `edits` is rebased to the newly-committed value after every push, so a second
    // commit on the same field (without an intervening Refresh(), which would rebuild this
    // closure from scratch) captures the right "before" for *that* edit, not the field's
    // original value from when it was first built.
    return [this, description, entities, getter, setter, edits]() mutable {
        if (entities.empty())
            return;

        ValueType current = getter(entities.front());
        if (!edits.empty() && current == edits.front().before)
            return;  // nothing was actually live-applied since this field was built/last committed

        m_CommandManager.ExecuteCommand(std::make_unique<PropertyEditCommand<ValueType>>(
            m_Context, description.toStdString(), edits, current, setter));

        for (auto& edit : edits)
            edit.before = current;
    };
}

void InspectorPanel::AddVec3Field(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue,
                                  std::function<Vector3(Entity)> getter, void (Transform::*setter)(const Vector3&))
{
    // Captured by value (not a reference into any entity's TransformComponent, which entt can
    // relocate) and re-fetched fresh each time the control emits ValueChanged. Editing it applies
    // the new value to every selected entity.
    std::vector<Entity> entities = m_SelectedEntities;

    auto applyToTransform = [setter](Entity entity, Vector3 value) {
        (entity.GetComponent<TransformComponent>().transform.*setter)(value);
    };

    Vec3ControlWidget* control = new Vec3ControlWidget(label, initialValue);
    connect(control, &Vec3ControlWidget::ValueChanged, this, [entities, applyToTransform](const Vector3& value) {
        for (Entity entity : entities)
            applyToTransform(entity, value);
    });
    connect(control, &Vec3ControlWidget::EditingFinished, this,
            MakeCommitHandler<Vector3>(label, entities, getter, applyToTransform));
    box->SetContent(control);

    m_LiveSyncCallbacks.push_back([this, control, getter] { control->SetValue(getter(m_SelectedEntities.front())); });
}

template <typename Component>
void InspectorPanel::AddStringField(ComponentBoxWidget* box, const QString& label, const QString& initialValue,
                                    std::string Component::* member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    auto getter = [member](Entity entity) { return QString::fromStdString(entity.GetComponent<Component>().*member); };
    auto setter = [member](Entity entity, QString value) { entity.GetComponent<Component>().*member = value.toStdString(); };

    StringFieldWidget* field = new StringFieldWidget(label, initialValue);
    // StringFieldWidget::ValueChanged already only fires once per commit (QLineEdit::
    // editingFinished), unlike the live-tick Vec3/Vec4/Float widgets - so the same signal both
    // applies the value and (connected second, so it observes the already-applied value) commits
    // the undo entry, rather than needing a separate EditingFinished signal.
    connect(field, &StringFieldWidget::ValueChanged, this, [entities, setter](const QString& value) {
        for (Entity entity : entities)
            setter(entity, value);
    });
    connect(field, &StringFieldWidget::ValueChanged, this, MakeCommitHandler<QString>(label, entities, getter, setter));
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, getter] { field->SetValue(getter(m_SelectedEntities.front())); });
}

template <typename Component>
void InspectorPanel::AddBoolField(ComponentBoxWidget* box, const QString& label, bool initialValue,
                                  bool Component::* member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    auto getter = [member](Entity entity) { return entity.GetComponent<Component>().*member; };
    auto setter = [member](Entity entity, bool value) { entity.GetComponent<Component>().*member = value; };

    BoolFieldWidget* field = new BoolFieldWidget(label, initialValue);
    // BoolFieldWidget::ValueChanged already only fires once per click (QCheckBox::toggled) - same
    // reasoning as AddStringField above.
    connect(field, &BoolFieldWidget::ValueChanged, this, [entities, setter](bool value) {
        for (Entity entity : entities)
            setter(entity, value);
    });
    connect(field, &BoolFieldWidget::ValueChanged, this, MakeCommitHandler<bool>(label, entities, getter, setter));
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, getter] { field->SetValue(getter(m_SelectedEntities.front())); });
}

template <typename Component>
void InspectorPanel::AddFloatField(ComponentBoxWidget* box, const QString& label, float initialValue,
                                   float Component::* member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    auto getter = [member](Entity entity) { return entity.GetComponent<Component>().*member; };
    // Every field commits through PropertyEditCommand, which calls Scene::NotifyChanged() itself
    // on Execute()/Undo() - see PropertyEditCommand.h. Unlike the old direct-write path, this
    // fixes numeric fields (which previously never notified) to correctly dirty the scene.
    auto setter = [member](Entity entity, float value) { entity.GetComponent<Component>().*member = value; };

    FloatFieldWidget* field = new FloatFieldWidget(label, initialValue);
    connect(field, &FloatFieldWidget::ValueChanged, this, [entities, setter](float value) {
        for (Entity entity : entities)
            setter(entity, value);
    });
    connect(field, &FloatFieldWidget::EditingFinished, this, MakeCommitHandler<float>(label, entities, getter, setter));
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, getter] { field->SetValue(getter(m_SelectedEntities.front())); });
}

template <typename Component>
void InspectorPanel::AddVec3Field(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue,
                                  Vector3 Component::* member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    auto getter = [member](Entity entity) { return entity.GetComponent<Component>().*member; };
    auto setter = [member](Entity entity, Vector3 value) { entity.GetComponent<Component>().*member = value; };

    Vec3ControlWidget* control = new Vec3ControlWidget(label, initialValue);
    connect(control, &Vec3ControlWidget::ValueChanged, this, [entities, setter](const Vector3& value) {
        for (Entity entity : entities)
            setter(entity, value);
    });
    connect(control, &Vec3ControlWidget::EditingFinished, this, MakeCommitHandler<Vector3>(label, entities, getter, setter));
    box->SetContent(control);

    m_LiveSyncCallbacks.push_back([this, control, getter] { control->SetValue(getter(m_SelectedEntities.front())); });
}

template <typename Component>
void InspectorPanel::AddVec4Field(ComponentBoxWidget* box, const QString& label, const Vector4& initialValue,
                                  Vector4 Component::* member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    auto getter = [member](Entity entity) { return entity.GetComponent<Component>().*member; };
    auto setter = [member](Entity entity, Vector4 value) { entity.GetComponent<Component>().*member = value; };

    Vec4ControlWidget* control = new Vec4ControlWidget(label, initialValue);
    connect(control, &Vec4ControlWidget::ValueChanged, this, [entities, setter](const Vector4& value) {
        for (Entity entity : entities)
            setter(entity, value);
    });
    connect(control, &Vec4ControlWidget::EditingFinished, this, MakeCommitHandler<Vector4>(label, entities, getter, setter));
    box->SetContent(control);

    m_LiveSyncCallbacks.push_back([this, control, getter] { control->SetValue(getter(m_SelectedEntities.front())); });
}

template <typename Component, typename Enum>
void InspectorPanel::AddEnumField(ComponentBoxWidget* box, const QString& label, const QStringList& options,
                                  Enum initialValue, Enum Component::* member)
{
    std::vector<Entity> entities = m_SelectedEntities;

    auto getter = [member](Entity entity) { return static_cast<int>(entity.GetComponent<Component>().*member); };
    auto setter = [member](Entity entity, int value) { entity.GetComponent<Component>().*member = static_cast<Enum>(value); };

    EnumFieldWidget* field = new EnumFieldWidget(label, options, static_cast<int>(initialValue));
    // EnumFieldWidget::ValueChanged already only fires once per selection (QComboBox::
    // currentIndexChanged) - same reasoning as AddStringField above.
    connect(field, &EnumFieldWidget::ValueChanged, this, [entities, setter](int value) {
        for (Entity entity : entities)
            setter(entity, value);
    });
    connect(field, &EnumFieldWidget::ValueChanged, this, MakeCommitHandler<int>(label, entities, getter, setter));
    box->SetContent(field);

    m_LiveSyncCallbacks.push_back([this, field, getter] { field->SetValue(getter(m_SelectedEntities.front())); });
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
