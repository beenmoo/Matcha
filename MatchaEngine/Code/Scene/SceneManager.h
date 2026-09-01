#pragma once

#include "Scene.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Matcha
{
class ResourceManager;

// Owns the single currently-active Scene, by pointer rather than by value - "load a different
// scene" is a NewScene()/OpenScene() call that constructs a fresh Scene and swaps the pointer,
// not an in-place edit of a permanently-fixed object. Nothing should hold onto the Scene&
// GetScene() returns across more than one call: fetch it fresh each time (EngineContext::
// GetScene() delegates here for exactly this reason), and use AddOnSceneReplaced() to know when
// a previously fetched reference/subscription is no longer valid.
class SceneManager
{
public:
    // resourceManager is only needed for Open/Save (SceneSerializer resolves resource handles
    // through it) - held by reference, not owned, same lifetime relationship EngineContext
    // already has with it (Application owns both, SceneManager included).
    explicit SceneManager(ResourceManager& resourceManager);

    [[nodiscard]] Scene& GetScene()
    {
        return *m_Scene;
    }

    [[nodiscard]] const Scene& GetScene() const
    {
        return *m_Scene;
    }

    [[nodiscard]] const std::string& GetFilePath() const
    {
        return m_FilePath;
    }

    [[nodiscard]] bool IsDirty() const
    {
        return m_IsDirty;
    }

    // Replaces the current scene with a fresh, empty one.
    void NewScene();

    // Deserializes into a fresh Scene and swaps it in (rather than clearing and repopulating the
    // existing one) - if the file is missing/invalid, SceneSerializer logs and the fresh Scene is
    // just left empty, matching NewScene() rather than silently keeping stale content around.
    void OpenScene(const std::string& filepath);

    // No-ops (with a warning) if no file path is set yet - call SaveSceneAs() first.
    void SaveScene();
    void SaveSceneAs(const std::string& filepath);

    // Fired whenever the underlying Scene object is replaced (NewScene/OpenScene) - not on every
    // in-scene edit, that's Scene::AddOnSceneChanged. A Scene that's swapped out is destroyed
    // along with every callback subscribed to its own AddOnSceneChanged, so anything that cares
    // about scene content (Scene Hierarchy, Inspector) needs this to know when to re-subscribe to
    // whatever Scene is current now, not just when that scene's own content changes.
    void AddOnSceneReplaced(std::function<void()> callback);

private:
    void BindDirtyTracking();
    void NotifySceneReplaced();

private:
    ResourceManager& m_ResourceManager;
    std::unique_ptr<Scene> m_Scene;
    std::string m_FilePath;
    bool m_IsDirty = false;
    std::vector<std::function<void()>> m_OnSceneReplaced;
};
}  // namespace Matcha
