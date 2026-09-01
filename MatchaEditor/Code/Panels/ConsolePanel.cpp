#include "ConsolePanel.h"
#include "Widgets/ConsoleWidget.h"

#include <DockManager.h>

namespace MatchaEditor
{
ConsolePanel::ConsolePanel(ads::CDockManager* dockManager, QWidget* parent)
    : ads::CDockWidget(dockManager, "Console Panel", parent)
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
