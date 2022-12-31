/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_UPDATECAMERACOMMAND_H_
#define NANOEM_EMAPP_COMMAND_UPDATECAMERACOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

namespace nanoem {

class ICamera;

namespace command {

class UpdateCameraCommand NANOEM_DECL_SEALED : public BaseUndoCommand {
public:
    ~UpdateCameraCommand() NANOEM_DECL_NOEXCEPT;
    static undo_command_t *create(void *messagePtr, ICamera *camera, Project *project);
    static undo_command_t *create(Project *project, ICamera *camera, const Vector3 &lookAt, const Vector3 &angle,
        nanoem_f32_t distance, nanoem_f32_t fov, bool isPerspective);
    static undo_command_t *create(Project *project, ICamera *camera, const ICamera &source);

    void undo(Error &error);
    void redo(Error &error);
    const char *name() const NANOEM_DECL_NOEXCEPT;

private:
    UpdateCameraCommand(Project *project, ICamera *camera, const Vector3 &lookAt, const Vector3 &angle,
        nanoem_f32_t distance, nanoem_f32_t fov, bool isPerspective);

    void read(const void *messagePtr);
    void write(void *messagePtr);
    void release(void *messagePtr);

    const nanoem_frame_index_t m_localFrameIndex;
    ICamera *m_camera;
    tinystl::pair<Vector3, Vector3> m_lookAt;
    tinystl::pair<Vector3, Vector3> m_angle;
    tinystl::pair<nanoem_f32_t, nanoem_f32_t> m_distance;
    tinystl::pair<nanoem_f32_t, nanoem_f32_t> m_fov;
    tinystl::pair<bool, bool> m_perspective;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_UPDATECAMERACOMMAND_H_ */
