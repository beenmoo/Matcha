#pragma once

#include "GLVertexArray.h"
#include "GLRenderData.h"

#include <vector>

namespace Matcha
{
class GLRenderer
{
public:
    GLRenderer();

    void Submit(const GLRenderData& renderData);
    void Flush();

private:
    void SortRenderData();

private:
    GLVertexArray mVertexArray;

    std::vector<GLRenderData> mRenderData;
};
}  // namespace Matcha