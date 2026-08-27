#pragma once

#include "Core/KeyCodes.h"

#include <optional>

namespace Matcha
{
// Translates a Qt::Key value (from QKeyEvent::key()) to Matcha's KeyCode. KeyCode's values are
// literal SDL scancodes, which have no numeric relationship to Qt's key values, so this is an
// explicit table rather than a conversion. Returns std::nullopt for keys with no mapping (e.g.
// exotic media keys) - callers should just drop those, not assert.
[[nodiscard]] std::optional<KeyCode> ToKeyCode(int qtKey);
}  // namespace Matcha
