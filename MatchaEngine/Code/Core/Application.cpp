#include "Application.h"

#include <sdl/SDL.h>

namespace Matcha
{
    Application::Application(const ApplicationSpecification& spec) :
        mAppSpec(spec),
        mWindow(mContext)
    {
        mContext.SetApplication(this);
        mContext.SetInput(&mInput);
        mContext.SetTime(&mTime);
        mContext.SetWindow(&mWindow);
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
            case SDL_QUIT:
                Quit();
                break;
            default:
                break;
            }

            mInput.ProcessEvents(evt);
            mWindow.ProcessEvents(evt);
        }
    }
}