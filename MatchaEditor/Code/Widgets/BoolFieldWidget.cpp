#include "BoolFieldWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
BoolFieldWidget::BoolFieldWidget(const QString& label, bool initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    QLabel* mainLabel = new QLabel(label, this);
    mainLabel->setFixedWidth(65);  // Aligned with Vec3ControlWidget's labels
    mainLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    layout->addWidget(mainLabel);

    m_CheckBox = new QCheckBox(this);
    m_CheckBox->setChecked(initialValue);
    layout->addWidget(m_CheckBox);
    layout->addStretch();

    connect(m_CheckBox, &QCheckBox::toggled, this, &BoolFieldWidget::ValueChanged);

    setLayout(layout);
}

void BoolFieldWidget::SetValue(bool value)
{
    if (m_CheckBox->hasFocus())
        return;

    const QSignalBlocker blocker(m_CheckBox);
    m_CheckBox->setChecked(value);
}
}  // namespace MatchaEditor
