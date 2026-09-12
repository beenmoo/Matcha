#pragma once

#include <Matcha.h>

#include <QWindow>
#include <QPoint>

#include <cstdint>
#include <functional>
#include <memory>

namespace Matcha
{
class SDLWindow;
}

namespace MatchaEditor
{
class ViewportPresenter;

// Replaces the old Qt engine backend's QtViewportWidget/QtWindow pair. The engine itself now only
// ever knows about SDL (see SDLWindow's m_Headless mode) - it renders into its own FrameBuffer
// (owned by Editor), never into a Qt-managed surface at all. This class is purely a *display*: a
// QWindow that hands the actual "put the engine's frame on screen" work to a ViewportPresenter
// (GLViewportPresenter today, a future VulkanViewportPresenter alongside a VulkanRendererAPI) -
// this class itself calls no graphics-API function at all, only translating its own Qt input
// events into Matcha's cross-platform Event system (SDLWindow::DispatchExternalEvent) instead of
// a parallel Input subclass.
//
// The key benefit over the old architecture: Qt can destroy and recreate the presenter's tiny GL
// context (a blit shader + a quad VAO) as often as ADS's docking system likes when the panel is
// dragged/floated/redocked - the real engine context and every GPU resource on it (meshes, shaders,
// textures) live in SDL's context, which Qt never touches, so redocking can no longer break
// rendering the way it used to when QtViewportWidget's own QOpenGLWidget context WAS the engine's
// real context.
//
// A QWindow (not QOpenGLWidget) specifically because QOpenGLWidget gives no public way to inject a
// pre-built shared QOpenGLContext - it always creates its own internally, and neither direction of
// establishing the share group afterwards works here: repointing Qt's global share context strands
// Qt's own compositing contexts (created during QApplication construction, long before the engine
// exists) in a separate group, and SDL_GL_SHARE_WITH_CURRENT_CONTEXT consults SDL's own record of
// the current context, which never sees a Qt one. The cost of this choice is that embedding a
// QWindow forces native windows on the sibling dock areas, so Qt logs a "must be a top level
// window" warning whenever it tries to give a popup one of them as a transient parent - cosmetic
// (Qt just skips the transient parent), but noisy.
//
// Embedded into the ADS dock widget tree via QWidget::createWindowContainer() (see Editor).
class EngineViewportWidget : public QWindow
{
    Q_OBJECT

public:
    explicit EngineViewportWidget(Matcha::SDLWindow& engineWindow, QWindow* parent = nullptr);
    ~EngineViewportWidget() override;

    // Called once, from Editor's constructor: gives the presenter a way to ask "what should I
    // display right now" each Render() without needing to know about Editor/FrameBuffer directly -
    // see ViewportPresenter::SetFrameSource().
    void SetColorTextureProvider(std::function<uint32_t()> provider);

    // Called once per Editor tick, right after Application::Tick() - forwards to the presenter's
    // own Render(). A no-op until the window is actually exposed.
    void Render();

    // Called from SDLWindow's per-Tick external-pump hook (see SDLWindow::SetExternalPumpCallback,
    // registered in this class's constructor) - same reasoning and timing requirement as the old
    // QtViewportWidget::PollCursorLock()/PollScrollDelta(): both need to run after Input::Update()
    // resets per-frame deltas and before Update()/OnUpdate() reads them, which PumpEvents() is the
    // one place that's guaranteed true regardless of when Qt actually delivered the underlying
    // event.
    void PollCursorLock();
    void PollScrollDelta();

    // Qt has no built-in relative mouse mode - hides the cursor and warps it back to this widget's
    // center after computing each frame's delta, ported as-is from the old QtViewportWidget.
    void SetCursorLocked(bool locked);

    // Absolute widget-local position, tracked on every press/move - ImGui's IO needs it every
    // frame (see ViewportInteraction::BeginGizmoFrame).
    [[nodiscard]] QPoint GetMousePosition() const { return m_AbsoluteMousePosition; }

signals:
    // Left-click only, and only when RMB (camera fly-look) isn't already held - see the .cpp.
    void Clicked(QPoint localPos, Qt::MouseButton button);
    void Released(Qt::MouseButton button);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    Matcha::SDLWindow& m_EngineWindow;

    // The one place this class decides which graphics API backs the viewport - everywhere else
    // (Render(), SetColorTextureProvider()) just forwards through the abstract interface. See
    // ViewportPresenter's own comment for why this seam exists ahead of there being a second
    // implementation to select between.
    std::unique_ptr<ViewportPresenter> m_Presenter;

    QPoint m_LastMousePosition;
    bool m_HasLastMousePosition = false;
    bool m_CursorLocked = false;
    QPoint m_AbsoluteMousePosition;

    // Tracked locally rather than queried from Input: this widget has no access to Application's
    // protected GetInput(), and it already sees every mouse press/release itself anyway (it's the
    // one translating them into Matcha events in the first place).
    bool m_RightButtonHeld = false;

    // Accumulated by wheelEvent(), drained and dispatched by PollScrollDelta() - see its own
    // comment for why a direct dispatch from wheelEvent() itself would lose most single-notch
    // scroll events.
    float m_AccumulatedScrollDeltaX = 0.0f;
    float m_AccumulatedScrollDeltaY = 0.0f;
};
}  // namespace MatchaEditor
