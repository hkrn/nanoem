/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_UUID_H_
#define NANOEM_EMAPP_UUID_H_

#include "emapp/Forward.h"

#include "bx/hash.h"
#include "bx/os.h"
#include "bx/rng.h"
#include "bx/timer.h"

namespace nanoem {

class UUID NANOEM_DECL_SEALED {
public:
    template <typename TRandomNumberGenerator>
    static UUID
    create(TRandomNumberGenerator &rng)
    {
        UUID uuid;
        for (size_t i = 0; i < BX_COUNTOF(uuid.m_value); i++) {
            uuid.m_value[i] = static_cast<nanoem_u8_t>(rng.gen() % 16);
        }
        return uuid;
    }
    ~UUID() NANOEM_DECL_NOEXCEPT;
    const nanoem_u8_t *bytes() const NANOEM_DECL_NOEXCEPT;
    String toString() const;

private:
    UUID();
    nanoem_u8_t m_value[31];
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_RAY_H_ */
