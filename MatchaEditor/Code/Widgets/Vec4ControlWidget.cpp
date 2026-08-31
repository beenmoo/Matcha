#include "Vec4ControlWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
Vec4ControlWidget::Vec4ControlWidget(const QString& label, const Vector4& initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    QLabel* mainLabel = new QLabel(label, this);
    mainLabel->setFixedWidth(65);  // Fixed width so X/Y/Z/W line up across rows!
    mainLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    layout->addWidget(mainLabel);

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
        outSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
        outSpinBox->setFixedHeight(20);
        outSpinBox->setStyleSheet(
            "QDoubleSpinBox {"
            "   background-color: #222222;"
            "   color: #dcdcdc;"
            "   border: 1px solid #1a1a1a;"
            "   border-radius: 2px;"
            "   padding-left: 2px;"
            "}");

        connect(outSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &Vec4ControlWidget::OnValuesChanged);

        axisLayout->addWidget(outSpinBox);

        QWidget* container = new QWidget(this);
        container->setLayout(axisLayout);
        return container;
    };

    layout->addWidget(createAxisWidget(initialValue.x, "X", "#ff5555", m_XSpinBox));
    layout->addWidget(createAxisWidget(initialValue.y, "Y", "#55ff55", m_YSpinBox));
    layout->addWidget(createAxisWidget(initialValue.z, "Z", "#5555ff", m_ZSpinBox));
    layout->addWidget(createAxisWidget(initialValue.w, "W", "#cccccc", m_WSpinBox));

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
