#include "QtViewportWidget.h"
#include "Core/Assert.h"
#include "QtInput.h"
#include "QtKeyCodeMap.h"

#include <glad/glad.h>

#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QWheelEvent>

#include <optional>

namespace Matcha
{
namespace
{
std::optional<Input::MouseButton> ToMouseButton(Qt::MouseButton button)
{
    switch (button)
    {
    case Qt::LeftButton:
        return Input::MouseButton::Left;
    case Qt::MiddleButton:
        return Input::MouseButton::Middle;
    case Qt::RightButton:
        return Input::MouseButton::Right;
    case Qt::BackButton:
        return Input::MouseButton::Back;
    case Qt::ForwardButton:
        return Input::MouseButton::Forward;
    default:
        return std::nullopt;
    }
}
}  // namespace

QtViewportWidget::QtViewportWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void QtViewportWidget::SetContextReadyCallback(std::function<void()> callback)
{
    m_ContextReadyCallback = std::move(callback);
}

void QtViewportWidget::SetTickCallback(std::function<void()> callback)
{
    m_TickCallback = std::move(callback);
}

void QtViewportWidget::SetEventDispatch(std::function<void(const Event&)> dispatch)
{
    m_EventDispatch = std::move(dispatch);
}

void QtViewportWidget::SetInput(QtInput* input)
{
    m_Input = input;
}

void QtViewportWidget::SetCursorLocked(bool locked)
{
    m_CursorLocked = locked;

    if (locked)
    {
        setCursor(Qt::BlankCursor);

        QPoint center = rect().center();
        QCursor::setPos(mapToGlobal(center));

        m_LastMousePosition = center;
        m_HasLastMousePosition = true;
    }
    else
    {
        unsetCursor();
    }
}

void QtViewportWidget::initializeGL()
{
    static const auto loader = [](const char* name) -> void* {
        return reinterpret_cast<void*>(QOpenGLContext::currentContext()->getProcAddress(name));
    };

    int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(+loader));

    MT_ASSERT(status, "Failed to load GL functions via Qt's context");

    if (m_ContextReadyCallback)
        m_ContextReadyCallback();
}

void QtViewportWidget::resizeGL(int width, int height)
{
    if (m_EventDispatch)
        m_EventDispatch(Event{.type = EventType::WindowResized, .width = width, .height = height});
}

void QtViewportWidget::paintGL()
{
    if (m_TickCallback)
        m_TickCallback();
}

void QtViewportWidget::PollCursorLock()
{
    // Deliberately not computed from mouseMoveEvent() deltas while locked: Qt coalesces rapid
    // consecutive QEvent::MouseMove events by default, and a real move immediately followed by
    // the synthetic move our own re-centering warp generates can merge into one delivered event
    // reporting only the final (already-recentered) position - making every per-event delta read
    // as zero regardless of how far the mouse actually moved. Polling the OS cursor position
    // directly once per frame sidesteps that entirely: whatever Qt did or didn't coalesce along
    // the way, this reads the real current offset from center.
    //
    // Called from QtWindow::PumpEvents(), which Application::Tick() invokes right after
    // Input::Update() resets the mouse axis and before Update()/OnUpdate() read it via
    // GetAxis() - not from paintGL() (which runs Tick() itself, Input::Update() included), or
    // the delta dispatched here would be wiped by that same frame's Update() before anything
    // ever saw it.
    if (!m_CursorLocked)
        return;

    QPoint center = rect().center();
    QPoint globalCenter = mapToGlobal(center);
    QPoint delta = QCursor::pos() - globalCenter;

    if (m_EventDispatch && (delta.x() != 0 || delta.y() != 0))
    {
        m_EventDispatch(Event{.type = EventType::MouseMoved, .x = static_cast<float>(delta.x()), .y = static_cast<float>(delta.y())});
        QCursor::setPos(globalCenter);
    }

    m_LastMousePosition = center;
    m_HasLastMousePosition = true;
}

void QtViewportWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    if (m_Input)
        if (std::optional<KeyCode> code = ToKeyCode(event->key()))
            m_Input->PushKeyDown(*code);

    QOpenGLWidget::keyPressEvent(event);
}

void QtViewportWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    if (m_Input)
        if (std::optional<KeyCode> code = ToKeyCode(event->key()))
            m_Input->PushKeyUp(*code);

    QOpenGLWidget::keyReleaseEvent(event);
}

void QtViewportWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_Input)
        if (std::optional<Input::MouseButton> button = ToMouseButton(event->button()))
            m_Input->PushMouseButtonDown(*button);

    setFocus();

    QOpenGLWidget::mousePressEvent(event);
}

void QtViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_Input)
        if (std::optional<Input::MouseButton> button = ToMouseButton(event->button()))
            m_Input->PushMouseButtonUp(*button);

    QOpenGLWidget::mouseReleaseEvent(event);
}

void QtViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    // While locked, the delta is computed once per frame in paintGL() instead (see there for
    // why) - this only tracks/dispatches the unlocked case here.
    if (!m_CursorLocked)
    {
        QPoint position = event->pos();

        if (m_HasLastMousePosition && m_EventDispatch)
        {
            QPoint delta = position - m_LastMousePosition;

            if (delta.x() != 0 || delta.y() != 0)
                m_EventDispatch(Event{.type = EventType::MouseMoved, .x = static_cast<float>(delta.x()), .y = static_cast<float>(delta.y())});
        }

        m_LastMousePosition = position;
        m_HasLastMousePosition = true;
    }

    QOpenGLWidget::mouseMoveEvent(event);
}

void QtViewportWidget::focusOutEvent(QFocusEvent* event)
{
    // Once this widget loses keyboard focus (alt-tab, clicking another panel), Qt stops calling
    // keyReleaseEvent() on it entirely - a key released while focus is elsewhere never reaches
    // PushKeyUp(), so it would otherwise be considered "held" indefinitely (e.g. a stuck-moving
    // camera). Treat losing focus as "every key is up" instead - same fix SDLInput applies via
    // SDL_ResetKeyboard() on SDL_EVENT_WINDOW_FOCUS_LOST.
    if (m_Input)
        m_Input->ResetKeyboard();

    QOpenGLWidget::focusOutEvent(event);
}

void QtViewportWidget::wheelEvent(QWheelEvent* event)
{
    if (m_EventDispatch)
    {
        // Qt reports wheel motion in eighths of a degree; 120 per notch is the standard step.
        QPoint angleDelta = event->angleDelta();

        m_EventDispatch(Event{.type = EventType::MouseScrolled,
                               .x = static_cast<float>(angleDelta.x()) / 120.0f,
                               .y = static_cast<float>(angleDelta.y()) / 120.0f});
    }

    QOpenGLWidget::wheelEvent(event);
}
}  // namespace Matcha
