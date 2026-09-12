#pragma once

#include <cstdint>
#include <functional>

class QWindow;

namespace MatchaEditor
{
// Displays the engine's rendered frame inside a Qt-owned QWindow, using whichever graphics API
// backs the engine's own RendererAPI - GLViewportPresenter today, a future VulkanViewportPresenter
// alongside a VulkanRendererAPI. EngineViewportWidget owns one of these through this interface, so
// it never calls a single graphics-API function itself - the same seam RendererAPI already draws
// between Renderer/ResourceManager and GLRendererAPI, just one layer further out, at the point
// where the engine's finished frame meets Qt's own window.
class ViewportPresenter
{
public:
    virtual ~ViewportPresenter() = default;

    // Called once, from EngineViewportWidget's constructor, before the window is ever shown -
    // configures `window`'s surface type/format for whichever graphics API this presenter uses
    // (QWindow::OpenGLSurface + a QSurfaceFormat for GLViewportPresenter; QWindow::VulkanSurface
    // for a future Vulkan one). Has to happen up front: Qt requires the surface type set before
    // the underlying native window is created, which showing the window (or querying most of its
    // properties) does implicitly.
    virtual void ConfigureSurface(QWindow* window) = 0;

    // Gives the presenter a way to ask "what should I display right now" each Render(), without
    // it needing to know about Editor/FrameBuffer directly - the handle can change across a
    // FrameBuffer::Invalidate() (e.g. on resize), so it's queried fresh every frame rather than
    // cached. The handle's meaning is backend-specific (a GL texture name for
    // GLViewportPresenter) - opaque here, the same convention FrameBuffer::GetColorAttachmentID()
    // already uses at the RendererAPI seam.
    virtual void SetFrameSource(std::function<uint32_t()> provider) = 0;

    // Displays the current frame into `target`, called once per Editor tick from
    // EngineViewportWidget::Render(). Guaranteed non-null and exposed - EngineViewportWidget
    // checks isExposed() itself before calling this.
    virtual void Render(QWindow* target) = 0;
};
}  // namespace MatchaEditor
