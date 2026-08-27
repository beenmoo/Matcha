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

#include <memory>

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
    std::string m_Title = "Application";
    std::string m_WorkingDirectory;
    ApplicationCommandLineArgs m_CommandLineArgs;
};

class Application
{
public:
    using ApplicationCommandLineArgs = Matcha::ApplicationCommandLineArgs;
    using ApplicationSpecification = Matcha::ApplicationSpecification;

public:
    Application(const ApplicationSpecification& spec = ApplicationSpecification());
    virtual ~Application();

    void Run();
    void Quit();

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

private:
    ApplicationSpecification m_AppSpec;

    Input m_Input;
    Logger m_Logger;
    Time m_Time;
    Window m_Window;
    std::unique_ptr<RendererAPI> m_RendererAPI;
    ResourceManager m_ResourceManager;
    Renderer m_Renderer;
    Scene m_Scene;
    EngineContext m_Context;

    bool m_IsRunning = false;
};

[[nodiscard]] Application* CreateApplication(const Application::ApplicationCommandLineArgs& args);
}  // namespace Matcha