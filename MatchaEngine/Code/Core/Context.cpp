#include "Context.h"
#include "Core.h"
#include "Application.h"
#include "Input.h"
#include "Time.h"
#include "Window.h"

namespace Matcha
{
    Context::Context(Application* application,
                     Input* input,
                     Time* time,
                     Window* window) :
        mApplication(application),
        mInput(input),
        mTime(time),
        mWindow(window)
    {}

    void Context::InitSDLContext()
    {
        if (mSDLInit)
        {
            MT_CORE_WARN("SDL already initialized!");

            return;
        }

        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
            MT_CORE_ERROR(SDL_GetError());

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        mSDLInit = true;
    }

    void Context::InitWindowContext()
    {
        if (mWindowInit)
        {
            MT_CORE_WARN("Window already initialized!");

            return;
        }

        mNativeWindow = SDL_CreateWindow(mWindow->GetWindowSpecification().mTitle.c_str(),
                                         mWindow->GetWindowSpecification().mWindowLocationX,
                                         mWindow->GetWindowSpecification().mWindowLocationY,
                                         mWindow->GetWindowSpecification().mWidth,
                                         mWindow->GetWindowSpecification().mHeight,
                                         mWindow->GetWindowSpecification().mFlags);

        if (!mWindow)
        {
            MT_ASSERT(mWindow, SDL_GetError());
            MT_CORE_ERROR(SDL_GetError());

            return;
        }

        mWindowInit = true;
    }

    void Context::InitGLContext()
    {
        if (mGLInit)
        {
            MT_CORE_WARN("GL context already initialized!");

            return;
        }

        mGLContext = SDL_GL_CreateContext(mNativeWindow);
        SDL_GL_MakeCurrent(mNativeWindow, mGLContext);

        int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

        if (!status)
        {
            MT_ASSERT(status, SDL_GetError());
            MT_CORE_ERROR(SDL_GetError());

            return;
        }

        mGLInit = true;
    }

    void Context::SetApplication(Application* app)
    {
        mApplication = app;
    }

    void Context::SetInput(Input* input)
    {
        mInput = input;
    }

    void Context::SetTime(Time* time)
    {
        mTime = time;
    }

    void Context::SetWindow(Window* window)
    {
        mWindow = window;
    }

    Application* Context::GetApplication()
    {
        return mApplication;
    }

    const Application* Context::GetApplication() const
    {
        return mApplication;
    }

    Input* Context::GetInput()
    {
        return mInput;
    }

    const Input* Context::GetInput() const
    {
        return mInput;
    }

    Time* Context::GetTime()
    {
        return mTime;
    }

    const Time* Context::GetTime() const
    {
        return mTime;
    }

    Window* Context::GetWindow()
    {
        return mWindow;
    }

    const Window* Context::GetWindow() const
    {
        return mWindow;
    }
    
    SDL_Window* Context::GetNativeWindow()
    {
        return mNativeWindow;
    }

    const SDL_Window* Context::GetNativeWindow() const
    {
        return mNativeWindow;
    }
    
    void Context::LogContext()
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

        SDL_version sdlVersion;
        SDL_VERSION(&sdlVersion);
        MT_CORE_INFO("SDL v{}.{}.{}", (int32_t)sdlVersion.major, (int32_t)sdlVersion.minor, (int32_t)sdlVersion.patch);
    }
}