#pragma once

#include "Scene/Entity.h"

#include <Matcha.h>
#include <DockWidget.h>
#include <QStringList>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

class QVBoxLayout;
class QPushButton;
class QTimer;

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
class ComponentBoxWidget;

class InspectorPanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit InspectorPanel(ads::CDockManager* dockManager, EngineContext& context, QWidget* parent = nullptr);

    void SetSelectedEntities(std::vector<Entity> entities);

private:
    void Refresh();
    void OnSceneChanged();

    // Subscribes to the *current* Scene's AddOnSceneChanged - called both at construction and
    // every time SceneManager::AddOnSceneReplaced fires, since a swapped-out Scene destroys its
    // own subscriber list along with it.
    void BindScene();

    // The scene itself (not just its content) was replaced - every currently-selected entity
    // belongs to a Scene that no longer exists, so there's nothing to prune, just drop the
    // selection outright and rebind to whatever Scene is current now.
    void OnSceneReplaced();

    // Pushes every registered field's current entity value into its widget (skipping any field
    // currently focused, so it doesn't clobber an in-progress edit) - called on a timer so values
    // changed by something other than this panel (a script moving the camera, physics, etc.) show
    // up without needing to reselect the entity.
    void SyncLiveValues();

    // Registers one component type's inspector: how to detect it, how to draw its fields, and
    // (if `addable`) how to add it to entities missing it. Called once per component type from
    // RegisterComponentInspectors(), keeping the type dispatch in one place instead of scattered
    // across Refresh() and the Add Component menu.
    void RegisterComponentInspectors();
    template <typename Component>
    void RegisterComponentInspector(const std::string& name, bool addable, std::function<void(ComponentBoxWidget*)> draw);
    void ShowAddComponentMenu(QPushButton* anchor);

    // Scripts bindable onto a NativeScriptComponent via the "Native Script" box's own "+ Add
    // Script" menu - a second, smaller registry alongside m_ComponentInspectors, since binding a
    // script isn't "add a component with defaults" (NativeScriptComponent holds a list of
    // type-erased bindings, not fields).
    void RegisterScripts();
    template <typename Script>
    void RegisterScript(const std::string& name);
    void ShowAddScriptMenu(QPushButton* anchor);

    // Adds one Vec3ControlWidget to `box`, wired to apply edits (via `setter`) to every
    // currently-selected entity's TransformComponent, and to keep displaying the first selected
    // entity's current value (via `getter`, re-read each sync tick) even when something other
    // than this field is what's changing it. A std::function rather than a pointer-to-member here
    // (unlike the templated Add*Field overloads below) because Transform::GetPosition() is a
    // deducing-this template - not a plain member function pointer-to-member can bind to.
    void AddVec3Field(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue,
                      std::function<Vector3()> getter, void (Transform::*setter)(const Vector3&));

    // Adds a field widget to `box`, wired to write `member` directly on every currently-selected
    // entity's Component. Templated on the pointer-to-member so the wiring (capture selection,
    // connect, batch-apply) is written once per value type instead of once per field.
    template <typename Component>
    void AddStringField(ComponentBoxWidget* box, const QString& label, const QString& initialValue,
                        std::string Component::*member);
    template <typename Component>
    void AddBoolField(ComponentBoxWidget* box, const QString& label, bool initialValue, bool Component::*member);
    template <typename Component>
    void AddFloatField(ComponentBoxWidget* box, const QString& label, float initialValue, float Component::*member);
    template <typename Component>
    void AddVec3Field(ComponentBoxWidget* box, const QString& label, const Vector3& initialValue, Vector3 Component::*member);
    template <typename Component>
    void AddVec4Field(ComponentBoxWidget* box, const QString& label, const Vector4& initialValue, Vector4 Component::*member);
    template <typename Component, typename Enum>
    void AddEnumField(ComponentBoxWidget* box, const QString& label, const QStringList& options, Enum initialValue,
                      Enum Component::*member);

    ComponentBoxWidget* CreateComponentBox(const std::string& name);

private:
    struct ComponentInspectorEntry
    {
        std::string name;
        bool addable = true;
        std::function<bool()> allHave;
        std::function<void(ComponentBoxWidget*)> draw;
        std::function<void()> addToSelection;
    };

    struct ScriptInspectorEntry
    {
        std::string name;
        std::function<void(Entity)> bind;
    };

    EngineContext& m_Context;
    std::vector<Entity> m_SelectedEntities;
    std::vector<ComponentInspectorEntry> m_ComponentInspectors;
    std::vector<ScriptInspectorEntry> m_ScriptInspectors;

    // Rebuilt every Refresh() alongside the widgets themselves - one entry per field currently
    // shown, each reading the first selected entity's current value and pushing it into that
    // field's widget. See SyncLiveValues().
    std::vector<std::function<void()>> m_LiveSyncCallbacks;
    QTimer* m_SyncTimer;

    QWidget* m_ContentWidget;
    QVBoxLayout* m_MainLayout;

    std::unordered_map<std::string, bool> m_ComponentCollapseStates;
};
}  // namespace MatchaEditor
