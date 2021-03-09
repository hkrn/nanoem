/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_UPDATEACCESSORYCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_UPDATEACCESSORYCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

namespace nanoem {

class Accessory;

namespace command {

class UpdateAccessoryCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    ~UpdateAccessoryCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, Accessory *accessory);
    static undo_command_t *create(
        Accessory *accessory, const Vector3 &lookAt, const Vector3 &angle, nanoem_f32_t distance, nanoem_f32_t fov);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    UpdateAccessoryCommand(
        Accessory *accessory, const Vector3 &lookAt, const Vector3 &angle, nanoem_f32_t distance, nanoem_f32_t fov);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    Accessory *m_accessory;
    tinystl::pair<Vector3, Vector3> m_translation;
    tinystl::pair<Vector3, Vector3> m_orientation;
    tinystl::pair<nanoem_f32_t, nanoem_f32_t> m_opacity;
    tinystl::pair<nanoem_f32_t, nanoem_f32_t> m_scaleFactor;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_UPDATEACCESSORYCOMMAND_H_ */
