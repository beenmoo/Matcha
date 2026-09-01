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

    // Fires once when the spin box commits an edit (Enter or focus-loss) - unlike ValueChanged,
    // which fires on every intermediate tick while dragging/typing. This is what undo should key
    // off of: one EditingFinished per committed edit, not one ValueChanged per tick.
    void EditingFinished();

private:
    QDoubleSpinBox* m_SpinBox;
};
}  // namespace MatchaEditor
