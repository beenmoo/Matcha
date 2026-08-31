#pragma once

#include <QWidget>
#include <QStringList>

class QComboBox;

namespace MatchaEditor
{
class EnumFieldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnumFieldWidget(const QString& label, const QStringList& options, int initialIndex, QWidget* parent = nullptr);

    // Updates the displayed value without emitting ValueChanged (via QSignalBlocker) - for
    // syncing the display from a value that changed elsewhere, not from editing here. No-op while
    // this field has focus, so it doesn't clobber an in-progress edit.
    void SetValue(int index);

signals:
    void ValueChanged(int newIndex);

private:
    QComboBox* m_ComboBox;
};
}  // namespace MatchaEditor
