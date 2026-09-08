#pragma once

#include "Scene/ComponentRegistry.h"

#include <Matcha.h>

#include <string>

namespace MatchaEditor
{
// Capture/restore of a single component's serialized data, shared by AddComponentCommand and
// RemoveComponentCommand. Both have the same problem: between Execute() and Undo() the component
// doesn't exist, so whatever data it held has to live somewhere until the other half of the cycle
// puts it back - a MeshComponent's assigned mesh, a PythonScriptComponent's bindings, not just a
// blank default. This is the same "snapshot, then replay" shape DeleteEntitiesCommand already uses
// for whole entities via SceneSerializer::SerializeEntities, scoped down to one component.
//
// Free functions taking Entity/ResourceManager rather than members on either command so they can
// be called - and tested - without needing an Application, SceneManager, or anything else only a
// running Application can construct.
//
// componentKey is the string ComponentRegistry.cpp registers the type under (e.g. "mesh",
// "light", "pythonScript"): the snapshot has to go through that specific entry's write()/read()
// pair, and ComponentSerializer has no way to answer "is this the entry for type Component" from
// the type alone.

[[nodiscard]] inline const ComponentSerializer* FindComponentSerializer(const std::string& componentKey)
{
    for (const ComponentSerializer& serializer : GetComponentSerializers())
        if (serializer.key == componentKey)
            return &serializer;

    return nullptr;
}

// Returns a json object with `componentKey` set to the component's data. Can legitimately come
// back without that key - an unregistered component type, or a registered one that had nothing
// worth writing (a PythonScriptComponent with no bindings, an imported mesh with no
// primitiveKind) - which RestoreComponent below treats as "restore a blank default".
[[nodiscard]] inline nlohmann::json CaptureComponent(Entity entity, const std::string& componentKey,
                                                     ResourceManager& resourceManager)
{
    nlohmann::json data;

    if (const ComponentSerializer* serializer = FindComponentSerializer(componentKey))
        serializer->write(data, entity, resourceManager);

    return data;
}

// The caller must have already established that `entity` does NOT currently have Component:
// ComponentRegistry's read() adds the component itself, and entt's emplace<T> asserts if the
// entity already has one.
//
// A `data` with no entry for componentKey (including a default-constructed/null json, which is
// what a command that has never captured anything holds) falls back to a blank default rather
// than doing nothing - restoring *something* beats silently dropping the component the undo/redo
// was supposed to bring back.
template <typename Component>
void RestoreComponent(Entity entity, const nlohmann::json& data, const std::string& componentKey,
                      ResourceManager& resourceManager)
{
    const ComponentSerializer* serializer = FindComponentSerializer(componentKey);

    if (serializer && data.contains(componentKey))
        serializer->read(data.at(componentKey), entity, resourceManager);
    else
        entity.AddComponent<Component>();
}
}  // namespace MatchaEditor
