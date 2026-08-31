#pragma once

#include <QWidget>

class QCheckBox;

namespace MatchaEditor
{
class BoolFieldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BoolFieldWidget(const QString& label, bool initialValue, QWidget* parent = nullptr);

    // Updates the displayed value without emitting ValueChanged (via QSignalBlocker) - for
    // syncing the display from a value that changed elsewhere, not from editing here. No-op while
    // this field has focus, so it doesn't clobber an in-progress edit.
    void SetValue(bool value);

signals:
    void ValueChanged(bool newValue);

private:
    QCheckBox* m_CheckBox;
};
}  // namespace MatchaEditor
