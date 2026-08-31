#pragma once

#include <Matcha.h>

// Attach to any entity with a TransformComponent to give it a spot light that follows it around -
// creates its own light entity in OnCreate() and re-pins that light's transform to match its host
// every OnUpdate(), so binding this alongside other scripts (e.g. CameraController) works without
// either needing to know about the other.
class Flashlight : public ScriptableEntity
{
protected:
    void OnCreate() override;
    void OnUpdate() override;

private:
    Entity m_Light;
};
