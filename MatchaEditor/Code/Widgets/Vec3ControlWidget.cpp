#include "Vec3ControlWidget.h"
#include "FieldLabel.h"

#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace MatchaEditor
{
Vec3ControlWidget::Vec3ControlWidget(const QString& label, const Vector3& initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);  // Keep elements close together

    // Main property label (e.g., "Position") on the left
    layout->addWidget(CreateFieldLabel(label, this));

    // Lambda helper to create a single Axis sub-layout (e.g., "X" + SpinBox)
    auto createAxisWidget = [this](double val, const QString& axisName, QDoubleSpinBox*& outSpinBox) {
        QHBoxLayout* axisLayout = new QHBoxLayout();
        axisLayout->setSpacing(2);
        axisLayout->setContentsMargins(0, 0, 0, 0);

        // Axis color-coding (red/green/blue for X/Y/Z, the standard 3D-tool convention) is
        // driven by Editor.qss's QLabel[axis="..."] rules, keyed off this property - not a color
        // baked into this widget, so both Vec3ControlWidget and Vec4ControlWidget share one
        // source of truth for the color mapping.
        QLabel* axisLabel = new QLabel(axisName, this);
        axisLabel->setProperty("axis", axisName.toLower());
        // Fixed, not the QLabel default of Preferred - otherwise this label claims a share of
        // whatever stretch space the container receives (see the stretch factor below), pushing
        // it away from the spin box instead of staying flush against it.
        axisLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        axisLayout->addWidget(axisLabel);

        outSpinBox = new QDoubleSpinBox(this);
        outSpinBox->setRange(-999999.0, 999999.0);
        outSpinBox->setValue(val);
        outSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);  // Hides default ugly arrows!
        ApplySquishPolicy(outSpinBox);

        connect(outSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Vec3ControlWidget::OnValuesChanged);

        axisLayout->addWidget(outSpinBox);

        // Wrap the sub-layout in a temporary container widget so we can add it to the main layout
        QWidget* container = new QWidget(this);
        container->setLayout(axisLayout);
        return container;
    };

    // Equal stretch factors so the three axes squish evenly together as the panel narrows,
    // rather than one shrinking before the others.
    layout->addWidget(createAxisWidget(initialValue.x, "X", m_XSpinBox), 1);
    layout->addWidget(createAxisWidget(initialValue.y, "Y", m_YSpinBox), 1);
    layout->addWidget(createAxisWidget(initialValue.z, "Z", m_ZSpinBox), 1);

    setLayout(layout);
}
void Vec3ControlWidget::OnValuesChanged()
{
    emit ValueChanged(Vector3(static_cast<float>(m_XSpinBox->value()), static_cast<float>(m_YSpinBox->value()), static_cast<float>(m_ZSpinBox->value())));
}

void Vec3ControlWidget::SetValue(const Vector3& value)
{
    if (!m_XSpinBox->hasFocus())
    {
        const QSignalBlocker blocker(m_XSpinBox);
        m_XSpinBox->setValue(value.x);
    }
    if (!m_YSpinBox->hasFocus())
    {
        const QSignalBlocker blocker(m_YSpinBox);
        m_YSpinBox->setValue(value.y);
    }
    if (!m_ZSpinBox->hasFocus())
    {
        const QSignalBlocker blocker(m_ZSpinBox);
        m_ZSpinBox->setValue(value.z);
    }
}
}  // namespace MatchaEditor