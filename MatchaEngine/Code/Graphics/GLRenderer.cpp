#include "GLRenderer.h"
#include "Core/Core.h"

namespace Matcha
{
    GLRenderer::GLRenderer()
    {
    }

    void GLRenderer::Submit(const GLRenderData& renderData)
    {
        mRenderData.emplace(renderData);
    }

    void GLRenderer::Flush()
    {

    }
}