#pragma once

#include "Core/Window.h"

namespace Matcha
{
class QtInput;
class QtViewportWidget;

class QtWindow final : public Window
{
public:
    explicit QtWindow(const WindowSpecification& spec, Input* input);
    ~QtWindow() override = default;

    void Resize(int width, int height) override;
    void SwapBuffers() override;
    void ProcessEvents(const Event& evt) override;
    void SetEventDispatch(std::function<void(const Event&)> dispatch) override;
    void PumpEvents() override;
    void SetContextReadyCallback(std::function<void()> callback) override;
    void SetTickCallback(std::function<void()> callback) override;
    void MakeContextCurrent() override;

    [[nodiscard]] bool IsMinimized() const override;

    // Qt-specific escape hatch: the editor needs to embed this into its own QMainWindow layout,
    // which isn't part of the abstract Window interface. Not owned here in the C++/unique_ptr
    // sense - once embedded (QMainWindow::setCentralWidget or similar), Qt's parent-child tree
    // owns and deletes it. QtWindow is guaranteed to be destroyed after the widget: Application's
    // members (and thus this Window) are base-class subobjects of Editor, so they're destroyed
    // after Editor's own members (which own the QMainWindow the widget gets reparented into).
    [[nodiscard]] QtViewportWidget* GetViewportWidget() const
    {
        return m_ViewportWidget;
    }

private:
    QtViewportWidget* m_ViewportWidget = nullptr;
    QtInput* m_Input = nullptr;
};
}  // namespace Matcha
