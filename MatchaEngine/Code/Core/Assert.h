#pragma once

#include "Core/Debug.h"
#include "Core/Logger.h"

#ifdef MT_ENABLE_ASSERTS
#define MT_ASSERT(x, msg)                                                                                 \
    if ((x))                                                                                              \
    {                                                                                                     \
    }                                                                                                     \
    else                                                                                                  \
    {                                                                                                     \
        MT_CORE_CRITICAL("ASSERT - {}\n\t{}\n\tin file: {}\n\ton line: {}", #x, msg, __FILE__, __LINE__); \
        MT_DEBUG_BREAK;                                                                                   \
    }
#else
#define MT_ASSERT(...)
#endif