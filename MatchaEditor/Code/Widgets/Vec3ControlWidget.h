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

signals:
    void ValueChanged(const Vector3& newValue);

private:
    void OnValuesChanged();

private:
    QDoubleSpinBox* m_XSpinBox;
    QDoubleSpinBox* m_YSpinBox;
    QDoubleSpinBox* m_ZSpinBox;
};
}  // namespace MatchaEditor