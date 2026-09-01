#include "EnumFieldWidget.h"
#include "FieldLabel.h"

#include <QHBoxLayout>
#include <QComboBox>
#include <QSignalBlocker>

namespace MatchaEditor
{
EnumFieldWidget::EnumFieldWidget(const QString& label, const QStringList& options, int initialIndex, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    layout->addWidget(CreateFieldLabel(label, this));

    m_ComboBox = new QComboBox(this);
    m_ComboBox->addItems(options);
    m_ComboBox->setCurrentIndex(initialIndex);
    ApplySquishPolicy(m_ComboBox);

    connect(m_ComboBox, &QComboBox::currentIndexChanged, this, &EnumFieldWidget::ValueChanged);

    layout->addWidget(m_ComboBox);
    setLayout(layout);
}

void EnumFieldWidget::SetValue(int index)
{
    if (m_ComboBox->hasFocus())
        return;

    const QSignalBlocker blocker(m_ComboBox);
    m_ComboBox->setCurrentIndex(index);
}
}  // namespace MatchaEditor
