#include "Application.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>

namespace Matcha
{
    Application::Application(const ApplicationSpecification& spec) :
        mAppSpec(spec),
        mContext(*this, mInput, mTime, mWindow)
    {
        LogContext();
    }

    Application::~Application()
    {
        SDL_Quit();
    }

    void Application::Run()
    {
        if (mIsRunning)
            return;

        mIsRunning = true;

        while (mIsRunning)
        {
            PollEvents();
            Update();
        }
    }

    void Application::Quit()
    {
        mIsRunning = false;
    }

    Context& Application::GetContext()
    {
        return mContext;
    }

    const Context& Application::GetContext() const
    {
        return mContext;
    }

    void Application::Update()
    {
        mInput.Update();
        mTime.Update();
        mWindow.SwapBuffers();
    }

    void Application::PollEvents()
    {
        SDL_Event evt;

        while (SDL_PollEvent(&evt))
        {
            switch (evt.type)
            {
            case SDL_EVENT_QUIT:
                Quit();
                break;
            default:
                break;
            }

            mInput.ProcessEvents(evt);
            mWindow.ProcessEvents(evt);
        }
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
    }
}