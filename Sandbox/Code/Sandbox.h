#pragma once

#include <Matcha.h>

namespace Matcha
{
    class Sandbox : public Application
    {
    public:
        Sandbox(const Application::ApplicationSpecification& spec);
        virtual ~Sandbox() = default;
    };
}