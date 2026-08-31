#pragma once

#include <Matcha.h>

class RotationComponent : public ScriptableEntity
{
protected:
    void OnUpdate() override;

private:
    float degreesPerSecond = 45.0f;
};
