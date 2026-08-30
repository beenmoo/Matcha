#include "Application.h"
#include "Scene/System/CameraSystem.h"
#include "Scene/System/LightSystem.h"
#include "Scene/System/RenderSystem.h"
#include "Scene/System/TransformSystem.h"
#include "Scene/System/ScriptSystem.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>

#ifdef MT_ENABLE_QT_BACKEND
#include <QtGlobal>
#endif

namespace Matcha
{
Application::Application(const ApplicationSpecification& spec)
    : m_AppSpec(spec),
      m_Input(Input::Create(spec.m_WindowBackend)),
      m_Window(Window::Create(spec.m_WindowBackend, WindowSpecification{.m_Title = spec.m_Title}, m_Input.get())),
      m_RendererAPI(RendererAPI::Create(RendererAPI::API::OpenGL)),
      m_Renderer(*m_RendererAPI, m_ResourceManager),
      m_Context(*this, *m_Input, m_Time, *m_Window, *m_RendererAPI, m_Renderer, m_ResourceManager, m_Scene)
{
    // SDL's GL context is current immediately, so this fires synchronously here. Qt's isn't
    // ready until QOpenGLWidget::initializeGL() runs later, so InitGraphics() is deferred until
    // then instead - see Window::SetContextReadyCallback.
    m_Window->SetContextReadyCallback([this] { InitGraphics(); });

    // Registered once, persistently: SDL uses it inside each PumpEvents() call, Qt invokes it
    // from the viewport widget's own event callbacks, which can fire at any time.
    m_Window->SetEventDispatch([this](const Event& evt) {
        if (evt.type == EventType::Quit)
            Quit();

        m_Input->ProcessEvents(evt);
        m_Window->ProcessEvents(evt);

        OnEvent(evt);
    });

    RegisterSystems();
}

// SDL_Quit() lives in SDLWindow's destructor, not here: it's only ever paired with the SDL_Init()
// SDLWindow::InitContext() calls, and under the Qt backend SDL_Init() is never called at all (Qt
// builds its own QtWindow/QtInput instead) - calling SDL_Quit() unconditionally here would shut
// down a subsystem this process may never have initialized.
Application::~Application() = default;

void Application::Run()
{
    if (m_IsRunning)
        return;

    m_IsRunning = true;

    while (m_IsRunning)
        Tick();
}

void Application::Quit()
{
    m_IsRunning = false;
}

void Application::Tick()
{
    // Must run before PollEvents(): Input::Update() resets per-frame deltas (mouse axis, scroll)
    // to zero before re-accumulating them from this frame's events. Running it after PollEvents()
    // (as part of Update(), where it used to live) would wipe out the deltas PollEvents() just
    // set, before OnUpdate() ever got to read them.
    m_Input->Update();

    PollEvents();
    Update();
    Render();
}

void Application::OnUpdate()
{
}

void Application::OnRender()
{
}

void Application::OnEvent(const Event& event)
{
}

void Application::Update()
{
    m_Time.Update();
    m_ResourceManager.ReloadModifiedShaders();

    for (auto& system : m_UpdateSystems)
        system();

    OnUpdate();
}

void Application::Render()
{
    m_Renderer.Clear();

    for (auto& system : m_RenderSystems)
        system();

    OnRender();
    m_Renderer.Flush();
    m_Window->SwapBuffers();
}

void Application::RegisterSystems()
{
    m_UpdateSystems.push_back([this] { ScriptSystem::Update(m_Scene, m_Context); });

    // Run in this order: Transform before Camera/Light/Render (which read world-space transforms
    // the cascade just computed), Render last (needs the camera/light state the others set up).
    m_RenderSystems.push_back([this] { TransformSystem::Update(m_Scene); });
    m_RenderSystems.push_back([this] { CameraSystem::Update(m_Scene); });
    m_RenderSystems.push_back([this] { LightSystem::Update(m_Scene, m_Renderer); });
    m_RenderSystems.push_back([this] { RenderSystem::Update(m_Scene, m_Renderer); });
}

void Application::PollEvents()
{
    m_Window->PumpEvents();
}

void Application::InitGraphics()
{
    m_RendererAPI->Init();
    m_Renderer.Init();

    LogContext();
}

void Application::LogContext()
{
    int matchaEngineMajor = 0, matchaEngineMinor = 1;

    MT_CORE_INFO("MatchaEngine v{}.{}", matchaEngineMajor, matchaEngineMinor);
#ifdef MT_PLATFORM_WINDOWS
    MT_CORE_INFO("Platform: WINDOWS");
#endif
#ifdef MT_PLATFORM_LINUX
    MT_CORE_INFO("Platform: LINUX");
#endif

    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    MT_CORE_INFO("OpenGL Info:");
    MT_CORE_INFO("  Vendor: {0}", vendor);
    MT_CORE_INFO("  Renderer: {0}", renderer);
    MT_CORE_INFO("  Version: {0}", version);

    MT_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 6),
              "Matcha requires at least OpenGL version 4.6!");

    int sdlVersion = SDL_GetVersion();

    MT_CORE_INFO("SDL v{}.{}.{}", SDL_VERSIONNUM_MAJOR(sdlVersion), SDL_VERSIONNUM_MINOR(sdlVersion), SDL_VERSIONNUM_MICRO(sdlVersion));

#ifdef MT_ENABLE_QT_BACKEND
    if (m_AppSpec.m_WindowBackend == WindowBackend::Qt)
        MT_CORE_INFO("Qt v{0} (compiled with v{1})", qVersion(), QT_VERSION_STR);
#endif
}
}  // namespace Matcha
