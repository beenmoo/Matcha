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
    layout->setContentsMargins(1, 1, 1, 1);
    layout->setSpacing(3);

    layout->addWidget(CreateFieldLabel(label, this));

    m_LineEdit = new QLineEdit(this);
    m_LineEdit->setText(initialValue);
    m_LineEdit->setFixedHeight(16);
    m_LineEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #222222;"
        "   color: #dcdcdc;"
        "   border: 1px solid #1a1a1a;"
        "   border-radius: 2px;"
        "   font-size: 10px;"
        "   padding-left: 4px;"
        "}");

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
