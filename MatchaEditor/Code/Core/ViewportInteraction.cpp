#include "ViewportInteraction.h"
#include "CommandManager.h"
#include "EditorCamera.h"
#include "Commands/PropertyEditCommand.h"
#include "EngineViewportWidget.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <algorithm>
#include <cstring>
#include <limits>

namespace MatchaEditor
{
ViewportInteraction::ViewportInteraction(SceneManager& sceneManager, ResourceManager& resourceManager,
                                         EditorCamera& editorCamera, EngineViewportWidget& viewport,
                                         CommandManager& commandManager, QObject* parent)
    : QObject(parent),
      m_SceneManager(sceneManager),
      m_ResourceManager(resourceManager),
      m_EditorCamera(editorCamera),
      m_Viewport(viewport),
      m_CommandManager(commandManager)
{
}

void ViewportInteraction::OnViewportClicked(QPoint localPos, Qt::MouseButton /*button*/)
{
    // The gizmo (if visible) gets first refusal on a click - ImGuizmo::IsOver() reports whether
    // the cursor is currently over one of its own handles, using the IO state BeginGizmoFrame()
    // already fed it this frame. Without this guard, clicking a handle would both start a gizmo
    // drag AND re-run entity picking underneath it, potentially changing the selection out from
    // under the drag that's about to start.
    if (ImGuizmo::IsOver())
        return;

    Vector2 screenPoint(static_cast<float>(localPos.x()), static_cast<float>(localPos.y()));
    Ray worldRay = ScreenPointToRay(screenPoint, ViewportSize(), m_EditorCamera.GetViewProjection());

    std::optional<Entity> picked = Pick(worldRay);

    emit EntityPicked(picked ? std::vector<Entity>{*picked} : std::vector<Entity>{});
}

void ViewportInteraction::OnViewportReleased(Qt::MouseButton /*button*/)
{
    // Undo commit is driven entirely by ImGuizmo::IsUsing()'s edge in DrawAndManipulateGizmo() -
    // nothing to do on mouse release itself (unlike the old hand-rolled gizmo, which owned its own
    // press/drag/release state machine).
}

void ViewportInteraction::SetSelectedEntities(std::vector<Entity> entities)
{
    m_Selected = std::move(entities);
}

void ViewportInteraction::BeginGizmoFrame(bool leftMouseDown, float deltaTime)
{
    if (!m_GizmoInitialized)
    {
        ImGui::CreateContext();
        ImGui_ImplOpenGL3_Init();
        m_GizmoInitialized = true;
    }

    Vector2 viewportSize = ViewportSize();
    QPoint mousePos = m_Viewport.GetMousePosition();

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(viewportSize.x, viewportSize.y);
    io.DeltaTime = deltaTime > 0.0f ? deltaTime : 1.0f / 60.0f;
    io.MousePos = ImVec2(static_cast<float>(mousePos.x()), static_cast<float>(mousePos.y()));
    io.MouseDown[0] = leftMouseDown;

    // QOpenGLWidget::resizeGL() (and so the GL viewport the 3D scene actually renders into, via
    // Window::HandleResizeEvent's glViewport call) receives sizes in *device* pixels, while
    // QWidget::width()/height() (ViewportSize(), above, and Qt's own mouse-event coordinates) are
    // *logical* pixels - the two only match at 100% display scaling. Left at the default (1,1),
    // ImGui_ImplOpenGL3_RenderDrawData sets a GL viewport sized to the logical DisplaySize, which
    // on any scaled display is smaller than the real framebuffer - rendering the whole gizmo
    // shrunk into a sub-rect anchored at GL's bottom-left origin instead of filling the viewport.
    float devicePixelRatio = static_cast<float>(m_Viewport.devicePixelRatio());
    io.DisplayFramebufferScale = ImVec2(devicePixelRatio, devicePixelRatio);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}

namespace
{
ImGuizmo::OPERATION ToImGuizmoOperation(ViewportInteraction::GizmoMode mode)
{
    switch (mode)
    {
    case ViewportInteraction::GizmoMode::Translate:
        return ImGuizmo::TRANSLATE;
    case ViewportInteraction::GizmoMode::Rotate:
        return ImGuizmo::ROTATE;
    case ViewportInteraction::GizmoMode::Scale:
        return ImGuizmo::SCALE;
    }
    return ImGuizmo::TRANSLATE;
}
}  // namespace

void ViewportInteraction::SetTranslateMode()
{
    m_GizmoMode = GizmoMode::Translate;
}

void ViewportInteraction::SetRotateMode()
{
    m_GizmoMode = GizmoMode::Rotate;
}

void ViewportInteraction::SetScaleMode()
{
    m_GizmoMode = GizmoMode::Scale;
}

void ViewportInteraction::DrawAndManipulateGizmo(const Matrix4& view, const Matrix4& projection)
{
    // Single-selection only: PropertyEditCommand<T> applies one shared `after` value to every
    // edit, correct for "set Position/Rotation/Scale to X" but not for "move each entity by the
    // same delta" a multi-selection drag would need - see the plan's scope notes.
    if (m_Selected.size() != 1 || !m_Selected.front().HasComponent<TransformComponent>())
        return;

    Entity entity = m_Selected.front();
    Vector2 viewportSize = ViewportSize();

    // No window/SetDrawlist() of our own needed: ImGuizmo::BeginFrame() (already called from
    // BeginGizmoFrame()) already opens its own full-viewport background window each frame and
    // points its internal drawlist at it. Opening a second overlapping window here and pointing
    // SetDrawlist() at that instead was actively wrong - it left ImGuizmo's own hover/hit-testing
    // (which looks up "which window owns the current drawlist") resolving against the wrong
    // window, breaking drag interaction entirely.
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetRect(0.0f, 0.0f, viewportSize.x, viewportSize.y);

    // Captured before Manipulate() runs, not after: on the very first frame of a drag, Manipulate()
    // both reports `changed` and flips IsUsing() to true in the same call - reading the property
    // afterward would already reflect that first frame's movement, corrupting the undo's "before".
    Transform& transform = entity.GetComponent<TransformComponent>().transform;
    Vector3 positionBeforeManipulate = transform.GetPosition();
    Quaternion rotationBeforeManipulate = transform.GetRotation();
    Vector3 scaleBeforeManipulate = transform.GetScale();

    float matrixBuffer[16];
    std::memcpy(matrixBuffer, entity.GetComponent<TransformComponent>().worldMatrix.GetData(), sizeof(matrixBuffer));

    bool changed = ImGuizmo::Manipulate(view.GetData(), projection.GetData(), ToImGuizmoOperation(m_GizmoMode), ImGuizmo::WORLD, matrixBuffer);

    if (changed)
    {
        // Manipulate() only ever changes the one component matching the current operation, so
        // decomposing and writing back just that one property (rather than all three) leaves
        // whichever the gizmo didn't touch exactly as they were.
        Vector3 scale;
        Quaternion rotation;
        Vector3 translation;
        Vector3 skew;
        Vector4 perspective;
        bool decomposed = Decompose(Matrix4(matrixBuffer), scale, rotation, translation, skew, perspective);

        if (!decomposed)
            return;

        // Live-apply only, no undo entry yet - mirrors Vec3ControlWidget::ValueChanged's own
        // live-apply in InspectorPanel::AddVec3Field. World-space values are used directly as
        // local, correct for a root/unparented entity (see the plan's scope notes on parent-space
        // conversion not being handled here).
        switch (m_GizmoMode)
        {
        case GizmoMode::Translate:
            transform.SetPosition(translation);
            break;
        case GizmoMode::Rotate:
            transform.SetRotation(rotation);
            break;
        case GizmoMode::Scale:
            transform.SetScale(scale);
            break;
        }
    }

    bool isUsing = ImGuizmo::IsUsing();
    if (isUsing && !m_WasUsingGizmo)
    {
        m_DragStartPosition = positionBeforeManipulate;
        m_DragStartRotation = rotationBeforeManipulate;
        m_DragStartScale = scaleBeforeManipulate;
    }
    else if (!isUsing && m_WasUsingGizmo)
    {
        // Drag just ended - commit one undo entry, the same PropertyEditCommand<T> shape
        // AddVec3Field's MakeCommitHandler already uses for Inspector Position/Scale edits.
        UUID entityId = entity.GetComponent<TagComponent>().id;

        switch (m_GizmoMode)
        {
        case GizmoMode::Translate:
        {
            Vector3 finalPosition = transform.GetPosition();
            if (finalPosition != m_DragStartPosition)
            {
                std::vector<PropertyEditCommand<Vector3>::Edit> edits{{entityId, m_DragStartPosition}};
                m_CommandManager.ExecuteCommand(std::make_unique<PropertyEditCommand<Vector3>>(
                    m_SceneManager, "Move Entity", std::move(edits), finalPosition,
                    [](Entity target, const Vector3& value) { target.GetComponent<TransformComponent>().transform.SetPosition(value); }));
            }
            break;
        }
        case GizmoMode::Rotate:
        {
            Quaternion finalRotation = transform.GetRotation();
            if (finalRotation != m_DragStartRotation)
            {
                std::vector<PropertyEditCommand<Quaternion>::Edit> edits{{entityId, m_DragStartRotation}};
                m_CommandManager.ExecuteCommand(std::make_unique<PropertyEditCommand<Quaternion>>(
                    m_SceneManager, "Rotate Entity", std::move(edits), finalRotation,
                    [](Entity target, const Quaternion& value) { target.GetComponent<TransformComponent>().transform.SetRotation(value); }));
            }
            break;
        }
        case GizmoMode::Scale:
        {
            Vector3 finalScale = transform.GetScale();
            if (finalScale != m_DragStartScale)
            {
                std::vector<PropertyEditCommand<Vector3>::Edit> edits{{entityId, m_DragStartScale}};
                m_CommandManager.ExecuteCommand(std::make_unique<PropertyEditCommand<Vector3>>(
                    m_SceneManager, "Scale Entity", std::move(edits), finalScale,
                    [](Entity target, const Vector3& value) { target.GetComponent<TransformComponent>().transform.SetScale(value); }));
            }
            break;
        }
        }
    }
    m_WasUsingGizmo = isUsing;
}

void ViewportInteraction::EndGizmoFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::optional<Entity> ViewportInteraction::Pick(const Ray& worldRay) const
{
    Scene& scene = m_SceneManager.GetScene();
    auto view = scene.View<TransformComponent, MeshComponent>();

    std::optional<Entity> nearestEntity;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto&& [handle, transform, meshComponent] : view.each())
    {
        if (!transform.activeInHierarchy || !meshComponent.mesh.IsValid())
            continue;

        Mesh* mesh = m_ResourceManager.GetMesh(meshComponent.mesh);
        if (!mesh)
            continue;

        // Transforms the local-space bounds' 8 corners into world space and re-derives an
        // axis-aligned box from them - exact for an unrotated/uniformly-scaled mesh, a looser
        // (but safe, since it can only grow the box) approximation of a true oriented box for a
        // rotated one. Good enough for a first pass; see the plan's scope notes.
        Vector3 worldMin(std::numeric_limits<float>::max());
        Vector3 worldMax(std::numeric_limits<float>::lowest());

        for (int i = 0; i < 8; ++i)
        {
            Vector3 localCorner((i & 1) ? mesh->localBoundsMax.x : mesh->localBoundsMin.x,
                                (i & 2) ? mesh->localBoundsMax.y : mesh->localBoundsMin.y,
                                (i & 4) ? mesh->localBoundsMax.z : mesh->localBoundsMin.z);
            Vector3 worldCorner = TransformPoint(transform.worldMatrix, localCorner);

            worldMin.x = std::min(worldMin.x, worldCorner.x);
            worldMin.y = std::min(worldMin.y, worldCorner.y);
            worldMin.z = std::min(worldMin.z, worldCorner.z);
            worldMax.x = std::max(worldMax.x, worldCorner.x);
            worldMax.y = std::max(worldMax.y, worldCorner.y);
            worldMax.z = std::max(worldMax.z, worldCorner.z);
        }

        float distance = 0.0f;
        if (IntersectRayAABB(worldRay, worldMin, worldMax, distance) && distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestEntity = Entity(handle, &scene);
        }
    }

    return nearestEntity;
}

Vector2 ViewportInteraction::ViewportSize() const
{
    return Vector2(static_cast<float>(m_Viewport.width()), static_cast<float>(m_Viewport.height()));
}
}  // namespace MatchaEditor
