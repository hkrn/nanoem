/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Forward.h"

namespace nanoem {
namespace test {

class IExecutor {
public:
    virtual ~IExecutor()
    {
    }
    virtual void start() = 0;
    virtual void finish() = 0;
};

} /* namespace test */
} /* namespace nanoem */
