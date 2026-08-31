#pragma once

#include <QPlainTextEdit>

namespace MatchaEditor
{
class ConsoleWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit ConsoleWidget(QWidget* parent = nullptr);

public slots:
    // level is an spdlog::level::level_enum, passed as int since queued signal/slot
    // connections need a registered meta type and int already is one.
    void AppendMessage(const QString& message, int level);
};
}  // namespace Matcha
