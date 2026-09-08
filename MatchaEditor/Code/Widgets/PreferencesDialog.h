#include <QDialog>

class QLineEdit;

namespace MatchaEditor
{
class PreferencesDialog : public QDialog
{
public:
    explicit PreferencesDialog(QWidget* parent = nullptr);

private slots:
    void OnBrowseClicked();
    void OnAccepted();

private:
    QLineEdit* m_EditorPathInput;
};
}  // namespace MatchaEditor