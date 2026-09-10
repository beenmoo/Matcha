#include "Editor.h"
#include "EditorMainWindow.h"
#include "EngineViewportWidget.h"
#include "ViewportInteraction.h"
#include "Core/SDL/SDLInput.h"
#include "Core/SDL/SDLWindow.h"
#include "Scene/System/RenderSystem.h"

#include <Scripting/PythonRuntime.h>

#include <QTimer>
#include <QWidget>

namespace MatchaEditor
{
Editor::Editor(const Application::ApplicationSpecification& spec)
    : Application(spec)
{
    auto* sdlWindow = dynamic_cast<SDLWindow*>(&GetWindow());

    MT_ASSERT(sdlWindow, "Editor requires ApplicationSpecification::headless == true (see main.cpp)");

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

    // Sized to the SDL window's own (default 1280x720) spec for now - corrected on the viewport
    // widget's first real resizeEvent (see OnEvent()'s WindowResized case below). Kept bound for
    // the rest of this Application's lifetime: nothing else ever needs the hidden SDL window's own
    // default framebuffer, so there's no reason to unbind between frames - see RenderCamera().
    m_ViewportFrameBuffer = GetRendererAPI().CreateFrameBuffer(
        FrameBufferSpecification{.width = static_cast<uint32_t>(GetWindow().GetWidth()), .height = static_cast<uint32_t>(GetWindow().GetHeight())});
    m_ViewportFrameBuffer->Bind();

    // Nothing ever set a clear color before, leaving GL's default of pure black - which made an
    // empty scene indistinguishable from "the viewport isn't rendering at all". A neutral grey-blue
    // (the convention every other editor uses) makes an empty-but-working viewport obvious.
    GetRenderer().SetClearColor(Vector4(0.18f, 0.20f, 0.24f, 1.0f));

    m_ViewportWidget = new EngineViewportWidget(*sdlWindow);
    m_ViewportWidget->SetColorTextureProvider([this] { return m_ViewportFrameBuffer->GetColorAttachmentID(); });

    // RMB fly-look asks Input to lock the cursor (see CameraController). SDL's own relative mouse
    // mode is meaningless here - it would act on the hidden window that never has focus - so the
    // request has to be redirected to the Qt widget that actually owns the cursor, which emulates
    // relative mode by hiding it and warping it back to center each frame (see PollCursorLock).
    // Without this the lock silently does nothing and mouse-look reports no movement at all.
    if (auto* sdlInput = dynamic_cast<SDLInput*>(&GetInput()))
        sdlInput->SetExternalCursorLockCallback([this](bool locked) { m_ViewportWidget->SetCursorLocked(locked); });

    // Reparents m_ViewportWidget under the returned container's native window - Qt's own
    // parent/child tree owns and deletes it from here on (see Editor.h's comment on m_ViewportWidget).
    QWidget* viewportContainer = QWidget::createWindowContainer(m_ViewportWidget);
    viewportContainer->setFocusPolicy(Qt::StrongFocus);

    m_MainWindow = std::make_unique<EditorMainWindow>(*this, GetSceneManager(), GetResourceManager(), GetPythonRuntime(),
                                                      GetWindow(), m_ViewportWidget, viewportContainer, m_EditorCamera);

    // Qt owns its own event loop (see main.cpp) - nothing calls Application::Tick() unless
    // something here does. MakeContextCurrent() at the top of Tick() (see Application.cpp) is what
    // lets this safely interleave with m_ViewportWidget->Render()'s own separate Qt-owned context
    // below.
    m_TickTimer = std::make_unique<QTimer>();
    QObject::connect(m_TickTimer.get(), &QTimer::timeout, [this] {
        Tick();
        m_ViewportWidget->Render();
    });
    m_TickTimer->start(16);
}

Editor::~Editor()
{
    // Must be set before any member below is torn down - see Editor.h's comment on m_ShuttingDown.
    m_ShuttingDown = true;
}

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
    if (m_ShuttingDown)
        return;

    // Guard against a zero width or height: fires transiently while a dock is being
    // resized/collapsed/floated (ADS can report a degenerate size mid-drag), and a 0-sized GL
    // texture fails GLFrameBuffer::Invalidate()'s own completeness check.
    if (event.type == EventType::WindowResized && event.width > 0 && event.height > 0)
    {
        m_EditorCamera.SetAspectRatio(static_cast<float>(event.width) / static_cast<float>(event.height));

        // Recorded only - deliberately no GL work here. See m_PendingViewportSize's comment for why
        // rebuilding the framebuffer from inside a Qt event callback is unsafe.
        m_PendingViewportSize = Vector2Int(event.width, event.height);
    }
}

void Editor::RenderCamera()
{
    Renderer& renderer = GetRenderer();

    // Reached only via Tick(), which re-asserts the engine's GL context at its top - the one place
    // it's safe to rebuild the framebuffer (see m_PendingViewportSize). Resize() deletes and
    // recreates the FBO's GL objects, which reverts GL's framebuffer binding to 0 as a side effect
    // (see FrameBuffer::Resize()), so re-Bind() to restore the "always bound" invariant.
    if (m_PendingViewportSize)
    {
        m_ViewportFrameBuffer->Resize(static_cast<uint32_t>(m_PendingViewportSize->x), static_cast<uint32_t>(m_PendingViewportSize->y));
        m_ViewportFrameBuffer->Bind();
        m_PendingViewportSize.reset();
    }

    const FrameBufferSpecification& spec = m_ViewportFrameBuffer->GetSpecification();
    GetRendererAPI().SetViewport(0, 0, spec.width, spec.height);

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
