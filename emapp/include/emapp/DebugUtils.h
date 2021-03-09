/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_DEBUGUTILS_H_
#define NANOEM_EMAPP_DEBUGUTILS_H_

#include "emapp/Forward.h"
#include "emapp/StringUtils.h"

#include "bx/debug.h"
#include <stdio.h>

namespace nanoem {

class DebugUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    static inline void
    print(const char *format, ...)
    {
#if BX_PLATFORM_WINDOWS
        char buffer[4096];
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, sizeof(buffer), format, args);
        va_end(args);
        MutableWideString ws;
        StringUtils::getWideCharString(buffer, ws);
        OutputDebugStringW(ws.data());
        OutputDebugStringW(L"\n");
#else /* BX_PLATFORM_WINDOWS */
        va_list args;
        va_start(args, format);
        bx::debugPrintfVargs(format, args);
#if !BX_PLATFORM_OSX
        bx::debugOutput("\n");
#endif
        va_end(args);
#endif /* BX_PLATFORM_WINDOWS */
    }
    static inline void
    markUnimplemented(const char *name)
    {
#if !defined(NDEBUG)
        print("%s not implemented", name);
#else
        BX_UNUSED_1(name);
#endif
    }
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_DEBUGUTILS_H_ */
