#pragma once

#include "EditorCamera.h"

#include <Matcha.h>

#include <memory>

class QTimer;

namespace MatchaEditor
{
class EditorMainWindow;

class Editor : public Application
{
public:
    explicit Editor(const Application::ApplicationSpecification& spec);
    ~Editor() override;

    void Show();

protected:
    void OnUpdate() override;
    void OnEvent(const Event& event) override;
    void RenderCamera() override;

private:
    std::unique_ptr<EditorMainWindow> m_MainWindow;
    std::unique_ptr<QTimer> m_TickTimer;
    EditorCamera m_EditorCamera;
};
}  // namespace Matcha
