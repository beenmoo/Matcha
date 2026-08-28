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
    QPoint position = event->pos();

    if (m_HasLastMousePosition && m_EventDispatch)
    {
        QPoint delta = position - m_LastMousePosition;

        if (delta.x() != 0 || delta.y() != 0)
            m_EventDispatch(Event{.type = EventType::MouseMoved, .x = static_cast<float>(delta.x()), .y = static_cast<float>(delta.y())});
    }

    if (m_CursorLocked)
    {
        // Warp back to center after every move so there's always room left to move into - the
        // resulting synthetic move event Qt delivers for the warp itself computes a (0,0) delta
        // against the m_LastMousePosition set here, so it's a correctly-filtered no-op above.
        QPoint center = rect().center();

        if (position != center)
            QCursor::setPos(mapToGlobal(center));

        m_LastMousePosition = center;
    }
    else
    {
        m_LastMousePosition = position;
    }

    m_HasLastMousePosition = true;

    QOpenGLWidget::mouseMoveEvent(event);
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
