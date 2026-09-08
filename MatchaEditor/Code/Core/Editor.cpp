#include "Editor.h"
#include "EditorMainWindow.h"
#include "ViewportInteraction.h"
#include "Core/Qt/QtViewportWidget.h"
#include "Core/Qt/QtWindow.h"
#include "Scene/System/RenderSystem.h"

#include <Scripting/PythonRuntime.h>

#include <QTimer>

namespace MatchaEditor
{
Editor::Editor(const Application::ApplicationSpecification& spec)
    : Application(spec)
{
    auto* qtWindow = dynamic_cast<QtWindow*>(&GetWindow());

    MT_ASSERT(qtWindow, "Editor requires ApplicationSpecification::m_WindowBackend == WindowBackend::Qt");

    m_EditorCamera.SetAspectRatio(GetWindow().GetAspectRatio());

    // Same relative-path convention (and reasoning) as Sandbox's own call: the working directory
    // is the executable's output directory, which is where MatchaEditor's Assets/ - Scripts
    // included - gets copied post-build.
    //
    // Registered here at startup, before any scene can be opened, rather than left to whenever
    // the Inspector's "Browse..." picker happens to register a directory: a scene stores a
    // binding as a (module, class) name pair for Python's import system to resolve, so opening
    // one in a fresh session fails with ModuleNotFoundError unless something has already put the
    // directory holding that module on the interpreter's path.
    GetPythonRuntime().RegisterScriptDirectory("Assets/Scripts");

    m_MainWindow = std::make_unique<EditorMainWindow>(*this, GetSceneManager(), GetResourceManager(), GetPythonRuntime(),
                                                      GetWindow(), qtWindow->GetViewportWidget(), m_EditorCamera);

    m_TickTimer = std::make_unique<QTimer>();
    QObject::connect(m_TickTimer.get(), &QTimer::timeout, [qtWindow] { qtWindow->GetViewportWidget()->update(); });
    m_TickTimer->start(16);
}

Editor::~Editor() = default;

void Editor::Show()
{
    m_MainWindow->showMaximized();
}

void Editor::OnUpdate()
{
    m_EditorCamera.Update(GetInput(), GetTime());

    // W/E/R gizmo-mode shortcuts (Unity/Unreal/Blender convention) - gated on RMB not being held
    // so they don't collide with WASD flying the camera (see CameraController, which now gates
    // its own WASD movement the same way).
    if (!GetInput().GetMouseButton(Input::MouseButton::Right))
    {
        ViewportInteraction& viewportInteraction = m_MainWindow->GetViewportInteraction();

        if (GetInput().GetKeyDown(KeyCode::W))
            viewportInteraction.SetTranslateMode();
        else if (GetInput().GetKeyDown(KeyCode::E))
            viewportInteraction.SetRotateMode();
        else if (GetInput().GetKeyDown(KeyCode::R))
            viewportInteraction.SetScaleMode();
    }
}

void Editor::OnEvent(const Event& event)
{
    // Guard against height == 0: fires transiently while a dock is being resized/collapsed.
    if (event.type == EventType::WindowResized && event.height > 0)
        m_EditorCamera.SetAspectRatio(static_cast<float>(event.width) / static_cast<float>(event.height));
}

void Editor::RenderCamera()
{
    Renderer& renderer = GetRenderer();

    m_MainWindow->GetViewportInteraction().BeginGizmoFrame(GetInput().GetMouseButton(Input::MouseButton::Left), GetTime().GetDeltaTime());

    renderer.SetViewProjection(m_EditorCamera.GetViewProjection());
    renderer.SetCameraPosition(m_EditorCamera.GetPosition());

    RenderSystem::Draw(GetScene(), renderer);

    m_MainWindow->GetViewportInteraction().DrawAndManipulateGizmo(m_EditorCamera.GetView(), m_EditorCamera.GetProjection());
}

void Editor::OnPostRender()
{
    m_MainWindow->GetViewportInteraction().EndGizmoFrame();
}
}  // namespace MatchaEditor
