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
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(3);

    layout->addWidget(CreateFieldLabel(label, this));

    m_ComboBox = new QComboBox(this);
    m_ComboBox->addItems(options);
    m_ComboBox->setCurrentIndex(initialIndex);
    m_ComboBox->setFixedHeight(16);
    m_ComboBox->setStyleSheet(
        "QComboBox {"
        "   background-color: #222222;"
        "   color: #dcdcdc;"
        "   border: 1px solid #1a1a1a;"
        "   border-radius: 2px;"
        "   font-size: 10px;"
        "   padding-left: 4px;"
        "}");

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
