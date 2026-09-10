#pragma once

#include "Scene/Entity.h"

#include <Matcha.h>

#include <QObject>
#include <QPoint>

#include <optional>
#include <vector>

namespace MatchaEditor
{
class CommandManager;
class EditorCamera;
class EngineViewportWidget;

// Turns a click in the 3D viewport into a scene-entity selection (ray-vs-mesh-bounds picking),
// and drives an ImGuizmo translate gizmo for whatever's currently selected - the viewport's
// counterpart to SceneHierarchyWidget's own tree-click selection, feeding the same downstream
// consumers (InspectorPanel) through the same EntityPicked -> SelectEntities path
// EditorMainWindow already wires the outliner through.
class ViewportInteraction : public QObject
{
    Q_OBJECT
public:
    ViewportInteraction(SceneManager& sceneManager, ResourceManager& resourceManager, EditorCamera& editorCamera,
                        EngineViewportWidget& viewport, CommandManager& commandManager, QObject* parent = nullptr);

    void OnViewportClicked(QPoint localPos, Qt::MouseButton button);
    void OnViewportReleased(Qt::MouseButton button);

    // Fed by EditorMainWindow from the same SceneHierarchyPanel::SelectionChanged signal
    // InspectorPanel already listens to - the gizmo tracks whatever's currently selected
    // regardless of whether that selection came from the outliner or from OnViewportClicked's own
    // EntityPicked round trip.
    void SetSelectedEntities(std::vector<Entity> entities);

    // Called from Editor::RenderCamera(), before the scene's own RenderSystem::Draw - lazily
    // creates the ImGui/ImGuizmo context on first call, then starts this frame's ImGui frame
    // (feeding it viewport size/mouse state) so ImGuizmo calls later this frame have somewhere to
    // record into. Must run before DrawAndManipulateGizmo() and be matched by EndGizmoFrame().
    // leftMouseDown/deltaTime are passed in rather than read from Input/Time directly so this
    // class doesn't need its own reference to either - Editor::RenderCamera() already has both
    // via Application's protected accessors.
    void BeginGizmoFrame(bool leftMouseDown, float deltaTime);

    // Called from Editor::RenderCamera(), after RenderSystem::Draw - draws (and, if dragging,
    // applies) an ImGuizmo translate gizmo for the single selected entity. No-op without exactly
    // one selection with a TransformComponent.
    void DrawAndManipulateGizmo(const Matrix4& view, const Matrix4& projection);

    // Called from Editor::OnPostRender(), after Renderer::Flush() has actually rasterized the
    // scene - only now is it correct to draw ImGui/ImGuizmo's overlay on top of it.
    void EndGizmoFrame();

    // W/E/R gizmo-mode shortcuts (Editor polls these via Input::GetKeyDown, gated on RMB not
    // being held so they don't fire while WASD is flying the camera - see CameraController).
    // Plain setters rather than exposing ImGuizmo::OPERATION here keeps ImGuizmo entirely an
    // implementation detail of the .cpp.
    void SetTranslateMode();
    void SetRotateMode();
    void SetScaleMode();

    enum class GizmoMode
    {
        Translate,
        Rotate,
        Scale
    };

signals:
    // Empty vector means the click hit nothing - i.e. deselect, matching how clicking empty space
    // in the outliner already behaves.
    void EntityPicked(std::vector<Entity> entities);

private:
    // Nearest entity (TransformComponent + MeshComponent) whose world-space bounds the ray hits,
    // if any.
    [[nodiscard]] std::optional<Entity> Pick(const Ray& worldRay) const;

    [[nodiscard]] Vector2 ViewportSize() const;

private:
    SceneManager& m_SceneManager;
    ResourceManager& m_ResourceManager;
    EditorCamera& m_EditorCamera;
    EngineViewportWidget& m_Viewport;
    CommandManager& m_CommandManager;

    std::vector<Entity> m_Selected;
    bool m_GizmoInitialized = false;
    GizmoMode m_GizmoMode = GizmoMode::Translate;

    // Edge-detected against ImGuizmo::IsUsing() in DrawAndManipulateGizmo() - false->true snapshots
    // whichever of these m_GizmoMode is currently active for undo, true->false pushes the matching
    // PropertyEditCommand<T>. Only one is meaningful per drag, but three plain fields are simpler
    // than a variant for just three fixed cases.
    bool m_WasUsingGizmo = false;
    Vector3 m_DragStartPosition;
    Quaternion m_DragStartRotation;
    Vector3 m_DragStartScale;
};
}  // namespace MatchaEditor
