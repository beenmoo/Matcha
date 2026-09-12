#include "EngineViewportWidget.h"
#include "GLViewportPresenter.h"
#include "Core/SDL/SDLWindow.h"

#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QWheelEvent>

#include <optional>

namespace MatchaEditor
{
namespace
{
// Translates a Qt::Key value (from QKeyEvent::key()) to Matcha's KeyCode. KeyCode's values are
// literal SDL scancodes, which have no numeric relationship to Qt's key values, so this is an
// explicit table rather than a conversion. Returns std::nullopt for keys with no mapping (e.g.
// exotic media keys) - callers should just drop those, not assert. Ported as-is from the deleted
// MatchaEngine/Code/Core/Qt/QtKeyCodeMap.cpp, now editor-only since the engine no longer knows
// about Qt at all.
std::optional<Matcha::KeyCode> ToKeyCode(int qtKey)
{
    using Matcha::KeyCode;

    switch (qtKey)
    {
    case Qt::Key_A:
        return KeyCode::A;
    case Qt::Key_B:
        return KeyCode::B;
    case Qt::Key_C:
        return KeyCode::C;
    case Qt::Key_D:
        return KeyCode::D;
    case Qt::Key_E:
        return KeyCode::E;
    case Qt::Key_F:
        return KeyCode::F;
    case Qt::Key_G:
        return KeyCode::G;
    case Qt::Key_H:
        return KeyCode::H;
    case Qt::Key_I:
        return KeyCode::I;
    case Qt::Key_J:
        return KeyCode::J;
    case Qt::Key_K:
        return KeyCode::K;
    case Qt::Key_L:
        return KeyCode::L;
    case Qt::Key_M:
        return KeyCode::M;
    case Qt::Key_N:
        return KeyCode::N;
    case Qt::Key_O:
        return KeyCode::O;
    case Qt::Key_P:
        return KeyCode::P;
    case Qt::Key_Q:
        return KeyCode::Q;
    case Qt::Key_R:
        return KeyCode::R;
    case Qt::Key_S:
        return KeyCode::S;
    case Qt::Key_T:
        return KeyCode::T;
    case Qt::Key_U:
        return KeyCode::U;
    case Qt::Key_V:
        return KeyCode::V;
    case Qt::Key_W:
        return KeyCode::W;
    case Qt::Key_X:
        return KeyCode::X;
    case Qt::Key_Y:
        return KeyCode::Y;
    case Qt::Key_Z:
        return KeyCode::Z;

    case Qt::Key_0:
        return KeyCode::ZERO;
    case Qt::Key_1:
        return KeyCode::ONE;
    case Qt::Key_2:
        return KeyCode::TWO;
    case Qt::Key_3:
        return KeyCode::THREE;
    case Qt::Key_4:
        return KeyCode::FOUR;
    case Qt::Key_5:
        return KeyCode::FIVE;
    case Qt::Key_6:
        return KeyCode::SIX;
    case Qt::Key_7:
        return KeyCode::SEVEN;
    case Qt::Key_8:
        return KeyCode::EIGHT;
    case Qt::Key_9:
        return KeyCode::NINE;

    case Qt::Key_F1:
        return KeyCode::F1;
    case Qt::Key_F2:
        return KeyCode::F2;
    case Qt::Key_F3:
        return KeyCode::F3;
    case Qt::Key_F4:
        return KeyCode::F4;
    case Qt::Key_F5:
        return KeyCode::F5;
    case Qt::Key_F6:
        return KeyCode::F6;
    case Qt::Key_F7:
        return KeyCode::F7;
    case Qt::Key_F8:
        return KeyCode::F8;
    case Qt::Key_F9:
        return KeyCode::F9;
    case Qt::Key_F10:
        return KeyCode::F10;
    case Qt::Key_F11:
        return KeyCode::F11;
    case Qt::Key_F12:
        return KeyCode::F12;

    case Qt::Key_Left:
        return KeyCode::LEFT;
    case Qt::Key_Right:
        return KeyCode::RIGHT;
    case Qt::Key_Up:
        return KeyCode::UP;
    case Qt::Key_Down:
        return KeyCode::DOWN;

    case Qt::Key_Shift:
        return KeyCode::LSHIFT;
    case Qt::Key_Control:
        return KeyCode::LCTRL;
    case Qt::Key_Alt:
        return KeyCode::LALT;
    case Qt::Key_Meta:
        return KeyCode::LGUI;

    case Qt::Key_Escape:
        return KeyCode::ESCAPE;
    case Qt::Key_Tab:
        return KeyCode::TAB;
    case Qt::Key_Backspace:
        return KeyCode::BACKSPACE;
    case Qt::Key_Return:
        return KeyCode::RETURN;
    case Qt::Key_Enter:
        return KeyCode::KP_ENTER;
    case Qt::Key_Space:
        return KeyCode::SPACE;

    case Qt::Key_CapsLock:
        return KeyCode::CAPSLOCK;
    case Qt::Key_NumLock:
        return KeyCode::NUMLOCKCLEAR;
    case Qt::Key_ScrollLock:
        return KeyCode::SCROLLLOCK;
    case Qt::Key_Print:
        return KeyCode::PRINTSCREEN;
    case Qt::Key_Pause:
        return KeyCode::PAUSE;
    case Qt::Key_Insert:
        return KeyCode::INSERT;
    case Qt::Key_Delete:
        return KeyCode::DEL;
    case Qt::Key_Home:
        return KeyCode::HOME;
    case Qt::Key_End:
        return KeyCode::END;
    case Qt::Key_PageUp:
        return KeyCode::PAGEUP;
    case Qt::Key_PageDown:
        return KeyCode::PAGEDOWN;

    case Qt::Key_Minus:
        return KeyCode::MINUS;
    case Qt::Key_Equal:
        return KeyCode::EQUALS;
    case Qt::Key_BracketLeft:
        return KeyCode::LEFTBRACKET;
    case Qt::Key_BracketRight:
        return KeyCode::RIGHTBRACKET;
    case Qt::Key_Backslash:
        return KeyCode::BACKSLASH;
    case Qt::Key_Semicolon:
        return KeyCode::SEMICOLON;
    case Qt::Key_Apostrophe:
        return KeyCode::APOSTROPHE;
    case Qt::Key_QuoteLeft:
        return KeyCode::GRAVE;
    case Qt::Key_Comma:
        return KeyCode::COMMA;
    case Qt::Key_Period:
        return KeyCode::PERIOD;
    case Qt::Key_Slash:
        return KeyCode::SLASH;

    default:
        return std::nullopt;
    }
}

std::optional<Matcha::MouseButton> ToMouseButton(Qt::MouseButton button)
{
    using Matcha::MouseButton;

    switch (button)
    {
    case Qt::LeftButton:
        return MouseButton::Left;
    case Qt::MiddleButton:
        return MouseButton::Middle;
    case Qt::RightButton:
        return MouseButton::Right;
    case Qt::BackButton:
        return MouseButton::Back;
    case Qt::ForwardButton:
        return MouseButton::Forward;
    default:
        return std::nullopt;
    }
}

}  // namespace

EngineViewportWidget::EngineViewportWidget(Matcha::SDLWindow& engineWindow, QWindow* parent)
    : QWindow(parent),
      m_EngineWindow(engineWindow),
      m_Presenter(std::make_unique<GLViewportPresenter>(engineWindow))
{
    m_Presenter->ConfigureSurface(this);

    m_EngineWindow.SetExternalPumpCallback([this] {
        PollCursorLock();
        PollScrollDelta();
    });
}

// Defined here (not defaulted inline in the header) even though it does nothing itself: the
// header only forward-declares ViewportPresenter, and std::unique_ptr<ViewportPresenter>'s
// destructor needs the complete type (visible here via "GLViewportPresenter.h"'s own include of
// "ViewportPresenter.h") to call it virtually.
EngineViewportWidget::~EngineViewportWidget() = default;

void EngineViewportWidget::SetColorTextureProvider(std::function<uint32_t()> provider)
{
    m_Presenter->SetFrameSource(std::move(provider));
}

void EngineViewportWidget::Render()
{
    if (!isExposed())
        return;

    m_Presenter->Render(this);
}

void EngineViewportWidget::resizeEvent(QResizeEvent* event)
{
    QWindow::resizeEvent(event);

    qreal dpr = devicePixelRatio();
    m_EngineWindow.DispatchExternalEvent(Matcha::Event{.type = Matcha::EventType::WindowResized,
                                                       .width = static_cast<int32_t>(width() * dpr),
                                                       .height = static_cast<int32_t>(height() * dpr)});
}

void EngineViewportWidget::SetCursorLocked(bool locked)
{
    m_CursorLocked = locked;

    if (locked)
    {
        setCursor(Qt::BlankCursor);

        QPoint center(width() / 2, height() / 2);
        QCursor::setPos(mapToGlobal(center));

        m_LastMousePosition = center;
        m_HasLastMousePosition = true;
    }
    else
    {
        unsetCursor();
    }
}

void EngineViewportWidget::PollCursorLock()
{
    if (!m_CursorLocked)
        return;

    QPoint center(width() / 2, height() / 2);
    QPoint globalCenter = mapToGlobal(center);
    QPoint delta = QCursor::pos() - globalCenter;

    if (delta.x() != 0 || delta.y() != 0)
    {
        m_EngineWindow.DispatchExternalEvent(
            Matcha::Event{.type = Matcha::EventType::MouseMoved, .x = static_cast<float>(delta.x()), .y = static_cast<float>(delta.y())});
        QCursor::setPos(globalCenter);
    }

    m_LastMousePosition = center;
    m_HasLastMousePosition = true;
}

void EngineViewportWidget::PollScrollDelta()
{
    if (m_AccumulatedScrollDeltaX == 0.0f && m_AccumulatedScrollDeltaY == 0.0f)
        return;

    m_EngineWindow.DispatchExternalEvent(
        Matcha::Event{.type = Matcha::EventType::MouseScrolled, .x = m_AccumulatedScrollDeltaX, .y = m_AccumulatedScrollDeltaY});

    m_AccumulatedScrollDeltaX = 0.0f;
    m_AccumulatedScrollDeltaY = 0.0f;
}

void EngineViewportWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    if (std::optional<Matcha::KeyCode> code = ToKeyCode(event->key()))
        m_EngineWindow.DispatchExternalEvent(Matcha::Event{.type = Matcha::EventType::KeyDown, .key = *code});
}

void EngineViewportWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->isAutoRepeat())
        return;

    if (std::optional<Matcha::KeyCode> code = ToKeyCode(event->key()))
        m_EngineWindow.DispatchExternalEvent(Matcha::Event{.type = Matcha::EventType::KeyUp, .key = *code});
}

void EngineViewportWidget::mousePressEvent(QMouseEvent* event)
{
    m_AbsoluteMousePosition = event->pos();

    if (event->button() == Qt::RightButton)
        m_RightButtonHeld = true;

    if (std::optional<Matcha::MouseButton> button = ToMouseButton(event->button()))
        m_EngineWindow.DispatchExternalEvent(Matcha::Event{.type = Matcha::EventType::MouseButtonDown, .mouseButton = *button});

    // Left-click only, and only when RMB isn't already held: RMB-drag is camera fly-look
    // (CameraController), a higher-priority interaction this shouldn't also fire a pick/gizmo
    // click underneath.
    if (event->button() == Qt::LeftButton && !m_RightButtonHeld)
        emit Clicked(event->pos(), event->button());

    requestActivate();
}

void EngineViewportWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
        m_RightButtonHeld = false;

    if (std::optional<Matcha::MouseButton> button = ToMouseButton(event->button()))
        m_EngineWindow.DispatchExternalEvent(Matcha::Event{.type = Matcha::EventType::MouseButtonUp, .mouseButton = *button});

    if (event->button() == Qt::LeftButton)
        emit Released(event->button());
}

void EngineViewportWidget::mouseMoveEvent(QMouseEvent* event)
{
    m_AbsoluteMousePosition = event->pos();

    // While locked, the delta is computed once per frame in PollCursorLock() instead (see its own
    // comment for why) - this only tracks/dispatches the unlocked case here.
    if (!m_CursorLocked)
    {
        QPoint position = event->pos();

        if (m_HasLastMousePosition)
        {
            QPoint delta = position - m_LastMousePosition;

            if (delta.x() != 0 || delta.y() != 0)
                m_EngineWindow.DispatchExternalEvent(
                    Matcha::Event{.type = Matcha::EventType::MouseMoved, .x = static_cast<float>(delta.x()), .y = static_cast<float>(delta.y())});
        }

        m_LastMousePosition = position;
        m_HasLastMousePosition = true;
    }
}

void EngineViewportWidget::focusOutEvent(QFocusEvent* /*event*/)
{
    // Once this widget loses keyboard focus (alt-tab, clicking another panel), Qt stops delivering
    // keyReleaseEvent() to it entirely - a key released while focus is elsewhere never reaches
    // KeyUp, so it would otherwise be considered "held" indefinitely (e.g. a stuck-moving camera).
    // SDLInput::ProcessEvents() treats WindowFocusLost as "every key is up".
    m_EngineWindow.DispatchExternalEvent(Matcha::Event{.type = Matcha::EventType::WindowFocusLost});
}

void EngineViewportWidget::wheelEvent(QWheelEvent* event)
{
    // Accumulate only - do not dispatch here. See PollScrollDelta()'s comment for why a direct
    // dispatch from here is lost almost every time for an event this infrequent.
    QPoint angleDelta = event->angleDelta();

    m_AccumulatedScrollDeltaX += static_cast<float>(angleDelta.x()) / 120.0f;
    m_AccumulatedScrollDeltaY += static_cast<float>(angleDelta.y()) / 120.0f;
}
}  // namespace MatchaEditor
