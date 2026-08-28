#include "Profiler.h"

#include <algorithm>

namespace Matcha
{
Profiler& Profiler::Get()
{
    static Profiler instance;
    return instance;
}

void Profiler::BeginSession(std::string_view name, const std::string& filepath)
{
    std::scoped_lock lock(m_Mutex);

    m_OutputStream.open(filepath);
    m_HasWrittenEntry = false;

    WriteHeader();
}

void Profiler::EndSession()
{
    std::scoped_lock lock(m_Mutex);

    WriteFooter();
    m_OutputStream.close();
}

void Profiler::WriteProfile(const ProfileResult& result)
{
    std::scoped_lock lock(m_Mutex);

    if (!m_OutputStream.is_open())
        return;

    if (m_HasWrittenEntry)
        m_OutputStream << ",";

    m_HasWrittenEntry = true;

    std::string name = result.name;
    std::replace(name.begin(), name.end(), '"', '\'');

    // Chrome Tracing's "tid" just needs to be a stable, distinguishing integer per thread - not
    // a real OS thread ID.
    uint32_t threadID = static_cast<uint32_t>(std::hash<std::thread::id>{}(result.threadID));

    m_OutputStream << "{";
    m_OutputStream << "\"cat\":\"function\",";
    m_OutputStream << "\"dur\":" << result.elapsed.count() << ',';
    m_OutputStream << "\"name\":\"" << name << "\",";
    m_OutputStream << "\"ph\":\"X\",";
    m_OutputStream << "\"pid\":0,";
    m_OutputStream << "\"tid\":" << threadID << ",";
    m_OutputStream << "\"ts\":" << result.start.count();
    m_OutputStream << "}";

    m_OutputStream.flush();
}

void Profiler::WriteHeader()
{
    m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
    m_OutputStream.flush();
}

void Profiler::WriteFooter()
{
    m_OutputStream << "]}";
    m_OutputStream.flush();
}

ProfileTimer::ProfileTimer(std::string_view name)
    : m_Name(name),
      m_StartTime(std::chrono::steady_clock::now())
{
}

ProfileTimer::~ProfileTimer()
{
    auto endTime = std::chrono::steady_clock::now();

    auto start = std::chrono::duration_cast<std::chrono::microseconds>(m_StartTime.time_since_epoch());
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_StartTime);

    Profiler::Get().WriteProfile({m_Name, start, elapsed, std::this_thread::get_id()});
}
}  // namespace Matcha
