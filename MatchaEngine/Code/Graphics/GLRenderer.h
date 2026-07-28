#pragma once

#include "GLVertexArray.h"
#include "GLRenderData.h"

#include <queue>

namespace Matcha
{
    class GLRenderer
    {
    public:
        GLRenderer();

        void Submit(const GLRenderData& renderData);
        void Flush();

    private:
        GLVertexArray mVertexArray;

        std::queue<GLRenderData> mRenderData;
    };
}