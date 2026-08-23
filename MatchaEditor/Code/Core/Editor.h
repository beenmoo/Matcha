#pragma once

#include <Matcha.h>

namespace Matcha
{
class Editor : public Application
{
public:
    Editor(const Application::ApplicationSpecification& spec);
    virtual ~Editor() = default;
};
}  // namespace Matcha