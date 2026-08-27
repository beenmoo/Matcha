#pragma once

#include "Graphics/Renderer.h"
#include "GLResourceManager.h"

#include <vector>

namespace Matcha
{
class GLRenderer final : public Renderer
{
public:
    explicit GLRenderer(GLResourceManager& resourceManager);

    void Submit(const RenderData& renderData) override;
    void Flush() override;
    void Clear() override;
    void SetClearColor(const Vector4& color) override;

private:
    void SortRenderData();

private:
    GLResourceManager& m_ResourceManager;

    std::vector<RenderData> m_RenderData;
    Vector4 m_ClearColor{0.0f, 0.0f, 0.0f, 1.0f};
};
}  // namespace Matcha
