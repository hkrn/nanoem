/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#if defined(NANOEM_ENABLE_ICU)
#include "nanoem/ext/icu.h"
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateICU
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyICU
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackICU
#elif defined(NANOEM_ENABLE_MBWC)
#include "nanoem/ext/mbwc.h"
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateMBWC
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyMBWC
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackMBWC
#elif defined(__APPLE__)
#include "nanoem/ext/cfstring.h"
#if defined(NANOEM_ENABLE_CFSTRING)
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateCF
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyCF
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackCF
#endif
#endif /* NANOEM_ENABLE_EXT */

using namespace nanoem;

namespace Catch {

std::string
StringMaker<nanoem::String>::convert(const nanoem::String &value)
{
    return std::string(value.c_str(), value.size());
}

std::string
StringMaker<Quaternion>::convert(const Quaternion &value)
{
    return glm::to_string(value);
}

std::string
StringMaker<Vector2>::convert(const Vector2 &value)
{
    return glm::to_string(value);
}

std::string
StringMaker<Vector3>::convert(const Vector3 &value)
{
    return glm::to_string(value);
}

std::string
StringMaker<Vector4>::convert(const Vector4 &value)
{
    return glm::to_string(value);
}

std::string
StringMaker<Vector4U8>::convert(const Vector4U8 &value)
{
    return glm::to_string(value);
}

} /* namespace Catch */
