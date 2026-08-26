#include "FileWatcher.h"
#include "Core/Logger.h"

#include <efsw/efsw.hpp>
#include <unordered_map>

namespace Matcha
{
namespace
{
FileAction ToFileAction(efsw::Action action)
{
    switch (action)
    {
    case efsw::Actions::Add:
        return FileAction::Add;
    case efsw::Actions::Delete:
        return FileAction::Delete;
    case efsw::Actions::Modified:
        return FileAction::Modified;
    case efsw::Actions::Moved:
        return FileAction::Moved;
    }

    return FileAction::Modified;
}
}  // namespace

struct FileWatcher::Impl : public efsw::FileWatchListener
{
    efsw::FileWatcher watcher;
    std::unordered_map<uint32_t, efsw::WatchID> watchIDs;
    std::unordered_map<efsw::WatchID, FileWatchCallback> callbacks;
    uint32_t nextHandleID = 1;

    void handleFileAction(efsw::WatchID watchID, const std::string& dir, const std::string& filename,
                           efsw::Action action, const std::string& oldFilename) override
    {
        auto it = callbacks.find(watchID);

        if (it != callbacks.end())
            it->second(dir, filename, ToFileAction(action));
    }
};

FileWatcher::FileWatcher() : mImpl(std::make_unique<Impl>())
{
}

FileWatcher::~FileWatcher() = default;

WatchHandle FileWatcher::AddWatch(std::string_view directory, FileWatchCallback callback, bool recursive)
{
    efsw::WatchID watchID = mImpl->watcher.addWatch(std::string(directory), mImpl.get(), recursive);

    if (watchID < 0)
    {
        MT_CORE_ERROR("Failed to watch directory: {}", directory);
        return WatchHandle();
    }

    WatchHandle handle(mImpl->nextHandleID++);

    mImpl->watchIDs.emplace(handle.GetID(), watchID);
    mImpl->callbacks.emplace(watchID, std::move(callback));

    return handle;
}

void FileWatcher::RemoveWatch(WatchHandle handle)
{
    auto it = mImpl->watchIDs.find(handle.GetID());

    if (it == mImpl->watchIDs.end())
        return;

    mImpl->watcher.removeWatch(it->second);
    mImpl->callbacks.erase(it->second);
    mImpl->watchIDs.erase(it);
}

void FileWatcher::Watch()
{
    mImpl->watcher.watch();
}
}  // namespace Matcha
