/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/UUID.h"

namespace nanoem {
namespace {

static void
fillBuffer(const nanoem_u8_t *p, int s, int e, char *&ptr)
{
    for (int i = s; i < e; i++) {
        nanoem_u8_t v = p[i];
        if (v < 10) {
            *ptr++ = static_cast<char>(v + '0');
        }
        else if (v >= 10 && v < 16) {
            *ptr++ = static_cast<char>((v - 10) + 'a');
        }
    }
}

} /* namespace anonymous */

UUID::~UUID() NANOEM_DECL_NOEXCEPT
{
}

const nanoem_u8_t *
UUID::bytes() const NANOEM_DECL_NOEXCEPT
{
    return m_value;
}

String
UUID::toString() const
{
    char buffer[37], *ptr = buffer;
    fillBuffer(m_value, 0, 8, ptr);
    *ptr++ = '-';
    fillBuffer(m_value, 8, 12, ptr);
    *ptr++ = '-';
    *ptr++ = static_cast<char>('4');
    fillBuffer(m_value, 12, 15, ptr);
    *ptr++ = '-';
    fillBuffer(m_value, 15, 19, ptr);
    *ptr++ = '-';
    fillBuffer(m_value, 19, 31, ptr);
    *ptr++ = 0;
    return String(buffer, sizeof(buffer));
}

UUID::UUID()
{
}

} /* namespace nanoem */
