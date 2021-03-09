/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/BaseApplicationClient.h"

#include "emapp/ApplicationPreference.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationService.h"
#include "emapp/internal/ApplicationUtils.h"
#include "emapp/private/CommonInclude.h"

#include "./protoc/application.pb-c.h"

#include "bx/handlealloc.h"
#include "protobuf-c/protobuf-c.h"
#include "sokol/sokol_time.h"

namespace nanoem {

BaseApplicationClient::InitializeMessageDescription::InitializeMessageDescription(const Vector2UI32 &logicalWindowSize,
    sg_pixel_format pixelFormat, nanoem_f32_t windowDevicePixelRatio, const String &sokolPath)
    : m_vendorId(0)
    , m_deviceId(0)
    , m_rendererType(0)
    , m_logicalWindowSize(logicalWindowSize)
    , m_windowDevicePixelRatio(windowDevicePixelRatio)
    , m_viewportDevicePixelRatio(windowDevicePixelRatio)
    , m_preferredEditingFPS(60)
    , m_sokolDllPath(sokolPath)
    , m_bufferPoolSize(ApplicationPreference::kGFXBufferPoolSizeDefaultValue)
    , m_imagePoolSize(ApplicationPreference::kGFXImagePoolSizeDefaultValue)
    , m_shaderPoolSize(ApplicationPreference::kGFXShaderPoolSizeDefaultValue)
    , m_pipelinePoolSize(ApplicationPreference::kGFXPipelinePoolSizeDefaultValue)
    , m_passPoolSize(ApplicationPreference::kGFXPassPoolSizeDefaultValue)
    , m_metalGlobalUniformBufferSize(ApplicationPreference::kGFXUniformBufferSizeDefaultValue)
    , m_pixelFormat(pixelFormat)
{
}

BaseApplicationClient::BaseApplicationClient()
{
}

BaseApplicationClient::~BaseApplicationClient() NANOEM_DECL_NOEXCEPT
{
}

void
BaseApplicationClient::sendInitializeMessage(const InitializeMessageDescription &desc)
{
    Nanoem__Application__InitializeCommand action = NANOEM__APPLICATION__INITIALIZE_COMMAND__INIT;
    action.device_id = desc.m_deviceId;
    action.window_device_pixel_ratio = desc.m_windowDevicePixelRatio;
    action.viewport_viewport_pixel_ratio = desc.m_viewportDevicePixelRatio;
    action.has_viewport_viewport_pixel_ratio = 1;
    action.renderer_type = desc.m_rendererType;
    action.vendor_id = desc.m_vendorId;
    action.window_width = desc.m_logicalWindowSize.x;
    action.window_height = desc.m_logicalWindowSize.y;
    action.preferred_editing_fps = desc.m_preferredEditingFPS;
    action.buffer_pool_size = desc.m_bufferPoolSize;
    action.image_pool_size = desc.m_imagePoolSize;
    action.shader_pool_size = desc.m_shaderPoolSize;
    action.pipeline_pool_size = desc.m_pipelinePoolSize;
    action.pass_pool_size = desc.m_passPoolSize;
    action.mtl_global_uniform_buffer_size = desc.m_metalGlobalUniformBufferSize;
    action.pixel_format = static_cast<nanoem_u32_t>(desc.m_pixelFormat);
    action.has_pixel_format = 1;
    MutableString sokolDllPath;
    action.sokol_dll_path = StringUtils::cloneString(desc.m_sokolDllPath.c_str(), sokolDllPath);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_INITIALIZE;
    command.initialize = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRenderFrameMessage()
{
    Nanoem__Application__RenderFrameCommand core = NANOEM__APPLICATION__ACTIVATE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_RENDER_FRAME;
    command.render_frame = &core;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendActivateMessage()
{
    Nanoem__Application__ActivateCommand core = NANOEM__APPLICATION__ACTIVATE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_ACTIVATE;
    command.activate = &core;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendDeactivateMessage()
{
    Nanoem__Application__DeactivateCommand action = NANOEM__APPLICATION__DEACTIVATE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_DEACTIVATE;
    command.deactivate = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendResizeWindowMessage(const Vector2UI32 &size)
{
    Nanoem__Application__WindowResizedCommand action = NANOEM__APPLICATION__WINDOW_RESIZED_COMMAND__INIT;
    action.width = size.x;
    action.height = size.y;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_WINDOW_RESIZED;
    command.window_resized = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendResizeViewportMessage(const Vector2UI32 &size)
{
    Nanoem__Application__ViewportResizedCommand action = NANOEM__APPLICATION__VIEWPORT_RESIZED_COMMAND__INIT;
    action.width = size.x;
    action.height = size.y;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_VIEWPORT_RESIZED;
    command.viewport_resized = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendCursorPressMessage(const Vector2SI32 &coord, int type, int modifiers)
{
    Nanoem__Application__CursorPressedCommand action;
    nanoem__application__cursor_pressed_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    action.modifiers = modifiers;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_PRESSED;
    command.cursor_pressed = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendCursorMoveMessage(
    const Vector2SI32 &coord, const Vector2SI32 &delta, int type, int modifiers)
{
    Nanoem__Application__CursorMovedCommand action;
    nanoem__application__cursor_moved_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.delta_x = delta.x;
    action.delta_y = delta.y;
    action.type = type;
    action.modifiers = modifiers;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_MOVED;
    command.cursor_moved = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendCursorReleaseMessage(const Vector2SI32 &coord, int type, int modifiers)
{
    Nanoem__Application__CursorReleasedCommand action;
    nanoem__application__cursor_released_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    action.modifiers = modifiers;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_RELEASED;
    command.cursor_released = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendCursorScrollMessage(const Vector2SI32 &coord, const Vector2SI32 &delta, int modifiers)
{
    Nanoem__Application__CursorScrolledCommand action;
    nanoem__application__cursor_scrolled_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.delta_x = delta.x;
    action.delta_y = delta.y;
    action.modifiers = modifiers;
    action.has_modifiers = modifiers != 0;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CURSOR_SCROLLED;
    command.cursor_scrolled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendPointerPressMessage(const Vector2 &coord, int type)
{
    Nanoem__Application__PointerPressedCommand action;
    nanoem__application__pointer_pressed_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_POINTER_PRESSED;
    command.pointer_pressed = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendPointerMoveMessage(const Vector2 &coord, const Vector2 &delta, int type)
{
    Nanoem__Application__PointerMovedCommand action;
    nanoem__application__pointer_moved_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.delta_x = delta.x;
    action.delta_y = delta.y;
    action.type = type;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_POINTER_MOVED;
    command.pointer_moved = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendPointerReleaseMessage(const Vector2 &coord, int type)
{
    Nanoem__Application__PointerReleasedCommand action;
    nanoem__application__pointer_released_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_POINTER_RELEASED;
    command.pointer_released = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendPointerScrollMessage(const Vector2 &coord, const Vector2 &delta, int modifiers)
{
    Nanoem__Application__PointerScrolledCommand action;
    nanoem__application__pointer_scrolled_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.delta_x = delta.x;
    action.delta_y = delta.y;
    action.modifiers = modifiers;
    action.has_modifiers = modifiers != 0;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_POINTER_SCROLLED;
    command.pointer_scrolled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendKeyPressMessage(nanoem_u32_t value)
{
    Nanoem__Application__KeyPressedCommand action = NANOEM__APPLICATION__KEY_PRESSED_COMMAND__INIT;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_KEY_PRESSED;
    command.key_pressed = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendKeyReleaseMessage(nanoem_u32_t value)
{
    Nanoem__Application__KeyReleasedCommand action = NANOEM__APPLICATION__KEY_RELEASED_COMMAND__INIT;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_KEY_RELEASED;
    command.key_released = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendUnicodeInputMessage(nanoem_u32_t value)
{
    Nanoem__Application__UnicodeInputCommand action = NANOEM__APPLICATION__UNICODE_INPUT_COMMAND__INIT;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_UNICODE_INPUT;
    command.unicode_input = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendMenuActionMessage(nanoem_u32_t value)
{
    Nanoem__Application__MenuActionCommand action = NANOEM__APPLICATION__MENU_ACTION_COMMAND__INIT;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_MENU_ACTION;
    command.menu_action = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendLoadFileMessage(const URI &fileURI, nanoem_u32_t type)
{
    Nanoem__Application__LoadFileCommand action = NANOEM__APPLICATION__LOAD_FILE_COMMAND__INIT;
    Nanoem__Application__URI uri;
    MutableString absolutePath, fragment;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    action.type = type;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_LOAD_FILE;
    command.load_file = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSaveFileMessage(const URI &fileURI, nanoem_u32_t type)
{
    Nanoem__Application__SaveFileCommand action = NANOEM__APPLICATION__SAVE_FILE_COMMAND__INIT;
    Nanoem__Application__URI uri;
    MutableString absolutePath, fragment;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    action.type = type;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SAVE_FILE;
    command.save_file = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendDropFileMessage(const URI &fileURI)
{
    Nanoem__Application__DropFileCommand action = NANOEM__APPLICATION__DROP_FILE_COMMAND__INIT;
    Nanoem__Application__URI uri;
    MutableString absolutePath, fragment;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_DROP_FILE;
    command.drop_file = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendNewProjectMessage()
{
    Nanoem__Application__NewProjectCommand action = NANOEM__APPLICATION__NEW_PROJECT_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_NEW_PROJECT;
    command.new_project = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendConfirmBeforeNewProjectMessage()
{
    Nanoem__Application__ConfirmBeforeNewProjectCommand action =
        NANOEM__APPLICATION__CONFIRM_BEFORE_NEW_PROJECT_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_NEW_PROJECT;
    command.confirm_before_new_project = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendConfirmBeforeOpenProjectMessage()
{
    Nanoem__Application__ConfirmBeforeOpenProjectCommand action =
        NANOEM__APPLICATION__CONFIRM_BEFORE_OPEN_PROJECT_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_OPEN_PROJECT;
    command.confirm_before_open_project = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendConfirmBeforeExitApplicationMessage()
{
    Nanoem__Application__ConfirmBeforeExitApplicationCommand action =
        NANOEM__APPLICATION__CONFIRM_BEFORE_EXIT_APPLICATION_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_EXIT_APPLICATION;
    command.confirm_before_exit_application = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendConfirmBeforeExportingImageMessage()
{
    Nanoem__Application__ConfirmBeforeExportingImageCommand action =
        NANOEM__APPLICATION__CONFIRM_BEFORE_EXPORTING_IMAGE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_EXPORTING_IMAGE;
    command.confirm_before_exporting_image = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendConfirmBeforeExportingVideoMessage()
{
    Nanoem__Application__ConfirmBeforeExportingVideoCommand action =
        NANOEM__APPLICATION__CONFIRM_BEFORE_EXPORTING_VIDEO_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CONFIRM_BEFORE_EXPORTING_VIDEO;
    command.confirm_before_exporting_video = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendChangeDevicePixelRatioMessage(nanoem_f32_t value)
{
    Nanoem__Application__ChangeDevicePixelRatioCommand action =
        NANOEM__APPLICATION__CHANGE_DEVICE_PIXEL_RATIO_COMMAND__INIT;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CHANGE_DEVICE_PIXEL_RATIO;
    command.change_device_pixel_ratio = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetActiveAccessoryMessage(nanoem_u16_t handle)
{
    Nanoem__Application__SetActiveAccessoryCommand action = NANOEM__APPLICATION__SET_ACTIVE_ACCESSORY_COMMAND__INIT;
    action.accessory_handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_ACCESSORY;
    command.set_active_accessory = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetActiveModelMessage(nanoem_u16_t handle)
{
    Nanoem__Application__SetActiveModelCommand action = NANOEM__APPLICATION__SET_ACTIVE_MODEL_COMMAND__INIT;
    action.model_handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL;
    command.set_active_model = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetActiveModelBoneMessage(const char *name)
{
    Nanoem__Application__SetActiveModelBoneCommand action = NANOEM__APPLICATION__SET_ACTIVE_MODEL_BONE_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.name = s.data();
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL_BONE;
    command.set_active_model_bone = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetActiveModelMorphMessage(const char *name, bool applyCategory)
{
    Nanoem__Application__SetActiveModelMorphCommand action = NANOEM__APPLICATION__SET_ACTIVE_MODEL_MORPH_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.name = s.data();
    if (applyCategory) {
        action.has_apply_category = action.apply_category = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACTIVE_MODEL_MORPH;
    command.set_active_model_morph = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendLoadAllDecoderPluginsMessage(const URIList &values)
{
    Nanoem__Application__LoadAllDecoderPluginsCommand action =
        NANOEM__APPLICATION__LOAD_ALL_DECODER_PLUGINS_COMMAND__INIT;
    MutableStringList absolutePathList, fragmentList;
    internal::ApplicationUtils::allocateURIList(
        values, action.file_uris, action.n_file_uris, absolutePathList, fragmentList);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_DECODER_PLUGINS;
    command.load_all_decoder_plugins = &action;
    sendCommandMessage(&command);
    internal::ApplicationUtils::freeURIList(action.file_uris, action.n_file_uris);
}

void
BaseApplicationClient::sendLoadAllEncoderPluginsMessage(const URIList &values)
{
    Nanoem__Application__LoadAllEncoderPluginsCommand action =
        NANOEM__APPLICATION__LOAD_ALL_ENCODER_PLUGINS_COMMAND__INIT;
    MutableStringList absolutePathList, fragmentList;
    internal::ApplicationUtils::allocateURIList(
        values, action.file_uris, action.n_file_uris, absolutePathList, fragmentList);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_ENCODER_PLUGINS;
    command.load_all_encoder_plugins = &action;
    sendCommandMessage(&command);
    internal::ApplicationUtils::freeURIList(action.file_uris, action.n_file_uris);
}

void
BaseApplicationClient::sendSeekMessage(nanoem_frame_index_t value)
{
    Nanoem__Application__SeekCommand action = NANOEM__APPLICATION__SEEK_COMMAND__INIT;
    action.local_frame_index = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SEEK;
    command.seek = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetCameraLookAtMessage(const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetCameraLookAtCommand action = NANOEM__APPLICATION__SET_CAMERA_LOOK_AT_COMMAND__INIT;
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    action.can_undo = canUndo;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_LOOK_AT;
    command.set_camera_look_at = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetCameraAngleMessage(const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetCameraAngleCommand action = NANOEM__APPLICATION__SET_CAMERA_ANGLE_COMMAND__INIT;
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_ANGLE;
    command.set_camera_angle = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetCameraFovMessage(nanoem_f32_t value, bool canUndo)
{
    Nanoem__Application__SetCameraFovCommand action = NANOEM__APPLICATION__SET_CAMERA_FOV_COMMAND__INIT;
    action.fov = value;
    action.can_undo = canUndo;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_FOV;
    command.set_camera_fov = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetCameraDistanceMessage(nanoem_f32_t value, bool canUndo)
{
    Nanoem__Application__SetCameraDistanceCommand action = NANOEM__APPLICATION__SET_CAMERA_DISTANCE_COMMAND__INIT;
    action.distance = value;
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_DISTANCE;
    command.set_camera_distance = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetLightColorMessage(const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetLightColorCommand action = NANOEM__APPLICATION__SET_LIGHT_COLOR_COMMAND__INIT;
    action.red = value.x;
    action.green = value.y;
    action.blue = value.z;
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_LIGHT_COLOR;
    command.set_light_color = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetLightDirectionMessage(const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetLightDirectionCommand action = NANOEM__APPLICATION__SET_LIGHT_DIRECTION_COMMAND__INIT;
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_LIGHT_DIRECTION;
    command.set_light_direction = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryTranslationMessage(nanoem_u16_t handle, const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetAccessoryTranslationCommand action =
        NANOEM__APPLICATION__SET_ACCESSORY_TRANSLATION_COMMAND__INIT;
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    if (handle != bx::kInvalidHandle) {
        action.has_accessory_handle = 1;
        action.accessory_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_TRANSLATION;
    command.set_accessory_translation = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryOrientationMessage(nanoem_u16_t handle, const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetAccessoryOrientationCommand action =
        NANOEM__APPLICATION__SET_ACCESSORY_ORIENTATION_COMMAND__INIT;
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    if (handle != bx::kInvalidHandle) {
        action.has_accessory_handle = 1;
        action.accessory_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_ORIENTATION;
    command.set_accessory_orientation = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryScaleFactorMessage(nanoem_u16_t handle, nanoem_f32_t value, bool canUndo)
{
    Nanoem__Application__SetAccessoryScaleFactorCommand action =
        NANOEM__APPLICATION__SET_ACCESSORY_SCALE_FACTOR_COMMAND__INIT;
    action.scale_factor = value;
    if (handle != bx::kInvalidHandle) {
        action.has_accessory_handle = 1;
        action.accessory_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_SCALE_FACTOR;
    command.set_accessory_scale_factor = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryOpacityMessage(nanoem_u16_t handle, nanoem_f32_t value, bool canUndo)
{
    Nanoem__Application__SetAccessoryOpacityCommand action = NANOEM__APPLICATION__SET_ACCESSORY_OPACITY_COMMAND__INIT;
    action.opacity = value;
    if (handle != bx::kInvalidHandle) {
        action.has_accessory_handle = 1;
        action.accessory_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_OPACITY;
    command.set_accessory_opacity = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelMorphWeightMessage(
    nanoem_u16_t handle, const String &name, nanoem_f32_t value, bool canUndo)
{
    Nanoem__Application__SetModelMorphWeightCommand action = NANOEM__APPLICATION__SET_MODEL_MORPH_WEIGHT_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.morph_name = s.data();
    action.weight = value;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_MORPH_WEIGHT;
    command.set_model_morph_weight = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelBoneTranslationMessage(
    nanoem_u16_t handle, const String &name, const Vector3 &value, bool canUndo)
{
    Nanoem__Application__SetModelBoneTranslationCommand action =
        NANOEM__APPLICATION__SET_MODEL_BONE_TRANSLATION_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.bone_name = s.data();
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_BONE_TRANSLATION;
    command.set_model_bone_translation = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelBoneOrientationMessage(
    nanoem_u16_t handle, const String &name, const Quaternion &value, bool canUndo)
{
    Nanoem__Application__SetModelBoneOrientationCommand action =
        NANOEM__APPLICATION__SET_MODEL_BONE_ORIENTATION_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.bone_name = s.data();
    action.x = value.x;
    action.y = value.y;
    action.z = value.z;
    action.w = value.w;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    if (canUndo) {
        action.has_can_undo = action.can_undo = 1;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_BONE_ORIENTATION;
    command.set_model_bone_orientation = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendUpdateModelMessage(nanoem_u16_t handle)
{
    Nanoem__Application__UpdateModelCommand action = NANOEM__APPLICATION__UPDATE_MODEL_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_UPDATE_MODEL;
    command.update_model = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRegisterAccessoryKeyframeMessage(nanoem_u16_t handle)
{
    Nanoem__Application__RegisterAccessoryKeyframeCommand action =
        NANOEM__APPLICATION__REGISTER_ACCESSORY_KEYFRAME_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_accessory_handle = 1;
        action.accessory_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_ACCESSORY_KEYFRAME;
    command.register_accessory_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRegisterAllSelectedBoneKeyframesMessage(nanoem_u16_t handle, const StringList &names)
{
    Nanoem__Application__RegisterAllSelectedBoneKeyframesCommand action =
        NANOEM__APPLICATION__REGISTER_ALL_SELECTED_BONE_KEYFRAMES_COMMAND__INIT;
    MutableStringList copy;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
        nanoem_rsize_t numNames = names.size();
        action.n_bone_names = numNames;
        action.bone_names = new char *[numNames];
        copy.resize(numNames);
        for (nanoem_rsize_t i = 0; i < numNames; i++) {
            action.bone_names[i] = StringUtils::cloneString(names[i].c_str(), copy[i]);
        }
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_ALL_SELECTED_BONE_KEYFRAMES;
    command.register_all_selected_bone_keyframes = &action;
    sendCommandMessage(&command);
    delete[] action.bone_names;
}

void
BaseApplicationClient::sendRegisterCameraKeyframeMessage()
{
    Nanoem__Application__RegisterCameraKeyframeCommand action =
        NANOEM__APPLICATION__REGISTER_CAMERA_KEYFRAME_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_CAMERA_KEYFRAME;
    command.register_camera_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRegisterLightKeyframeMessage()
{
    Nanoem__Application__RegisterLightKeyframeCommand action =
        NANOEM__APPLICATION__REGISTER_LIGHT_KEYFRAME_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_LIGHT_KEYFRAME;
    command.register_light_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRegisterModelKeyframeMessage(nanoem_u16_t handle)
{
    Nanoem__Application__RegisterModelKeyframeCommand action =
        NANOEM__APPLICATION__REGISTER_MODEL_KEYFRAME_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_MODEL_KEYFRAME;
    command.register_model_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRegisterAllSelectedMorphKeyframesMessage(nanoem_u16_t handle, const StringList &names)
{
    Nanoem__Application__RegisterAllSelectedMorphKeyframesCommand action =
        NANOEM__APPLICATION__REGISTER_ALL_SELECTED_MORPH_KEYFRAMES_COMMAND__INIT;
    MutableStringList copy;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
        nanoem_rsize_t numNames = names.size();
        action.n_morph_names = numNames;
        action.morph_names = new char *[numNames];
        copy.resize(numNames);
        for (nanoem_rsize_t i = 0; i < numNames; i++) {
            action.morph_names[i] = StringUtils::cloneString(names[i].c_str(), copy[i]);
        }
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_ALL_SELECTED_MORPH_KEYFRAMES;
    command.register_all_selected_morph_keyframes = &action;
    sendCommandMessage(&command);
    delete[] action.morph_names;
}

void
BaseApplicationClient::sendRegisterSelfShadowKeyframeMessage()
{
    Nanoem__Application__RegisterSelfShadowKeyframeCommand action =
        NANOEM__APPLICATION__REGISTER_SELF_SHADOW_KEYFRAME_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REGISTER_SELF_SHADOW_KEYFRAME;
    command.register_self_shadow_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRemoveAccessoryKeyframeMessage(nanoem_u16_t handle)
{
    Nanoem__Application__RemoveAccessoryKeyframeCommand action =
        NANOEM__APPLICATION__REMOVE_ACCESSORY_KEYFRAME_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_accessory_handle = 1;
        action.accessory_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_ACCESSORY_KEYFRAME;
    command.remove_accessory_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRemoveAllSelectedBoneKeyframesMessage(nanoem_u16_t handle, const StringList &names)
{
    Nanoem__Application__RemoveAllSelectedBoneKeyframesCommand action =
        NANOEM__APPLICATION__REMOVE_ALL_SELECTED_BONE_KEYFRAMES_COMMAND__INIT;
    MutableStringList copy;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
        nanoem_rsize_t numNames = names.size();
        action.n_bone_names = numNames;
        action.bone_names = new char *[numNames];
        copy.resize(numNames);
        for (nanoem_rsize_t i = 0; i < numNames; i++) {
            action.bone_names[i] = StringUtils::cloneString(names[i].c_str(), copy[i]);
        }
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_ALL_SELECTED_BONE_KEYFRAMES;
    command.remove_all_selected_bone_keyframes = &action;
    sendCommandMessage(&command);
    delete[] action.bone_names;
}

void
BaseApplicationClient::sendRemoveCameraKeyframeMessage()
{
    Nanoem__Application__RemoveCameraKeyframeCommand action = NANOEM__APPLICATION__REMOVE_CAMERA_KEYFRAME_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_CAMERA_KEYFRAME;
    command.remove_camera_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRemoveLightKeyframeMessage()
{
    Nanoem__Application__RemoveLightKeyframeCommand action = NANOEM__APPLICATION__REMOVE_LIGHT_KEYFRAME_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_LIGHT_KEYFRAME;
    command.remove_light_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRemoveModelKeyframeMessage(nanoem_u16_t handle)
{
    Nanoem__Application__RemoveModelKeyframeCommand action = NANOEM__APPLICATION__REMOVE_MODEL_KEYFRAME_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_MODEL_KEYFRAME;
    command.remove_model_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRemoveAllSelectedMorphKeyframesMessage(nanoem_u16_t handle, const StringList &names)
{
    Nanoem__Application__RemoveAllSelectedMorphKeyframesCommand action =
        NANOEM__APPLICATION__REMOVE_ALL_SELECTED_MORPH_KEYFRAMES_COMMAND__INIT;
    MutableStringList copy;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
        nanoem_rsize_t numNames = names.size();
        action.n_morph_names = numNames;
        action.morph_names = new char *[numNames];
        copy.resize(numNames);
        for (nanoem_rsize_t i = 0; i < numNames; i++) {
            action.morph_names[i] = StringUtils::cloneString(names[i].c_str(), copy[i]);
        }
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_ALL_SELECTED_MORPH_KEYFRAMES;
    command.remove_all_selected_morph_keyframes = &action;
    sendCommandMessage(&command);
    delete[] action.morph_names;
}

void
BaseApplicationClient::sendRemoveSelfShadowKeyframeMessage()
{
    Nanoem__Application__RemoveSelfShadowKeyframeCommand action =
        NANOEM__APPLICATION__REMOVE_SELF_SHADOW_KEYFRAME_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REMOVE_SELF_SHADOW_KEYFRAME;
    command.remove_self_shadow_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendBoneBezierControlPointMessage(
    nanoem_u16_t handle, const String &name, const glm::u8vec4 &value, nanoem_u32_t type)
{
    Nanoem__Application__SetBoneBezierControlPointCommand action =
        NANOEM__APPLICATION__SET_BONE_BEZIER_CONTROL_POINT_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.bone_name = s.data();
    action.type = type;
    action.x0 = value.x;
    action.y0 = value.y;
    action.x1 = value.z;
    action.y1 = value.w;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_BONE_BEZIER_CONTROL_POINT;
    command.set_bone_bezier_control_point = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendCameraBezierControlPointMessage(const glm::u8vec4 &value, nanoem_u32_t type)
{
    Nanoem__Application__SetCameraBezierControlPointCommand action =
        NANOEM__APPLICATION__SET_CAMERA_BEZIER_CONTROL_POINT_COMMAND__INIT;
    action.type = type;
    action.x0 = value.x;
    action.y0 = value.y;
    action.x1 = value.z;
    action.y1 = value.w;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_BEZIER_CONTROL_POINT;
    command.set_camera_bezier_control_point = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectBoneMessage(nanoem_u16_t handle, const String &name)
{
    Nanoem__Application__SelectBoneCommand action = NANOEM__APPLICATION__SELECT_BONE_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.bone_name = s.data();
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_BONE;
    command.select_bone = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectAllBonesMessage(nanoem_u16_t handle)
{
    Nanoem__Application__SelectAllBonesCommand action = NANOEM__APPLICATION__SELECT_ALL_BONES_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_BONES;
    command.select_all_bones = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectAllDirtyBonesMessage(nanoem_u16_t handle)
{
    Nanoem__Application__SelectAllDirtyBonesCommand action = NANOEM__APPLICATION__SELECT_ALL_DIRTY_BONES_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_DIRTY_BONES;
    command.select_all_dirty_bones = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectAllMovableDirtyBonesMessage(nanoem_u16_t handle)
{
    Nanoem__Application__SelectAllMovableBonesCommand action =
        NANOEM__APPLICATION__SELECT_ALL_MOVABLE_BONES_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_MOVABLE_BONES;
    command.select_all_movable_bones = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendClearSelectBoneMessage(nanoem_u16_t handle, const String &name)
{
    Nanoem__Application__ClearSelectBoneCommand action = NANOEM__APPLICATION__CLEAR_SELECT_BONE_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(name, s);
    action.bone_name = s.data();
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CLEAR_SELECT_BONE;
    command.clear_select_bone = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendClearSelectAllBonesMessage(nanoem_u16_t handle)
{
    Nanoem__Application__ClearSelectAllBonesCommand action = NANOEM__APPLICATION__CLEAR_SELECT_ALL_BONES_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_model_handle = 1;
        action.model_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CLEAR_SELECT_ALL_BONES;
    command.clear_select_all_bones = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectMotionKeyframeMessage(
    nanoem_u16_t handle, nanoem_frame_index_t frameIndex, const String &trackName)
{
    Nanoem__Application__SelectMotionKeyframeCommand action = NANOEM__APPLICATION__SELECT_MOTION_KEYFRAME_COMMAND__INIT;
    action.local_frame_index = frameIndex;
    MutableString s;
    StringUtils::copyString(trackName, s);
    action.track_name = s.data();
    if (handle != bx::kInvalidHandle) {
        action.has_motion_handle = 1;
        action.motion_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_MOTION_KEYFRAME;
    command.select_motion_keyframe = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectAllRowMotionKeyframesMessage(nanoem_u16_t handle, const String &trackName)
{
    Nanoem__Application__SelectAllRowMotionKeyframesCommand action =
        NANOEM__APPLICATION__SELECT_ALL_ROW_MOTION_KEYFRAMES_COMMAND__INIT;
    MutableString s;
    StringUtils::copyString(trackName, s);
    action.track_name = s.data();
    if (handle != bx::kInvalidHandle) {
        action.has_motion_handle = 1;
        action.motion_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_ROW_MOTION_KEYFRAMES;
    command.select_all_row_motion_keyframes = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSelectAllColumnMotionKeyframesMessage(nanoem_u16_t handle, nanoem_frame_index_t frameIndex)
{
    Nanoem__Application__SelectAllColumnMotionKeyframesCommand action =
        NANOEM__APPLICATION__SELECT_ALL_COLUMN_MOTION_KEYFRAMES_COMMAND__INIT;
    action.local_frame_index = frameIndex;
    if (handle != bx::kInvalidHandle) {
        action.has_motion_handle = 1;
        action.motion_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SELECT_ALL_COLUMN_MOTION_KEYFRAMES;
    command.select_all_column_motion_keyframes = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendClearSelectAllMotionKeyframesMessage(nanoem_u16_t handle)
{
    Nanoem__Application__ClearSelectAllMotionKeyframesCommand action =
        NANOEM__APPLICATION__CLEAR_SELECT_ALL_MOTION_KEYFRAMES_COMMAND__INIT;
    if (handle != bx::kInvalidHandle) {
        action.has_motion_handle = 1;
        action.motion_handle = handle;
    }
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_CLEAR_SELECT_ALL_MOTION_KEYFRAMES;
    command.clear_select_all_motion_keyframes = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendUpdateCurrentFPSMessage(nanoem_u32_t value)
{
    Nanoem__Application__UpdateCurrentFPSCommand action = NANOEM__APPLICATION__UPDATE_CURRENT_FPSCOMMAND__INIT;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_UPDATE_CURRENT_FPS;
    command.update_current_fps = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendReloadAccessoryEffectMessage(nanoem_u16_t handle)
{
    Nanoem__Application__ReloadAccessoryEffectCommand action =
        NANOEM__APPLICATION__RELOAD_ACCESSORY_EFFECT_COMMAND__INIT;
    action.accessory_handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_RELOAD_ACCESSORY_EFFECT;
    command.reload_accessory_effect = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendReloadModelEffectMessage(nanoem_u16_t handle)
{
    Nanoem__Application__ReloadModelEffectCommand action = NANOEM__APPLICATION__RELOAD_MODEL_EFFECT_COMMAND__INIT;
    action.model_handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_RELOAD_MODEL_EFFECT;
    command.reload_model_effect = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelVisibleMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetModelVisibleCommand action = NANOEM__APPLICATION__SET_MODEL_VISIBLE_COMMAND__INIT;
    action.model_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_VISIBLE;
    command.set_model_visible = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelEdgeColorMessage(nanoem_u16_t handle, const Vector4 &value)
{
    Nanoem__Application__SetModelEdgeColorCommand action = NANOEM__APPLICATION__SET_MODEL_EDGE_COLOR_COMMAND__INIT;
    action.model_handle = handle;
    action.red = value.x;
    action.blue = value.y;
    action.green = value.z;
    action.alpha = value.w;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_EDGE_COLOR;
    command.set_model_edge_color = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelEdgeSizeMessage(nanoem_u16_t handle, nanoem_f32_t value)
{
    Nanoem__Application__SetModelEdgeSizeCommand action = NANOEM__APPLICATION__SET_MODEL_EDGE_SIZE_COMMAND__INIT;
    action.model_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_EDGE_SIZE;
    command.set_model_edge_size = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelConstraintStateMessage(nanoem_u16_t handle, const String &constraintName, bool value)
{
    MutableString mutableConstraintName;
    Nanoem__Application__SetModelConstraintStateCommand action =
        NANOEM__APPLICATION__SET_MODEL_CONSTRAINT_STATE_COMMAND__INIT;
    action.model_handle = handle;
    action.constraint_name = StringUtils::cloneString(constraintName.c_str(), mutableConstraintName);
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_CONSTRAINT_STATE;
    command.set_model_constraint_state = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelOutsideParentMessage(
    nanoem_u16_t handle, const String &boneName, const String &parentModelName, const String &parentModelBoneName)
{
    MutableString mutableBoneName, mutableParnetModelName, mutableParentModelBoneName;
    Nanoem__Application__SetModelOutsideParentCommand action =
        NANOEM__APPLICATION__SET_MODEL_OUTSIDE_PARENT_COMMAND__INIT;
    action.model_handle = handle;
    action.bone_name = StringUtils::cloneString(boneName.c_str(), mutableBoneName);
    action.parent_model_name = StringUtils::cloneString(parentModelName.c_str(), mutableParnetModelName);
    action.parent_model_bone_name = StringUtils::cloneString(parentModelBoneName.c_str(), mutableParentModelBoneName);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_OUTSIDE_PARENT;
    command.set_model_outside_parent = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryOutsideParentMessage(
    nanoem_u16_t handle, const String &parentModelName, const String &parentModelBoneName)
{
    MutableString mutableParentModelName, mutableParentModelBoneName;
    Nanoem__Application__SetAccessoryOutsideParentCommand action =
        NANOEM__APPLICATION__SET_ACCESSORY_OUTSIDE_PARENT_COMMAND__INIT;
    action.accessory_handle = handle;
    action.parent_model_name = StringUtils::cloneString(parentModelName.c_str(), mutableParentModelName);
    action.parent_model_bone_name = StringUtils::cloneString(parentModelBoneName.c_str(), mutableParentModelBoneName);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_OUTSIDE_PARENT;
    command.set_accessory_outside_parent = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRecoveryMessage(const URI &fileURI)
{
    Nanoem__Application__RecoveryCommand action = NANOEM__APPLICATION__RECOVERY_COMMAND__INIT;
    Nanoem__Application__URI uri;
    MutableString absolutePath, fragment;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_RECOVERY;
    command.recovery = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryAddBlendEnabledMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetAccessoryAddBlendEnabledCommand action =
        NANOEM__APPLICATION__SET_ACCESSORY_ADD_BLEND_ENABLED_COMMAND__INIT;
    action.accessory_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_ADD_BLEND_ENABLED;
    command.set_accessory_add_blend_enabled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryShadowEnabledMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetAccessoryShadowEnabledCommand action =
        NANOEM__APPLICATION__SET_ACCESSORY_SHADOW_ENABLED_COMMAND__INIT;
    action.accessory_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_SHADOW_ENABLED;
    command.set_accessory_shadow_enabled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetAccessoryVisibleMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetAccessoryVisibleCommand action = NANOEM__APPLICATION__SET_ACCESSORY_VISIBLE_COMMAND__INIT;
    action.accessory_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_ACCESSORY_VISIBLE;
    command.set_accessory_visible = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelAddBlendEnabledMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetModelAddBlendEnabledCommand action =
        NANOEM__APPLICATION__SET_MODEL_ADD_BLEND_ENABLED_COMMAND__INIT;
    action.model_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_ADD_BLEND_ENABLED;
    command.set_model_add_blend_enabled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelShadowEnabledMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetModelShadowEnabledCommand action =
        NANOEM__APPLICATION__SET_MODEL_SHADOW_ENABLED_COMMAND__INIT;
    action.model_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_SHADOW_ENABLED;
    command.set_model_shadow_enabled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelShadowMapEnabledMessage(nanoem_u16_t handle, bool value)
{
    Nanoem__Application__SetModelShadowMapEnabledCommand action =
        NANOEM__APPLICATION__SET_MODEL_SHADOW_MAP_ENABLED_COMMAND__INIT;
    action.model_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_SHADOW_MAP_ENABLED;
    command.set_model_shadow_map_enabled = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetDrawableOrderIndexMessage(nanoem_u16_t handle, int value)
{
    Nanoem__Application__SetDrawableOrderIndexCommand action =
        NANOEM__APPLICATION__SET_DRAWABLE_ORDER_INDEX_COMMAND__INIT;
    action.handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_DRAWABLE_ORDER_INDEX;
    command.set_drawable_order_index = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelTransformOrderIndexMessage(nanoem_u16_t handle, int value)
{
    Nanoem__Application__SetModelTransformOrderIndexCommand action =
        NANOEM__APPLICATION__SET_MODEL_TRANSFORM_ORDER_INDEX_COMMAND__INIT;
    action.model_handle = handle;
    action.value = value;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_TRANSFORM_ORDER_INDEX;
    command.set_model_transform_order_index = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetModelBoneKeyframeInterpolationMessage(nanoem_u16_t handle, const glm::u8vec4 *values)
{
    Nanoem__Application__SetModelBoneKeyframeInterpolationCommand action =
        NANOEM__APPLICATION__SET_MODEL_BONE_KEYFRAME_INTERPOLATION_COMMAND__INIT;
    action.model_handle = handle;
    Nanoem__Application__BoneInterpolation interpolation = NANOEM__APPLICATION__BONE_INTERPOLATION__INIT;
    Nanoem__Common__Interpolation x;
    interpolation.x = internal::ApplicationUtils::assignInteprolation(
        x, values[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X]);
    Nanoem__Common__Interpolation y;
    interpolation.y = internal::ApplicationUtils::assignInteprolation(
        y, values[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y]);
    Nanoem__Common__Interpolation z;
    interpolation.z = internal::ApplicationUtils::assignInteprolation(
        z, values[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z]);
    Nanoem__Common__Interpolation orientation = NANOEM__COMMON__INTERPOLATION__INIT;
    interpolation.orientation = &orientation;
    action.interpolation = &interpolation;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_MODEL_BONE_KEYFRAME_INTERPOLATION;
    command.set_model_bone_keyframe_interpolation = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendUpdatePerformanceMonitorMessage(
    nanoem_f32_t cpu, nanoem_u64_t currentMemorySize, nanoem_u64_t maxMemorySize)
{
    Nanoem__Application__UpdatePerformanceMonitorCommand action =
        NANOEM__APPLICATION__UPDATE_PERFORMANCE_MONITOR_COMMAND__INIT;
    action.current_cpu_percentage = cpu;
    action.current_resident_memory_bytes = currentMemorySize;
    action.max_resident_memory_bytes = maxMemorySize;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_UPDATE_PERFORMANCE_MONITOR;
    command.update_performance_monitor = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendLoadAllModelPluginsMessage(const URIList &values)
{
    Nanoem__Application__LoadAllModelIOPluginsCommand action =
        NANOEM__APPLICATION__LOAD_ALL_MODEL_IOPLUGINS_COMMAND__INIT;
    MutableStringList absolutePathList, fragmentList;
    internal::ApplicationUtils::allocateURIList(
        values, action.file_uris, action.n_file_uris, absolutePathList, fragmentList);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_MODEL_IO_PLUGINS;
    command.load_all_model_io_plugins = &action;
    sendCommandMessage(&command);
    internal::ApplicationUtils::freeURIList(action.file_uris, action.n_file_uris);
}

void
BaseApplicationClient::sendLoadAllMotionPluginsMessage(const URIList &values)
{
    Nanoem__Application__LoadAllMotionIOPluginsCommand action =
        NANOEM__APPLICATION__LOAD_ALL_MOTION_IOPLUGINS_COMMAND__INIT;
    MutableStringList absolutePathList, fragmentList;
    internal::ApplicationUtils::allocateURIList(
        values, action.file_uris, action.n_file_uris, absolutePathList, fragmentList);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_LOAD_ALL_MOTION_IO_PLUGINS;
    command.load_all_motion_io_plugins = &action;
    sendCommandMessage(&command);
    internal::ApplicationUtils::freeURIList(action.file_uris, action.n_file_uris);
}

void
BaseApplicationClient::sendExecuteModelPluginMessage(nanoem_u16_t plugin, nanoem_u32_t function)
{
    Nanoem__Application__ExecuteModelIOPluginCommand action = NANOEM__APPLICATION__EXECUTE_MODEL_IOPLUGIN_COMMAND__INIT;
    action.plugin_handle = plugin;
    action.function_handle = function;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_MODEL_IO_PLUGIN;
    command.execute_model_io_plugin = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendExecuteMotionPluginMessage(nanoem_u16_t plugin, nanoem_u32_t function)
{
    Nanoem__Application__ExecuteMotionIOPluginCommand action =
        NANOEM__APPLICATION__EXECUTE_MOTION_IOPLUGIN_COMMAND__INIT;
    action.plugin_handle = plugin;
    action.function_handle = function;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_MOTION_IO_PLUGIN;
    command.execute_motion_io_plugin = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRequestExportImageConfigurationMessage()
{
    Nanoem__Application__RequestExportImageConfigurationCommand action =
        NANOEM__APPLICATION__REQUEST_EXPORT_IMAGE_CONFIGURATION_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REQUEST_EXPORT_IMAGE_CONFIGURATION;
    command.request_export_image_configuration = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendRequestExportVideoConfigurationMessage()
{
    Nanoem__Application__RequestExportVideoConfigurationCommand action =
        NANOEM__APPLICATION__REQUEST_EXPORT_IMAGE_CONFIGURATION_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_REQUEST_EXPORT_VIDEO_CONFIGURATION;
    command.request_export_video_configuration = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendExecuteExportingImageMessage(const URI &fileURI)
{
    MutableString absolutePath, fragment;
    Nanoem__Application__ExecuteExportingImageCommand action =
        NANOEM__APPLICATION__EXECUTE_EXPORTING_IMAGE_COMMAND__INIT;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_EXPORTING_IMAGE;
    command.execute_exporting_image = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendExecuteExportingVideoMessage(const URI &fileURI)
{
    MutableString absolutePath, fragment;
    Nanoem__Application__ExecuteExportingVideoCommand action =
        NANOEM__APPLICATION__EXECUTE_EXPORTING_VIDEO_COMMAND__INIT;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_EXECUTE_EXPORTING_VIDEO;
    command.execute_exporting_video = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendScreenCursorPressMessage(const Vector2SI32 &coord, int type, int modifiers)
{
    Nanoem__Application__ScreenCursorPressCommand action;
    nanoem__application__screen_cursor_press_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    action.modifiers = modifiers;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SCREEN_CURSOR_PRESS;
    command.screen_cursor_press = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendScreenCursorMoveMessage(const Vector2SI32 &coord, int type, int modifiers)
{
    Nanoem__Application__ScreenCursorMoveCommand action;
    nanoem__application__screen_cursor_move_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    action.modifiers = modifiers;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SCREEN_CURSOR_MOVE;
    command.screen_cursor_move = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendScreenCursorReleaseMessage(const Vector2SI32 &coord, int type, int modifiers)
{
    Nanoem__Application__ScreenCursorReleaseCommand action;
    nanoem__application__screen_cursor_release_command__init(&action);
    action.x = coord.x;
    action.y = coord.y;
    action.type = type;
    action.modifiers = modifiers;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SCREEN_CURSOR_RELEASE;
    command.screen_cursor_release = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendSetCameraKeyframeInterpolationMessage(const glm::u8vec4 *values)
{
    Nanoem__Application__SetCameraKeyframeInterpolationCommand action =
        NANOEM__APPLICATION__SET_CAMERA_KEYFRAME_INTERPOLATION_COMMAND__INIT;
    Nanoem__Application__CameraInterpolation interpolation = NANOEM__APPLICATION__CAMERA_INTERPOLATION__INIT;
    Nanoem__Common__Interpolation x;
    interpolation.x = internal::ApplicationUtils::assignInteprolation(
        x, values[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X]);
    Nanoem__Common__Interpolation y;
    interpolation.y = internal::ApplicationUtils::assignInteprolation(
        y, values[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y]);
    Nanoem__Common__Interpolation z;
    interpolation.z = internal::ApplicationUtils::assignInteprolation(
        z, values[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z]);
    Nanoem__Common__Interpolation angle;
    interpolation.angle = internal::ApplicationUtils::assignInteprolation(
        angle, values[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE]);
    Nanoem__Common__Interpolation fov;
    interpolation.fov = internal::ApplicationUtils::assignInteprolation(
        fov, values[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV]);
    Nanoem__Common__Interpolation distance;
    interpolation.distance = internal::ApplicationUtils::assignInteprolation(
        distance, values[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE]);
    action.interpolation = &interpolation;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_SET_CAMERA_KEYFRAME_INTERPOLATION;
    command.set_camera_keyframe_interpolation = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendIsProjectDirtyRequestMessage(pfn_isProjectDirtyCallback callback, void *userData)
{
    Nanoem__Application__IsProjectDirtyRequestCommand action =
        NANOEM__APPLICATION__IS_PROJECT_DIRTY_REQUEST_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_IS_PROJECT_DIRTY;
    command.is_project_dirty = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.isProjectDirty = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetProjectFileURIRequestMessage(pfn_getProjectFileURICallback callback, void *userData)
{
    Nanoem__Application__GetProjectFileURIRequestCommand action =
        NANOEM__APPLICATION__GET_PROJECT_FILE_URIREQUEST_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_PROJECT_FILE_URI;
    command.get_project_file_uri = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getProjectFileURI = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetAllModelBonesRequestMessage(
    nanoem_u16_t handle, pfn_getAllModelBonesCallback callback, void *userData)
{
    Nanoem__Application__GetAllModelBonesRequestCommand action =
        NANOEM__APPLICATION__GET_ALL_MODEL_BONES_REQUEST_COMMAND__INIT;
    action.model_handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_MODEL_BONES;
    command.get_all_model_bones = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getAllModelBones = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetAllModelMorphsRequestMessages(
    nanoem_u16_t handle, pfn_getAllModelMorphsCallback callback, void *userData)
{
    Nanoem__Application__GetAllModelMorphsRequestCommand action =
        NANOEM__APPLICATION__GET_ALL_MODEL_MORPHS_REQUEST_COMMAND__INIT;
    action.model_handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_MODEL_MORPHS;
    command.get_all_model_morphs = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getAllModelMorphs = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetBackgroundImageTextureHandleMessage(
    pfn_getBackgroundTextureImageHandleCallback callback, void *userData)
{
    Nanoem__Application__GetBackgroundImageTextureHandleRequestCommand action =
        NANOEM__APPLICATION__GET_BACKGROUND_IMAGE_TEXTURE_HANDLE_REQUEST_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_BACKGROUND_IMAGE_TEXTURE_HANDLE;
    command.get_background_image_texture_handle = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getBackgroundTextureImageHandle = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetAllAccessoriesRequestMessage(pfn_getAllAccessoriesCallback callback, void *userData)
{
    Nanoem__Application__GetAllAccessoriesRequestCommand action =
        NANOEM__APPLICATION__GET_ALL_ACCESSORIES_REQUEST_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_ACCESSORIES;
    command.get_all_accessories = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getAllAccessories = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetAllModelsRequestMessage(pfn_getAllModelsCallback callback, void *userData)
{
    Nanoem__Application__GetAllModelsRequestCommand action = NANOEM__APPLICATION__GET_ALL_MODELS_REQUEST_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_ALL_MODELS;
    command.get_all_models = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getAllModels = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendQueryOpenSingleFileDialogMessage(nanoem_u32_t type, const URI &fileURI)
{
    MutableString absolutePath, fragment;
    Nanoem__Application__QueryOpenSingleFileDialogCommand action =
        NANOEM__APPLICATION__QUERY_OPEN_SINGLE_FILE_DIALOG_COMMAND__INIT;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    action.type = type;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_QUERY_OPEN_SINGLE_FILE_DIALOG;
    command.query_open_single_file_dialog = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendQueryOpenMultipleFilesDialogMessage(nanoem_u32_t type, const URIList &fileURIs)
{
    MutableString absolutePath, fragment;
    Nanoem__Application__QueryOpenMultipleFilesDialogCommand action =
        NANOEM__APPLICATION__QUERY_OPEN_MULTIPLE_FILES_DIALOG_COMMAND__INIT;
    action.type = type;
    MutableStringList absolutePathList, fragmentList;
    internal::ApplicationUtils::allocateURIList(
        fileURIs, action.file_uri, action.n_file_uri, absolutePathList, fragmentList);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_QUERY_OPEN_SINGLE_FILE_DIALOG;
    command.query_open_multiple_files_dialog = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendQuerySaveFileDialogMessage(nanoem_u32_t type, const URI &fileURI)
{
    MutableString absolutePath, fragment;
    Nanoem__Application__QuerySaveFileDialogCommand action = NANOEM__APPLICATION__QUERY_SAVE_FILE_DIALOG_COMMAND__INIT;
    Nanoem__Application__URI uri = NANOEM__APPLICATION__URI__INIT;
    action.type = type;
    action.file_uri = internal::ApplicationUtils::assignURI(&uri, absolutePath, fragment, fileURI);
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_QUERY_SAVE_FILE_DIALOG;
    command.query_save_file_dialog = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendGetHandleFileURIRequestMessage(
    nanoem_u16_t handle, pfn_getHandleFileURICallback callback, void *userData)
{
    Nanoem__Application__GetHandleFileURIRequestCommand action =
        NANOEM__APPLICATION__GET_HANDLE_FILE_URIREQUEST_COMMAND__INIT;
    action.handle = handle;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    nanoem_u64_t timestamp = internal::ApplicationUtils::timestamp();
    command.timestamp = timestamp;
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_GET_HANDLE_FILE_URI;
    command.get_handle_file_uri = &action;
    RequestCallback c = { nanoem_u32_t(command.type_case), userData, { nullptr } };
    c.u.getHandleFileURI = callback;
    m_requestCallbacks.insert(tinystl::make_pair(timestamp, c));
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendStartDebugCaptureMessage()
{
    Nanoem__Application__StartDebugCaptureCommand action = NANOEM__APPLICATION__START_DEBUG_CAPTURE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_START_DEBUG_CAPTURE;
    command.start_debug_capture = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendStopDebugCaptureMessage()
{
    Nanoem__Application__StopDebugCaptureCommand action = NANOEM__APPLICATION__STOP_DEBUG_CAPTURE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_STOP_DEBUG_CAPTURE;
    command.stop_debug_capture = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendDestroyMessage()
{
    Nanoem__Application__DestroyCommand action = NANOEM__APPLICATION__DESTROY_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_DESTROY;
    command.destroy = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::sendTerminateMessage()
{
    Nanoem__Application__TerminateCommand action = NANOEM__APPLICATION__TERMINATE_COMMAND__INIT;
    Nanoem__Application__Command command = NANOEM__APPLICATION__COMMAND__INIT;
    command.timestamp = internal::ApplicationUtils::timestamp();
    command.type_case = NANOEM__APPLICATION__COMMAND__TYPE_TERMINATE;
    command.terminate = &action;
    sendCommandMessage(&command);
}

void
BaseApplicationClient::addTrackEventListener(pfn_handleTrackEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleTrack = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TRACK].push_back(l);
}

void
BaseApplicationClient::addUndoEventListener(pfn_handleUndoEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleUndo = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_UNDO].push_back(l);
}

void
BaseApplicationClient::addRedoEventListener(pfn_handleRedoEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleRedo = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_REDO].push_back(l);
}

void
BaseApplicationClient::addUndoChangeEventListener(pfn_handleUndoChangeEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleUndoChange = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_UNDO_CHANGE].push_back(l);
}

void
BaseApplicationClient::addAddModelEventListener(pfn_handleAddModelEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAddModel = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_ADD_MODEL].push_back(l);
}

void
BaseApplicationClient::addSetActiveModelEventListener(pfn_handleSetActiveModelEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetActiveModel = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_MODEL].push_back(l);
}

void
BaseApplicationClient::addRemoveModelEventListener(pfn_handleRemoveModelEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleRemoveModel = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_REMOVE_MODEL].push_back(l);
}

void
BaseApplicationClient::addAddAccessoryEventListener(pfn_handleAddAccessoryEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAddAccessory = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_ADD_ACCESSORY].push_back(l);
}

void
BaseApplicationClient::addSetActiveAccessoryEventListener(
    pfn_handleSetActiveAccessoryEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetActiveAccessory = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_ACCESSORY].push_back(l);
}

void
BaseApplicationClient::addRemoveAccessoryEventListener(
    pfn_handleRemoveAccessoryEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleRemoveAccessory = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_REMOVE_ACCESSORY].push_back(l);
}

void
BaseApplicationClient::addPlayEvent(pfn_handlePlayEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handlePlay = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_PLAY].push_back(l);
}

void
BaseApplicationClient::addStopEvent(pfn_handleStopEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleStop = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_STOP].push_back(l);
}

void
BaseApplicationClient::addPauseEvent(pfn_handlePauseEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handlePause = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_PAUSE].push_back(l);
}

void
BaseApplicationClient::addResumeEvent(pfn_handleResumeEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleResume = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_RESUME].push_back(l);
}

void
BaseApplicationClient::addSeekEvent(pfn_handleSeekEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSeek = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SEEK].push_back(l);
}

void
BaseApplicationClient::addUpdateDurationEvent(pfn_handleUpdateDurationEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleUpdateDuration = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_UPDATE_DURATION].push_back(l);
}

void
BaseApplicationClient::addSaveProjectAfterConfirmEventListener(
    pfn_handleSaveProjectAfterConfirmEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSaveProjectAfterConfirm = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SAVE_PROJECT_AFTER_CONFIRM].push_back(l);
}

void
BaseApplicationClient::addDiscardProjectAfterConfirmEventListener(
    pfn_handleDiscardProjectAfterConfirmEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleDiscardProjectAfterConfirm = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_DISCARD_PROJECT_AFTER_CONFIRM].push_back(l);
}

void
BaseApplicationClient::addToggleProjectEffectEnabledEvent(
    pfn_handleToggleProjectEffectEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleProjectEffectEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_EFFECT_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleProjectGroundShadowEnabledEvent(
    pfn_handleToggleProjectGroundShadowEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleProjectGroundShadowEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_GROUND_SHADOW_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleProjectVertexShaderSkinningEnabledEvent(
    pfn_handleToggleProjectVertexShaderSkinningEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleProjectVertexShaderSkinningEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_VERTEX_SHADER_SKINNING_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleProjectComputeShaderSkinningEnabledEvent(
    pfn_handleToggleProjectComputeShaderSkinningEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleProjectComputeShaderSkinningEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_COMPUTE_SHADER_SKINNING_ENABLED].push_back(l);
}

void
BaseApplicationClient::addSetProjectSampleLevelEvent(
    pfn_handleSetProjectSampleLevelEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetProjectSampleLevel = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_PROJECT_SAMPLE_LEVEL].push_back(l);
}

void
BaseApplicationClient::addToggleGridEnabledEvent(pfn_handleToggleGridEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleGridEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_GRID_ENABLED].push_back(l);
}

void
BaseApplicationClient::addSetGridCellEvent(pfn_handleSetGridCellEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetGridCell = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_GRID_CELL].push_back(l);
}

void
BaseApplicationClient::addSetGridSizeEvent(pfn_handleSetGridSizeEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetGridSize = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_GRID_SIZE].push_back(l);
}

void
BaseApplicationClient::addSetPreferredMotionFPSEvent(
    pfn_handleSetPreferredMotionFPSEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetPreferredMotionFPS = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_PREFERRED_MOTION_FPS].push_back(l);
}

void
BaseApplicationClient::addSetPhysicsSimulationModeEvent(
    pfn_handleSetPhysicsSimulationModeEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetPhysicsSimulationMode = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_PHYSICS_SIMULATION_MODE].push_back(l);
}

void
BaseApplicationClient::addSetPhysicsSimulationEngineDebugFlagEvent(
    pfn_handleSetPhysicsSimulationEngineDebugFlagEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetPhysicsSimulationEngineDebugFlag = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_PHYSICS_SIMULATION_ENGINE_DEBUG_FLAG].push_back(l);
}

void
BaseApplicationClient::addToggleShadowMapEnabledEvent(
    pfn_handleToggleShadowMapEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleShadowMapEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_SHADOW_MAP_ENABLED].push_back(l);
}

void
BaseApplicationClient::addSetShadowMapModeEvent(pfn_handleSetShadowMapModeEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetShadowMapMode = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_SHADOW_MAP_MODE].push_back(l);
}

void
BaseApplicationClient::addSetShadowMapDistanceEvent(
    pfn_handleSetShadowMapDistanceEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetShadowMapDistance = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_SHADOW_MAP_DISTANCE].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelAddBlendEnabledEvent(
    pfn_handleToggleActiveModelAddBlendEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelAddBlendEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_ADD_BLEND_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelShadowMapEnabledEvent(
    pfn_handleToggleActiveModelShadowMapEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelShadowMapEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHADOW_MAP_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelVisibleEvent(
    pfn_handleToggleActiveModelVisibleEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelVisible = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_VISIBLE].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelComputeShaderSkinningEnabledEvent(
    pfn_handleToggleActiveModelComputeShaderSkinningEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelComputeShaderSkinningEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_COMPUTE_SHADER_SKINNING_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelShowAllBonesEvent(
    pfn_handleToggleActiveModelShowAllBonesEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelShowAllBones = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_BONES].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelShowAllRigidBodiesEvent(
    pfn_handleToggleActiveModelShowAllRigidBodiesEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelShowAllRigidBodies = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_RIGID_BODIES].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelShowAllVertexFacesEvent(
    pfn_handleToggleActiveModelShowAllVertexFacesEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelShowAllVertexFaces = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_FACES].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelShowAllVertexPointsEvent(
    pfn_handleToggleActiveModelShowAllVertexPointsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelShowAllVertexPoints = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_POINTS].push_back(l);
}

void
BaseApplicationClient::addToggleActiveModelVertexShaderSkinningEnabledEvent(
    pfn_handleToggleActiveModelVertexShaderSkinningEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveModelVertexShaderSkinningEnabled = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_VERTEX_SHADER_SKINNING].push_back(l);
}

void
BaseApplicationClient::addAvailableAllImportingAudioExtensionsEvent(
    pfn_handleAvailableAllImportingAudioExtensionsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAvailableAllImportingAudioExtensions = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_IMPORTING_AUDIO_EXTENSIONS].push_back(l);
}

void
BaseApplicationClient::addAvailableAllImportingVideoExtensionsEvent(
    pfn_handleAvailableAllImportingVideoExtensionsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAvailableAllImportingVideoExtensions = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_IMPORTING_VIDEO_EXTENSIONS].push_back(l);
}

void
BaseApplicationClient::addAvailableAllExportingImageExtensionsEvent(
    pfn_handleAvailableAllExportingImageExtensionsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAvailableAllExportingImageExtensions = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_EXPORTING_IMAGE_EXTENSIONS].push_back(l);
}

void
BaseApplicationClient::addAvailableAllExportingVideoExtensionsEvent(
    pfn_handleAvailableAllExportingVideoExtensionsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAvailableAllExportingVideoExtensions = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_EXPORTING_VIDEO_EXTENSIONS].push_back(l);
}

void
BaseApplicationClient::addDisableCursorEventListener(pfn_handleDisableCursorEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleDisableCursor = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_DISABLE_CURSOR].push_back(l);
}

void
BaseApplicationClient::addEnableCursorEventListener(pfn_handleEnableCursorEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleEnableCursor = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_ENABLE_CURSOR].push_back(l);
}

void
BaseApplicationClient::addErrorEventListener(pfn_handleErrorEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleError = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_ERROR].push_back(l);
}

void
BaseApplicationClient::addStartRecordingViewportPassEventListener(
    pfn_handleStartRecordingViewportPassEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleStartRecordingViewportPass = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_START_RECORDING_VIEWPORT_PASS].push_back(l);
}

void
BaseApplicationClient::addStopRecordingViewportPassEventListener(
    pfn_handleStopRecordingViewportPassEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleStopRecordingViewportPass = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_STOP_RECORDING_VIEWPORT_PASS].push_back(l);
}

void
BaseApplicationClient::addCompleteLoadingFileEventListener(
    pfn_handleCompleteLoadingFileEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteLoadingFile = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_FILE].push_back(l);
}

void
BaseApplicationClient::addCompleteSavingFileEventListener(
    pfn_handleCompleteSavingFileEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteSavingFile = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_SAVING_FILE].push_back(l);
}

void
BaseApplicationClient::addSetPreferredEditingFPSEvent(
    pfn_handleSetPreferredEditingFPSEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetPreferredEditingFPS = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_PREFERRED_EDITING_FPS].push_back(l);
}

void
BaseApplicationClient::addConsumePassEventListener(pfn_handleConsumePassEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleConsumePass = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_CONSUME_PASS].push_back(l);
}

void
BaseApplicationClient::addAddModalDialogEventListener(pfn_handleAddModalDialogEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleAddModalDialog = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_ADD_MODAL_DIALOG].push_back(l);
}

void
BaseApplicationClient::addClearModalDialogEventListener(
    pfn_handleClearModalDialogEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleClearModalDialog = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_CLEAR_MODAL_DIALOG].push_back(l);
}

void
BaseApplicationClient::addSetLanguageEventListener(pfn_handleSetLanguageEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetLanguage = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_LANGUAGE].push_back(l);
}

void
BaseApplicationClient::addToggleProjectPlayingWithLoopEventListener(
    pfn_handleToggleProjectPlayingWithLoopEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleProjectPlayingWithLoopEnabledEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_PLAYING_WITH_LOOP_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleActiveAccessoryAddBlendEnabledEventListener(
    pfn_handleToggleActiveAccessoryAddBlendEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveAccessoryAddBlendEnabledEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_ADD_BLEND_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleActiveAccessoryShadowEnabledEventListener(
    pfn_handleToggleActiveAccessoryShadowEnabledEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveAccessoryAddBlendEnabledEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_SHADOW_ENABLED].push_back(l);
}

void
BaseApplicationClient::addToggleActiveAccessoryVisibleEventListener(
    pfn_handleToggleActiveAccessoryVisibleEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleToggleActiveAccessoryAddBlendEnabledEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_VISIBLE].push_back(l);
}

void
BaseApplicationClient::addUpdateProgressEventListener(pfn_handleUpdateProgressEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleUpdateProgressEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_UPDATE_PROGRESS].push_back(l);
}

void
BaseApplicationClient::addStartProgressEventListener(pfn_handleStartProgressEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleStartProgressEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_START_PROGRESS].push_back(l);
}

void
BaseApplicationClient::addStopProgressEventListener(pfn_handleStopProgressEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleStopProgressEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_STOP_PROGRESS].push_back(l);
}

void
BaseApplicationClient::addSetupProjectEventListener(pfn_handleSetupProjectEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetupProjectEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SETUP_PROJECT].push_back(l);
}

void
BaseApplicationClient::addSetEditingModeEventListener(pfn_handleSetEditingModeEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetEditingModeEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_EDITING_MODE].push_back(l);
}

void
BaseApplicationClient::addCompleteLoadingAllModelPluginsEventListener(
    pfn_handleCompleteLoadingAllModelPluginsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteLoadingAllModelPluginsEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_ALL_MODEL_IO_PLUGINS].push_back(l);
}

void
BaseApplicationClient::addCompleteLoadingAllMotionPluginsEventListener(
    pfn_handleCompleteLoadingAllMotionPluginsEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteLoadingAllMotionPluginsEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_ALL_MOTION_IO_PLUGINS].push_back(l);
}

void
BaseApplicationClient::addCompleteExportImageConfigurationEventListener(
    pfn_handleCompleteExportImageConfigurationEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteExportImageConfigurationEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_EXPORT_IMAGE_CONFIGURATION].push_back(l);
}

void
BaseApplicationClient::addCompleteExportVideoConfigurationEventListener(
    pfn_handleCompleteExportVideoConfigurationEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteExportVideoConfigurationEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_EXPORT_VIDEO_CONFIGURATION].push_back(l);
}

void
BaseApplicationClient::addQueryOpenSingleFileDialogEventListener(
    pfn_handleQueryOpenSingleFileDialogEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleQueryOpenSingleFileDialogEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_QUERY_OPEN_SINGLE_FILE_DIALOG].push_back(l);
}

void
BaseApplicationClient::addQueryOpenMultipleFilesDialogEventListener(
    pfn_handleQueryOpenMultipleFilesDialogEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleQueryOpenMultipleFilesDialogEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_QUERY_OPEN_MULTIPLE_FILES_DIALOG].push_back(l);
}

void
BaseApplicationClient::addQuerySaveFileDialogEventListener(
    pfn_handleQuerySaveFileDialogEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleQuerySaveFileDialogEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_QUERY_SAVE_FILE_DIALOG].push_back(l);
}

void
BaseApplicationClient::addCanCopyEventListener(pfn_handleCanCopyEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCanCopyEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_CAN_COPY_EVENT].push_back(l);
}

void
BaseApplicationClient::addCanPasteEventListener(pfn_handleCanPasteEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCanPasteEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_CAN_PASTE_EVENT].push_back(l);
}

void
BaseApplicationClient::addInitializationCompleteEventListener(
    pfn_handleInitializationCompleteEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleInitializationComplete = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_INITIALIZATION_COMPLETE].push_back(l);
}

void
BaseApplicationClient::addCompleteDestructionEventListener(
    pfn_handleCompleteDestructionEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteDestruction = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_DESTRUCTION].push_back(l);
}

void
BaseApplicationClient::addCompleteTerminationEventListener(
    pfn_handleCompleteTerminationEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleCompleteTermination = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_TERMINATION].push_back(l);
}

void
BaseApplicationClient::addSetWindowDevicePixelRatioEventListener(
    pfn_handleSetWindowDevicePixelRatioEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetWindowDevicePixelRatioEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_WINDOW_DEVICE_PIXEL_RATIO_EVENT].push_back(l);
}

void
BaseApplicationClient::addSetViewportDevicePixelRatioEventListener(
    pfn_handleSetViewportDevicePixelRatioEvent listener, void *userData, bool once)
{
    EventListener l = { userData, once, { nullptr } };
    l.u.handleSetViewportDevicePixelRatioEvent = listener;
    m_eventListeners[NANOEM__APPLICATION__EVENT__TYPE_SET_VIEWPORT_DEVICE_PIXEL_RATIO_EVENT].push_back(l);
}

void
BaseApplicationClient::clearAllProjectAfterConfirmOnceEventListeners()
{
    clearAllOnceEventListeners(NANOEM__APPLICATION__EVENT__TYPE_SAVE_PROJECT_AFTER_CONFIRM);
    clearAllOnceEventListeners(NANOEM__APPLICATION__EVENT__TYPE_DISCARD_PROJECT_AFTER_CONFIRM);
}

void
BaseApplicationClient::clearAllCompleteLoadingFileOnceEventListeners()
{
    clearAllOnceEventListeners(NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_FILE);
}

void
BaseApplicationClient::clearAllCompleteSavingFileOnceEventListeners()
{
    clearAllOnceEventListeners(NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_SAVING_FILE);
}

nanoem_rsize_t
BaseApplicationClient::sizeofCommandMessage(const Nanoem__Application__Command *command) NANOEM_DECL_NOEXCEPT
{
    return nanoem__application__command__get_packed_size(command);
}

void
BaseApplicationClient::packCommandMessage(const Nanoem__Application__Command *command, nanoem_u8_t *data)
{
    nanoem__application__command__pack(command, data);
}

void
BaseApplicationClient::dispatchEventMessage(const nanoem_u8_t *data, size_t size)
{
    if (Nanoem__Application__Event *event = nanoem__application__event__unpack(g_protobufc_allocator, size, data)) {
        switch (event->type_case) {
        case NANOEM__APPLICATION__EVENT__TYPE_TRACK: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__TrackEvent *e = event->track;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleTrack(listener.userData, e->screen, e->category, e->action, e->label);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_UNDO: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__UndoEvent *e = event->undo;
                const bool canUndo = e->can_undo != 0;
                const bool canRedo = e->can_redo != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleUndo(listener.userData, canUndo, canRedo);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_REDO: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__RedoEvent *e = event->redo;
                const bool canUndo = e->can_undo != 0;
                const bool canRedo = e->can_redo != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleRedo(listener.userData, canRedo, canUndo);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_UNDO_CHANGE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleUndoChange(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_ADD_ACCESSORY: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__AddAccessoryEvent *e = event->add_accessory;
                const nanoem_u16_t handle = e->accessory_handle & 0xffff;
                const char *name = e->name;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAddAccessory(listener.userData, handle, name);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_ADD_MODEL: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__AddModelEvent *e = event->add_model;
                const nanoem_u16_t handle = e->model_handle & 0xffff;
                const char *name = e->name;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAddModel(listener.userData, handle, name);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_ACCESSORY: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetActiveAccessoryEvent *e = event->set_active_accessory;
                const nanoem_u16_t handle = e->accessory_handle & 0xffff;
                const char *name = e->name;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetActiveAccessory(listener.userData, handle, name);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_MODEL: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetActiveModelEvent *e = event->set_active_model;
                const nanoem_u16_t handle = e->model_handle & 0xffff;
                const char *name = e->name;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetActiveModel(listener.userData, handle, name);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_REMOVE_ACCESSORY: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__RemoveAccessoryEvent *e = event->remove_accessory;
                const nanoem_u16_t handle = e->accessory_handle & 0xffff;
                const char *name = e->name;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleRemoveAccessory(listener.userData, handle, name);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_REMOVE_MODEL: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__RemoveModelEvent *e = event->remove_model;
                const nanoem_u16_t handle = e->model_handle & 0xffff;
                const char *name = e->name;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleRemoveModel(listener.userData, handle, name);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_ADD_MOTION: {
            /* not implemented */
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_REMOVE_MOTION: {
            /* not implemented */
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_BONE: {
            /* not implemented */
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_ACTIVE_MORPH: {
            /* not implemented */
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SAVE_PROJECT_AFTER_CONFIRM: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSaveProjectAfterConfirm(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            clearAllOnceEventListeners(NANOEM__APPLICATION__EVENT__TYPE_DISCARD_PROJECT_AFTER_CONFIRM);
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_DISCARD_PROJECT_AFTER_CONFIRM: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleDiscardProjectAfterConfirm(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            clearAllOnceEventListeners(NANOEM__APPLICATION__EVENT__TYPE_SAVE_PROJECT_AFTER_CONFIRM);
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_ADD_BLEND_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_add_blend_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelAddBlendEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_COMPUTE_SHADER_SKINNING_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_compute_shader_skinning_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelComputeShaderSkinningEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHADOW_MAP_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_shadow_map_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelShadowMapEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_BONES: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_show_all_bones->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelShowAllBones(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_RIGID_BODIES: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_show_all_rigid_bodies->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelShowAllRigidBodies(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_FACES: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_show_all_vertex_faces->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelShowAllVertexFaces(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_SHOW_ALL_VERTEX_POINTS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_show_all_vertex_points->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelShowAllVertexPoints(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_VERTEX_SHADER_SKINNING: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_vertex_shader_skinning->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelVertexShaderSkinningEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_MODEL_VISIBLE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_active_model_visible->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveModelVisible(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_GRID_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_grid_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleGridEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_COMPUTE_SHADER_SKINNING_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_project_compute_shader_skinning_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleProjectComputeShaderSkinningEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_EFFECT_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_project_effect_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleProjectEffectEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_GROUND_SHADOW_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_project_ground_shadow_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleProjectGroundShadowEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_VERTEX_SHADER_SKINNING_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_project_vertex_shader_skinning_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleProjectVertexShaderSkinningEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_SHADOW_MAP_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                bool value = event->toggle_shadow_map_enabled->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleShadowMapEnabled(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_GRID_CELL: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetGridCellEvent *e = event->set_grid_cell;
                const Vector2 value(e->x, e->y);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetGridCell(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_GRID_SIZE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetGridSizeEvent *e = event->set_grid_size;
                const Vector2 value(e->x, e->y);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetGridSize(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_PHYSICS_SIMULATION_ENGINE_DEBUG_FLAG: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                nanoem_u32_t value = event->set_physics_simulation_engine_debug_flag->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetPhysicsSimulationEngineDebugFlag(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_PHYSICS_SIMULATION_MODE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                nanoem_u32_t value = event->set_physics_simulation_mode->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetPhysicsSimulationMode(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_PREFERRED_MOTION_FPS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetPreferredMotionFPSEvent *e = event->set_preferred_motion_fps;
                nanoem_u32_t value = e->value;
                bool unlimited = e->unlimited != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetPreferredMotionFPS(listener.userData, value, unlimited);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_PROJECT_SAMPLE_LEVEL: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                nanoem_u32_t value = event->set_project_sample_level->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetProjectSampleLevel(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_SHADOW_MAP_DISTANCE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                nanoem_f32_t value = event->set_shadow_map_distance->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetShadowMapDistance(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_SHADOW_MAP_MODE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                nanoem_u32_t value = event->set_shadow_map_mode->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetShadowMapMode(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_DISABLE_CURSOR: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__DisableCursorEvent *e = event->disable_cursor;
                const Vector2SI32 coord(e->x, e->y);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleDisableCursor(listener.userData, coord);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_ENABLE_CURSOR: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__EnableCursorEvent *e = event->enable_cursor;
                const Vector2SI32 coord(e->x, e->y);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleEnableCursor(listener.userData, coord);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_IS_PROJECT_DIRTY: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    c.u.isProjectDirty(c.userData, event->is_project_dirty->dirty != 0);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_MODEL_BONES: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    const Nanoem__Application__GetAllModelBonesResponseEvent *e = event->get_all_model_bones;
                    StringList values;
                    for (size_t i = 0, numItems = e->n_bones; i < numItems; i++) {
                        values.push_back(e->bones[i]);
                    }
                    const nanoem_u16_t handle = e->model_handle & 0xffff;
                    c.u.getAllModelBones(c.userData, handle, values);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_MODEL_MORPHS: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    const Nanoem__Application__GetAllModelMorphsResponseEvent *e = event->get_all_model_morphs;
                    StringList eyes, eyebrows, lips, others;
                    for (size_t i = 0, numItems = e->n_eye_morphs; i < numItems; i++) {
                        eyes.push_back(e->eye_morphs[i]);
                    }
                    for (size_t i = 0, numItems = e->n_eyebrow_morphs; i < numItems; i++) {
                        eyebrows.push_back(e->eyebrow_morphs[i]);
                    }
                    for (size_t i = 0, numItems = e->n_lip_morphs; i < numItems; i++) {
                        lips.push_back(e->lip_morphs[i]);
                    }
                    for (size_t i = 0, numItems = e->n_other_morphs; i < numItems; i++) {
                        others.push_back(e->other_morphs[i]);
                    }
                    const nanoem_u16_t handle = e->model_handle & 0xffff;
                    c.u.getAllModelMorphs(c.userData, handle, eyes, eyebrows, lips, others);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_PROJECT_FILE_URI: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    const Nanoem__Application__URI *uri = event->get_project_file_uri->file_uri;
                    const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
                    c.u.getProjectFileURI(c.userData, fileURI);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_IMPORTING_AUDIO_EXTENSIONS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__AvailableAllImportingAudioExtensionsEvent *e =
                    event->available_all_importing_audio_extensions;
                StringList values;
                for (size_t i = 0, numItems = e->n_extensions; i < numItems; i++) {
                    values.push_back(e->extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAvailableAllImportingAudioExtensions(listener.userData, values);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_IMPORTING_VIDEO_EXTENSIONS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__AvailableAllImportingVideoExtensionsEvent *e =
                    event->available_all_importing_video_extensions;
                StringList values;
                for (size_t i = 0, numItems = e->n_extensions; i < numItems; i++) {
                    values.push_back(e->extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAvailableAllImportingVideoExtensions(listener.userData, values);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_EXPORTING_IMAGE_EXTENSIONS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__AvailableAllExportingImageExtensionsEvent *e =
                    event->available_all_exporting_image_extensions;
                StringList values;
                for (size_t i = 0, numItems = e->n_extensions; i < numItems; i++) {
                    values.push_back(e->extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAvailableAllExportingImageExtensions(listener.userData, values);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_AVAILABLE_ALL_EXPORTING_VIDEO_EXTENSIONS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__AvailableAllExportingVideoExtensionsEvent *e =
                    event->available_all_exporting_video_extensions;
                StringList values;
                for (size_t i = 0, numItems = e->n_extensions; i < numItems; i++) {
                    values.push_back(e->extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAvailableAllExportingVideoExtensions(listener.userData, values);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_INITIALIZATION_COMPLETE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleInitializationComplete(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_DESTRUCTION: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteDestruction(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_TERMINATION: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteTermination(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_PAUSE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__PauseEvent *e = event->pause;
                nanoem_frame_index_t duration = nanoem_frame_index_t(e->duration),
                                     localFrameIndex = nanoem_frame_index_t(e->local_frame_index);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handlePause(listener.userData, duration, localFrameIndex);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_PLAY: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__PlayEvent *e = event->play;
                nanoem_frame_index_t duration = nanoem_frame_index_t(e->duration),
                                     localFrameIndex = nanoem_frame_index_t(e->local_frame_index);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handlePlay(listener.userData, duration, localFrameIndex);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_RESUME: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ResumeEvent *e = event->resume;
                nanoem_frame_index_t duration = nanoem_frame_index_t(e->duration),
                                     localFrameIndex = nanoem_frame_index_t(e->local_frame_index);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleResume(listener.userData, duration, localFrameIndex);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SEEK: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SeekEvent *e = event->seek;
                nanoem_frame_index_t duration = nanoem_frame_index_t(e->duration),
                                     localFrameIndexFrom = nanoem_frame_index_t(e->local_frame_index_from),
                                     localFrameIndexTo = nanoem_frame_index_t(e->local_frame_index_to);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSeek(listener.userData, duration, localFrameIndexTo, localFrameIndexFrom);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_STOP: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__StopEvent *e = event->stop;
                nanoem_frame_index_t duration = nanoem_frame_index_t(e->duration),
                                     localFrameIndex = nanoem_frame_index_t(e->local_frame_index);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleStop(listener.userData, duration, localFrameIndex);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_UPDATE_DURATION: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__UpdateDurationEvent *e = event->update_duration;
                nanoem_frame_index_t currentDuration = nanoem_frame_index_t(e->current_duration),
                                     lastDuration = nanoem_frame_index_t(e->last_duration);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleUpdateDuration(listener.userData, currentDuration, lastDuration);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_ERROR: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ErrorEvent *e = event->error;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleError(listener.userData, e->code, e->reason, e->recovery_suggestion);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_START_RECORDING_VIEWPORT_PASS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__StartRecordingViewportPassEvent *e = event->start_recording_viewport_pass;
                const Vector2UI32 size(e->width, e->height);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleStartRecordingViewportPass(listener.userData, size);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_STOP_RECORDING_VIEWPORT_PASS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleStopRecordingViewportPass(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_FILE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__CompleteLoadingFileEvent *e = event->complete_loading_file;
                const Nanoem__Application__URI *uri = e->file_uri;
                const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
                const nanoem_u32_t type = e->type;
                const nanoem_u64_t ticks = e->ticks;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteLoadingFile(listener.userData, fileURI, type, ticks);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_SAVING_FILE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__CompleteSavingFileEvent *e = event->complete_saving_file;
                const Nanoem__Application__URI *uri = e->file_uri;
                const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
                const nanoem_u32_t type = e->type;
                const nanoem_u64_t ticks = e->ticks;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteSavingFile(listener.userData, fileURI, type, ticks);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_PREFERRED_EDITING_FPS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetPreferredEditingFPSEvent *e = event->set_preferred_editing_fps;
                nanoem_u32_t value = e->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetPreferredEditingFPS(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_BACKGROUND_IMAGE_TEXTURE_HANDLE: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    c.u.getBackgroundTextureImageHandle(c.userData, event->get_background_image_texture_handle->handle);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_CONSUME_PASS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ConsumePassEvent *e = event->consume_pass;
                nanoem_u64_t value = e->global_frame_index;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleConsumePass(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_ADD_MODAL_DIALOG: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleAddModalDialog(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_CLEAR_MODAL_DIALOG: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleClearModalDialog(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_LANGUAGE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetLanguageEvent *e = event->set_language;
                nanoem_u32_t language = e->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetLanguage(listener.userData, language);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_QUERY_OPEN_SINGLE_FILE_DIALOG: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__QueryOpenSingleFileDialogEvent *e = event->query_open_single_file_dialog;
                nanoem_u32_t type = e->type;
                StringList allowedExtensions;
                for (nanoem_rsize_t i = 0, numItems = e->n_allowed_extensions; i < numItems; i++) {
                    allowedExtensions.push_back(e->allowed_extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleQueryOpenSingleFileDialogEvent(listener.userData, type, allowedExtensions);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_QUERY_OPEN_MULTIPLE_FILES_DIALOG: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__QueryOpenMultipleFilesDialogEvent *e =
                    event->query_open_multiple_files_dialog;
                nanoem_u32_t type = e->type;
                StringList allowedExtensions;
                for (nanoem_rsize_t i = 0, numItems = e->n_allowed_extensions; i < numItems; i++) {
                    allowedExtensions.push_back(e->allowed_extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleQueryOpenMultipleFilesDialogEvent(listener.userData, type, allowedExtensions);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_QUERY_SAVE_FILE_DIALOG: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__QuerySaveFileDialogEvent *e = event->query_save_file_dialog;
                nanoem_u32_t type = e->type;
                StringList allowedExtensions;
                for (nanoem_rsize_t i = 0, numItems = e->n_allowed_extensions; i < numItems; i++) {
                    allowedExtensions.push_back(e->allowed_extensions[i]);
                }
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleQuerySaveFileDialogEvent(listener.userData, type, allowedExtensions);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_ADD_BLEND_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ToggleActiveAccessoryAddBlendEnabledEvent *e =
                    event->toggle_active_accessory_add_blend_enabled;
                bool enabled = e->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveAccessoryAddBlendEnabledEvent(listener.userData, enabled);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_SHADOW_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ToggleActiveAccessoryShadowEnabledEvent *e =
                    event->toggle_active_accessory_shadow_enabled;
                bool enabled = e->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveAccessoryShadowEnabledEvent(listener.userData, enabled);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_ACTIVE_ACCESSORY_VISIBLE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ToggleActiveAccessoryVisibleEvent *e =
                    event->toggle_active_accessory_visible;
                bool enabled = e->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleActiveAccessoryVisibleEvent(listener.userData, enabled);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_TOGGLE_PROJECT_PLAYING_WITH_LOOP_ENABLED: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__ToggleProjectPlayingWithLoopEnabledEvent *e =
                    event->toggle_project_playing_with_loop_enabled;
                bool enabled = e->value != 0;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleToggleProjectPlayingWithLoopEnabledEvent(listener.userData, enabled);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_ACCESSORIES: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    const Nanoem__Application__GetAllAccessoriesResponseEvent *e = event->get_all_accessories;
                    DrawableItemList items;
                    for (nanoem_rsize_t i = 0, numItems = e->n_items; i < numItems; i++) {
                        const Nanoem__Application__GetAllAccessoriesResponseEvent__Item *item = e->items[i];
                        DrawableItem drawable;
                        drawable.m_handle = item->handle;
                        drawable.m_name = item->name;
                        drawable.m_active = item->is_active != 0;
                        items.push_back(drawable);
                    }
                    c.u.getAllAccessories(c.userData, items);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_ALL_MODELS: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    const Nanoem__Application__GetAllModelsResponseEvent *e = event->get_all_models;
                    DrawableItemList items;
                    for (nanoem_rsize_t i = 0, numItems = e->n_items; i < numItems; i++) {
                        const Nanoem__Application__GetAllModelsResponseEvent__Item *item = e->items[i];
                        DrawableItem drawable;
                        drawable.m_handle = item->handle;
                        drawable.m_name = item->name;
                        drawable.m_active = item->is_active != 0;
                        items.push_back(drawable);
                    }
                    c.u.getAllModels(c.userData, items);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_UPDATE_PROGRESS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__UpdateProgressEvent *e = event->update_progress;
                nanoem_u32_t value = e->value, total = e->total, type = e->type_case;
                const char *text = e->text;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleUpdateProgressEvent(listener.userData, value, total, type, text);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_START_PROGRESS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__StartProgressEvent *e = event->start_progress;
                nanoem_u32_t total = e->total;
                const char *title = e->title, *text = e->text;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleStartProgressEvent(listener.userData, title, text, total);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_STOP_PROGRESS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleStopProgressEvent(listener.userData);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SETUP_PROJECT: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetupProjectEvent *e = event->setup_project;
                const Vector2 windowSize(e->window_width, e->window_height);
                const nanoem_f32_t windowDevicePixelRatio = e->window_device_pixel_ratio;
                const nanoem_f32_t viewportDevicePixelRatio =
                    e->has_viewport_device_pixel_ratio ? e->viewport_device_pixel_ratio : 1.0f;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetupProjectEvent(
                        listener.userData, windowSize, windowDevicePixelRatio, viewportDevicePixelRatio);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_EDITING_MODE: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__SetEditingModeEvent *e = event->set_editing_mode;
                const nanoem_u32_t value = e->value;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetEditingModeEvent(listener.userData, value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_ALL_MODEL_IO_PLUGINS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__CompleteLoadingAllModelIOPluginsEvent *e =
                    event->complete_loading_all_model_io_plugins;
                PluginItemList plugins;
                internal::ApplicationUtils::constructPluginList(e->items, e->n_items, plugins);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteLoadingAllModelPluginsEvent(listener.userData, plugins);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_LOADING_ALL_MOTION_IO_PLUGINS: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                EventListenerList &listeners = it->second;
                const Nanoem__Application__CompleteLoadingAllMotionIOPluginsEvent *e =
                    event->complete_loading_all_motion_io_plugins;
                PluginItemList plugins;
                internal::ApplicationUtils::constructPluginList(e->items, e->n_items, plugins);
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteLoadingAllMotionPluginsEvent(listener.userData, plugins);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_EXPORT_IMAGE_CONFIGURATION: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                const Nanoem__Application__CompleteExportImageConfigurationEvent *e =
                    event->complete_export_image_configuration;
                StringList availableExtensions;
                for (size_t i = 0, numItems = e->n_available_extensions; i < numItems; i++) {
                    availableExtensions.push_back(e->available_extensions[i]);
                }
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteExportImageConfigurationEvent(listener.userData, availableExtensions);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_COMPLETE_EXPORT_VIDEO_CONFIGURATION: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                const Nanoem__Application__CompleteExportVideoConfigurationEvent *e =
                    event->complete_export_video_configuration;
                StringList availableExtensions;
                for (size_t i = 0, numItems = e->n_available_extensions; i < numItems; i++) {
                    availableExtensions.push_back(e->available_extensions[i]);
                }
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCompleteExportVideoConfigurationEvent(listener.userData, availableExtensions);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_CAN_COPY_EVENT: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                const Nanoem__Application__CanCopyEvent *e = event->can_copy_event;
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCanCopyEvent(listener.userData, e->value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_WINDOW_DEVICE_PIXEL_RATIO_EVENT: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                const Nanoem__Application__SetWindowDevicePixelRatioEvent *e =
                    event->set_window_device_pixel_ratio_event;
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetWindowDevicePixelRatioEvent(listener.userData, e->value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_SET_VIEWPORT_DEVICE_PIXEL_RATIO_EVENT: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                const Nanoem__Application__SetViewportDevicePixelRatioEvent *e =
                    event->set_viewport_device_pixel_ratio_event;
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleSetViewportDevicePixelRatioEvent(listener.userData, e->value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_CAN_PASTE_EVENT: {
            EventListenerListMap::iterator it = m_eventListeners.find(event->type_case);
            if (it != m_eventListeners.end()) {
                const Nanoem__Application__CanPasteEvent *e = event->can_paste_event;
                EventListenerList &listeners = it->second;
                for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
                    const EventListener &listener = *it2;
                    listener.u.handleCanPasteEvent(listener.userData, e->value);
                    it2 = listener.once ? listeners.erase(it2) : it2 + 1;
                }
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE_GET_HANDLE_FILE_URI: {
            RequestCallbackMap::const_iterator it = m_requestCallbacks.find(event->requested_timestamp);
            if (it != m_requestCallbacks.end()) {
                const RequestCallback &c = it->second;
                if (c.type == event->type_case) {
                    const Nanoem__Application__URI *uri = event->get_handle_file_uri->file_uri;
                    const URI &fileURI = URI::createFromFilePath(uri->absolute_path, uri->fragment);
                    c.u.getHandleFileURI(c.userData, event->get_handle_file_uri->handle, fileURI);
                }
                m_requestCallbacks.erase(it);
            }
            break;
        }
        case NANOEM__APPLICATION__EVENT__TYPE__NOT_SET:
        default:
            break;
        }
        nanoem__application__event__free_unpacked(event, g_protobufc_allocator);
    }
}

void
BaseApplicationClient::clearAllOnceEventListeners(nanoem_u32_t type)
{
    EventListenerListMap::iterator it = m_eventListeners.find(type);
    if (it != m_eventListeners.end()) {
        EventListenerList &listeners = it->second;
        for (EventListenerList::iterator it2 = listeners.begin(); it2 != listeners.end();) {
            const EventListener &listener = *it2;
            it2 = listener.once ? listeners.erase(it2) : it2 + 1;
        }
    }
}

} /* namespace nanoem */
