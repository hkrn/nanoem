/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ResourceBundle.h"

#include "emapp/private/resources/credits.h"

namespace nanoem {
namespace resources {

void
getCredits(const nanoem_u8_t *&text, size_t &size)
{
    text = credits_md_data;
    size = credits_md_size;
}

} /* namespace resources */
} /* namespace nanoem */
