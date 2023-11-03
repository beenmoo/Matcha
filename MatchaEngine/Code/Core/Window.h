#pragma once

#include "Math/Vector.h"

#include <cstdint>
#include <string>
#include <sdl/SDL.h>

namespace Matcha
{
    class Context;

    class Window
    {
    public:
        struct WindowSpecification
        {
            std::string mTitle = "Application";
            int mWidth = 1280;
            int mHeight = 720;
            int mWindowLocationX = SDL_WINDOWPOS_CENTERED;
            int mWindowLocationY = SDL_WINDOWPOS_CENTERED;
            uint32_t mFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
        };

    public:
        Window(Context& ctx, 
               const WindowSpecification& spec = WindowSpecification());

        void Resize(int width, int height);
        void SwapBuffers();
        void ProcessEvents(const SDL_Event& evt);

        int GetWidth() const;
        int GetHeight() const;
        Vector2Int GetCenter() const;
        float GetAspectRatio() const;

        const WindowSpecification& GetWindowSpecification() const;

        bool IsMinimized() const;

    private:
        void InitSDLContext();
        void InitWindow();
        void InitGLContext();

    private:
        SDL_Window* mNativeWindow = nullptr;
        SDL_GLContext mGLContext;

        Context& mContext;
        WindowSpecification mWindowSpec;
        bool mIsOpen = false;
    };
}