#pragma once

#include <sdl/SDL.h>

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
        Context(Application* application,
                Input* input,
                Time* time,
                Window* window);
        ~Context();

        Application* GetApplication();
        const Application* GetApplication() const;
        Input* GetInput();
        const Input* GetInput() const;
        Time* GetTime();
        const Time* GetTime() const;
        Window* GetWindow();
        const Window* GetWindow() const;

        SDL_Window* GetNativeWindow();
        const SDL_Window* GetNativeWindow() const;

    private:
        void LogContext();

    private:
        Application* mApplication;
        Input* mInput;
        Time* mTime;
        Window* mWindow;

        bool mSDLInit = false;
        bool mWindowInit = false;
        bool mGLInit = false;
    };
}