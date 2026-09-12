#pragma once

#include "ViewportPresenter.h"

#include <cstdint>
#include <functional>
#include <memory>

class QOpenGLContext;

namespace Matcha
{
class SDLWindow;
}

namespace MatchaEditor
{
// The only ViewportPresenter today: blits the engine's FrameBuffer color texture onto a
// fullscreen quad using a small QOpenGLContext of its own, created sharing GL objects with the
// engine's real SDL-owned context (see EnsureGLResourcesInitialized()). Qt can destroy and
// recreate this tiny context (a shader + a quad VAO) as often as ADS's docking system likes when
// the panel is dragged/floated/redocked - the real engine context and every GPU resource on it
// (meshes, shaders, textures) live in SDL's context, which this never touches, so redocking can't
// break rendering the way it could when the old QtViewportWidget's own context WAS the engine's
// real context.
class GLViewportPresenter final : public ViewportPresenter
{
public:
    explicit GLViewportPresenter(Matcha::SDLWindow& engineWindow);
    ~GLViewportPresenter() override;

    void ConfigureSurface(QWindow* window) override;
    void SetFrameSource(std::function<uint32_t()> provider) override;
    void Render(QWindow* target) override;

private:
    // Lazily, on first successful Render(): wraps the engine's real native GL context (current on
    // this thread at that point - see Render()'s ordering requirement) via Qt's native-interface
    // adapter purely to establish a share group, then creates this presenter's own QOpenGLContext
    // sharing against that wrapper, and compiles the blit shader/quad. See the .cpp for the exact
    // handshake - WGL on Windows, EGL on Linux (this qtbase build has no GLX; see
    // MatchaEditor/CMakeLists.txt's qt_import_plugins() comment).
    void EnsureGLResourcesInitialized(QWindow* target);

private:
    Matcha::SDLWindow& m_EngineWindow;

    // The window last passed to Render() - kept only so the destructor has something to make
    // m_Context current against for GL teardown (QOpenGLContext::makeCurrent() needs a target).
    QWindow* m_Target = nullptr;

    // m_SharedContextWrapper wraps the engine's foreign native context purely to give m_Context
    // something to share against - kept alive for m_Context's whole lifetime since a
    // QOpenGLContext's share context must remain valid for as long as the sharing context does.
    std::unique_ptr<QOpenGLContext> m_SharedContextWrapper;
    std::unique_ptr<QOpenGLContext> m_Context;
    bool m_GLResourcesInitialized = false;

    unsigned int m_BlitShaderProgram = 0;
    unsigned int m_QuadVertexArray = 0;
    unsigned int m_QuadVertexBuffer = 0;
    std::function<uint32_t()> m_ColorTextureProvider;
};
}  // namespace MatchaEditor
