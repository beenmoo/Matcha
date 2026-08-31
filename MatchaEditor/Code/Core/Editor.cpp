#include "Editor.h"
#include "EditorMainWindow.h"
#include "Core/Qt/QtViewportWidget.h"
#include "Core/Qt/QtWindow.h"
#include "Scene/System/RenderSystem.h"

#include <QTimer>

namespace MatchaEditor
{
Editor::Editor(const Application::ApplicationSpecification& spec)
    : Application(spec)
{
    auto* qtWindow = dynamic_cast<QtWindow*>(&GetContext().GetWindow());

    MT_ASSERT(qtWindow, "Editor requires ApplicationSpecification::m_WindowBackend == WindowBackend::Qt");

    m_EditorCamera.SetAspectRatio(GetContext().GetWindow().GetAspectRatio());

    m_MainWindow = std::make_unique<EditorMainWindow>(GetContext(), qtWindow->GetViewportWidget());

    m_TickTimer = std::make_unique<QTimer>();
    QObject::connect(m_TickTimer.get(), &QTimer::timeout, [qtWindow] { qtWindow->GetViewportWidget()->update(); });
    m_TickTimer->start(16);
}

Editor::~Editor() = default;

void Editor::Show()
{
    m_MainWindow->show();
}

void Editor::OnUpdate()
{
    m_EditorCamera.Update(GetContext());
}

void Editor::OnEvent(const Event& event)
{
    // Guard against height == 0: fires transiently while a dock is being resized/collapsed.
    if (event.type == EventType::WindowResized && event.height > 0)
        m_EditorCamera.SetAspectRatio(static_cast<float>(event.width) / static_cast<float>(event.height));
}

void Editor::RenderCamera()
{
    Renderer& renderer = GetContext().GetRenderer();

    renderer.SetViewProjection(m_EditorCamera.GetViewProjection());
    renderer.SetCameraPosition(m_EditorCamera.GetPosition());

    RenderSystem::Draw(GetContext().GetScene(), renderer);
}
}  // namespace Matcha
