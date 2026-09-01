#include "FloatFieldWidget.h"
#include "FieldLabel.h"

#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
FloatFieldWidget::FloatFieldWidget(const QString& label, float initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(3);

    layout->addWidget(CreateFieldLabel(label, this));

    m_SpinBox = new QDoubleSpinBox(this);
    m_SpinBox->setRange(-999999.0, 999999.0);
    m_SpinBox->setValue(initialValue);
    m_SpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
    m_SpinBox->setFixedHeight(16);
    m_SpinBox->setStyleSheet(
        "QDoubleSpinBox {"
        "   background-color: #222222;"
        "   color: #dcdcdc;"
        "   border: 1px solid #1a1a1a;"
        "   border-radius: 2px;"
        "   font-size: 10px;"
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
