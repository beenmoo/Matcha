#include "Window.h"
#include "Assert.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>

namespace Matcha
{
struct Window::Impl
{
    SDL_Window* m_NativeWindow = nullptr;
    SDL_GLContext m_GLContext = nullptr;
};

Window::Window(const WindowSpecification& spec)
    : m_Impl(std::make_unique<Impl>()),
      m_WindowSpec(spec)
{
    InitContext();
}

Window::~Window() = default;
Window::Window(Window&&) noexcept = default;
Window& Window::operator=(Window&&) noexcept = default;

void Window::ProcessEvents(const Event& evt)
{
    switch (evt.type)
    {
    case EventType::WindowResized:
        m_WindowSpec.m_Width = evt.width;
        m_WindowSpec.m_Height = evt.height;
        glViewport(0, 0, m_WindowSpec.m_Width, m_WindowSpec.m_Height);
        break;
    default:
        break;
    }
}

void Window::Resize(int width, int height)
{
    SDL_SetWindowSize(m_Impl->m_NativeWindow, width, height);
}

void Window::SwapBuffers()
{
    SDL_GL_SwapWindow(m_Impl->m_NativeWindow);
}

int Window::GetWidth() const
{
    return m_WindowSpec.m_Width;
}

int Window::GetHeight() const
{
    return m_WindowSpec.m_Height;
}

Vector2Int Window::GetCenter() const
{
    return Vector2Int(m_WindowSpec.m_Width / 2, m_WindowSpec.m_Height / 2);
}

float Window::GetAspectRatio() const
{
    return static_cast<float>(m_WindowSpec.m_Width) / m_WindowSpec.m_Height;
}

const Window::WindowSpecification& Window::GetWindowSpecification() const
{
    return m_WindowSpec;
}

bool Window::IsMinimized() const
{
    return SDL_GetWindowFlags(m_Impl->m_NativeWindow) & SDL_WINDOW_MINIMIZED;
}

void Window::InitContext()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        MT_CORE_ERROR("{}", SDL_GetError());

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    uint32_t flags = SDL_WINDOW_OPENGL;

    if (m_WindowSpec.m_Resizable)
        flags |= SDL_WINDOW_RESIZABLE;

    m_Impl->m_NativeWindow = SDL_CreateWindow(GetWindowSpecification().m_Title.c_str(),
                                              GetWindowSpecification().m_Width,
                                              GetWindowSpecification().m_Height,
                                              flags);

    MT_ASSERT(m_Impl->m_NativeWindow, SDL_GetError());

    if (m_WindowSpec.m_Position)
        SDL_SetWindowPosition(m_Impl->m_NativeWindow, m_WindowSpec.m_Position->x, m_WindowSpec.m_Position->y);
    else
        SDL_SetWindowPosition(m_Impl->m_NativeWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    m_Impl->m_GLContext = SDL_GL_CreateContext(m_Impl->m_NativeWindow);
    SDL_GL_MakeCurrent(m_Impl->m_NativeWindow, m_Impl->m_GLContext);

    int status = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    MT_ASSERT(status, SDL_GetError());
}
}  // namespace Matcha