/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_IUNDOCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_IUNDOCOMMAND_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace command {

class Error;
class Project;

class IUndoCommand {
public:
    virtual ~IUndoCommand()
    {
    }

    virtual void undo(Error &error) = 0;
    virtual void redo(Error &error) = 0;
    virtual Project *currentProject() = 0;
    virtual const char *name() const NANOEM_DECL_NOEXCEPT = 0;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_IUNDOCOMMAND_H_ */
