#pragma once

#include "Math/Vector.h"
#include "Core/Types.h"

#include <cstdint>
#include <string>
#include <SDL3/SDL.h>

namespace Matcha
{
    class Context;

    class Window
    {
    public:
        struct WindowSpecification
        {
            String mTitle = "Application";
            int mWidth = 1280;
            int mHeight = 720;
            int mWindowLocationX = SDL_WINDOWPOS_CENTERED;
            int mWindowLocationY = SDL_WINDOWPOS_CENTERED;
            uint32_t mFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        };

    public:
        Window(const WindowSpecification& spec = WindowSpecification());

        void Resize(int width, int height);
        void SwapBuffers();
        void ProcessEvents(const Event& evt);

        int GetWidth() const;
        int GetHeight() const;
        Vector2Int GetCenter() const;
        float GetAspectRatio() const;

        const WindowSpecification& GetWindowSpecification() const;

        bool IsMinimized() const;

    private:
        void InitContext();

    private:
        GLWindow* mNativeWindow = nullptr;
        GLContext mGLContext;

        WindowSpecification mWindowSpec;
        bool mIsOpen = false;
    };
}