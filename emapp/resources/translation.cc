/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ResourceBundle.h"

#include "emapp/DefaultTranslator.h"
#include "emapp/private/resources/pb_translations.h"

#include "lz4/lib/lz4.h"

namespace nanoem {
namespace resources {

bool
loadBuiltInTranslation(DefaultTranslator *translator)
{
    bool result = true;
    ByteArray bytes(pb_translations_inflated_size);
    int decompressedSize = LZ4_decompress_safe(reinterpret_cast<const char *>(pb_translations_data),
        reinterpret_cast<char *>(bytes.data()), pb_translations_deflated_size, pb_translations_inflated_size);
    result = translator->loadFromMemory(bytes.data(), decompressedSize);
    return result;
}

} /* namespace resources */
} /* namespace nanoem */
