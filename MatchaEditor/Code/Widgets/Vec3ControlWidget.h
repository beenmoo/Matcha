#pragma once

#include <Matcha.h>
#include <QWidget>

class QDoubleSpinBox;

namespace MatchaEditor
{
class Vec3ControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit Vec3ControlWidget(const QString& label, const Vector3& initialValue, QWidget* parent = nullptr);

    // Updates the displayed value without emitting ValueChanged (via QSignalBlocker) - for
    // syncing the display from a value that changed elsewhere (e.g. a script moving the entity),
    // not from editing here. Skips any axis currently focused, so it doesn't clobber an in-progress
    // edit on that one axis while still syncing the other two.
    void SetValue(const Vector3& value);

signals:
    void ValueChanged(const Vector3& newValue);

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
};
}  // namespace MatchaEditor