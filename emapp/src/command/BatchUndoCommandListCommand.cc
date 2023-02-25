/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/command/BatchUndoCommandListCommand.h"

#include "../CommandMessage.inl"

#include "emapp/Accessory.h"
#include "emapp/private/CommonInclude.h"

#include "emapp/command/AddAccessoryKeyframeCommand.h"
#include "emapp/command/AddBoneKeyframeCommand.h"
#include "emapp/command/AddCameraKeyframeCommand.h"
#include "emapp/command/AddLightKeyframeCommand.h"
#include "emapp/command/AddModelKeyframeCommand.h"
#include "emapp/command/AddMorphKeyframeCommand.h"
#include "emapp/command/AddSelfShadowKeyframeCommand.h"
#include "emapp/command/InsertEmptyTimelineFrameCommand.h"
#include "emapp/command/MotionSnapshotCommand.h"
#include "emapp/command/RemoveAccessoryKeyframeCommand.h"
#include "emapp/command/RemoveBoneKeyframeCommand.h"
#include "emapp/command/RemoveCameraKeyframeCommand.h"
#include "emapp/command/RemoveLightKeyframeCommand.h"
#include "emapp/command/RemoveModelKeyframeCommand.h"
#include "emapp/command/RemoveMorphKeyframeCommand.h"
#include "emapp/command/RemoveSelfShadowKeyframeCommand.h"
#include "emapp/command/RemoveTimelineFrameCommand.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/command/TransformMorphCommand.h"
#include "emapp/command/UpdateAccessoryCommand.h"
#include "emapp/command/UpdateCameraCommand.h"
#include "emapp/command/UpdateLightCommand.h"

#include "undo/undo.h"

namespace nanoem {
namespace command {

BatchUndoCommandListCommand::~BatchUndoCommandListCommand() NANOEM_DECL_NOEXCEPT
{
    for (UndoCommandList::const_iterator it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
        undo_command_t *cmd = *it;
        undoCommandDestroy(cmd);
    }
    m_commands.clear();
}

undo_command_t *
BatchUndoCommandListCommand::create(const void *messagePtr, const Model *model, Project *project)
{
    BatchUndoCommandListCommand *command = nanoem_new(BatchUndoCommandListCommand(UndoCommandList(), model, project));
    command->read(messagePtr);
    return command->createCommand();
}

undo_command_t *
BatchUndoCommandListCommand::create(const UndoCommandList &commands, const Model *model, Project *project)
{
    undo_command_t *cmd = 0;
    if (commands.size() > 1) {
        BatchUndoCommandListCommand *command = nanoem_new(BatchUndoCommandListCommand(commands, model, project));
        cmd = command->createCommand();
    }
    else {
        cmd = commands[0];
    }
    return cmd;
}

void
BatchUndoCommandListCommand::undo(Error &error)
{
    BX_UNUSED_1(error);
    for (UndoCommandList::const_iterator it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
        undo_command_t *cmd = *it;
        if (undo_command_on_redo_t undo = undoCommandGetOnUndoCallback(cmd)) {
            undo(cmd);
        }
    }
}

void
BatchUndoCommandListCommand::redo(Error &error)
{
    BX_UNUSED_1(error);
    for (UndoCommandList::const_iterator it = m_commands.begin(), end = m_commands.end(); it != end; ++it) {
        undo_command_t *cmd = *it;
        if (undo_command_on_redo_t redo = undoCommandGetOnRedoCallback(cmd)) {
            redo(cmd);
        }
    }
}

const char *
BatchUndoCommandListCommand::name() const NANOEM_DECL_NOEXCEPT
{
    return "BatchUndoCommandListCommand";
}

BatchUndoCommandListCommand::BatchUndoCommandListCommand(
    const UndoCommandList &commands, const Model *model, Project *project)
    : BaseUndoCommand(project)
    , m_model(model)
    , m_commands(commands)
{
}

void
BatchUndoCommandListCommand::read(const void *messagePtr)
{
    if (const Nanoem__Application__RedoBatchUndoCommandListCommand *command =
            static_cast<const Nanoem__Application__Command *>(messagePtr)->redo_batch_undo_command_list) {
        Project *project = currentProject();
        for (nanoem_rsize_t i = 0, numCommands = command->n_commands; i < numCommands; i++) {
            const ProtobufCBinaryData &bd = command->commands[i];
            if (Nanoem__Application__Command *c =
                    nanoem__application__command__unpack(g_protobufc_allocator, bd.len, bd.data)) {
                undo_command_t *undo = 0;
                switch (c->type_case) {
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_ACCESSORY_KEYFRAME: {
                    const Nanoem__Application__RedoAccessoryKeyframeCommand *c2 = c->redo_add_accessory_keyframe;
                    Accessory *accessory = project->resolveRedoAccessory(c2->accessory_handle);
                    Motion *motion = project->resolveMotion(accessory);
                    if (accessory && motion) {
                        undo = AddAccessoryKeyframeCommand::create(c, accessory, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_BONE_KEYFRAME: {
                    const Nanoem__Application__RedoBoneKeyframeCommand *c2 = c->redo_add_bone_keyframe;
                    Model *model = project->resolveRedoModel(c2->model_handle);
                    Motion *motion = project->resolveMotion(model);
                    if (model && motion) {
                        undo = AddBoneKeyframeCommand::create(c, model, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_CAMERA_KEYFRAME: {
                    ICamera *camera = project->globalCamera();
                    Motion *motion = project->cameraMotion();
                    undo = AddCameraKeyframeCommand::create(c, camera, motion);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_LIGHT_KEYFRAME: {
                    ILight *light = project->globalLight();
                    Motion *motion = project->cameraMotion();
                    undo = AddLightKeyframeCommand::create(c, light, motion);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MODEL_KEYFRAME: {
                    const Nanoem__Application__RedoModelKeyframeCommand *c2 = c->redo_add_model_keyframe;
                    Model *model = project->resolveRedoModel(c2->model_handle);
                    Motion *motion = project->resolveMotion(model);
                    if (model && motion) {
                        undo = AddModelKeyframeCommand::create(c, model, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_MORPH_KEYFRAME: {
                    const Nanoem__Application__RedoMorphKeyframeCommand *c2 = c->redo_add_morph_keyframe;
                    Model *model = project->resolveRedoModel(c2->model_handle);
                    Motion *motion = project->resolveMotion(model);
                    if (model && motion) {
                        undo = AddMorphKeyframeCommand::create(c, model, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_ADD_SELF_SHADOW_KEYFRAME: {
                    ShadowCamera *shadow = project->shadowCamera();
                    Motion *motion = project->selfShadowMotion();
                    undo = AddSelfShadowKeyframeCommand::create(c, shadow, motion);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_ACCESSORY_KEYFRAME: {
                    const Nanoem__Application__RedoAccessoryKeyframeCommand *c2 = c->redo_remove_accessory_keyframe;
                    Accessory *accessory = project->resolveRedoAccessory(c2->accessory_handle);
                    Motion *motion = project->resolveMotion(accessory);
                    if (accessory && motion) {
                        undo = RemoveAccessoryKeyframeCommand::create(c, accessory, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_BONE_KEYFRAME: {
                    const Nanoem__Application__RedoBoneKeyframeCommand *c2 = c->redo_remove_bone_keyframe;
                    Model *model = project->resolveRedoModel(c2->model_handle);
                    Motion *motion = project->resolveMotion(model);
                    if (model && motion) {
                        undo = RemoveBoneKeyframeCommand::create(c, model, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_CAMERA_KEYFRAME: {
                    ICamera *camera = project->globalCamera();
                    Motion *motion = project->cameraMotion();
                    undo = RemoveCameraKeyframeCommand::create(c, camera, motion);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_LIGHT_KEYFRAME: {
                    ILight *light = project->globalLight();
                    Motion *motion = project->cameraMotion();
                    undo = RemoveLightKeyframeCommand::create(c, light, motion);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MODEL_KEYFRAME: {
                    const Nanoem__Application__RedoModelKeyframeCommand *c2 = c->redo_remove_model_keyframe;
                    Model *model = project->resolveRedoModel(c2->model_handle);
                    Motion *motion = project->resolveMotion(model);
                    if (model && motion) {
                        undo = RemoveModelKeyframeCommand::create(c, model, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_MORPH_KEYFRAME: {
                    const Nanoem__Application__RedoMorphKeyframeCommand *c2 = c->redo_remove_morph_keyframe;
                    Model *model = project->resolveRedoModel(c2->model_handle);
                    Motion *motion = project->resolveMotion(model);
                    if (model && motion) {
                        undo = RemoveMorphKeyframeCommand::create(c, model, motion);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_SELF_SHADOW_KEYFRAME: {
                    ShadowCamera *shadow = project->shadowCamera();
                    Motion *motion = project->selfShadowMotion();
                    undo = RemoveSelfShadowKeyframeCommand::create(c, shadow, motion);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_SAVE_MOTION_SNAPSHOT: {
                    undo = MotionSnapshotCommand::create(c, project);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_INSERT_EMPTY_TIMELINE_FRAME: {
                    undo = InsertEmptyTimelineFrameCommand::create(c, project);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_REMOVE_TIMELINE_FRAME: {
                    undo = RemoveTimelineFrameCommand::create(c, project);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_BONE: {
                    const Nanoem__Application__RedoTransformBoneCommand *c2 = c->redo_transform_bone;
                    if (Model *model = project->resolveRedoModel(c2->model_handle)) {
                        undo = TransformBoneCommand::create(c, model, project);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_TRANSFORM_MORPH: {
                    const Nanoem__Application__RedoTransformMorphCommand *c2 = c->redo_transform_morph;
                    if (Model *model = project->resolveRedoModel(c2->model_handle)) {
                        undo = TransformMorphCommand::create(c, model, project);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_ACCESSORY: {
                    const Nanoem__Application__RedoUpdateAccessoryCommand *c2 = c->redo_update_accessory;
                    if (Accessory *accessory = project->resolveRedoAccessory(c2->accessory_handle)) {
                        undo = UpdateAccessoryCommand::create(c, accessory);
                    }
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_CAMERA: {
                    undo = UpdateCameraCommand::create(c, project->globalCamera(), project);
                    break;
                }
                case NANOEM__APPLICATION__COMMAND__TYPE_REDO_UPDATE_LIGHT: {
                    undo = UpdateLightCommand::create(c, project->globalLight(), project);
                    break;
                }
                default:
                    break;
                }
                if (undo) {
                    m_commands.push_back(undo);
                }
                nanoem__application__command__free_unpacked(c, g_protobufc_allocator);
            }
        }
    }
}

void
BatchUndoCommandListCommand::write(void *messagePtr)
{
    Nanoem__Application__RedoBatchUndoCommandListCommand *command =
        nanoem_new(Nanoem__Application__RedoBatchUndoCommandListCommand);
    nanoem__application__redo_batch_undo_command_list_command__init(command);
    command->n_commands = m_commands.size();
    command->commands = new ProtobufCBinaryData[command->n_commands];
    if (m_model) {
        command->has_model_handle = 1;
        command->model_handle = m_model->handle();
    }
    nanoem_rsize_t offset = 0;
    for (UndoCommandList::const_iterator it = m_commands.begin(), end = m_commands.end(); it != end; ++it, ++offset) {
        const undo_command_t *c = *it;
        BaseUndoCommand *commandPtr = static_cast<BaseUndoCommand *>(undoCommandGetOpaqueData(c));
        Nanoem__Application__Command action = NANOEM__APPLICATION__COMMAND__INIT;
        commandPtr->write(&action);
        ProtobufCBinaryData &binary = command->commands[offset];
        binary.len = nanoem__application__command__get_packed_size(&action);
        binary.data = new nanoem_u8_t[binary.len];
        nanoem__application__command__pack(&action, binary.data);
        commandPtr->release(&action);
    }
    writeCommandMessage(command, NANOEM__APPLICATION__COMMAND__TYPE_REDO_BATCH_UNDO_COMMAND_LIST, messagePtr);
}

void
BatchUndoCommandListCommand::release(void *messagePtr)
{
    Nanoem__Application__RedoBatchUndoCommandListCommand *command =
        static_cast<Nanoem__Application__Command *>(messagePtr)->redo_batch_undo_command_list;
    for (nanoem_rsize_t i = 0, numCommands = command->n_commands; i < numCommands; i++) {
        ProtobufCBinaryData &binary = command->commands[i];
        delete[] binary.data;
    }
    delete[] command->commands;
    nanoem_delete(command);
}

} /* namespace command */
} /* namespace nanoem */
