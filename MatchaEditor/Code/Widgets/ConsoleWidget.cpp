#include "ConsoleWidget.h"

#include <spdlog/common.h>

#include <QScrollBar>
#include <QTextCharFormat>
#include <QTextCursor>

namespace MatchaEditor
{
namespace
{
QColor ColorForLevel(spdlog::level::level_enum level)
{
    switch (level)
    {
        case spdlog::level::trace: return QColor(0x80, 0x80, 0x80);
        case spdlog::level::debug: return QColor(0x60, 0xa0, 0xe0);
        case spdlog::level::warn: return QColor(0xe0, 0xb0, 0x40);
        case spdlog::level::err: return QColor(0xe0, 0x50, 0x50);
        case spdlog::level::critical: return QColor(0xff, 0x60, 0x60);
        default: return QColor(0xd0, 0xd0, 0xd0);
    }
}
}  // namespace

ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setMaximumBlockCount(2000);
    setFont(QFont("Consolas", 9));
}

void ConsoleWidget::AppendMessage(const QString& message, int level)
{
    QTextCharFormat format;
    format.setForeground(ColorForLevel(static_cast<spdlog::level::level_enum>(level)));

    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(message.trimmed() + "\n", format);

    QScrollBar* scrollBar = verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}
}  // namespace Matcha
