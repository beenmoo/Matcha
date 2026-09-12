#include "GLViewportPresenter.h"
#include "Core/SDL/SDLWindow.h"

#include <Matcha.h>

#include <glad/glad.h>

#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QWindow>
#include <QtGui/qopenglcontext_platform.h>

#include <SDL3/SDL.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
// The real Khronos header (not glad's - glad here only ever generates a core GL loader, no
// EGL/GLX loaders) for eglGetCurrentContext()/eglGetCurrentDisplay() below - same reasoning as
// wglGetCurrentContext() on Windows: these are fixed EGL 1.0 entry points, not something worth
// routing through an extension loader. Provided by libegl1-mesa-dev in CI; linked via OpenGL::EGL
// (see MatchaEditor/CMakeLists.txt).
#include <EGL/egl.h>
#endif

namespace MatchaEditor
{
namespace
{
// Trivial fullscreen-quad blit: samples the engine's FrameBuffer color texture and writes it
// straight to the target window's own default framebuffer. No lighting/transform uniforms needed
// - two triangles covering clip space, one texture sample per fragment.
constexpr const char* kBlitVertexSource = R"(#version 330 core
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;
out vec2 v_TexCoord;
void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 0.0, 1.0);
}
)";

constexpr const char* kBlitFragmentSource = R"(#version 330 core
in vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Texture;
void main()
{
    o_Color = texture(u_Texture, v_TexCoord);
}
)";

// x, y, u, v per vertex - a fullscreen triangle strip's worth of quad, flipped in V since the
// engine's FrameBuffer texture (like every other GL texture here) has its origin at the
// bottom-left while this blit's own clip-space quad is most naturally authored top-left-first.
constexpr float kQuadVertices[] = {
    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,

    -1.0f, -1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
};

unsigned int CompileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    MT_ASSERT(success, "GLViewportPresenter: blit shader failed to compile");

    return shader;
}
}  // namespace

GLViewportPresenter::GLViewportPresenter(Matcha::SDLWindow& engineWindow)
    : m_EngineWindow(engineWindow)
{
}

GLViewportPresenter::~GLViewportPresenter()
{
    if (m_Context && m_Context->isValid() && m_Target)
    {
        m_Context->makeCurrent(m_Target);
        glDeleteProgram(m_BlitShaderProgram);
        glDeleteVertexArrays(1, &m_QuadVertexArray);
        glDeleteBuffers(1, &m_QuadVertexBuffer);
        m_Context->doneCurrent();

        // Qt's context was current for the teardown above; hand the engine's back, since
        // everything else in the editor assumes it (see Render()).
        m_EngineWindow.MakeContextCurrent();
    }
}

void GLViewportPresenter::ConfigureSurface(QWindow* window)
{
    window->setSurfaceType(QWindow::OpenGLSurface);
    window->setFormat(QSurfaceFormat::defaultFormat());
}

void GLViewportPresenter::SetFrameSource(std::function<uint32_t()> provider)
{
    m_ColorTextureProvider = std::move(provider);
}

void GLViewportPresenter::EnsureGLResourcesInitialized(QWindow* target)
{
    if (m_GLResourcesInitialized)
        return;

#ifdef _WIN32
    // The engine's real native context must be current on this thread right now - Editor's tick
    // timer calls Application::Tick() (which re-asserts it via Window::MakeContextCurrent())
    // immediately before calling Render(), which is the only caller of this method.
    HGLRC engineContext = wglGetCurrentContext();
    MT_ASSERT(engineContext, "GLViewportPresenter: engine GL context must be current before first Render()");

    HWND engineHwnd = static_cast<HWND>(SDL_GetPointerProperty(
        SDL_GetWindowProperties(m_EngineWindow.GetNativeWindow()), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    MT_ASSERT(engineHwnd, "GLViewportPresenter: failed to get engine window's native HWND");

    // Wraps the engine's foreign context purely to give Qt something to share against below - Qt
    // takes no ownership of engineContext itself (SDLWindow still owns and destroys it).
    m_SharedContextWrapper.reset(QNativeInterface::QWGLContext::fromNative(engineContext, engineHwnd));
    MT_ASSERT(m_SharedContextWrapper, "GLViewportPresenter: QWGLContext::fromNative failed");
#else
    // The engine's real native context must be current on this thread right now - same
    // precondition as the WGL branch above (Application::Tick() re-asserts it via
    // Window::MakeContextCurrent() immediately before calling Render()). It's an EGL context and
    // not a GLXContext only because main.cpp forces SDL onto EGL via SDL_HINT_VIDEO_FORCE_EGL
    // before SDL_Init() - this vcpkg-built qtbase has no GLX at all (see MatchaEditor/CMakeLists.txt's
    // qt_import_plugins() comment), so a GLX context would give Qt nothing to wrap.
    EGLContext engineContext = eglGetCurrentContext();
    MT_ASSERT(engineContext != EGL_NO_CONTEXT, "GLViewportPresenter: engine GL context must be current before first Render()");

    EGLDisplay engineDisplay = eglGetCurrentDisplay();
    MT_ASSERT(engineDisplay != EGL_NO_DISPLAY, "GLViewportPresenter: failed to get engine's current EGL display");

    // Unlike QWGLContext::fromNative(), EGL's wrapper needs no window handle - an EGLContext isn't
    // tied to a drawable the way a WGL context is tied to the HDC it was created against, so
    // there's no equivalent of the HWND lookup in the _WIN32 branch above.
    m_SharedContextWrapper.reset(QNativeInterface::QEGLContext::fromNative(engineContext, engineDisplay));
    MT_ASSERT(m_SharedContextWrapper, "GLViewportPresenter: QEGLContext::fromNative failed");
#endif

    m_Context = std::make_unique<QOpenGLContext>();
    m_Context->setFormat(QSurfaceFormat::defaultFormat());
    m_Context->setShareContext(m_SharedContextWrapper.get());

    bool created = m_Context->create();
    MT_ASSERT(created, "GLViewportPresenter: failed to create shared Qt GL context");

    m_Context->makeCurrent(target);

    // Deliberately NOT reloading glad here. glad's function pointers are process-wide globals, so
    // re-resolving them against this context would replace the ones the engine loaded against its
    // own context at startup - and any that Qt resolves differently (or to null) then get called by
    // the engine's own rendering. That failure mode is vicious: GL 1.1 entry points like glClear()
    // come straight out of opengl32.dll and keep working, so the viewport still clears correctly,
    // while the modern entry points the mesh path needs segfault the moment a scene with actual
    // geometry is drawn. Both contexts come from the same driver and share an object namespace, so
    // the engine's pointers are valid here too.
    unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, kBlitVertexSource);
    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, kBlitFragmentSource);

    m_BlitShaderProgram = glCreateProgram();
    glAttachShader(m_BlitShaderProgram, vertexShader);
    glAttachShader(m_BlitShaderProgram, fragmentShader);
    glLinkProgram(m_BlitShaderProgram);

    int linked = 0;
    glGetProgramiv(m_BlitShaderProgram, GL_LINK_STATUS, &linked);
    MT_ASSERT(linked, "GLViewportPresenter: blit shader failed to link");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(m_BlitShaderProgram);
    glUniform1i(glGetUniformLocation(m_BlitShaderProgram, "u_Texture"), 0);
    glUseProgram(0);

    glGenVertexArrays(1, &m_QuadVertexArray);
    glGenBuffers(1, &m_QuadVertexBuffer);

    glBindVertexArray(m_QuadVertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVertices), kQuadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));

    glBindVertexArray(0);

    // Verifies GL object sharing actually took effect, rather than assuming it. Qt's create() can
    // succeed while silently falling back to an *unshared* context if the underlying platform call
    // (wglShareLists() on Windows, eglCreateContext()'s share_context on Linux) fails - and the
    // only visible symptom would be a uniformly black viewport, which is easy to mistake for an
    // empty scene. The engine's FrameBuffer color attachment is a texture created in the engine's
    // context; texture objects are shared across a share group on either platform, so its name is
    // valid here if and only if sharing worked.
    if (m_ColorTextureProvider && !glIsTexture(m_ColorTextureProvider()))
        MT_CORE_ERROR("GLViewportPresenter: GL sharing failed - the engine's framebuffer texture does not exist in this "
                      "context, so the viewport will render black. Context sharing is unsupported here; this needs a CPU "
                      "readback bridge instead.");

    m_GLResourcesInitialized = true;
}

void GLViewportPresenter::Render(QWindow* target)
{
    m_Target = target;

    EnsureGLResourcesInitialized(target);

    if (!m_Context->makeCurrent(target))
        MT_CORE_ERROR("GLViewportPresenter: QOpenGLContext::makeCurrent() failed (native handle likely recreated by a dock/float operation)");

    qreal dpr = target->devicePixelRatio();
    glViewport(0, 0, static_cast<int>(target->width() * dpr), static_cast<int>(target->height() * dpr));
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_BlitShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_ColorTextureProvider ? m_ColorTextureProvider() : 0);
    glBindVertexArray(m_QuadVertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    m_Context->swapBuffers(target);

    // Hand the engine's context back before returning. Everything else in the editor runs between
    // ticks, from Qt callbacks (a menu action creating a primitive mesh, compiling a shader, an
    // asset import), and all of it creates GL resources assuming "the" context is current - which
    // was trivially true when the viewport widget owned the only context in the process. Leaving
    // this presenter's blit context current instead silently breaks that: buffers/textures/programs
    // would still work (they're shared across the group), but vertex array objects specifically are
    // NOT shared, so a mesh created from a menu action would build its VAO here and then be
    // invisible - or crash the driver - when the engine tried to draw it in its own context.
    m_EngineWindow.MakeContextCurrent();
}
}  // namespace MatchaEditor
