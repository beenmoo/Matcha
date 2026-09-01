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
        // storage<entt::entity>() iterates every identifier slot ever created, including
        // tombstones left behind by destroy() (kept around for version/recycling purposes) - a
        // destroyed entity has no components left, so without this check it would also satisfy
        // "no HierarchyComponent" and get misreported as a root.
        if (!m_Registry.valid(handle))
            continue;

        Entity entity(handle, this);

        if (!entity.HasComponent<HierarchyComponent>() || entity.GetComponent<HierarchyComponent>().parent == entt::null)
            roots.push_back(entity);
    }

    return roots;
}

Entity Scene::FindEntityByUUID(UUID id)
{
    for (entt::entity handle : View<TagComponent>())
    {
        Entity entity(handle, this);
        if (entity.GetComponent<TagComponent>().id == id)
            return entity;
    }

    return Entity();
}

void Scene::AddOnSceneChanged(std::function<void()> callback)
{
    m_OnSceneChanged.push_back(std::move(callback));
}

void Scene::NotifyChanged()
{
    for (auto& callback : m_OnSceneChanged)
        callback();
}
}  // namespace Matcha
