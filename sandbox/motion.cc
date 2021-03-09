/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanoem/ext/motion.h"
#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/color_space.hpp"

#include <iostream>

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);

namespace {

static void
generateCameraMotion(nanoem_unicode_string_factory_t *factory)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *motion = nanoemMutableMotionCreate(factory, &status);
    for (nanoem_frame_index_t i = 0; i <= 360; i++) {
        nanoem_mutable_motion_camera_keyframe_t *keyframe =
            nanoemMutableMotionCameraKeyframeCreate(nanoemMutableMotionGetOriginObject(motion), &status);
        nanoemMutableMotionCameraKeyframeSetAngle(keyframe, glm::value_ptr(glm::vec3(0, glm::vec3::value_type(i), 0)));
        nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, glm::value_ptr(glm::vec3(0, 15, 0)));
        nanoemMutableMotionCameraKeyframeSetDistance(keyframe, -25);
        nanoemMutableMotionCameraKeyframeSetFov(keyframe, 30);
        nanoemMutableMotionAddCameraKeyframe(motion, keyframe, i, &status);
        nanoemMutableMotionCameraKeyframeDestroy(keyframe);
    }
    nanoem_mutable_buffer_t *buffer = nanoemMutableBufferCreate(&status);
    nanoemMutableMotionSaveToBuffer(motion, buffer, &status);
    FILE *fp = fopen("camera.vmd", "wb");
    nanoem_buffer_t *output = nanoemMutableBufferCreateBufferObject(buffer, &status);
    fwrite(nanoemBufferGetDataPtr(output), nanoemBufferGetLength(output), 1, fp);
    fclose(fp);
    nanoemMutableMotionDestroy(motion);
    nanoemMutableBufferDestroy(buffer);
    nanoemBufferDestroy(output);
}

static void
generateLightMotion(nanoem_unicode_string_factory_t *factory)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *motion = nanoemMutableMotionCreate(factory, &status);
    for (nanoem_frame_index_t i = 0; i <= 360; i++) {
        nanoem_mutable_motion_light_keyframe_t *keyframe =
            nanoemMutableMotionLightKeyframeCreate(nanoemMutableMotionGetOriginObject(motion), &status);
        const glm::vec3 &rgb = glm::rgbColor(glm::vec3(i, 1.0f, 1.0f));
        nanoemMutableMotionLightKeyframeSetColor(keyframe, glm::value_ptr(rgb));
        nanoemMutableMotionLightKeyframeSetDirection(keyframe, glm::value_ptr(glm::vec3(-1, 1, 1)));
        nanoemMutableMotionAddLightKeyframe(motion, keyframe, i, &status);
        nanoemMutableMotionLightKeyframeDestroy(keyframe);
    }
    nanoem_mutable_buffer_t *buffer = nanoemMutableBufferCreate(&status);
    nanoemMutableMotionSaveToBuffer(motion, buffer, &status);
    FILE *fp = fopen("light.vmd", "wb");
    nanoem_buffer_t *output = nanoemMutableBufferCreateBufferObject(buffer, &status);
    fwrite(nanoemBufferGetDataPtr(output), nanoemBufferGetLength(output), 1, fp);
    fclose(fp);
    nanoemMutableMotionDestroy(motion);
    nanoemMutableBufferDestroy(buffer);
    nanoemBufferDestroy(output);
}

static void
loadMotion(nanoem_unicode_string_factory_t *factory, nanoem_u8_t *data, size_t size, nanoem_status_t *status)
{
    nanoem_motion_t *motion = nanoemMotionCreate(factory, status);
    nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, status);
    nanoemMotionLoadFromBuffer(motion, buffer, 0, status);
    {
        nanoem_mutable_motion_t *new_motion = nanoemMutableMotionCreateAsReference(motion, status);
        nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(status);
        nanoem_status_t mutable_status;
#if 0
        nanoem_global_allocator_t allocator = {};
        allocator.malloc = [](void *, nanoem_rsize_t size, const char *file, int line) {
            fprintf(stdout, "[malloc] %ld bytes at %s:%d\n", size, file, line);
            return ::malloc(size);
        };
        allocator.calloc = [](void *, nanoem_rsize_t count, nanoem_rsize_t size, const char *file, int line) {
            fprintf(stdout, "[calloc] %ld bytes at %s:%d\n", size * count, file, line);
            return ::calloc(count, size);
        };
        allocator.realloc = [](void *, void *ptr, nanoem_rsize_t size, const char *file, int line) {
            fprintf(stdout, "[realloc] %ld bytes at %s:%d\n", size, file, line);
            return ::realloc(ptr, size);
        };
        allocator.free = [](void *, void *ptr, const char *file, int line) {
            fprintf(stdout, "[free] at %s:%d\n", file, line);
            ::free(ptr);
        };
        nanoemGlobalSetCustomAllocator(&allocator);
        nanoemMutableMotionSaveToBuffer(new_motion, mutable_buffer, &mutable_status);
        nanoemGlobalSetCustomAllocator(0);
#else
        nanoemMutableMotionSaveToBuffer(new_motion, mutable_buffer, &mutable_status);
#endif
        nanoemMutableBufferDestroy(mutable_buffer);
        nanoemMutableMotionDestroy(new_motion);
    }
    nanoemBufferDestroy(buffer);
    nanoemMotionDestroy(motion);
}

static void
loadMotionFromFile(nanoem_unicode_string_factory_t *factory, const char *input_path)
{
    nanoem_status_t status;
    if (FILE *fp = fopen(input_path, "rb")) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        nanoem_u8_t *data = new nanoem_u8_t[size];
        fread(data, size, 1, fp);
        loadMotion(factory, data, size, &status);
        delete[] data;
        fclose(fp);
        fprintf(stderr, "%s: %d\n", input_path, status);
    }
}

} /* namespace anonymous */

int
main(int argc, char **argv)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
    if (argc > 1) {
        const char *input_path = argv[1];
        if (strstr(input_path, ".txt")) {
            if (FILE *fp = fopen(input_path, "r")) {
                char buffer[1024];
                while (fgets(buffer, sizeof(buffer), fp)) {
                    buffer[strlen(buffer) - 1] = 0;
                    loadMotionFromFile(factory, buffer);
                }
                fclose(fp);
            }
        }
        else {
            loadMotionFromFile(factory, input_path);
        }
    }
    else {
        generateCameraMotion(factory);
        generateLightMotion(factory);
        static const nanoem_u8_t expected_interpolation[] = { 12, 24, 36, 48 };
        static const nanoem_f32_t expected_translation[] = { 1, 2, 3 };
        static const nanoem_f32_t expected_orientation[] = { 0.1f, 0.2f, 0.3f, 0.4f };
        nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(&status);
        // nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT();
        nanoem_unicode_string_t *name = nanoemUnicodeStringFactoryCreateString(factory,
            reinterpret_cast<const nanoem_u8_t *>("this_is_a_long_bone_keyframe_name"),
            strlen("this_is_a_long_bone_keyframe_name"), &status);
        {
            nanoem_motion_t *motion = nanoemMotionCreate(factory, &status);
            nanoem_mutable_motion_t *mutable_motion = nanoemMutableMotionCreateAsReference(motion, &status);
            nanoem_mutable_motion_bone_keyframe_t *mutable_keyframe =
                nanoemMutableMotionBoneKeyframeCreate(motion, &status);
            nanoemMutableMotionBoneKeyframeSetTranslation(mutable_keyframe, expected_translation);
            nanoemMutableMotionBoneKeyframeSetOrientation(mutable_keyframe, expected_orientation);
            for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
                 i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                nanoemMutableMotionBoneKeyframeSetInterpolation(
                    mutable_keyframe, (nanoem_motion_bone_keyframe_interpolation_type_t) i, expected_interpolation);
            }
            nanoemMutableMotionBoneKeyframeSetStageIndex(mutable_keyframe, 7);
            nanoemMutableMotionAddBoneKeyframe(mutable_motion, mutable_keyframe, name, 42, &status);
            nanoemMutableMotionBoneKeyframeDestroy(mutable_keyframe);
            nanoemMutableMotionSaveToBufferNMD(mutable_motion, mutable_buffer, &status);
            nanoemMutableMotionDestroy(mutable_motion);
            nanoemMotionDestroy(motion);
        }
    }
    // nanoemUnicodeStringFactoryDestroyEXT(factory);
    return 0;
}
