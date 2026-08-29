#pragma once

#include "Utility/UUID.h"

#include <string>

namespace Matcha
{
struct TagComponent
{
    std::string name = "Entity";
    UUID id;
    bool isActive = true;
};
}  // namespace Matcha