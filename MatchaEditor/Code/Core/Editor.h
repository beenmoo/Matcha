#pragma once

#include <Matcha.h>

#include <memory>

class QTimer;

namespace Matcha
{
class EditorMainWindow;

class Editor : public Application
{
public:
    Editor(const Application::ApplicationSpecification& spec);
    ~Editor() override;

    void Show();

private:
    std::unique_ptr<EditorMainWindow> m_MainWindow;
    std::unique_ptr<QTimer> m_TickTimer;
};
}  // namespace Matcha
