#pragma once

#include <unordered_map>
#include <string>
#include <initializer_list>
#include <utility>
#include <glad/glad.h>

namespace Matcha
{
    class Shader
    {
    public:
        Shader(std::string_view name, 
               const std::initializer_list<std::string>& paths);
        ~Shader();

        void Bind() const;
        void Unbind() const;

        const std::string& GetName() const;

    private:
        bool ParseFile(const std::string& path);
        bool CreateProgram();

    private:
        uint32_t mObjectID;

        std::string mName;
        std::unordered_map<GLenum, std::pair<std::string, std::string>> mSources;
    };
}