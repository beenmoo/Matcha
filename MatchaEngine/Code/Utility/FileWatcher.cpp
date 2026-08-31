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
    // watcher must be declared last: members are destroyed in reverse declaration order, and
    // its destructor is what stops/joins the background thread. Declaring it first would destroy
    // watchIDs/callbacks while that thread could still be calling handleFileAction() on them.
    std::unordered_map<uint32_t, efsw::WatchID> watchIDs;
    std::unordered_map<efsw::WatchID, FileWatchCallback> callbacks;
    uint32_t nextHandleID = 1;
    efsw::FileWatcher watcher;

    void handleFileAction(efsw::WatchID watchID, const std::string& dir, const std::string& filename,
                          efsw::Action action, const std::string& oldFilename) override
    {
        auto it = callbacks.find(watchID);

        if (it != callbacks.end())
            it->second(dir, filename, ToFileAction(action));
    }
};

FileWatcher::FileWatcher()
    : m_Impl(std::make_unique<Impl>())
{
}

FileWatcher::~FileWatcher() = default;

WatchHandle FileWatcher::AddWatch(std::string_view directory, FileWatchCallback callback, bool recursive)
{
    efsw::WatchID watchID = m_Impl->watcher.addWatch(std::string(directory), m_Impl.get(), recursive);

    if (watchID < 0)
    {
        MT_CORE_ERROR("Failed to watch directory: {}", directory);
        return WatchHandle();
    }

    WatchHandle handle(m_Impl->nextHandleID++);

    m_Impl->watchIDs.emplace(handle.GetID(), watchID);
    m_Impl->callbacks.emplace(watchID, std::move(callback));

    return handle;
}

void FileWatcher::RemoveWatch(WatchHandle handle)
{
    auto it = m_Impl->watchIDs.find(handle.GetID());

    if (it == m_Impl->watchIDs.end())
        return;

    m_Impl->watcher.removeWatch(it->second);
    m_Impl->callbacks.erase(it->second);
    m_Impl->watchIDs.erase(it);
}

void FileWatcher::Watch()
{
    m_Impl->watcher.watch();
}
}  // namespace Matcha
