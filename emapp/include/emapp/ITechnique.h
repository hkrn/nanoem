/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_ITECHNIQUE_H_
#define NANOEM_EMAPP_ITECHNIQUE_H_

#include "emapp/Forward.h"

namespace nanoem {

class IDrawable;
class IPass;

class ITechnique : private NonCopyable {
public:
    virtual ~ITechnique() NANOEM_DECL_NOEXCEPT
    {
    }
    virtual IPass *execute(const IDrawable *drawable, bool scriptExternalColor) = 0;
    virtual void resetScriptCommandState() = 0;
    virtual void resetScriptExternalColor() = 0;
    virtual bool hasNextScriptCommand() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_ITECHNIQUE_H_ */
