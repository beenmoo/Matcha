#include "StringFieldWidget.h"

#include <QHBoxLayout>
#include <QLabel>
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

    QLabel* mainLabel = new QLabel(label, this);
    mainLabel->setFixedWidth(65);  // Aligned with Vec3ControlWidget's labels
    mainLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    layout->addWidget(mainLabel);

    m_LineEdit = new QLineEdit(this);
    m_LineEdit->setText(initialValue);
    m_LineEdit->setFixedHeight(20);
    m_LineEdit->setStyleSheet(
        "QLineEdit {"
        "   background-color: #222222;"
        "   color: #dcdcdc;"
        "   border: 1px solid #1a1a1a;"
        "   border-radius: 2px;"
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
