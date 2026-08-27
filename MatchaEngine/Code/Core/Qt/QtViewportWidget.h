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

private:
    std::function<void()> m_ContextReadyCallback;
    std::function<void()> m_TickCallback;
    std::function<void(const Event&)> m_EventDispatch;
    QtInput* m_Input = nullptr;

    QPoint m_LastMousePosition;
    bool m_HasLastMousePosition = false;
};
}  // namespace Matcha
