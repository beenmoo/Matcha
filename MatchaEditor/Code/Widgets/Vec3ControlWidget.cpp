#include "Vec3ControlWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>

namespace MatchaEditor
{
Vec3ControlWidget::Vec3ControlWidget(const QString& label, const Vector3& initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(new QLabel(label, this));

    // Helper lambda to create a styled spinbox
    auto createSpinBox = [this](double val) {
        auto* spin = new QDoubleSpinBox(this);
        spin->setRange(-999999.0, 999999.0);
        spin->setValue(val);
        spin->setSingleStep(0.1);
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Vec3ControlWidget::OnValuesChanged);
        return spin;
    };

    m_XSpinBox = createSpinBox(initialValue.x);
    m_YSpinBox = createSpinBox(initialValue.y);
    m_ZSpinBox = createSpinBox(initialValue.z);

    layout->addWidget(new QLabel("X", this));
    layout->addWidget(m_XSpinBox);
    layout->addWidget(new QLabel("Y", this));
    layout->addWidget(m_YSpinBox);
    layout->addWidget(new QLabel("Z", this));
    layout->addWidget(m_ZSpinBox);

    setLayout(layout);
}
void Vec3ControlWidget::OnValuesChanged()
{
    emit ValueChanged(Vector3(static_cast<float>(m_XSpinBox->value()), static_cast<float>(m_YSpinBox->value()), static_cast<float>(m_ZSpinBox->value())));
}
}  // namespace MatchaEditor