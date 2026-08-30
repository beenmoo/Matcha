#pragma once

#include <QWidget>

class QLineEdit;

namespace MatchaEditor
{
class StringFieldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StringFieldWidget(const QString& label, const QString& initialValue, QWidget* parent = nullptr);

    // Updates the displayed value without emitting ValueChanged (via QSignalBlocker) - for
    // syncing the display from a value that changed elsewhere, not from editing here. No-op while
    // this field has focus, so it doesn't clobber in-progress typing (this field only commits on
    // editingFinished, so an overwrite mid-edit would be especially disruptive here).
    void SetValue(const QString& value);

signals:
    void ValueChanged(const QString& newValue);

private:
    QLineEdit* m_LineEdit;
};
}  // namespace MatchaEditor
