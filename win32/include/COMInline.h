/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#include "emapp/Error.h"
#include "emapp/Forward.h"
#include "emapp/StringUtils.h"

namespace nanoem {
namespace win32 {

class COMInline : private NonCopyable {
public:
    template <typename TCOMInterface>
    static inline void
    safeRelease(TCOMInterface *&value)
    {
        if (value) {
            value->Release();
            value = nullptr;
        }
    }
    static inline void
    wrapCall(HRESULT result, Error &error)
    {
        if (FAILED(result) && !error.hasReason()) {
            wchar_t buffer[Error::kMaxReasonLength];
            MutableString msg;
            FormatMessageW(
                FORMAT_MESSAGE_FROM_SYSTEM, nullptr, result, LANG_USER_DEFAULT, buffer, ARRAYSIZE(buffer), 0);
            StringUtils::getMultiBytesString(buffer, msg);
            error = Error(msg.data(), result, Error::kDomainTypeOS);
        }
    }
};

} /* namespace win32 */
} /* namespace nanoem */
