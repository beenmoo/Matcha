#pragma once

#include <SDL3/SDL.h>

namespace Matcha
{
    class Application;
    class Input;
    class Logger;
    class Time;
    class Window;

    class Context
    {
    public:
        Context(Application& application,
                Input& input,
                Time& time,
                Window& window);

        Application& GetApplication();
        const Application& GetApplication() const;
        Input& GetInput();
        const Input& GetInput() const;
        Time& GetTime();
        const Time& GetTime() const;
        Window& GetWindow();
        const Window& GetWindow() const;

    private:
        Application& mApplication;
        Input& mInput;
        Time& mTime;
        Window& mWindow;
    };
}