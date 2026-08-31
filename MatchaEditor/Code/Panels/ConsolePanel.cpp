#include "ConsolePanel.h"
#include "Widgets/ConsoleWidget.h"

namespace MatchaEditor
{
ConsolePanel::ConsolePanel(QWidget* parent)
    : QDockWidget("Console Panel", parent)
{
    setObjectName("ConsolePanel");

    m_ConsoleWidget = new ConsoleWidget(this);

    setWidget(m_ConsoleWidget);
}
void ConsolePanel::AppendMessage(const QString& message, int level)
{
    m_ConsoleWidget->AppendMessage(message, level);
}
}  // namespace MatchaEditor