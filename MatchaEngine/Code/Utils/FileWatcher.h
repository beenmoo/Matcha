#pragma once

#include "Core/Handle.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace Matcha
{
using WatchHandle = Handle<struct WatchTag>;

enum class FileAction
{
    Add,
    Delete,
    Modified,
    Moved
};

using FileWatchCallback = std::function<void(const std::string& directory, const std::string& filename, FileAction action)>;

class FileWatcher
{
public:
    FileWatcher();
    ~FileWatcher();

    FileWatcher(const FileWatcher&) = delete;
    FileWatcher& operator=(const FileWatcher&) = delete;

    [[nodiscard]] WatchHandle AddWatch(std::string_view directory, FileWatchCallback callback, bool recursive = true);
    void RemoveWatch(WatchHandle handle);

    void Watch();

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};
}  // namespace Matcha
