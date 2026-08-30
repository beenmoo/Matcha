#include "QtWindow.h"
#include "QtInput.h"
#include "QtViewportWidget.h"

#include <glad/glad.h>

namespace Matcha
{
QtWindow::QtWindow(const WindowSpecification& spec, Input* input)
    : Window(spec)
{
    m_ViewportWidget = new QtViewportWidget();
    m_ViewportWidget->resize(spec.m_Width, spec.m_Height);
    m_ViewportWidget->setWindowTitle(QString::fromStdString(spec.m_Title));

    if (auto* qtInput = dynamic_cast<QtInput*>(input))
    {
        m_Input = qtInput;
        m_ViewportWidget->SetInput(qtInput);
        qtInput->SetViewportWidget(m_ViewportWidget);
    }
}

void QtWindow::Resize(int width, int height)
{
    m_ViewportWidget->resize(width, height);
}

void QtWindow::SwapBuffers()
{
    // Qt composites and presents the widget itself after paintGL() returns - nothing to do here.
}

void QtWindow::ProcessEvents(const Event& evt)
{
    HandleResizeEvent(evt);
}

void QtWindow::SetEventDispatch(std::function<void(const Event&)> dispatch)
{
    m_ViewportWidget->SetEventDispatch(std::move(dispatch));
}

void QtWindow::PumpEvents()
{
    // Resize/mouse-move/scroll events are otherwise dispatched directly from Qt's own
    // resizeGL()/mouseMoveEvent()/wheelEvent() callbacks, whenever Qt delivers them. The two
    // calls below are for the things that specifically need to happen at this point in
    // Application::Tick() - right after Input::Update() shifts current into prev, before
    // Update()/OnUpdate() read GetKeyDown()/GetMouseButtonDown()/the mouse-look delta - see
    // QtInput::ApplyPendingInput() and QtViewportWidget::PollCursorLock() for why.
    if (m_Input)
        m_Input->ApplyPendingInput();

    m_ViewportWidget->PollCursorLock();
}

void QtWindow::SetContextReadyCallback(std::function<void()> callback)
{
    m_ViewportWidget->SetContextReadyCallback(std::move(callback));
}

void QtWindow::SetTickCallback(std::function<void()> callback)
{
    m_ViewportWidget->SetTickCallback(std::move(callback));
}

void QtWindow::MakeContextCurrent()
{
    m_ViewportWidget->makeCurrent();
}

bool QtWindow::IsMinimized() const
{
    return !m_ViewportWidget->isVisible();
}
}  // namespace Matcha
