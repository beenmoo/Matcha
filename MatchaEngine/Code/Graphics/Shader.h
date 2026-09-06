#pragma once

#include "Math/Matrix.h"
#include "Math/Vector.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Matcha
{
class Shader
{
public:
    virtual ~Shader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual bool Reload() = 0;

    virtual void SetMat4(std::string_view name, const Matrix4& value) = 0;
    virtual void SetFloat(std::string_view name, float value) = 0;
    virtual void SetFloat3(std::string_view name, const Vector3& value) = 0;
    virtual void SetFloat4(std::string_view name, const Vector4& value) = 0;
    virtual void SetInt(std::string_view name, int value) = 0;

    [[nodiscard]] const std::vector<std::string>& GetPaths() const
    {
        return m_Paths;
    }


protected:
    void SetPaths(std::vector<std::string> paths)
    {
        m_Paths = std::move(paths);
    }

private:
    std::vector<std::string> m_Paths;
};
}  // namespace Matcha
