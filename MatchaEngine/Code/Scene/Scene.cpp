#include "Scene.h"

namespace Matcha
{
Entity Scene::CreateEntity()
{
    return Entity(m_Registry.create(), this);
}

void Scene::DestroyEntity(Entity entity)
{
    m_Registry.destroy(entity.GetHandle());
}
}  // namespace Matcha
