#pragma once

#include "Core/Command.h"

#include <Matcha.h>

#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace MatchaEditor
{
// Shared undo command for every Inspector field edit (Transform position/rotation/scale, Light/
// Camera/Material fields, TagComponent name/active, enum fields via an int cast at the call
// site) - one template instead of a hand-written Command per field type. `before` is captured
// per-entity (a multi-selection can have genuinely different starting values even though the
// field widget only displays the first selected entity's), `after` is a single value applied to
// every entity, matching the Inspector's existing "one edited value, applied to the whole
// selection" behavior.
template <typename ValueType>
class PropertyEditCommand : public Command
{
public:
    struct Edit
    {
        UUID id;
        ValueType before;
    };

    PropertyEditCommand(EngineContext& context, std::string description, std::vector<Edit> edits, ValueType after,
                        std::function<void(Entity, const ValueType&)> setter)
        : m_Context(context),
          m_Description(std::move(description)),
          m_Edits(std::move(edits)),
          m_After(std::move(after)),
          m_Setter(std::move(setter))
    {
    }

    void Execute() override
    {
        Scene& scene = m_Context.GetScene();

        for (const Edit& edit : m_Edits)
        {
            Entity entity = scene.FindEntityByUUID(edit.id);
            if (entity.IsValid())
                m_Setter(entity, m_After);
        }

        scene.NotifyChanged();
    }

    void Undo() override
    {
        Scene& scene = m_Context.GetScene();

        for (const Edit& edit : m_Edits)
        {
            Entity entity = scene.FindEntityByUUID(edit.id);
            if (entity.IsValid())
                m_Setter(entity, edit.before);
        }

        scene.NotifyChanged();
    }

    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

private:
    EngineContext& m_Context;
    std::string m_Description;
    std::vector<Edit> m_Edits;
    ValueType m_After;
    std::function<void(Entity, const ValueType&)> m_Setter;
};
}  // namespace MatchaEditor
