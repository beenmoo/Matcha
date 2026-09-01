#include "BoolFieldWidget.h"
#include "FieldLabel.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
BoolFieldWidget::BoolFieldWidget(const QString& label, bool initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(3);

    layout->addWidget(CreateFieldLabel(label, this));

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
