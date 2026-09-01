#include <DockWidget.h>

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
class ConsoleWidget;

class ConsolePanel : public ads::CDockWidget
{
    Q_OBJECT
public:
    explicit ConsolePanel(ads::CDockManager* dockManager, QWidget* parent = nullptr);
    ~ConsolePanel() = default;

    // Slot/Method to receive logs and pass them to the inner widget
    void AppendMessage(const QString& message, int level);

private:
    ConsoleWidget* m_ConsoleWidget;
};
}  // namespace MatchaEditor
