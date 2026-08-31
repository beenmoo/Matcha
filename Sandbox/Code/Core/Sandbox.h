#pragma once

#include <Matcha.h>

class Sandbox : public Application
{
public:
    Sandbox(const Application::ApplicationSpecification& spec);
    virtual ~Sandbox() = default;

protected:
    void OnUpdate() override;

private:
    Entity m_Cube;
};