#include "pch.h"
#include "Utility/FileWatcher.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace Matcha;

namespace
{
// Creates a unique scratch directory for a test and removes it (and its contents) on teardown.
class TempDirectory
{
public:
    TempDirectory()
    {
        auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        m_Path = std::filesystem::temp_directory_path() / ("MatchaFileWatcherTest-" + std::to_string(unique));
        std::filesystem::create_directories(m_Path);
    }

    ~TempDirectory()
    {
        std::error_code ec;
        std::filesystem::remove_all(m_Path, ec);
    }

    [[nodiscard]] const std::filesystem::path& GetPath() const
    {
        return m_Path;
    }

private:
    std::filesystem::path m_Path;
};

// Collects FileWatcher callback invocations from the watcher's background thread and lets the
// test thread block until a matching event arrives.
class WatchEventCollector
{
public:
    struct Event
    {
        std::string directory;
        std::string filename;
        FileAction action;
    };

    [[nodiscard]] FileWatchCallback GetCallback()
    {
        return [this](const std::string& directory, const std::string& filename, FileAction action) {
            std::scoped_lock lock(m_Mutex);
            m_Events.push_back({directory, filename, action});
            m_Condition.notify_all();
        };
    }

    template <typename Predicate>
    [[nodiscard]] bool WaitFor(Predicate predicate, std::chrono::milliseconds timeout)
    {
        std::unique_lock lock(m_Mutex);
        return m_Condition.wait_for(lock, timeout, [&] { return std::any_of(m_Events.begin(), m_Events.end(), predicate); });
    }

private:
    std::mutex m_Mutex;
    std::condition_variable m_Condition;
    std::vector<Event> m_Events;
};

void WriteFile(const std::filesystem::path& path, std::string_view contents)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << contents;
}
}  // namespace

TEST(FileWatcherTests, DetectsNewFile)
{
    // Declaration order matters for teardown safety: WatchEventCollector must outlive
    // FileWatcher, since FileWatcher's destructor joins its background thread and only
    // then is it safe to destroy the object the callback captured by reference.
    TempDirectory dir;
    WatchEventCollector collector;
    FileWatcher watcher;

    (void)watcher.AddWatch(dir.GetPath().string(), collector.GetCallback(), false);
    watcher.Watch();

    // Give the platform backend a moment to start watching before triggering the event.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    WriteFile(dir.GetPath() / "test.txt", "hello");

    bool detected = collector.WaitFor(
        [](const WatchEventCollector::Event& e) { return e.filename == "test.txt"; },
        std::chrono::seconds(5));

    EXPECT_TRUE(detected);
}

TEST(FileWatcherTests, DetectsModifiedFile)
{
    TempDirectory dir;
    std::filesystem::path filePath = dir.GetPath() / "test.txt";
    WriteFile(filePath, "initial");

    WatchEventCollector collector;
    FileWatcher watcher;

    (void)watcher.AddWatch(dir.GetPath().string(), collector.GetCallback(), false);
    watcher.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    WriteFile(filePath, "initial modified");

    bool detected = collector.WaitFor(
        [](const WatchEventCollector::Event& e) { return e.filename == "test.txt" && e.action == FileAction::Modified; },
        std::chrono::seconds(5));

    EXPECT_TRUE(detected);
}

TEST(FileWatcherTests, RemoveWatchStopsNotifications)
{
    TempDirectory dir;
    WatchEventCollector collector;
    FileWatcher watcher;

    WatchHandle handle = watcher.AddWatch(dir.GetPath().string(), collector.GetCallback(), false);
    watcher.Watch();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    watcher.RemoveWatch(handle);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    WriteFile(dir.GetPath() / "test.txt", "hello");

    bool detected = collector.WaitFor(
        [](const WatchEventCollector::Event&) { return true; },
        std::chrono::milliseconds(1500));

    EXPECT_FALSE(detected);
}
