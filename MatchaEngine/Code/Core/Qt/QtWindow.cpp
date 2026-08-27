#include "QtWindow.h"
#include "QtInput.h"
#include "QtViewportWidget.h"

#include <glad/glad.h>

namespace Matcha
{
QtWindow::QtWindow(const WindowSpecification& spec, Input* input)
    : m_WindowSpec(spec)
{
    m_ViewportWidget = new QtViewportWidget();
    m_ViewportWidget->resize(spec.m_Width, spec.m_Height);
    m_ViewportWidget->setWindowTitle(QString::fromStdString(spec.m_Title));
    m_ViewportWidget->SetInput(dynamic_cast<QtInput*>(input));
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
    switch (evt.type)
    {
    case EventType::WindowResized:
        m_WindowSpec.m_Width = evt.width;
        m_WindowSpec.m_Height = evt.height;
        glViewport(0, 0, m_WindowSpec.m_Width, m_WindowSpec.m_Height);
        break;
    default:
        break;
    }
}

void QtWindow::SetEventDispatch(std::function<void(const Event&)> dispatch)
{
    m_ViewportWidget->SetEventDispatch(std::move(dispatch));
}

void QtWindow::PumpEvents()
{
    // No-op: Qt already delivered input via the viewport widget's own callbacks (using the same
    // dispatch registered above) before this is ever called - resize/mouse-move/scroll events are
    // dispatched directly from resizeGL()/mouseMoveEvent()/wheelEvent().
}

void QtWindow::SetContextReadyCallback(std::function<void()> callback)
{
    m_ViewportWidget->SetContextReadyCallback(std::move(callback));
}

void QtWindow::SetTickCallback(std::function<void()> callback)
{
    m_ViewportWidget->SetTickCallback(std::move(callback));
}

int QtWindow::GetWidth() const
{
    return m_WindowSpec.m_Width;
}

int QtWindow::GetHeight() const
{
    return m_WindowSpec.m_Height;
}

Vector2Int QtWindow::GetCenter() const
{
    return Vector2Int(m_WindowSpec.m_Width / 2, m_WindowSpec.m_Height / 2);
}

float QtWindow::GetAspectRatio() const
{
    return static_cast<float>(m_WindowSpec.m_Width) / m_WindowSpec.m_Height;
}

const Window::WindowSpecification& QtWindow::GetWindowSpecification() const
{
    return m_WindowSpec;
}

bool QtWindow::IsMinimized() const
{
    return !m_ViewportWidget->isVisible();
}
}  // namespace Matcha
