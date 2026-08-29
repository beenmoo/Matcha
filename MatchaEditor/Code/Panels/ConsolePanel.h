#include <QDockWidget>

namespace MatchaEditor
{
class ConsoleWidget;

class ConsolePanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit ConsolePanel(QWidget* parent = nullptr);
    ~ConsolePanel() = default;

    // Slot/Method to receive logs and pass them to the inner widget
    void AppendMessage(const QString& message, int level);

private:
    ConsoleWidget* m_ConsoleWidget;
};
}  // namespace MatchaEditor