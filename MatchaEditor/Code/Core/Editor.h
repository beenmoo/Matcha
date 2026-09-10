#pragma once

#include "EditorCamera.h"

#include <Matcha.h>

#include <memory>
#include <optional>

class QTimer;

namespace Matcha
{
class SDLWindow;
}  // namespace Matcha

namespace MatchaEditor
{
class EditorMainWindow;
class EngineViewportWidget;

class Editor : public Application
{
public:
    explicit Editor(const Application::ApplicationSpecification& spec);
    ~Editor() override;

    void Show();

protected:
    void OnUpdate() override;
    void OnEvent(const Event& event) override;
    void RenderCamera() override;
    void OnPostRender() override;

private:
    std::unique_ptr<EditorMainWindow> m_MainWindow;
    std::unique_ptr<QTimer> m_TickTimer;
    EditorCamera m_EditorCamera;

    // The engine renders the whole scene (plus the ImGuizmo overlay) into this every frame, kept
    // continuously bound for the entire session - EngineViewportWidget displays its color texture
    // separately, in its own Qt-owned GL context. See RenderCamera()/OnEvent()'s WindowResized case.
    std::unique_ptr<FrameBuffer> m_ViewportFrameBuffer;

    // Not owned here (raw pointer): once QWidget::createWindowContainer() embeds it into
    // EditorMainWindow's dock layout, Qt's own parent/child tree owns and deletes it - same
    // reasoning the old QtWindow::GetViewportWidget() comment documented. Guaranteed to be
    // destroyed before the SDLWindow it references (an Application base-class member, so
    // destroyed after every one of Editor's own members - see Application.h).
    EngineViewportWidget* m_ViewportWidget = nullptr;

    // Set at the very top of ~Editor(), before any member is torn down - m_MainWindow's own
    // destructor (which runs after this class's body, but is what actually destroys the Qt widget
    // tree, EngineViewportWidget included) can synchronously fire cascading resize/hide events as
    // it collapses the layout, reaching OnEvent() while this Editor object is technically still
    // alive but partway through destruction. Without this guard, a resize arriving that way would
    // touch m_ViewportFrameBuffer/m_EditorCamera - both already destroyed by then, since members
    // are torn down in reverse declaration order and both are declared above m_MainWindow.
    bool m_ShuttingDown = false;

    // Last viewport size reported by a WindowResized event, applied to m_ViewportFrameBuffer at the
    // top of the next RenderCamera() rather than from OnEvent() directly.
    //
    // OnEvent() runs synchronously from Qt's own resizeEvent(), at an arbitrary point in Qt's event
    // loop where there is no guarantee about which GL context is current (EngineViewportWidget's
    // own blit context is current for most of the time between ticks, and during teardown there may
    // be none at all). Rebuilding the framebuffer there put its GL objects in whichever context
    // happened to be active - producing framebuffer handles that don't exist in the engine's own
    // context, which surfaces as glCheckNamedFramebufferStatus() returning 0 (the call itself
    // erroring) rather than any real incompleteness. Deferring to RenderCamera() - reached only via
    // Tick(), which re-asserts the engine's context at its top - makes the context unambiguous, and
    // coalesces a drag's worth of resize events into one rebuild per frame instead of dozens.
    std::optional<Vector2Int> m_PendingViewportSize;
};
}  // namespace MatchaEditor
