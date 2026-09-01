#pragma once

#include <Matcha.h>
#include <QWidget>

class QDoubleSpinBox;

namespace MatchaEditor
{
class Vec4ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit Vec4ControlWidget(const QString& label, const Vector4& initialValue, QWidget* parent = nullptr);

    // Updates the displayed value without emitting ValueChanged (via QSignalBlocker) - for
    // syncing the display from a value that changed elsewhere, not from editing here. Skips any
    // axis currently focused, so it doesn't clobber an in-progress edit on that one axis.
    void SetValue(const Vector4& value);

signals:
    void ValueChanged(const Vector4& newValue);

    // Fires once per axis whose spin box commits an edit (Enter or focus-loss) - unlike
    // ValueChanged, which fires on every intermediate tick while dragging/typing. This is what
    // undo should key off of: one EditingFinished per committed edit, not one ValueChanged per
    // tick.
    void EditingFinished();

private:
    void OnValuesChanged();

private:
    QDoubleSpinBox* m_XSpinBox;
    QDoubleSpinBox* m_YSpinBox;
    QDoubleSpinBox* m_ZSpinBox;
    QDoubleSpinBox* m_WSpinBox;
};
}  // namespace MatchaEditor
