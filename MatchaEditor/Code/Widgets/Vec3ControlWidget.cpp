#include "Vec3ControlWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
Vec3ControlWidget::Vec3ControlWidget(const QString& label, const Vector3& initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);  // Keep elements close together

    // Main property label (e.g., "Position") on the left
    QLabel* mainLabel = new QLabel(label, this);
    mainLabel->setFixedWidth(65);  // Fixed width so X/Y/Z line up across rows!
    mainLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    layout->addWidget(mainLabel);

    // Lambda helper to create a single Axis sub-layout (e.g., "X" + SpinBox)
    auto createAxisWidget = [this](double val, const QString& axisName, const QString& textColor, QDoubleSpinBox*& outSpinBox) {
        QHBoxLayout* axisLayout = new QHBoxLayout();
        axisLayout->setSpacing(2);
        axisLayout->setContentsMargins(0, 0, 0, 0);

        QLabel* axisLabel = new QLabel(axisName, this);
        axisLabel->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px;").arg(textColor));
        axisLayout->addWidget(axisLabel);

        outSpinBox = new QDoubleSpinBox(this);
        outSpinBox->setRange(-999999.0, 999999.0);
        outSpinBox->setValue(val);
        outSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);  // Hides default ugly arrows!
        outSpinBox->setFixedHeight(20);                             // Sleek and thin like Unity
        outSpinBox->setStyleSheet(
            "QDoubleSpinBox {"
            "   background-color: #222222;"
            "   color: #dcdcdc;"
            "   border: 1px solid #1a1a1a;"
            "   border-radius: 2px;"
            "   padding-left: 2px;"
            "}");

        connect(outSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Vec3ControlWidget::OnValuesChanged);

        axisLayout->addWidget(outSpinBox);

        // Wrap the sub-layout in a temporary container widget so we can add it to the main layout
        QWidget* container = new QWidget(this);
        container->setLayout(axisLayout);
        return container;
    };

    // Unity-accurate axis colors (Red for X, Green for Y, Blue for Z)
    layout->addWidget(createAxisWidget(initialValue.x, "X", "#ff5555", m_XSpinBox));
    layout->addWidget(createAxisWidget(initialValue.y, "Y", "#55ff55", m_YSpinBox));
    layout->addWidget(createAxisWidget(initialValue.z, "Z", "#5555ff", m_ZSpinBox));

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