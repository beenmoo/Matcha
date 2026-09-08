#pragma once

#include "Core/Assert.h"
#include "Core/Logger.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"
#include "Graphics/Renderer.h"
#include "Graphics/RendererAPI.h"
#include "Graphics/ResourceManager.h"
#include "Scene/SceneManager.h"

#include <filesystem>
#include <functional>
#include <memory>
#include <vector>

int main(int argc, char** argv);

namespace Matcha
{
class PythonRuntime;

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

    // Where Application::GetAssetsPath() resolves to. Left empty (the default) falls back to
    // "<current working directory>/Assets" - the convention every asset load in this codebase
    // already assumes (Sandbox.cpp/Editor.cpp's RegisterScriptDirectory("Assets/Scripts"), the
    // post-build matcha_copy_engine_assets() step that puts an "Assets" folder next to the
    // executable). Set this when that assumption doesn't hold - a build that runs from a
    // directory other than its own output directory, or a future multi-project editor that needs
    // to point at whichever project is currently open. A relative path here is resolved against
    // the current working directory at construction time, same as the empty-path default is.
    std::filesystem::path assetsPath;

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

    // Absolute path to this application's "Assets" directory, resolved once at construction -
    // ApplicationSpecification::assetsPath if the caller set one, otherwise "<current working
    // directory>/Assets", the convention every asset load in this codebase already assumes
    // (Sandbox.cpp/Editor.cpp's own RegisterScriptDirectory("Assets/Scripts"), MaterialComponent's
    // serialized shaderPaths, the post-build matcha_copy_engine_assets() step that puts an
    // "Assets" folder next to the executable in the first place). Exists so editor UI that deals
    // in absolute filesystem paths (AssetBrowserWidget's QFileSystemModel, a QFileDialog result)
    // has one place to resolve against, instead of every call site reconstructing
    // "current_path() / Assets" - or worse, hardcoding "Assets" as a relative literal the way the
    // engine-side call sites above still do, which only works if nothing has ever chdir'd or
    // asked for a different assets root, an assumption this makes explicit exactly once.
    [[nodiscard]] const std::filesystem::path& GetAssetsPath() const
    {
        return m_AssetsPath;
    }

protected:
    // Every member below is privately owned by Application, so a subclass (Editor, Sandbox) has
    // no other way to reach them - these exist purely for that, not for anything outside the
    // inheritance hierarchy (EditorMainWindow and every Panel/Command below it takes the specific
    // reference(s) it needs directly as constructor parameters instead, resolved once by whichever
    // subclass constructs them).
    [[nodiscard]] Input& GetInput() { return *m_Input; }
    [[nodiscard]] Time& GetTime() { return m_Time; }
    [[nodiscard]] Window& GetWindow() { return *m_Window; }
    [[nodiscard]] Renderer& GetRenderer() { return m_Renderer; }
    [[nodiscard]] ResourceManager& GetResourceManager() { return m_ResourceManager; }
    [[nodiscard]] PythonRuntime& GetPythonRuntime() { return *m_PythonRuntime; }
    [[nodiscard]] SceneManager& GetSceneManager() { return m_SceneManager; }

    // Delegates to m_SceneManager rather than returning a cached Scene& - the Scene it returns can
    // be swapped out from under any caller that holds onto it (SceneManager::NewScene()/
    // OpenScene()), so this must re-fetch the live one every call rather than caching it.
    [[nodiscard]] Scene& GetScene() { return m_SceneManager.GetScene(); }

    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnEvent(const Event& event);

    // Called once per frame from Render(), after the Transform/Camera/Light systems and before
    // OnRender(). Default draws from the scene's own primary CameraComponent entity, via
    // RenderSystem::Update() - override to draw from a different source instead (e.g. MatchaEditor's
    // standalone, non-Scene EditorCamera). Deliberately not folded into RegisterSystems()/
    // m_RenderSystems: that's populated from Application's own constructor, where a virtual call
    // can never resolve to a derived override (the derived vtable isn't installed yet) - this is
    // called from Render(), well after construction, where virtual dispatch works normally.
    virtual void RenderCamera();

    // Called once per frame from Render(), after Renderer::Flush() has issued the scene's actual
    // GL draw calls but before the buffer swap - unlike RenderCamera()/OnRender() (both called
    // before Flush(), while the frame's geometry is only queued, not yet rasterized), this is the
    // one hook that runs once everything opaque is actually on screen. Needed for an overlay that
    // must draw on top of the rendered scene (MatchaEditor's ImGuizmo viewport gizmo) rather than
    // underneath it.
    virtual void OnPostRender();

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
    std::filesystem::path m_AssetsPath;

    std::unique_ptr<Input> m_Input;
    Logger m_Logger;
    Time m_Time;
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<RendererAPI> m_RendererAPI;
    ResourceManager m_ResourceManager;
    Renderer m_Renderer;

    // Forward-declared/unique_ptr, same reason as m_RendererAPI above: PythonRuntime.h pulls in
    // <pybind11/embed.h>, which needs Python's own headers/libs available to whoever includes it -
    // fine for Application.cpp, not something every consumer of this widely-included header
    // (most of MatchaEditor, via Application.h) should be forced to link against.
    //
    // Must be declared (and therefore destroyed) after every Scene-owning member above it and
    // before every one below - members destroy in reverse declaration order, and a
    // PythonScriptComponent still holding a live Python object when this interpreter shuts down
    // is undefined behavior. m_SceneManager (below) outlives every Scene it owns, so this only
    // needs to precede m_SceneManager itself, not enumerate every Scene-owning member individually.
    std::unique_ptr<PythonRuntime> m_PythonRuntime;

    SceneManager m_SceneManager;

    // Run in registration order every Update()/Render() - see RegisterSystems().
    std::vector<std::function<void()>> m_UpdateSystems;
    std::vector<std::function<void()>> m_RenderSystems;

    bool m_IsRunning = false;
};

[[nodiscard]] Application* CreateApplication(const Application::ApplicationCommandLineArgs& args);
}  // namespace Matcha