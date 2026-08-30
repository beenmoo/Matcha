#pragma once

#include <Matcha.h>
#include <ranges>
#include <algorithm>
#include <concepts>

namespace MatchaEditor
{
template <typename ComponentType, typename Range>
requires std::ranges::range<Range>
bool AllEntitiesHaveComponent(const Range& entities)
{
    if (std::ranges::empty(entities))
        return false;

    // Use std::ranges::all_of to check every element seamlessly
    return std::ranges::all_of(entities, [](const auto& entity) {
        return entity.template HasComponent<ComponentType>();
    });
}
}  // namespace MatchaEditor