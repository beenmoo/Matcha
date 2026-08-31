#pragma once

#include "Core/Assert.h"
#include "Core/Logger.h"
#include "EngineContext.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"
#include "Graphics/Renderer.h"
#include "Graphics/RendererAPI.h"
#include "Graphics/ResourceManager.h"
#include "Scene/Scene.h"

#include <functional>
#include <memory>
#include <vector>

int main(int argc, char** argv);

namespace Matcha
{
struct ApplicationCommandLineArgs
{
    int m_Count = 0;
    char** m_Args = nullptr;

    [[nodiscard]] const char* operator[](int index) const
    {
        MT_ASSERT(index < m_Count && index >= 0, "Out of range");

        return m_Args[index];
    }
};

struct ApplicationSpecification
{
    std::string title = "Application";
    std::string workingDirectory;
    ApplicationCommandLineArgs commandLineArgs;
    WindowBackend windowBackend = WindowBackend::SDL;
    RendererAPI::API rendererAPI = GetDefaultRendererAPI();

    [[nodiscard]] RendererAPI::API GetDefaultRendererAPI() const;
};

class Application
{
public:
    using ApplicationCommandLineArgs = Matcha::ApplicationCommandLineArgs;
    using ApplicationSpecification = Matcha::ApplicationSpecification;

public:
    explicit Application(const ApplicationSpecification& spec = ApplicationSpecification());
    virtual ~Application();

    // Blocking loop: while (m_IsRunning) Tick(); - what SDL-backed apps (Sandbox, an SDL-mode
    // Editor) call. Never called under the Qt backend: Qt owns its own event loop, so a
    // Qt-backed Editor calls Tick() directly from the viewport widget's paintGL() instead.
    void Run();
    void Quit();

    // One frame: PollEvents, Update, Render.
    void Tick();

protected:
    template <typename Self>
    [[nodiscard]] auto& GetContext(this Self& self)
    {
        return self.m_Context;
    }

    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnEvent(const Event& event);

private:
    void Update();
    void Render();
    void PollEvents();
    void LogContext();

    // Populates m_UpdateSystems/m_RenderSystems - the one place a new engine System gets wired
    // into the frame loop, so Update()/Render() themselves never need editing to add one.
    void RegisterSystems();

    // Deferred out of the constructor because the GL context isn't necessarily ready when the
    // constructor returns (Qt: not until QOpenGLWidget::initializeGL() fires, later than
    // construction). Invoked via m_Window's context-ready callback, registered in the
    // constructor.
    void InitGraphics();

private:
    ApplicationSpecification m_AppSpec;

    std::unique_ptr<Input> m_Input;
    Logger m_Logger;
    Time m_Time;
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<RendererAPI> m_RendererAPI;
    ResourceManager m_ResourceManager;
    Renderer m_Renderer;
    Scene m_Scene;
    EngineContext m_Context;

    // Run in registration order every Update()/Render() - see RegisterSystems().
    std::vector<std::function<void()>> m_UpdateSystems;
    std::vector<std::function<void()>> m_RenderSystems;

    bool m_IsRunning = false;
};

[[nodiscard]] Application* CreateApplication(const Application::ApplicationCommandLineArgs& args);
}  // namespace Matcha