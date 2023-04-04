#pragma once

#include "PlatformDetection.h"

#ifdef MT_DEBUG
#if defined(MT_PLATFORM_WINDOWS)
#define MT_DEBUG_BREAK __debugbreak()
#elif defined(MT_PLATFORM_LINUX)
#include <signal.h>
#define MT_DEBUG_BREAK raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define MT_ENABLE_ASSERTS
#else
#define MT_DEBUG_BREAK
#endif

#include "Logger.h"
#include "Assert.h"