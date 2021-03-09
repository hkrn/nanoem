/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ENUMUTILS_H_
#define NANOEM_EMAPP_ENUMUTILS_H_

#include "emapp/Forward.h"

namespace nanoem {

class EnumUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    template <typename T>
    static inline void
    setEnabledT(const T mask, T &flags, bool enabled) NANOEM_DECL_NOEXCEPT
    {
        flags = enabled ? (flags | mask) : (flags & ~mask);
    }
    static inline void
    setEnabled(const nanoem_u32_t mask, nanoem_u32_t &flags, bool enabled) NANOEM_DECL_NOEXCEPT
    {
        setEnabledT<nanoem_u32_t>(mask, flags, enabled);
    }
    static inline void
    setEnabled(const nanoem_u64_t mask, nanoem_u64_t &flags, bool enabled) NANOEM_DECL_NOEXCEPT
    {
        setEnabledT<nanoem_u64_t>(mask, flags, enabled);
    }
    template <typename T>
    static inline bool
    isEnabledT(const T mask, const T flags) NANOEM_DECL_NOEXCEPT
    {
        return (flags & mask) != 0;
    }
    static inline bool
    isEnabled(const nanoem_u16_t mask, const nanoem_u16_t flags) NANOEM_DECL_NOEXCEPT
    {
        return isEnabledT<nanoem_u16_t>(mask, flags);
    }
    static inline bool
    isEnabled(const nanoem_u32_t mask, const nanoem_u32_t flags) NANOEM_DECL_NOEXCEPT
    {
        return isEnabledT<nanoem_u32_t>(mask, flags);
    }
    static inline bool
    isEnabled(const nanoem_u64_t mask, const nanoem_u64_t flags) NANOEM_DECL_NOEXCEPT
    {
        return isEnabledT<nanoem_u64_t>(mask, flags);
    }
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ENUMUTILS_H_ */
