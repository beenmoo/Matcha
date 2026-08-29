#include "Scene.h"
#include "Component/HierarchyComponent.h"
#include "Component/TagComponent.h"
#include "Component/TransformComponent.h"

#include <utility>

namespace Matcha
{
Entity Scene::CreateEntity(std::string name)
{
    return CreateEntity(UUID(), std::move(name));
}

Entity Scene::CreateEntity(UUID id, std::string name)
{
    Entity entity(m_Registry.create(), this);
    entity.AddComponent<TagComponent>();

    auto& tag = entity.GetComponent<TagComponent>();
    if (!name.empty())
        tag.name = std::move(name);
    tag.id = id;

    entity.AddComponent<TransformComponent>();

    NotifyChanged();

    return entity;
}

void Scene::DestroyEntity(Entity entity, bool notify)
{
    m_Registry.destroy(entity.GetHandle());
    if (notify)
        NotifyChanged();
}

std::vector<Entity> Scene::GetRootEntities()
{
    std::vector<Entity> roots;

    for (auto handle : m_Registry.storage<entt::entity>())
    {
        Entity entity(handle, this);

        if (!entity.HasComponent<HierarchyComponent>() || entity.GetComponent<HierarchyComponent>().parent == entt::null)
            roots.push_back(entity);
    }

    return roots;
}

void Scene::SetOnSceneChanged(std::function<void()> callback)
{
    m_OnSceneChanged = std::move(callback);
}

void Scene::NotifyChanged()
{
    if (m_OnSceneChanged)
        m_OnSceneChanged();
}
}  // namespace Matcha
