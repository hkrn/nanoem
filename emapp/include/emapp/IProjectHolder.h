/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IPROJECTHOLDER_H_
#define NANOEM_EMAPP_IPROJECTHOLDER_H_

#include "emapp/Forward.h"

namespace nanoem {

class Project;

class IProjectHolder {
public:
    virtual ~IProjectHolder() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual const Project *currentProject() const = 0;
    virtual Project *currentProject() = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IPROJECT_HOLDER_H_ */
