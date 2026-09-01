#include "StringFieldWidget.h"
#include "FieldLabel.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QSignalBlocker>

namespace MatchaEditor
{
StringFieldWidget::StringFieldWidget(const QString& label, const QString& initialValue, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(4);

    layout->addWidget(CreateFieldLabel(label, this));

    m_LineEdit = new QLineEdit(this);
    m_LineEdit->setText(initialValue);

    // editingFinished takes no arguments (fires on Enter/focus-loss, not per keystroke) - read
    // the committed text from m_LineEdit itself rather than from the signal.
    connect(m_LineEdit, &QLineEdit::editingFinished, this, [this] { emit ValueChanged(m_LineEdit->text()); });

    layout->addWidget(m_LineEdit);
    setLayout(layout);
}

void StringFieldWidget::SetValue(const QString& value)
{
    if (m_LineEdit->hasFocus())
        return;

    const QSignalBlocker blocker(m_LineEdit);
    m_LineEdit->setText(value);
}
}  // namespace MatchaEditor
