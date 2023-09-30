#include "Shader.h"
#include "Core/Core.h"
#include "Utils/ShaderUtils.h"

#include <fstream>
#include <filesystem>
#include <vector>

namespace Matcha
{
    Shader::Shader(std::string_view name, 
                   const std::initializer_list<std::string>& paths) :
        mObjectID(glCreateProgram()),
        mName(name)
    {
        for (const auto& p : paths)
        {
            if (!ParseFile(p))
            {
                MT_CORE_ERROR("Failed to create shader: {0}", mName);

                return;
            }
        }

        if (!CreateProgram())
            MT_CORE_ERROR("Failed to create shader: {0}", mName);
    }

    Shader::~Shader()
    {
        glDeleteProgram(mObjectID);
    }

    void Shader::Bind() const
    {
        glUseProgram(mObjectID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    const std::string& Shader::GetName() const
    {
        return mName;
    }

    bool Shader::ParseFile(const std::string& path)
    {
        std::filesystem::path filePath(path);

        if (!filePath.has_extension())
        {
            MT_CORE_ERROR("Shader file does not have an extension: {0}", path);

            return false;
        }

        GLenum shaderType = ShaderUtils::ShaderTypeFromString(filePath.extension().string());

        if (shaderType == GL_NONE)
        {
            MT_CORE_ERROR("Shader type does not exist: {0}", filePath.extension().string());

            return false;
        }

        std::ifstream file(path, std::ios::in | std::ios::binary);

        if (!file.is_open())
        {
            MT_CORE_ERROR("Failed to open file: {0}", path);

            return false;
        }

        std::string source{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

        mSources[shaderType] = std::make_pair(path, source);

        return true;
    }
    
    bool Shader::CreateProgram()
    {
        if (mSources.find(GL_VERTEX_SHADER) == mSources.end() || 
            mSources.find(GL_FRAGMENT_SHADER) == mSources.end())
            return false;

        int32_t success = 0;

        std::vector<uint32_t> shaderIDs;

        for (const auto& s : mSources)
        {
            uint32_t shaderID = glCreateShader(s.first);
            shaderIDs.emplace_back(shaderID);

            const char* code = s.second.second.c_str();
            
            glShaderSource(shaderID, 1, &code, nullptr);
            glCompileShader(shaderID);

            glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
            
            if (!success)
            {
                std::vector<char> infoLog = ShaderUtils::GetShaderErrorInfo(shaderID, GL_COMPILE_STATUS);

                MT_CORE_ERROR("Shader compilation failed ({0}):\n{1}", s.second.first, infoLog.data());

                break;
            }

            glAttachShader(mObjectID, shaderID);
        }

        if (success)
        {
            glLinkProgram(mObjectID);
            glGetProgramiv(mObjectID, GL_LINK_STATUS, &success);

            if (!success)
            {
                std::vector<char> infoLog = ShaderUtils::GetShaderErrorInfo(mObjectID, GL_LINK_STATUS);

                MT_CORE_ERROR("Shader linking failed ({0}):\n{1}", mName, infoLog.data());
            }
        }

        for (const auto& id : shaderIDs)
            glDeleteShader(id);

        return success;
    }
}