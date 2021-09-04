/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "nanoem/nanoem.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

TEST_CASE("motion_copy_all_bone_keyframes", "[emapp][motion]")
{
    TestScope scope;
    ProjectPtr first = scope.createProject();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *source = nanoemMotionCreate(first->m_project->unicodeStringFactory(), &status);
    nanoem_mutable_buffer_t *mb = nullptr;
    {
        nanoem_u8_t signature[30] = "Vocaloid Motion Data 0002";
        nanoem_u8_t targetModelName[20] = "test";
        nanoem_u8_t targetBoneName[15] = { 0x83, 0x5a, 0x83, 0x93, 0x83, 0x5e, 0x81, 0x5b, 0x0 };
        mb = nanoemMutableBufferCreate(&status);
        nanoemMutableBufferWriteByteArray(mb, signature, sizeof(signature), &status);
        nanoemMutableBufferWriteByteArray(mb, targetModelName, sizeof(targetModelName), &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 2, &status);
        for (int i = 0; i < 2; i++) {
            nanoemMutableBufferWriteByteArray(mb, targetBoneName, sizeof(targetBoneName), &status);
            // fill zero with frame index + translation (3 components) + orientation (4 components) + interpolation (16
            // components)
            for (int i = 0; i < 24; i++) {
                nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
            }
        }
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
    }
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mb, &status);
    nanoemMotionLoadFromBuffer(source, buffer, 0, &status);
    nanoem_motion_bone_keyframe_t *const *keyframes = nanoemMotionGetAllBoneKeyframeObjects(source, &numKeyframes);
    nanoem_mutable_motion_t *dest = nanoemMutableMotionCreate(first->m_project->unicodeStringFactory(), &status);
    Model *bindingModel = first->createModel();
    first->m_project->addModel(bindingModel);
    {
        CHECK(numKeyframes == 2);
        Motion::copyAllBoneKeyframes(keyframes, numKeyframes, nullptr, bindingModel, dest, 0, status);
        CHECK(status != NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    nanoemMutableMotionDestroy(dest);
    nanoemMotionDestroy(source);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mb);
}

TEST_CASE("motion_copy_all_camera_keyframes", "[emapp][motion]")
{
    TestScope scope;
    ProjectPtr first = scope.createProject();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *source = nanoemMotionCreate(first->m_project->unicodeStringFactory(), &status);
    nanoem_mutable_buffer_t *mb = nullptr;
    {
        nanoem_u8_t signature[30] = "Vocaloid Motion Data 0002";
        nanoem_u8_t targetModelName[20] = "test";
        mb = nanoemMutableBufferCreate(&status);
        nanoemMutableBufferWriteByteArray(mb, signature, sizeof(signature), &status);
        nanoemMutableBufferWriteByteArray(mb, targetModelName, sizeof(targetModelName), &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 2, &status);
        for (int i = 0; i < 2; i++) {
            // fill zero with frame index + distance + lookat (3 components) + angle (3 components) + interpolation (6
            // components) + fov
            for (int i = 0; i < 15; i++) {
                nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
            }
            nanoemMutableBufferWriteByte(mb, 0, &status);
        }
    }
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mb, &status);
    nanoemMotionLoadFromBuffer(source, buffer, 0, &status);
    nanoem_motion_camera_keyframe_t *const *keyframes = nanoemMotionGetAllCameraKeyframeObjects(source, &numKeyframes);
    nanoem_mutable_motion_t *dest = nanoemMutableMotionCreate(first->m_project->unicodeStringFactory(), &status);
    {
        CHECK(numKeyframes == 2);
        Motion::copyAllCameraKeyframes(keyframes, numKeyframes, dest, 0, status);
        CHECK(status != NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    nanoemMutableMotionDestroy(dest);
    nanoemMotionDestroy(source);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mb);
}

TEST_CASE("motion_copy_all_light_keyframes", "[emapp][motion]")
{
    TestScope scope;
    ProjectPtr first = scope.createProject();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *source = nanoemMotionCreate(first->m_project->unicodeStringFactory(), &status);
    nanoem_mutable_buffer_t *mb = nullptr;
    {
        nanoem_u8_t signature[30] = "Vocaloid Motion Data 0002";
        nanoem_u8_t targetModelName[20] = "test";
        mb = nanoemMutableBufferCreate(&status);
        nanoemMutableBufferWriteByteArray(mb, signature, sizeof(signature), &status);
        nanoemMutableBufferWriteByteArray(mb, targetModelName, sizeof(targetModelName), &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 2, &status);
        for (int i = 0; i < 2; i++) {
            // fill zero with frame index + color (3 components) + direction (3 components)
            for (int i = 0; i < 7; i++) {
                nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
            }
        }
    }
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mb, &status);
    nanoemMotionLoadFromBuffer(source, buffer, 0, &status);
    nanoem_motion_light_keyframe_t *const *keyframes = nanoemMotionGetAllLightKeyframeObjects(source, &numKeyframes);
    nanoem_mutable_motion_t *dest = nanoemMutableMotionCreate(first->m_project->unicodeStringFactory(), &status);
    {
        CHECK(numKeyframes == 2);
        Motion::copyAllLightKeyframes(keyframes, numKeyframes, dest, 0, status);
        CHECK(status != NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    nanoemMutableMotionDestroy(dest);
    nanoemMotionDestroy(source);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mb);
}

TEST_CASE("motion_copy_all_model_keyframes", "[emapp][motion]")
{
    TestScope scope;
    ProjectPtr first = scope.createProject();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *source = nanoemMotionCreate(first->m_project->unicodeStringFactory(), &status);
    nanoem_mutable_buffer_t *mb = nullptr;
    {
        nanoem_u8_t signature[30] = "Vocaloid Motion Data 0002";
        nanoem_u8_t targetModelName[20] = "test";
        mb = nanoemMutableBufferCreate(&status);
        nanoemMutableBufferWriteByteArray(mb, signature, sizeof(signature), &status);
        nanoemMutableBufferWriteByteArray(mb, targetModelName, sizeof(targetModelName), &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 2, &status);
        for (int i = 0; i < 2; i++) {
            // fill zero with frame index + visible + constraints
            nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
            nanoemMutableBufferWriteByte(mb, 0, &status);
            nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        }
    }
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mb, &status);
    nanoemMotionLoadFromBuffer(source, buffer, 0, &status);
    nanoem_motion_model_keyframe_t *const *keyframes = nanoemMotionGetAllModelKeyframeObjects(source, &numKeyframes);
    nanoem_mutable_motion_t *dest = nanoemMutableMotionCreate(first->m_project->unicodeStringFactory(), &status);
    {
        CHECK(numKeyframes == 2);
        Motion::copyAllModelKeyframes(keyframes, numKeyframes, nullptr, dest, 0, status);
        CHECK(status != NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    nanoemMutableMotionDestroy(dest);
    nanoemMotionDestroy(source);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mb);
}

TEST_CASE("motion_copy_all_morph_keyframes", "[emapp][motion]")
{
    TestScope scope;
    ProjectPtr first = scope.createProject();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *source = nanoemMotionCreate(first->m_project->unicodeStringFactory(), &status);
    nanoem_mutable_buffer_t *mb = nullptr;
    {
        nanoem_u8_t signature[30] = "Vocaloid Motion Data 0002";
        nanoem_u8_t targetModelName[20] = "test";
        nanoem_u8_t targetMorphName[15] = { 0x82, 0xdc, 0x82, 0xce, 0x82, 0xbd, 0x82, 0xab, 0x0 };
        mb = nanoemMutableBufferCreate(&status);
        nanoemMutableBufferWriteByteArray(mb, signature, sizeof(signature), &status);
        nanoemMutableBufferWriteByteArray(mb, targetModelName, sizeof(targetModelName), &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 2, &status);
        for (int i = 0; i < 2; i++) {
            nanoemMutableBufferWriteByteArray(mb, targetMorphName, sizeof(targetMorphName), &status);
            // fill zero with frame index + weight
            for (int i = 0; i < 2; i++) {
                nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
            }
        }
    }
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mb, &status);
    nanoemMotionLoadFromBuffer(source, buffer, 0, &status);
    nanoem_motion_morph_keyframe_t *const *keyframes = nanoemMotionGetAllMorphKeyframeObjects(source, &numKeyframes);
    nanoem_mutable_motion_t *dest = nanoemMutableMotionCreate(first->m_project->unicodeStringFactory(), &status);
    Model *bindingModel = first->createModel();
    first->m_project->addModel(bindingModel);
    {
        CHECK(numKeyframes == 2);
        Motion::copyAllMorphKeyframes(keyframes, numKeyframes, nullptr, bindingModel, dest, 0, status);
        CHECK(status != NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    nanoemMutableMotionDestroy(dest);
    nanoemMotionDestroy(source);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mb);
}

TEST_CASE("motion_copy_all_self_shadow_keyframes", "[emapp][motion]")
{
    TestScope scope;
    ProjectPtr first = scope.createProject();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *source = nanoemMotionCreate(first->m_project->unicodeStringFactory(), &status);
    nanoem_mutable_buffer_t *mb = nullptr;
    {
        nanoem_u8_t signature[30] = "Vocaloid Motion Data 0002";
        nanoem_u8_t targetModelName[20] = "test";
        mb = nanoemMutableBufferCreate(&status);
        nanoemMutableBufferWriteByteArray(mb, signature, sizeof(signature), &status);
        nanoemMutableBufferWriteByteArray(mb, targetModelName, sizeof(targetModelName), &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        nanoemMutableBufferWriteInt32LittleEndian(mb, 2, &status);
        for (int i = 0; i < 2; i++) {
            // fill zero with frame index + mode + distance
            nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
            nanoemMutableBufferWriteByte(mb, 0, &status);
            nanoemMutableBufferWriteInt32LittleEndian(mb, 0, &status);
        }
    }
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mb, &status);
    nanoemMotionLoadFromBuffer(source, buffer, 0, &status);
    nanoem_motion_self_shadow_keyframe_t *const *keyframes =
        nanoemMotionGetAllSelfShadowKeyframeObjects(source, &numKeyframes);
    nanoem_mutable_motion_t *dest = nanoemMutableMotionCreate(first->m_project->unicodeStringFactory(), &status);
    {
        CHECK(numKeyframes == 2);
        Motion::copyAllSelfShadowKeyframes(keyframes, numKeyframes, dest, 0, status);
        CHECK(status != NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS);
        CHECK(status == NANOEM_STATUS_SUCCESS);
    }
    nanoemMutableMotionDestroy(dest);
    nanoemMotionDestroy(source);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mb);
}
