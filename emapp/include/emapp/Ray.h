/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_RAY_H_
#define NANOEM_EMAPP_RAY_H_

#include "emapp/Forward.h"

namespace nanoem {

struct Ray {
    Vector3 from;
    Vector3 to;
    Vector3 direction;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_RAY_H_ */
