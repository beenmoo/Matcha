#include "ConsoleSink.h"

#include <spdlog/spdlog.h>

namespace MatchaEditor
{
void ConsoleSink::sink_it_(const spdlog::details::log_msg& msg)
{
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    emit MessageLogged(QString::fromUtf8(formatted.data(), static_cast<int>(formatted.size())), static_cast<int>(msg.level));
}

void ConsoleSink::flush_()
{
}
}  // namespace Matcha
