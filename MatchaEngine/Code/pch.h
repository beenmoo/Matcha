#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// glad is genuinely used everywhere (every GL* file, plus Application/Window backends), so it
// earns its place here despite most of those files also self-including it for standalone
// compilability. glm and spdlog are deliberately NOT here even though they're "engine-wide"
// concepts - each is only ever used from one or two .cpp files (Math/Matrix.cpp+Quaternion.cpp,
// Core/Logger.cpp respectively), which already include exactly what they need directly. Both are
// heavy, template-instantiation-costly headers; putting them in a PCH every other translation
// unit pays to load would slow down the whole engine's build for zero benefit to those other TUs.
#include <glad/glad.h>