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
#include "Core/Commands/AddComponentCommand.h"
#include "Core/Commands/PropertyEditCommand.h"
#include "Core/Commands/RemoveComponentCommand.h"
#include "Scene/Component/PythonScriptComponent.h"
#include "Scene/Component/TagComponent.h"

#include <Scripting/PythonRuntime.h>

#include <DockManager.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
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

// Best-effort default for the Class field after picking a .py file - every script Sandbox ships
// follows "snake_case_file.py" -> "PascalCaseClass" (rotation_component.py -> RotationComponent),
// so this guesses that and leaves it for the user to correct via the Class field if their script
// doesn't follow it (e.g. a file with multiple classes, or a different naming convention).
QString GuessClassNameFromFileStem(const QString& stem)
{
    QString result;
    bool capitalizeNext = true;

    for (QChar c : stem)
    {
        if (c == '_')
        {
            capitalizeNext = true;
            continue;
        }

        result += capitalizeNext ? c.toUpper() : c;
        capitalizeNext = false;
    }

    return result;
}

// A script's live Python instance is untrusted input the same way a scene file is - reading or
// writing one of its attributes can throw (a custom __setattr__, an attribute deleted since the
// field widget was built) at a point this file has no other exception boundary around (a Qt
// signal handler triggered from user interaction or the sync timer). Mirrors
// PythonScriptSystem::Update's own py::error_already_set boundary.
template <typename Func>
void SafeCall(Func&& func)
{
    try
    {
        func();
    }
    catch (const py::error_already_set& e)
    {
        MT_CORE_ERROR("Python script field access failed: {}", e.what());
    }
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

InspectorPanel::InspectorPanel(ads::CDockManager* dockManager, SceneManager& sceneManager, ResourceManager& resourceManager,
                               PythonRuntime& pythonRuntime, CommandManager& commandManager, QWidget* parent)
    : ads::CDockWidget(dockManager, "Inspector Panel", parent),
      m_SceneManager(sceneManager),
      m_ResourceManager(resourceManager),
      m_PythonRuntime(pythonRuntime),
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

    m_SceneManager.AddOnSceneReplaced([this] { OnSceneReplaced(); });
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
    m_SceneManager.GetScene().AddOnSceneChanged([this] { OnSceneChanged(); });
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
    // Deliberately not a blanket "refresh on every scene change" - that would fight in-progress
    // edits, destroying and rebuilding the very widget the user is typing in on every keystroke
    // that dirties the scene. Only two kinds of change actually invalidate what's on screen:

    // A selected entity was deleted out from under the panel (e.g. via the Scene Hierarchy
    // panel) - prune it so the panel falls back to "No entity selected" (or keeps editing
    // whatever in the selection is still alive) instead of showing controls for a dead entity.
    size_t countBefore = m_SelectedEntities.size();
    std::erase_if(m_SelectedEntities, [](const Entity& entity) { return !entity.IsValid(); });

    if (m_SelectedEntities.size() != countBefore)
    {
        Refresh();
        return;
    }

    // The selection is unchanged but its *components* aren't - a box appeared or disappeared, or
    // a Python binding was added/removed. Undo/redo is the case that matters here: unlike the Add
    // Component menu and the (x) button, which drive this panel directly, an undo comes from the
    // Edit menu or Ctrl+Z with no idea this panel exists, so before this the panel just kept
    // showing boxes for components that were no longer there (and vice versa) until the user
    // reselected the entity. Individual field *values* need no rebuild - SyncLiveValues() already
    // pushes those into their widgets every tick, which is why this compares only structure.
    if (ComputeLayoutSignature() != m_LayoutSignature)
        Refresh();
}

std::string InspectorPanel::ComputeLayoutSignature() const
{
    std::string signature;

    for (const ComponentInspectorEntry& entry : m_ComponentInspectors)
    {
        if (!entry.allHave())
            continue;

        signature += entry.name;
        signature += ';';
    }

    // PythonScriptComponent is the one component whose box contents vary with scene data rather
    // than just with the component's presence: one Module/Class field pair (plus that binding's
    // auto-generated script fields) per binding. Its draw lambda reads only the front selected
    // entity, so that's all this needs to fingerprint.
    //
    // Deliberately the binding *count* and not each binding's module/class text: those are what
    // this box's own Module/Class fields edit, and folding them in here would mean every commit in
    // one of those fields rebuilt the box out from under the user mid-edit. Nothing changes a
    // binding's text without also changing the count on any undoable path anyway (the Module/Class
    // fields don't push commands), so the count catches every case that matters - a binding added
    // via Browse..., and a whole component's worth of bindings coming and going through Add/
    // RemoveComponentCommand.
    if (!m_SelectedEntities.empty() && m_SelectedEntities.front().HasComponent<PythonScriptComponent>())
    {
        signature += "bindings=";
        signature += std::to_string(m_SelectedEntities.front().GetComponent<PythonScriptComponent>().bindings.size());
        signature += ';';
    }

    return signature;
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
    // Recorded before anything is built (m_SelectedEntities is already final by here) so
    // OnSceneChanged has an accurate picture of what ended up on screen, including the
    // "No entity selected" case below.
    m_LayoutSignature = ComputeLayoutSignature();
    m_LiveSyncCallbacks.clear();

    // deleteLater() rather than a plain delete: Refresh() is routinely reached from inside one of
    // these very widgets' signal handlers - the (x) button's RemoveRequested, a Browse... click -
    // by way of the command it runs calling Scene::NotifyChanged(), and destroying a widget while
    // it's still emitting leaves Qt returning into freed memory. Reparenting to nullptr first (and
    // hiding, so an unparented widget doesn't flash as a top-level window) takes them off screen
    // immediately, with the actual destruction deferred to the next event-loop turn, once the
    // emitting widget's own call stack has unwound.
    QLayoutItem* item;
    while ((item = m_MainLayout->takeAt(0)) != nullptr)
    {
        if (QWidget* widget = item->widget())
        {
            widget->hide();
            widget->setParent(nullptr);
            widget->deleteLater();
        }

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

        ComponentBoxWidget* box = CreateComponentBox(entry.name, entry.addable);
        if (entry.addable)
            connect(box, &ComponentBoxWidget::RemoveRequested, this, entry.removeFromSelection);
        entry.draw(box);
        m_MainLayout->addWidget(box);
    }

    QPushButton* addButton = CreateAddButton("+ Add Component", m_ContentWidget);
    connect(addButton, &QPushButton::clicked, this, [this, addButton] { ShowAddComponentMenu(addButton); });
    m_MainLayout->addWidget(addButton);
}

void InspectorPanel::RegisterComponentInspectors()
{
    RegisterComponentInspector<TagComponent>("Entity Properties", "", false, [this](ComponentBoxWidget* box) {
        TagComponent& tagComponent = m_SelectedEntities.front().GetComponent<TagComponent>();
        AddBoolField<TagComponent>(box, "Active", tagComponent.isActive, &TagComponent::isActive);
        AddStringField<TagComponent>(box, "Name", QString::fromStdString(tagComponent.name), &TagComponent::name);
    });

    RegisterComponentInspector<TransformComponent>("Transform", "", false, [this](ComponentBoxWidget* box) {
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

    RegisterComponentInspector<LightComponent>("Light", "light", true, [this](ComponentBoxWidget* box) {
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

    RegisterComponentInspector<CameraComponent>("Camera", "camera", true, [this](ComponentBoxWidget* box) {
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

    RegisterComponentInspector<MaterialComponent>("Material", "material", true, [this](ComponentBoxWidget* box) {
        MaterialComponent& material = m_SelectedEntities.front().GetComponent<MaterialComponent>();
        AddVec4Field<MaterialComponent>(box, "Albedo", material.albedoColor, &MaterialComponent::albedoColor);
        AddFloatField<MaterialComponent>(box, "Specular", material.specularStrength, &MaterialComponent::specularStrength);
        AddFloatField<MaterialComponent>(box, "Shininess", material.shininess, &MaterialComponent::shininess);
        // Shader/texture are resource handles - no asset picker UI exists yet, so they aren't editable here.
    });

    RegisterComponentInspector<MeshComponent>("Mesh", "mesh", true, [this](ComponentBoxWidget* box) {
        MeshComponent& meshComponent = m_SelectedEntities.front().GetComponent<MeshComponent>();
        // No asset picker UI exists yet, so the mesh handle is shown read-only rather than editable.
        QString info = meshComponent.mesh.IsValid() ? QString("Handle #%1").arg(meshComponent.mesh.GetID()) : "None";
        QLabel* label = new QLabel(info, box);
        label->setObjectName("ReadOnlyInfoLabel");
        box->SetContent(label);
    });

    RegisterComponentInspector<PythonScriptComponent>("Python Script", "pythonScript", true, [this](ComponentBoxWidget* box) {
        // Unlike every other field in this file, editing only applies to the front-most selected
        // entity, not the whole selection - the Add*Field family assumes one pointer-to-member
        // shared across every selected entity's Component, which doesn't describe "the i-th
        // element of a per-entity vector that can differ in length between entities". No undo
        // support here either, unlike the templated fields below - both worth revisiting once
        // this box has settled more.
        Entity entity = m_SelectedEntities.front();
        PythonScriptComponent& script = entity.GetComponent<PythonScriptComponent>();

        for (size_t i = 0; i < script.bindings.size(); ++i)
        {
            const PythonScriptComponent::Binding& binding = script.bindings[i];

            StringFieldWidget* moduleField = new StringFieldWidget("Module", QString::fromStdString(binding.moduleName), box);
            connect(moduleField, &StringFieldWidget::ValueChanged, this, [this, entity, i](const QString& value) mutable {
                entity.GetComponent<PythonScriptComponent>().bindings[i].moduleName = value.toStdString();
                m_SceneManager.GetScene().NotifyChanged();
            });
            box->SetContent(moduleField);

            StringFieldWidget* classField = new StringFieldWidget("Class", QString::fromStdString(binding.className), box);
            connect(classField, &StringFieldWidget::ValueChanged, this, [this, entity, i](const QString& value) mutable {
                entity.GetComponent<PythonScriptComponent>().bindings[i].className = value.toStdString();
                m_SceneManager.GetScene().NotifyChanged();
            });
            box->SetContent(classField);

            AddPythonScriptFields(box, entity, i);

            if (i + 1 < script.bindings.size())
                box->SetContent(CreateSeparator(box));
        }

        QPushButton* browseButton = CreateAddButton("Browse...", box);
        connect(browseButton, &QPushButton::clicked, this, [this, entity]() mutable { BrowseForScript(entity); });
        box->SetContent(browseButton);
    });
}

void InspectorPanel::BrowseForScript(Entity entity)
{
    QString path = QFileDialog::getOpenFileName(this, "Add Python Script", QString(), "Python Scripts (*.py)");
    if (path.isEmpty())
        return;

    QFileInfo fileInfo(path);

    // The picked file can be anywhere, not necessarily under a directory already registered -
    // register its own directory so LoadScriptModule's import-by-name can actually resolve it.
    m_PythonRuntime.RegisterScriptDirectory(fileInfo.absolutePath().toStdString());

    entity.GetComponent<PythonScriptComponent>().Bind(fileInfo.baseName().toStdString(), GuessClassNameFromFileStem(fileInfo.baseName()).toStdString());
    // Adds a Binding to an already-existing PythonScriptComponent, so entry.addToSelection's own
    // NotifyChanged() (which only fires for a component this entity didn't have yet) doesn't
    // cover this call - moduleName/className are real, serialized scene data either way. That
    // notification is also what rebuilds the box to show the new binding, since the layout
    // signature OnSceneChanged compares includes each binding's count.
    m_SceneManager.GetScene().NotifyChanged();
}

void InspectorPanel::AddPythonScriptFields(ComponentBoxWidget* box, Entity entity, size_t bindingIndex)
{
    // Deliberately doesn't call Scene::NotifyChanged() anywhere below, unlike every other edit in
    // this file (including this same box's own Module/Class fields): a live instance's attributes
    // aren't serialized at all - ComponentRegistry only writes moduleName/className - so editing
    // one here doesn't produce anything Save could actually capture. Marking the scene dirty for
    // an edit Save can't persist would be a false "you have unsaved changes" signal, worse than
    // this field simply not affecting the dirty flag.
    py::object instance = entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex].instance;

    // Nothing to draw until PythonScriptSystem's first Update() has actually instantiated this
    // binding (on_create() is what would set any fields beyond what __init__ already did) - an
    // entity added this frame, or one that's inactive, has no instance yet. Refreshing the
    // Inspector (reselecting, or any other change that triggers Refresh()) after the next tick
    // picks the fields up once it exists.
    if (!instance || instance.is_none())
    {
        QLabel* notRunningLabel = new QLabel("(fields appear once this script is running)", box);
        notRunningLabel->setObjectName("ReadOnlyInfoLabel");
        box->SetContent(notRunningLabel);
        return;
    }

    py::dict fields = instance.attr("__dict__").cast<py::dict>();
    bool anyField = false;

    for (auto item : fields)
    {
        std::string key = py::str(item.first).cast<std::string>();

        // entity/context are engine plumbing PythonScriptSystem sets on every instance, not
        // fields the script author exposed - and a leading underscore is the same "not public"
        // convention Python itself already uses for anything meant to stay internal.
        if (key == "entity" || key == "context" || (!key.empty() && key[0] == '_'))
            continue;

        anyField = true;
        py::handle value = item.second;
        QString label = QString::fromStdString(key);

        // bool before int: in Python, bool is a subclass of int, so an isinstance<int_> check
        // alone would misclassify every bool field as an integer one.
        if (py::isinstance<py::bool_>(value))
        {
            BoolFieldWidget* field = new BoolFieldWidget(label, value.cast<bool>(), box);
            connect(field, &BoolFieldWidget::ValueChanged, this, [entity, bindingIndex, key](bool newValue) mutable {
                SafeCall([&] { entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex].instance.attr(key.c_str()) = newValue; });
            });
            box->SetContent(field);

            m_LiveSyncCallbacks.push_back([this, field, entity, bindingIndex, key]() mutable {
                SafeCall([&] {
                    py::object liveInstance = entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex].instance;
                    if (liveInstance && py::hasattr(liveInstance, key.c_str()))
                        field->SetValue(liveInstance.attr(key.c_str()).cast<bool>());
                });
            });
        }
        else if (py::isinstance<py::int_>(value) || py::isinstance<py::float_>(value))
        {
            // Python int fields round-trip through this widget as float (QDoubleSpinBox is the
            // only numeric widget in this file) - editing one turns it into a Python float from
            // then on. Acceptable for the scripts this was built for (RotationComponent/
            // CameraController's numeric fields are already float), not worth a dedicated
            // int-only widget for yet.
            bool isInt = py::isinstance<py::int_>(value);
            FloatFieldWidget* field = new FloatFieldWidget(label, value.cast<float>(), box);
            connect(field, &FloatFieldWidget::ValueChanged, this, [entity, bindingIndex, key, isInt](float newValue) mutable {
                SafeCall([&] {
                    PythonScriptComponent::Binding& binding = entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex];
                    if (isInt)
                        binding.instance.attr(key.c_str()) = static_cast<int>(newValue);
                    else
                        binding.instance.attr(key.c_str()) = newValue;
                });
            });
            box->SetContent(field);

            m_LiveSyncCallbacks.push_back([this, field, entity, bindingIndex, key]() mutable {
                SafeCall([&] {
                    py::object liveInstance = entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex].instance;
                    if (liveInstance && py::hasattr(liveInstance, key.c_str()))
                        field->SetValue(liveInstance.attr(key.c_str()).cast<float>());
                });
            });
        }
        else if (py::isinstance<py::str>(value))
        {
            StringFieldWidget* field = new StringFieldWidget(label, QString::fromStdString(value.cast<std::string>()), box);
            connect(field, &StringFieldWidget::ValueChanged, this, [entity, bindingIndex, key](const QString& newValue) mutable {
                SafeCall([&] {
                    entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex].instance.attr(key.c_str()) = newValue.toStdString();
                });
            });
            box->SetContent(field);

            m_LiveSyncCallbacks.push_back([this, field, entity, bindingIndex, key]() mutable {
                SafeCall([&] {
                    py::object liveInstance = entity.GetComponent<PythonScriptComponent>().bindings[bindingIndex].instance;
                    if (liveInstance && py::hasattr(liveInstance, key.c_str()))
                        field->SetValue(QString::fromStdString(liveInstance.attr(key.c_str()).cast<std::string>()));
                });
            });
        }
        else
        {
            // Anything else (a Vector3, a nested object, a list...) has no generic editable
            // widget yet - shown read-only via Python's own str() so the field is at least
            // visible rather than silently missing, matching MeshComponent's own read-only
            // fallback above for a handle with no asset-picker UI yet.
            QLabel* readOnlyLabel = new QLabel(QString("%1: %2").arg(label, QString::fromStdString(py::str(value).cast<std::string>())), box);
            readOnlyLabel->setObjectName("ReadOnlyInfoLabel");
            box->SetContent(readOnlyLabel);
        }
    }

    if (!anyField)
    {
        QLabel* noFieldsLabel = new QLabel("(no public fields)", box);
        noFieldsLabel->setObjectName("ReadOnlyInfoLabel");
        box->SetContent(noFieldsLabel);
    }
}

template <typename Component>
void InspectorPanel::RegisterComponentInspector(const std::string& name, const std::string& componentKey, bool addable,
                                                std::function<void(ComponentBoxWidget*)> draw)
{
    ComponentInspectorEntry entry;
    entry.name = name;
    entry.addable = addable;
    entry.allHave = [this] { return AllEntitiesHaveComponent<Component>(m_SelectedEntities); };
    entry.draw = std::move(draw);
    entry.addToSelection = [this, name, componentKey] {
        std::vector<UUID> entityIds;
        for (Entity entity : m_SelectedEntities)
            if (!entity.HasComponent<Component>())
                entityIds.push_back(entity.GetComponent<TagComponent>().id);

        if (!entityIds.empty())
        {
            m_CommandManager.ExecuteCommand(std::make_unique<AddComponentCommand<Component>>(
                m_SceneManager, m_ResourceManager, "Add " + name + " Component", componentKey, std::move(entityIds)));
        }
        // No explicit Refresh() here (or in removeFromSelection below): the command's own
        // Scene::NotifyChanged() reaches OnSceneChanged, which now rebuilds whenever the set of
        // components on the selection changes - the same path an undo/redo of this command takes.
    };
    entry.removeFromSelection = [this, name, componentKey] {
        std::vector<Entity> entities;
        for (Entity entity : m_SelectedEntities)
            if (entity.HasComponent<Component>())
                entities.push_back(entity);

        if (!entities.empty())
        {
            m_CommandManager.ExecuteCommand(std::make_unique<RemoveComponentCommand<Component>>(
                m_SceneManager, m_ResourceManager, "Remove " + name + " Component", componentKey, entities));
        }
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
            m_SceneManager, description.toStdString(), edits, current, setter));

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

ComponentBoxWidget* InspectorPanel::CreateComponentBox(const std::string& name, bool removable)
{
    bool isCollapsed = m_ComponentCollapseStates[name];

    ComponentBoxWidget* box = new ComponentBoxWidget(QString::fromStdString(name), isCollapsed, removable, m_ContentWidget);
    connect(box, &ComponentBoxWidget::CollapseStateChanged, this, [this, name](bool collapsed) {
        m_ComponentCollapseStates[name] = collapsed;
    });

    return box;
}
}  // namespace MatchaEditor
