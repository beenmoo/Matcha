#include "Vec4ControlWidget.h"
#include "FieldLabel.h"

#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace MatchaEditor
{
Vec4ControlWidget::Vec4ControlWidget(const QString& label, const Vector4& initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    layout->addWidget(CreateFieldLabel(label, this));

    auto createAxisWidget = [this](double val, const QString& axisName, QDoubleSpinBox*& outSpinBox) {
        QHBoxLayout* axisLayout = new QHBoxLayout();
        axisLayout->setSpacing(2);
        axisLayout->setContentsMargins(0, 0, 0, 0);

        // Axis color-coding (red/green/blue/white for X/Y/Z/W, the standard 3D-tool convention)
        // is driven by Editor.qss's QLabel[axis="..."] rules, keyed off this property - see
        // Vec3ControlWidget's identical lambda for why it isn't a color baked into this widget.
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
        outSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        ApplySquishPolicy(outSpinBox);

        connect(outSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Vec4ControlWidget::OnValuesChanged);
        connect(outSpinBox, &QDoubleSpinBox::editingFinished, this, &Vec4ControlWidget::EditingFinished);

        axisLayout->addWidget(outSpinBox);

        QWidget* container = new QWidget(this);
        container->setLayout(axisLayout);
        return container;
    };

    // Equal stretch factors so the four axes squish evenly together as the panel narrows.
    layout->addWidget(createAxisWidget(initialValue.x, "X", m_XSpinBox), 1);
    layout->addWidget(createAxisWidget(initialValue.y, "Y", m_YSpinBox), 1);
    layout->addWidget(createAxisWidget(initialValue.z, "Z", m_ZSpinBox), 1);
    layout->addWidget(createAxisWidget(initialValue.w, "W", m_WSpinBox), 1);

    setLayout(layout);
}
void Vec4ControlWidget::OnValuesChanged()
{
    emit ValueChanged(Vector4(static_cast<float>(m_XSpinBox->value()), static_cast<float>(m_YSpinBox->value()),
                              static_cast<float>(m_ZSpinBox->value()), static_cast<float>(m_WSpinBox->value())));
}

void Vec4ControlWidget::SetValue(const Vector4& value)
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
    if (!m_WSpinBox->hasFocus())
    {
        const QSignalBlocker blocker(m_WSpinBox);
        m_WSpinBox->setValue(value.w);
    }
}
}  // namespace MatchaEditor
