/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_UPDATEMODELOBJECTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_UPDATEMODELOBJECTCOMMAND_H_

#include "emapp/command/BaseModelObjectCommand.h"

namespace nanoem {
namespace command {

class UpdateModelObjectCommand NANOEM_DECL_SEALED : public BaseModelObjectCommand {
public:
    ~UpdateModelObjectCommand() NANOEM_DECL_NOEXCEPT;

    static undo_command_t *create(Model *model, Object *o, const tinystl::pair<bool, bool> &value);
    static undo_command_t *create(Model *model, Object *o, const tinystl::pair<int, int> &value);
    static undo_command_t *create(Model *model, Object *o, const tinystl::pair<nanoem_f32_t, nanoem_f32_t> &value);
    static undo_command_t *create(Model *model, Object *o, const tinystl::pair<void *, void *> &value,
        destructor_t destructor, void *context, nanoem_rsize_t index = 0);
    static undo_command_t *create(
        Model *model, Object *o, const tinystl::pair<const void *, const void *> &value, nanoem_rsize_t index = 0);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    UpdateModelObjectCommand(Model *model, ExecutionBlock *block);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_UPDATEMODELOBJECTCOMMAND_H_ */
