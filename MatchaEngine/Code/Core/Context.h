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
        Context(Application* application = nullptr,
                Input* input = nullptr,
                Time* time = nullptr,
                Window* window = nullptr);
        ~Context();

        void InitSDLContext();
        void InitWindowContext();
        void InitGLContext();

        void SetApplication(Application* app);
        void SetInput(Input* input);
        void SetTime(Time* time);
        void SetWindow(Window* window);

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

        void LogContext();

    private:
        Application* mApplication;
        Input* mInput;
        Time* mTime;
        Window* mWindow;

        SDL_Window* mNativeWindow = nullptr;
        SDL_GLContext mGLContext;

        bool mSDLInit = false;
        bool mWindowInit = false;
        bool mGLInit = false;
    };
}