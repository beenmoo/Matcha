#pragma once

#include <spdlog/sinks/base_sink.h>

#include <QObject>
#include <QString>

#include <mutex>

namespace MatchaEditor
{
// Bridges spdlog log calls (which can happen on any thread, e.g. efsw's file-watcher thread)
// onto the Qt GUI thread: MessageLogged is a normal Qt signal, so Qt automatically queues the
// delivery when emitted from a non-GUI thread instead of touching widgets off-thread.
class ConsoleSink : public QObject, public spdlog::sinks::base_sink<std::mutex>
{
    Q_OBJECT

signals:
    void MessageLogged(const QString& message, int level);

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;
};
}  // namespace Matcha
