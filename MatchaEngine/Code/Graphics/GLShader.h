#pragma once

#include "Core/Types.h"

#include <unordered_map>
#include <string>
#include <initializer_list>
#include <utility>
#include <glad/glad.h>

namespace Matcha
{
    class GLShader
    {
    public:
        GLShader(
            std::string_view name,
            const std::initializer_list<String>& paths);
        ~GLShader();

        void Bind() const;
        void Unbind() const;

        const String& GetName() const;

    private:
        bool ParseFile(const String& path);
        bool CreateProgram();

    private:
        GLuint mObjectID;

        String mName;
        std::unordered_map<GLenum, std::pair<String, String>> mSources;
    };
}