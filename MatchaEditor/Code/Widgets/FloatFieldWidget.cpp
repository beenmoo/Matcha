#include "FloatFieldWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
FloatFieldWidget::FloatFieldWidget(const QString& label, float initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    QLabel* mainLabel = new QLabel(label, this);
    mainLabel->setFixedWidth(65);  // Aligned with Vec3ControlWidget's labels
    mainLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    layout->addWidget(mainLabel);

    m_SpinBox = new QDoubleSpinBox(this);
    m_SpinBox->setRange(-999999.0, 999999.0);
    m_SpinBox->setValue(initialValue);
    m_SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_SpinBox->setFixedHeight(20);
    m_SpinBox->setStyleSheet(
        "QDoubleSpinBox {"
        "   background-color: #222222;"
        "   color: #dcdcdc;"
        "   border: 1px solid #1a1a1a;"
        "   border-radius: 2px;"
        "   padding-left: 2px;"
        "}");

    connect(m_SpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            [this](double value) { emit ValueChanged(static_cast<float>(value)); });

    layout->addWidget(m_SpinBox);
    setLayout(layout);
}

void FloatFieldWidget::SetValue(float value)
{
    if (m_SpinBox->hasFocus())
        return;

    const QSignalBlocker blocker(m_SpinBox);
    m_SpinBox->setValue(value);
}
}  // namespace MatchaEditor
