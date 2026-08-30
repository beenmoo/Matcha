#include "Editor.h"
#include "EditorMainWindow.h"
#include "Core/Qt/QtViewportWidget.h"
#include "Core/Qt/QtWindow.h"

#include <QTimer>

namespace MatchaEditor
{
Editor::Editor(const Application::ApplicationSpecification& spec)
    : Application(spec)
{
    auto* qtWindow = dynamic_cast<QtWindow*>(&GetContext().GetWindow());

    MT_ASSERT(qtWindow, "Editor requires ApplicationSpecification::m_WindowBackend == WindowBackend::Qt");

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
}  // namespace Matcha
