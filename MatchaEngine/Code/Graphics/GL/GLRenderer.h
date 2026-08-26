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

private:
    void SortRenderData();

private:
    GLResourceManager& m_ResourceManager;

    std::vector<RenderData> m_RenderData;
};
}  // namespace Matcha
