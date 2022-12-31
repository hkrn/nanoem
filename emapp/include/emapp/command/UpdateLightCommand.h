/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_UPDATELIGHTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_UPDATELIGHTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

namespace nanoem {

class ILight;

namespace command {

class UpdateLightCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    ~UpdateLightCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(const void *messagePtr, ILight *light, Project *project);
    static undo_command_t *create(Project *project, ILight *light, const Vector3 &color, const Vector3 &direction);
    static undo_command_t *create(Project *project, ILight *light, const ILight &source);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    UpdateLightCommand(Project *project, ILight *light, const Vector3 &color, const Vector3 &direction);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    const nanoem_frame_index_t m_localFrameIndex;
    ILight *m_light;
    tinystl::pair<Vector3, Vector3> m_color;
    tinystl::pair<Vector3, Vector3> m_direction;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_UPDATELIGHTCOMMAND_H_ */
