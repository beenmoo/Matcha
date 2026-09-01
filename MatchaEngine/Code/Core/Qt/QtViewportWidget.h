#pragma once

#include "Core/Event.h"

#include <QOpenGLWidget>

#include <functional>

namespace Matcha
{
class QtInput;

class QtViewportWidget final : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit QtViewportWidget(QWidget* parent = nullptr);

    void SetContextReadyCallback(std::function<void()> callback);
    void SetTickCallback(std::function<void()> callback);
    void SetEventDispatch(std::function<void(const Event&)> dispatch);
    void SetInput(QtInput* input);

    // Qt has no built-in relative mouse mode (unlike SDL's SDL_SetWindowRelativeMouseMode): this
    // hides the cursor and, on every subsequent move, warps it back to the widget's center after
    // computing the delta - keeps mouse-look working indefinitely without running out of screen.
    void SetCursorLocked(bool locked);

    // Called by QtWindow::PumpEvents() once per frame, at the point Application::Tick() polls
    // for input - a no-op unless the cursor is locked. See the .cpp for why this can't just be
    // computed from mouseMoveEvent() deltas.
    void PollCursorLock();

protected:
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    std::function<void()> m_ContextReadyCallback;
    std::function<void()> m_TickCallback;
    std::function<void(const Event&)> m_EventDispatch;
    QtInput* m_Input = nullptr;

    QPoint m_LastMousePosition;
    bool m_HasLastMousePosition = false;
    bool m_CursorLocked = false;
};
}  // namespace Matcha
