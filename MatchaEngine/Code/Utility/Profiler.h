#pragma once

#include <chrono>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

namespace Matcha
{
struct ProfileResult
{
    std::string name;
    std::chrono::microseconds start;
    std::chrono::microseconds elapsed;
    std::thread::id threadID;
};

// Writes scope timings to a Chrome Tracing JSON file - open the result in chrome://tracing or
// https://ui.perfetto.dev for a timeline view. Not gated behind MT_DEBUG (see MT_ENABLE_PROFILING
// below): unlike logging, profiling needs to work in Release builds too, to tell "is this
// actually slow" apart from "just Debug-STL overhead".
class Profiler
{
public:
    [[nodiscard]] static Profiler& Get();

    void BeginSession(std::string_view name, const std::string& filepath = "profile_results.json");
    void EndSession();

    void WriteProfile(const ProfileResult& result);

private:
    Profiler() = default;

    void WriteHeader();
    void WriteFooter();

private:
    std::mutex m_Mutex;
    std::ofstream m_OutputStream;
    bool m_HasWrittenEntry = false;
};

// RAII scope timer - records its own lifetime and writes one entry to the active Profiler
// session when it goes out of scope. Use via MT_PROFILE_SCOPE/MT_PROFILE_FUNCTION below rather
// than directly.
class ProfileTimer
{
public:
    explicit ProfileTimer(std::string_view name);
    ~ProfileTimer();

    ProfileTimer(const ProfileTimer&) = delete;
    ProfileTimer& operator=(const ProfileTimer&) = delete;

private:
    std::string m_Name;
    std::chrono::steady_clock::time_point m_StartTime;
};
}  // namespace Matcha

#ifdef MT_ENABLE_PROFILING
#define MT_PROFILER_CONCAT_INNER(a, b) a##b
#define MT_PROFILER_CONCAT(a, b) MT_PROFILER_CONCAT_INNER(a, b)

// Times the enclosing scope under the given name and writes one entry to the active Profiler
// session on scope exit. No-op (compiles away entirely) when MT_ENABLE_PROFILING isn't defined.
#define MT_PROFILE_SCOPE(name) ::Matcha::ProfileTimer MT_PROFILER_CONCAT(mt_profileTimer_, __LINE__)(name)

#if defined(_MSC_VER)
#define MT_PROFILE_FUNCTION() MT_PROFILE_SCOPE(__FUNCSIG__)
#else
#define MT_PROFILE_FUNCTION() MT_PROFILE_SCOPE(__PRETTY_FUNCTION__)
#endif
#else
#define MT_PROFILE_SCOPE(name)
#define MT_PROFILE_FUNCTION()
#endif
