#pragma once

#include <string>

namespace MatchaEditor
{
class Command
{
public:
    virtual ~Command() = default;

    virtual void Execute() = 0;
    virtual void Undo() = 0;

    // Shown in the Edit menu ("Undo <description>" / "Redo <description>").
    [[nodiscard]] virtual std::string GetDescription() const = 0;
};
}  // namespace MatchaEditor