#include "SceneManager.h"
#include "SceneSerializer.h"
#include "Core/Logger.h"

namespace Matcha
{
SceneManager::SceneManager(ResourceManager& resourceManager)
    : m_ResourceManager(resourceManager),
      m_Scene(std::make_unique<Scene>())
{
    BindDirtyTracking();
}

void SceneManager::NewScene()
{
    m_Scene = std::make_unique<Scene>();
    m_FilePath.clear();
    m_IsDirty = false;

    BindDirtyTracking();
    NotifySceneReplaced();
}

void SceneManager::OpenScene(const std::string& filepath)
{
    auto scene = std::make_unique<Scene>();
    SceneSerializer::Deserialize(filepath, scene.get(), m_ResourceManager);

    m_Scene = std::move(scene);
    m_FilePath = filepath;
    m_IsDirty = false;

    BindDirtyTracking();
    NotifySceneReplaced();
}

void SceneManager::SaveScene()
{
    if (m_FilePath.empty())
    {
        MT_CORE_WARN("SceneManager::SaveScene() - no file path set yet; call SaveSceneAs() first.");
        return;
    }

    SceneSerializer::Serialize(m_FilePath, m_Scene.get(), m_ResourceManager);
    SetDirty(false);
}

void SceneManager::SaveSceneAs(const std::string& filepath)
{
    m_FilePath = filepath;
    SaveScene();
}

void SceneManager::AddOnSceneReplaced(std::function<void()> callback)
{
    m_OnSceneReplaced.push_back(std::move(callback));
}

void SceneManager::AddOnDirtyChanged(std::function<void()> callback)
{
    m_OnDirtyChanged.push_back(std::move(callback));
}

void SceneManager::BindDirtyTracking()
{
    m_Scene->AddOnSceneChanged([this] { SetDirty(true); });
}

void SceneManager::SetDirty(bool dirty)
{
    if (m_IsDirty == dirty)
        return;

    m_IsDirty = dirty;
    NotifyDirtyChanged();
}

void SceneManager::NotifySceneReplaced()
{
    for (auto& callback : m_OnSceneReplaced)
        callback();
}

void SceneManager::NotifyDirtyChanged()
{
    for (auto& callback : m_OnDirtyChanged)
        callback();
}
}  // namespace Matcha
