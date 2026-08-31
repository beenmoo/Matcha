#pragma once

#include <QWidget>

class QDoubleSpinBox;

namespace MatchaEditor
{
class FloatFieldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FloatFieldWidget(const QString& label, float initialValue, QWidget* parent = nullptr);

    // Updates the displayed value without emitting ValueChanged (via QSignalBlocker) - for
    // syncing the display from a value that changed elsewhere, not from editing here. No-op while
    // this field has focus, so it doesn't clobber an in-progress edit.
    void SetValue(float value);

signals:
    void ValueChanged(float newValue);

private:
    QDoubleSpinBox* m_SpinBox;
};
}  // namespace MatchaEditor
