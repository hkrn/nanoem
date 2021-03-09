/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_IVECTORVALUESTATE_H_
#define NANOEM_EMAPP_IVECTORVALUESTATE_H_

#include "emapp/Forward.h"

namespace nanoem {

class IVectorValueState {
public:
    virtual ~IVectorValueState() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual const nanoem_f32_t *initialValue() const = 0;
    virtual const nanoem_f32_t *currentValue() const = 0;
    virtual nanoem_rsize_t numComponents() const = 0;
    virtual void setValue(const nanoem_f32_t *value) = 0;
    virtual void commit() = 0;
    virtual void rollback() = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_IVECTORVALUESTATE_H_ */
