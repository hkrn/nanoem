/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_STUB_H_
#define NANOEM_EMAPP_INTERNAL_STUB_H_

#if !BX_CONFIG_SUPPORTS_THREADING
namespace bx {
struct Semaphore {
    Semaphore()
    {
    }
    ~Semaphore()
    {
    }
    void
    post()
    {
    }
    void
    wait()
    {
    }
};
}
#endif /* BX_CONFIG_SUPPORTS_THREADING */

#endif /* NANOEM_EMAPP_INTERNAL_STUB_H_ */
