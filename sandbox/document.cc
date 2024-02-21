/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "nanoem/ext/document.h"
#include "nanoem/nanoem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <string>
#include <unordered_map>
#include <vector>

NANOEM_DECL_API nanoem_unicode_string_factory_t *APIENTRY nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);
NANOEM_DECL_API void APIENTRY nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory,
    const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity,
    nanoem_status_t *status);

static std::string
getString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *name)
{
    nanoem_status_t status;
    nanoem_rsize_t length;
    nanoem_u8_t *bytes =
        nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, name, &length, NANOEM_CODEC_TYPE_UTF8, &status);
    std::string s(reinterpret_cast<const char *>(bytes), length);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
    return s;
}

void
replacePathSeparator(nanoem_u8_t *p)
{
    while (*p) {
        if (*p == '\\') {
            *p = '/';
        }
        p++;
    }
}

static nanoem_model_t *
callback(void *user_data, const nanoem_unicode_string_t *path, nanoem_unicode_string_factory_t *factory,
    nanoem_status_t *status)
{
    static const char needle[] = "/UserFile/Model/";
    nanoem_model_t *model = NULL;
    nanoem_rsize_t length;
    const char *pmm_path = static_cast<const char *>(user_data);
#if _WIN32
    nanoem_u8_t *s =
        nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, path, &length, NANOEM_CODEC_TYPE_SJIS, status);
#else
    nanoem_u8_t *s = nanoemUnicodeStringFactoryGetByteArray(factory, path, &length, status);
#endif
    replacePathSeparator(s);
    if (const char *ptr = strstr(reinterpret_cast<const char *>(s), needle)) {
        const char *p = ptr + sizeof(needle) - 1;
        char full_path[255] = { 0 };
        if (const char *filename_ptr = strrchr(pmm_path, '/')) {
            strncpy(full_path, pmm_path, filename_ptr - pmm_path);
            strcat(full_path, "/Model/");
            strcat(full_path, p);
        }
        if (FILE *fp = fopen(full_path, "rb")) {
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            nanoem_u8_t *data = new nanoem_u8_t[size];
            fread(data, size, 1, fp);
            fclose(fp);
            nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, status);
            model = nanoemModelCreate(factory, status);
            nanoemModelLoadFromBuffer(model, buffer, status);
            nanoemBufferDestroy(buffer);
            delete[] data;
        }
    }
    nanoemUnicodeStringFactoryDestroyByteArray(factory, s);
    return model;
}

static void
copyDocument(nanoem_unicode_string_factory_t *factory, const nanoem_document_t *src, nanoem_mutable_document_t *dst)
{
    using ModelBoneMap = std::unordered_map<std::string, std::vector<std::string>>;
    ModelBoneMap model_bone_map;
    ModelBoneMap model_constraint_map;
    ModelBoneMap model_morph_map;
    nanoem_mark_unused(factory);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableDocumentSetAudioPath(dst, nanoemDocumentGetAudioPath(src), &status);
    nanoemMutableDocumentSetAudioEnabled(dst, nanoemDocumentIsAudioEnabled(src));
    nanoemMutableDocumentSetBackgroundImageEnabled(dst, nanoemDocumentIsBackgroundImageEnabled(src));
    nanoemMutableDocumentSetBackgroundImageOffsetX(dst, nanoemDocumentGetBackgroundImageOffsetX(src));
    nanoemMutableDocumentSetBackgroundImageOffsetY(dst, nanoemDocumentGetBackgroundImageOffsetY(src));
    nanoemMutableDocumentSetBackgroundImageScaleFactor(dst, nanoemDocumentGetBackgroundImageScaleFactor(src));
    nanoemMutableDocumentSetBackgroundImagePath(dst, nanoemDocumentGetBackgroundImagePath(src), &status);
    nanoemMutableDocumentSetBackgroundVideoEnabled(dst, nanoemDocumentIsBackgroundVideoEnabled(src));
    nanoemMutableDocumentSetBackgroundVideoOffsetX(dst, nanoemDocumentGetBackgroundVideoOffsetX(src));
    nanoemMutableDocumentSetBackgroundVideoOffsetY(dst, nanoemDocumentGetBackgroundVideoOffsetY(src));
    nanoemMutableDocumentSetBackgroundVideoScaleFactor(dst, nanoemDocumentGetBackgroundVideoScaleFactor(src));
    nanoemMutableDocumentSetBackgroundVideoPath(dst, nanoemDocumentGetBackgroundVideoPath(src), &status);
    nanoemMutableDocumentSetEdgeColor(dst, nanoemDocumentGetEdgeColor(src));
    nanoemMutableDocumentSetPhysicsSimulationMode(dst, nanoemDocumentGetPhysicsSimulationMode(src));
    nanoemMutableDocumentSetPreferredFPS(dst, nanoemDocumentGetPreferredFPS(src));
    nanoemMutableDocumentSetTimelineWidth(dst, nanoemDocumentGetTimelineWidth(src));
    nanoemMutableDocumentSetViewportHeight(dst, nanoemDocumentGetViewportHeight(src));
    nanoemMutableDocumentSetViewportWidth(dst, nanoemDocumentGetViewportWidth(src));
    const nanoem_document_camera_t *camera = nanoemDocumentGetCameraObject(src);
    nanoem_mutable_document_camera_t *c = nanoemMutableDocumentCameraCreate(dst, &status);
    nanoemMutableDocumentCameraSetAngle(c, nanoemDocumentCameraGetAngle(camera));
    nanoemMutableDocumentCameraSetDistance(c, nanoemDocumentCameraGetDistance(camera));
    nanoemMutableDocumentCameraSetFov(c, nanoemDocumentCameraGetFov(camera));
    nanoemMutableDocumentCameraSetLookAt(c, nanoemDocumentCameraGetLookAt(camera));
    nanoemMutableDocumentCameraSetPerspectiveEnabled(c, nanoemDocumentCameraIsPerspectiveEnabled(camera));
    nanoemMutableDocumentCameraSetPosition(c, nanoemDocumentCameraGetPosition(camera));
    static bool s_write_all_camera_keyframes = true;
    auto find_model = [factory, src, dst](const nanoem_document_model_t *model) -> const nanoem_document_model_t * {
        nanoem_rsize_t num_models;
        nanoem_document_model_t *const *models =
            nanoemDocumentGetAllModelObjects(nanoemMutableDocumentGetOrigin(dst), &num_models);
        std::vector<const nanoem_document_model_t *> founds;
        for (nanoem_rsize_t i = 0; i < num_models; i++) {
            const nanoem_document_model_t *item = models[i];
            if (nanoemUnicodeStringFactoryCompareString(factory,
                    nanoemDocumentModelGetName(item, NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
                    nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_FIRST_ENUM)) == 0) {
                founds.push_back(item);
            }
        }
        switch (founds.size()) {
        case 0:
            return 0;
        case 1:
            return founds.front();
        default:
            for (const auto &item : founds) {
                if (nanoemDocumentModelGetIndex(item) == nanoemDocumentModelGetIndex(model)) {
                    return item;
                }
            }
            return founds.front();
        }
    };
    static bool s_write_all_models = true;
    if (s_write_all_models) {
        nanoem_rsize_t num_models;
        nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(src, &num_models);
        for (nanoem_rsize_t i = 0; i < num_models; i++) {
            const nanoem_document_model_t *model = models[i];
            nanoem_mutable_document_model_t *m = nanoemMutableDocumentModelCreate(dst, &status);
            nanoemMutableDocumentModelSetBlendEnabled(m, nanoemDocumentModelIsBlendEnabled(model));
            nanoemMutableDocumentModelSetDrawOrderIndex(m, nanoemDocumentModelGetDrawOrderIndex(model));
            nanoemMutableDocumentModelSetEdgeWidth(m, nanoemDocumentModelGetEdgeWidth(model));
            nanoemMutableDocumentModelSetLastFrameIndex(m, nanoemDocumentModelGetLastFrameIndex(model));
            nanoemMutableDocumentModelSetName(m, NANOEM_LANGUAGE_TYPE_JAPANESE,
                nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_JAPANESE), &status);
            nanoemMutableDocumentModelSetName(m, NANOEM_LANGUAGE_TYPE_ENGLISH,
                nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_ENGLISH), &status);
            nanoemMutableDocumentModelSetName(m, NANOEM_LANGUAGE_TYPE_SIMPLIFIED_CHINESE,
                nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_SIMPLIFIED_CHINESE), &status);
            nanoemMutableDocumentModelSetName(m, NANOEM_LANGUAGE_TYPE_TRADITIONAL_CHINESE,
                nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_TRADITIONAL_CHINESE), &status);
            nanoemMutableDocumentModelSetPath(m, nanoemDocumentModelGetPath(model), &status);
            nanoemMutableDocumentModelSetTransformOrderIndex(m, nanoemDocumentModelGetTransformOrderIndex(model));
            // bone keyframes
            static bool s_write_all_bone_keyframes = true;
            if (s_write_all_bone_keyframes) {
                nanoem_rsize_t num_keyframes;
                nanoem_document_model_bone_keyframe_t *const *keyframes =
                    nanoemDocumentModelGetAllBoneKeyframeObjects(model, &num_keyframes);
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_bone_keyframe_t *keyframe = keyframes[j];
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                        nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe));
                    if (frame_index != 0)
                        continue;
                    const nanoem_unicode_string_t *name = nanoemDocumentModelBoneKeyframeGetName(keyframe);
                    nanoemMutableDocumentModelRegisterBone(m, name, &status);
                    nanoem_mutable_document_model_bone_keyframe_t *k =
                        nanoemMutableDocumentModelBoneKeyframeCreate(m, name, &status);
                    for (int t = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
                         t < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; t++) {
                        nanoem_motion_bone_keyframe_interpolation_type_t type =
                            (nanoem_motion_bone_keyframe_interpolation_type_t) t;
                        nanoemMutableDocumentModelBoneKeyframeSetInterpolation(
                            k, type, nanoemDocumentModelBoneKeyframeGetInterpolation(keyframe, type));
                    }
                    nanoemMutableDocumentModelBoneKeyframeSetOrientation(
                        k, nanoemDocumentModelBoneKeyframeGetOrientation(keyframe));
                    nanoemMutableDocumentModelBoneKeyframeSetPhysicsSimulationDisabled(
                        k, nanoemDocumentModelBoneKeyframeIsPhysicsSimulationDisabled(keyframe));
                    nanoemMutableDocumentModelBoneKeyframeSetTranslation(
                        k, nanoemDocumentModelBoneKeyframeGetTranslation(keyframe));
                    nanoemMutableDocumentModelAddBoneKeyframeObject(m, k, name, frame_index, &status);
                    nanoemMutableDocumentModelBoneKeyframeDestroy(k);
                    {
                        nanoem_rsize_t length;
                        nanoem_u8_t *model_name, *bone_name;
                        model_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, nanoemDocumentModelGetPath(model), &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        bone_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, name, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        // fprintf(stderr, "BONE:%zu:%s:%s\n", j, model_name, bone_name);
                        model_bone_map[reinterpret_cast<const char *>(model_name)].push_back(
                            reinterpret_cast<const char *>(bone_name));
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
                    }
                }
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_bone_keyframe_t *keyframe = keyframes[j];
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                        nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe));
                    if (frame_index == 0)
                        continue;
                    const nanoem_unicode_string_t *name = nanoemDocumentModelBoneKeyframeGetName(keyframe);
                    nanoem_mutable_document_model_bone_keyframe_t *k =
                        nanoemMutableDocumentModelBoneKeyframeCreate(m, name, &status);
                    for (int t = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
                         t < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; t++) {
                        nanoem_motion_bone_keyframe_interpolation_type_t type =
                            (nanoem_motion_bone_keyframe_interpolation_type_t) t;
                        nanoemMutableDocumentModelBoneKeyframeSetInterpolation(
                            k, type, nanoemDocumentModelBoneKeyframeGetInterpolation(keyframe, type));
                    }
                    nanoemMutableDocumentModelBoneKeyframeSetOrientation(
                        k, nanoemDocumentModelBoneKeyframeGetOrientation(keyframe));
                    nanoemMutableDocumentModelBoneKeyframeSetPhysicsSimulationDisabled(
                        k, nanoemDocumentModelBoneKeyframeIsPhysicsSimulationDisabled(keyframe));
                    nanoemMutableDocumentModelBoneKeyframeSetTranslation(
                        k, nanoemDocumentModelBoneKeyframeGetTranslation(keyframe));
                    nanoemMutableDocumentModelAddBoneKeyframeObject(m, k, name, frame_index, &status);
                    nanoemMutableDocumentModelBoneKeyframeDestroy(k);
                }
            }
            // debug bone keyframes
            {
#if 0
                nanoem_rsize_t num_keyframes;
                nanoem_document_model_bone_keyframe_t *const *keyframes = nanoemDocumentModelGetAllBoneKeyframeObjects(
                    nanoemMutableDocumentModelGetOrigin(m), &num_keyframes);
                fprintf(stderr, "[B] %zu\n", num_keyframes);
#if 0
                for (nanoem_rsize_t j = 0; j < 30; j++) {
                    const nanoem_document_model_bone_keyframe_t *keyframe = keyframes[j];
                    const nanoem_document_base_keyframe_t *current =
                        nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe);
                    int offset = 0;
                    while (1) {
                        const nanoem_document_model_bone_keyframe_t *pk = 0, *nk = 0;
                        int index = nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(current);
                        if (index >= 0) {
                            pk = keyframes[index];
                        }
                        const nanoem_document_base_keyframe_t *prev =
                            nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(pk);
                        index = nanoemDocumentBaseKeyframeGetNextKeyframeIndex(current);
                        if (index >= 0) {
                            nk = keyframes[index];
                        }
                        const nanoem_unicode_string_t *s = nanoemDocumentModelBoneKeyframeGetName(keyframe);
                        nanoem_rsize_t length;
                        nanoem_u8_t *b = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, s, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        const nanoem_document_base_keyframe_t *next =
                            nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(nk);
                        fprintf(stderr, "[%zu:%d:%s]:%d:%d:%d\n", j, offset++, b,
                            nanoemDocumentBaseKeyframeGetFrameIndex(prev),
                            nanoemDocumentBaseKeyframeGetFrameIndex(current),
                            nanoemDocumentBaseKeyframeGetFrameIndex(next));
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, b);
                        if (nk != 0 && nanoemDocumentBaseKeyframeGetFrameIndex(next) != 0 && current != next) {
                            current = next;
                        }
                        else {
                            break;
                        }
                    }
                }
#else
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_bone_keyframe_t *keyframe = keyframes[j];
                    const nanoem_unicode_string_t *s = nanoemDocumentModelBoneKeyframeGetName(keyframe);
                    const nanoem_document_base_keyframe_t *base =
                        nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe);
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(base);
                    nanoem_rsize_t length;
                    nanoem_u8_t *b = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                        factory, s, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                    fprintf(stderr, "[A] %zu:%s:%d %d <- : -> %d\n", j, b, frame_index,
                        nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(base),
                        nanoemDocumentBaseKeyframeGetNextKeyframeIndex(base));
                    nanoemUnicodeStringFactoryDestroyByteArray(factory, b);
                }
#endif
                fflush(stderr);
#endif
            }
            // morph keyframes
            static bool s_write_all_morph_keyframes = true;
            if (s_write_all_morph_keyframes) {
                nanoem_rsize_t num_keyframes;
                nanoem_document_model_morph_keyframe_t *const *keyframes =
                    nanoemDocumentModelGetAllMorphKeyframeObjects(model, &num_keyframes);
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_morph_keyframe_t *keyframe = keyframes[j];
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                        nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe));
                    if (frame_index != 0)
                        continue;
                    const nanoem_unicode_string_t *name = nanoemDocumentModelMorphKeyframeGetName(keyframe);
                    int registered_as = nanoemMutableDocumentModelRegisterMorph(m, name, &status);
                    if (0) { // if (j == 0) {
                        nanoem_unicode_string_t *base_name = nanoemUnicodeStringFactoryCreateString(
                            factory, reinterpret_cast<const nanoem_u8_t *>("base"), 4, nullptr);
                        if (nanoemUnicodeStringFactoryCompareString(factory, name, base_name) != 0) {
                            nanoem_mutable_document_model_morph_keyframe_t *k =
                                nanoemMutableDocumentModelMorphKeyframeCreate(m, name, &status);
                            nanoemMutableDocumentModelAddMorphKeyframeObject(m, k, base_name, 0, &status);
                            nanoemMutableDocumentModelMorphKeyframeDestroy(k);
                            nanoem_rsize_t length;
                            nanoem_u8_t *model_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                                factory, nanoemDocumentModelGetPath(model), &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                            model_morph_map[reinterpret_cast<const char *>(model_name)].push_back("base");
                            nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
                        }
                        nanoemUnicodeStringFactoryDestroyString(factory, base_name);
                    }
                    nanoem_mutable_document_model_morph_keyframe_t *k =
                        nanoemMutableDocumentModelMorphKeyframeCreate(m, name, &status);
                    nanoemMutableDocumentModelMorphKeyframeSetWeight(
                        k, nanoemDocumentModelMorphKeyframeGetWeight(keyframe));
                    nanoemMutableDocumentModelAddMorphKeyframeObject(m, k, name, frame_index, &status);
                    nanoemMutableDocumentModelMorphKeyframeDestroy(k);
                    { // if (i == 2) {
                        nanoem_rsize_t length;
                        nanoem_u8_t *model_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, nanoemDocumentModelGetPath(model), &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        nanoem_u8_t *morph_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, name, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        fprintf(stderr, "MORPH:%zu:%s => %d\n", j, morph_name, registered_as);
                        model_morph_map[reinterpret_cast<const char *>(model_name)].push_back(
                            reinterpret_cast<const char *>(morph_name));
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, morph_name);
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
                    }
                }
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_morph_keyframe_t *keyframe = keyframes[j];
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                        nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe));
                    if (frame_index == 0)
                        continue;
                    const nanoem_unicode_string_t *name = nanoemDocumentModelMorphKeyframeGetName(keyframe);
                    nanoem_mutable_document_model_morph_keyframe_t *k =
                        nanoemMutableDocumentModelMorphKeyframeCreate(m, name, &status);
                    nanoemMutableDocumentModelMorphKeyframeSetWeight(
                        k, nanoemDocumentModelMorphKeyframeGetWeight(keyframe));
                    nanoemMutableDocumentModelAddMorphKeyframeObject(m, k, name, frame_index, &status);
                    nanoemMutableDocumentModelMorphKeyframeDestroy(k);
                }
            }
            // debug morph keyframes
            if (s_write_all_morph_keyframes) {
#if 0
                nanoem_rsize_t num_keyframes;
                nanoem_document_model_morph_keyframe_t *const *keyframes = nanoemDocumentModelGetAllMorphKeyframeObjects(
                    nanoemMutableDocumentModelGetOrigin(m), &num_keyframes);
                fprintf(stderr, "[B] %zu\n", num_keyframes);
#if 0
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_morph_keyframe_t *keyframe = keyframes[j];
                    const nanoem_document_base_keyframe_t *current =
                        nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe);
                    int offset = 0;
                    while (1) {
                        const nanoem_document_model_morph_keyframe_t *pk = 0, *nk = 0;
                        int index = nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(current);
                        if (index >= 0) {
                            pk = keyframes[index];
                        }
                        const nanoem_document_base_keyframe_t *prev =
                            nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(pk);
                        index = nanoemDocumentBaseKeyframeGetNextKeyframeIndex(current);
                        if (index >= 0) {
                            nk = keyframes[index];
                        }
                        const nanoem_unicode_string_t *s = nanoemDocumentModelMorphKeyframeGetName(keyframe);
                        nanoem_rsize_t length;
                        nanoem_u8_t *b = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, s, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        const nanoem_document_base_keyframe_t *next =
                            nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(nk);
                        fprintf(stderr, "[%zu:%d:%s]:%d:%d:%d\n", j, offset++, b,
                            nanoemDocumentBaseKeyframeGetFrameIndex(prev),
                            nanoemDocumentBaseKeyframeGetFrameIndex(current),
                            nanoemDocumentBaseKeyframeGetFrameIndex(next));
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, b);
                        if (nk != 0 && nanoemDocumentBaseKeyframeGetFrameIndex(next) != 0 && current != next) {
                            current = next;
                        }
                        else {
                            break;
                        }
                    }
                }
#else
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_morph_keyframe_t *keyframe = keyframes[j];
                    const nanoem_unicode_string_t *s = nanoemDocumentModelMorphKeyframeGetName(keyframe);
                    const nanoem_document_base_keyframe_t *base =
                        nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe);
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(base);
                    nanoem_rsize_t length;
                    nanoem_u8_t *b = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                        factory, s, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                    fprintf(stderr, "[A] %zu:%s:%d %d <- : -> %d\n", j, b, frame_index,
                        nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(base),
                        nanoemDocumentBaseKeyframeGetNextKeyframeIndex(base));
                    nanoemUnicodeStringFactoryDestroyByteArray(factory, b);
                }
#endif
                fflush(stderr);
#endif
            }
            // model bone states
            if (true) {
                nanoem_rsize_t num_states;
                nanoem_document_model_bone_state_t *const *states =
                    nanoemDocumentModelGetAllBoneStateObjects(model, &num_states);
                for (nanoem_rsize_t j = 0; j < num_states; j++) {
                    const nanoem_document_model_bone_state_t *state = states[j];
                    nanoem_mutable_document_model_bone_state_t *s =
                        nanoemMutableDocumentModelBoneStateCreate(m, &status);
                    nanoemMutableDocumentModelBoneStateSetOrientation(
                        s, nanoemDocumentModelBoneStateGetOrientation(state));
                    nanoemMutableDocumentModelBoneStateSetTranslation(
                        s, nanoemDocumentModelBoneStateGetTranslation(state));
                    nanoemMutableDocumentModelBoneStateSetPhysicsSimulationDisabled(
                        s, nanoemDocumentModelBoneStateIsPhysicsSimulationDisabled(state));
                    nanoemMutableDocumentModelAddModelBoneStateObject(
                        m, s, nanoemDocumentModelBoneStateGetName(state), &status);
                    nanoemMutableDocumentModelBoneStateDestroy(s);
                }
            }
            // model constraint states
            if (true) {
                nanoem_rsize_t num_states;
                nanoem_document_model_constraint_state_t *const *states =
                    nanoemDocumentModelGetAllConstraintStateObjects(model, &num_states);
                for (nanoem_rsize_t j = 0; j < num_states; j++) {
                    const nanoem_document_model_constraint_state_t *state = states[j];
                    const nanoem_unicode_string_t *name = nanoemDocumentModelConstraintStateGetName(state);
                    nanoem_mutable_document_model_constraint_state_t *s =
                        nanoemMutableDocumentModelConstraintStateCreate(m, &status);
                    {
                        nanoem_rsize_t length;
                        nanoem_u8_t *model_name, *constraint_name;
                        model_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, nanoemDocumentModelGetPath(model), &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        constraint_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(
                            factory, name, &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                        // fprintf(stderr, "BONE:%zu:%s:%s\n", j, model_name, bone_name);
                        model_constraint_map[reinterpret_cast<const char *>(model_name)].push_back(
                            reinterpret_cast<const char *>(constraint_name));
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, constraint_name);
                        nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
                    }
                    nanoemMutableDocumentModelAddModelConstraintStateObject(m, s, name, &status);
                    nanoemMutableDocumentModelConstraintStateDestroy(s);
                }
            }
            // model morph states
            if (true) {
                nanoem_rsize_t num_states;
                nanoem_document_model_morph_state_t *const *states =
                    nanoemDocumentModelGetAllMorphStateObjects(model, &num_states);
                for (nanoem_rsize_t j = 0; j < num_states; j++) {
                    const nanoem_document_model_morph_state_t *state = states[j];
                    const nanoem_unicode_string_t *name = nanoemDocumentModelMorphStateGetName(state);
                    nanoem_mutable_document_model_morph_state_t *s =
                        nanoemMutableDocumentModelMorphStateCreate(m, &status);
                    nanoemMutableDocumentModelMorphStateSetWeight(s, nanoemDocumentModelMorphStateGetWeight(state));
                    nanoemMutableDocumentModelAddModelMorphStateObject(m, s, name, &status);
                    nanoemMutableDocumentModelMorphStateDestroy(s);
                }
            }
            // model keyframes
            if (true) {
                nanoem_rsize_t num_keyframes;
                nanoem_document_model_keyframe_t *const *keyframes =
                    nanoemDocumentModelGetAllModelKeyframeObjects(model, &num_keyframes);
                for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                    const nanoem_document_model_keyframe_t *keyframe = keyframes[j];
                    nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                        nanoemDocumentModelKeyframeGetBaseKeyframeObject(keyframe));
                    nanoem_mutable_document_model_keyframe_t *k = nanoemMutableDocumentModelKeyframeCreate(m, &status);
                    nanoemMutableDocumentModelKeyframeSetVisible(k, nanoemDocumentModelKeyframeIsVisible(keyframe));
                    nanoem_rsize_t num_states;
                    nanoem_document_model_constraint_state_t *const *states =
                        nanoemDocumentModelGetAllConstraintStateObjects(model, &num_states);
                    for (nanoem_rsize_t l = 0; l < num_states; l++) {
                        const nanoem_document_model_constraint_state_t *state = states[l];
                        nanoemMutableDocumentModelKeyframeSetConstraintEnabled(
                            k, nanoemDocumentModelConstraintStateGetName(state), true, &status);
                    }
                    nanoemMutableDocumentModelAddModelKeyframeObject(m, k, frame_index, &status);
                    nanoemMutableDocumentModelKeyframeDestroy(k);
                }
            }
            nanoemMutableDocumentInsertModelObject(dst, m, -1, &status);
            nanoemMutableDocumentModelDestroy(m);
        }
        // model bindings
        static bool s_write_all_model_bindings = false;
        if (s_write_all_model_bindings) {
            nanoem_document_model_t *const *mutable_models =
                nanoemDocumentGetAllModelObjects(nanoemMutableDocumentGetOrigin(dst), &num_models);
            for (nanoem_rsize_t i = 0; i < num_models; i++) {
                const nanoem_document_model_t *model = models[i];
                nanoem_mutable_document_model_t *m =
                    nanoemMutableDocumentModelCreateAsReference(dst, mutable_models[i], &status);
                nanoemMutableDocumentModelSetSelectedBoneName(
                    m, nanoemDocumentModelGetSelectedBoneName(model), &status);
                for (int j = NANOEM_MODEL_MORPH_CATEGORY_EYE; j < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; j++) {
                    nanoem_model_morph_category_t category = (nanoem_model_morph_category_t) j;
                    nanoemMutableDocumentModelSetSelectedMorphName(
                        m, category, nanoemDocumentModelGetSelectedMorphName(model, category), &status);
                }
                {
                    nanoem_rsize_t num_states;
                    nanoem_document_model_outside_parent_state_t *const *states =
                        nanoemDocumentModelGetAllOutsideParentStateObjects(model, &num_states);
                    for (nanoem_rsize_t j = 0; j < num_states; j++) {
                        const nanoem_document_model_outside_parent_state_t *state = states[j];
                        nanoem_mutable_document_model_outside_parent_state_t *s =
                            nanoemMutableDocumentModelOutsideParentStateCreate(m, &status);
                        nanoemMutableDocumentModelOutsideParentStateSetBeginFrameIndex(
                            s, nanoemDocumentModelOutsideParentStateGetBeginFrameIndex(state));
                        nanoemMutableDocumentModelOutsideParentStateSetEndFrameIndex(
                            s, nanoemDocumentModelOutsideParentStateGetEndFrameIndex(state));
                        nanoemMutableDocumentModelOutsideParentStateSetTargetBoneName(
                            s, nanoemDocumentModelOutsideParentStateGetTargetBoneName(state));
                        nanoemMutableDocumentModelOutsideParentStateSetTargetModelObject(
                            s, nanoemDocumentModelOutsideParentStateGetTargetModelObject(state));
                        nanoemMutableDocumentModelInsertModelOutsideParentStateObject(m, s, -1, &status);
                        nanoemMutableDocumentModelOutsideParentStateDestroy(s);
                    }
                }
                nanoemMutableDocumentModelDestroy(m);
            }
            nanoemMutableDocumentSetAccessoryIndexAfterModel(dst, nanoemDocumentGetAccessoryIndexAfterModel(src));
            nanoemMutableDocumentSetSelectedAccessoryIndex(dst, nanoemDocumentGetSelectedAccessoryIndex(src));
            nanoemMutableDocumentSetSelectedModelIndex(dst, nanoemDocumentGetSelectedModelIndex(src));
        }
    }
    if (s_write_all_camera_keyframes) {
        nanoem_rsize_t num_keyframes;
        nanoem_document_camera_keyframe_t *const *keyframes =
            nanoemDocumentCameraGetAllCameraKeyframeObjects(camera, &num_keyframes);
        for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
            const nanoem_document_camera_keyframe_t *keyframe = keyframes[j];
            nanoem_mutable_document_camera_keyframe_t *k = nanoemMutableDocumentCameraKeyframeCreate(c, &status);
            nanoem_frame_index_t frame_index =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentCameraKeyframeGetBaseKeyframeObject(keyframe));
            nanoemMutableDocumentCameraKeyframeSetAngle(k, nanoemDocumentCameraKeyframeGetAngle(keyframe));
            nanoemMutableDocumentCameraKeyframeSetDistance(k, nanoemDocumentCameraKeyframeGetDistance(keyframe));
            nanoemMutableDocumentCameraKeyframeSetFov(k, nanoemDocumentCameraKeyframeGetFov(keyframe));
            nanoemMutableDocumentCameraKeyframeSetLookAt(k, nanoemDocumentCameraKeyframeGetLookAt(keyframe));
            nanoemMutableDocumentCameraKeyframeSetParentModelObject(
                k, find_model(nanoemDocumentCameraKeyframeGetParentModelObject(keyframe)));
            nanoemMutableDocumentCameraKeyframeSetParentModelBoneName(
                k, nanoemDocumentCameraKeyframeGetParentModelBoneName(keyframe), &status);
            nanoemMutableDocumentCameraKeyframeSetPerspectiveView(
                k, nanoemDocumentCameraKeyframeIsPerspectiveView(keyframe));
            for (int t = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
                 t < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; t++) {
                nanoem_motion_camera_keyframe_interpolation_type_t type =
                    (nanoem_motion_camera_keyframe_interpolation_type_t) t;
                nanoemMutableDocumentCameraKeyframeSetInterpolation(
                    k, type, nanoemDocumentCameraKeyframeGetInterpolation(keyframe, type));
            }
            nanoemMutableDocumentCameraAddCameraKeyframeObject(c, k, frame_index, &status);
            nanoemMutableDocumentCameraKeyframeDestroy(k);
        }
#if 0
        nanoem_rsize_t num_all_camera_keyframes;
        nanoem_document_camera_keyframe_t *const *all_camera_keyframes =
            nanoemDocumentCameraGetAllCameraKeyframeObjects(
                nanoemMutableDocumentCameraGetOrigin(c), &num_all_camera_keyframes);
        fprintf(stderr, "[C] %zu\n", num_all_camera_keyframes);
#if 1
        const nanoem_document_base_keyframe_t *current =
            nanoemDocumentCameraKeyframeGetBaseKeyframeObject(all_camera_keyframes[0]);
        int offset = 0;
        while (1) {
            const nanoem_document_camera_keyframe_t *pk = 0, *nk = 0;
            int index = nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(current);
            if (index >= 0) {
                pk = all_camera_keyframes[index];
            }
            const nanoem_document_base_keyframe_t *prev = nanoemDocumentCameraKeyframeGetBaseKeyframeObject(pk);
            index = nanoemDocumentBaseKeyframeGetNextKeyframeIndex(current);
            if (index >= 0) {
                nk = all_camera_keyframes[index];
            }
            const nanoem_document_base_keyframe_t *next = nanoemDocumentCameraKeyframeGetBaseKeyframeObject(nk);
            fprintf(stderr, "%d:%d:%d:%d\n", offset++, nanoemDocumentBaseKeyframeGetFrameIndex(prev),
                nanoemDocumentBaseKeyframeGetFrameIndex(current), nanoemDocumentBaseKeyframeGetFrameIndex(next));
            if (nk != 0 && nanoemDocumentBaseKeyframeGetFrameIndex(next) != 0) {
                current = next;
            }
            else {
                break;
            }
        }
#else
        for (nanoem_rsize_t i = 0; i < num_all_camera_keyframes; i++) {
            const nanoem_document_camera_keyframe_t *k = all_camera_keyframes[i];
            const nanoem_document_base_keyframe_t *b = nanoemDocumentCameraKeyframeGetBaseKeyframeObject(k);
            fprintf(stderr, "[D] %zu:%d:%d:%d\n", i, nanoemDocumentBaseKeyframeGetFrameIndex(b),
                nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(b),
                nanoemDocumentBaseKeyframeGetNextKeyframeIndex(b));
        }
#endif
#endif
    }
    nanoemMutableDocumentSetCameraObject(dst, c);
    nanoemMutableDocumentCameraDestroy(c);
    const nanoem_document_gravity_t *gravity = nanoemDocumentGetGravityObject(src);
    nanoem_mutable_document_gravity_t *g = nanoemMutableDocumentGravityCreate(dst, &status);
    nanoemMutableDocumentGravitySetAcceleration(g, nanoemDocumentGravityGetAcceleration(gravity));
    nanoemMutableDocumentGravitySetDirection(g, nanoemDocumentGravityGetDirection(gravity));
    nanoemMutableDocumentGravitySetNoise(g, nanoemDocumentGravityGetNoise(gravity));
    nanoemMutableDocumentGravitySetNoiseEnabled(g, nanoemDocumentGravityIsNoiseEnabled(gravity));
    static bool s_write_all_gravity_keyframes = true;
    if (s_write_all_gravity_keyframes) {
        nanoem_rsize_t num_keyframes;
        nanoem_document_gravity_keyframe_t *const *keyframes =
            nanoemDocumentGravityGetAllGravityKeyframeObjects(gravity, &num_keyframes);
        for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
            const nanoem_document_gravity_keyframe_t *keyframe = keyframes[j];
            nanoem_mutable_document_gravity_keyframe_t *k = nanoemMutableDocumentGravityKeyframeCreate(g, &status);
            nanoem_frame_index_t frame_index =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentGravityKeyframeGetBaseKeyframeObject(keyframe));
            nanoemMutableDocumentGravityKeyframeSetAcceleration(
                k, nanoemDocumentGravityKeyframeGetAcceleration(keyframe));
            nanoemMutableDocumentGravityKeyframeSetDirection(k, nanoemDocumentGravityKeyframeGetDirection(keyframe));
            nanoemMutableDocumentGravityKeyframeSetNoise(k, nanoemDocumentGravityKeyframeGetNoise(keyframe));
            nanoemMutableDocumentGravityKeyframeSetNoiseEnabled(
                k, nanoemDocumentGravityKeyframeIsNoiseEnabled(keyframe));
            nanoemMutableDocumentGravityAddGravityKeyframeObject(g, k, frame_index, &status);
            nanoemMutableDocumentGravityKeyframeDestroy(k);
        }
    }
    nanoemMutableDocumentSetGravityObject(dst, g);
    nanoemMutableDocumentGravityDestroy(g);
    const nanoem_document_light_t *light = nanoemDocumentGetLightObject(src);
    nanoem_mutable_document_light_t *l = nanoemMutableDocumentLightCreate(dst, &status);
    nanoemMutableDocumentLightSetColor(l, nanoemDocumentLightGetColor(light));
    nanoemMutableDocumentLightSetDirection(l, nanoemDocumentLightGetDirection(light));
    static bool s_write_all_light_keyframes = true;
    if (s_write_all_light_keyframes) {
        nanoem_rsize_t num_keyframes;
        nanoem_document_light_keyframe_t *const *keyframes =
            nanoemDocumentLightGetAllLightKeyframeObjects(light, &num_keyframes);
        for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
            const nanoem_document_light_keyframe_t *keyframe = keyframes[j];
            nanoem_mutable_document_light_keyframe_t *k = nanoemMutableDocumentLightKeyframeCreate(l, &status);
            nanoem_frame_index_t frame_index =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentLightKeyframeGetBaseKeyframeObject(keyframe));
            nanoemMutableDocumentLightKeyframeSetColor(k, nanoemDocumentLightKeyframeGetColor(keyframe));
            nanoemMutableDocumentLightKeyframeSetDirection(k, nanoemDocumentLightKeyframeGetDirection(keyframe));
            nanoemMutableDocumentLightAddLightKeyframeObject(l, k, frame_index, &status);
            nanoemMutableDocumentLightKeyframeDestroy(k);
        }
    }
    nanoemMutableDocumentSetLightObject(dst, l);
    nanoemMutableDocumentLightDestroy(l);
    const nanoem_document_self_shadow_t *self_shadow = nanoemDocumentGetSelfShadowObject(src);
    nanoem_mutable_document_self_shadow_t *s = nanoemMutableDocumentSelfShadowCreate(dst, &status);
    nanoemMutableDocumentSelfShadowSetDistance(s, nanoemDocumentSelfShadowGetDistance(self_shadow));
    nanoemMutableDocumentSelfShadowSetEnabled(s, nanoemDocumentSelfShadowIsEnabled(self_shadow));
    static bool s_write_all_self_shadow_keyframes = true;
    if (s_write_all_self_shadow_keyframes) {
        nanoem_rsize_t num_keyframes;
        nanoem_document_self_shadow_keyframe_t *const *keyframes =
            nanoemDocumentSelfShadowGetAllSelfShadowKeyframeObjects(self_shadow, &num_keyframes);
        for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
            const nanoem_document_self_shadow_keyframe_t *keyframe = keyframes[j];
            nanoem_mutable_document_self_shadow_keyframe_t *k =
                nanoemMutableDocumentSelfShadowKeyframeCreate(s, &status);
            nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(keyframe));
            nanoemMutableDocumentSelfShadowKeyframeSetDistance(
                k, nanoemDocumentSelfShadowKeyframeGetDistance(keyframe));
            nanoemMutableDocumentSelfShadowKeyframeSetMode(k, nanoemDocumentSelfShadowKeyframeGetMode(keyframe));
            nanoemMutableDocumentSelfShadowAddSelfShadowKeyframeObject(s, k, frame_index, &status);
            nanoemMutableDocumentSelfShadowKeyframeDestroy(k);
        }
    }
    nanoemMutableDocumentSetSelfShadowObject(dst, s);
    nanoemMutableDocumentSelfShadowDestroy(s);
    static bool s_write_all_accessories = true;
    if (s_write_all_accessories) {
        nanoem_rsize_t num_accessories;
        nanoem_document_accessory_t *const *accessories = nanoemDocumentGetAllAccessoryObjects(src, &num_accessories);
        for (nanoem_rsize_t i = 0; i < num_accessories; i++) {
            const nanoem_document_accessory_t *accessory = accessories[i];
            nanoem_mutable_document_accessory_t *a = nanoemMutableDocumentAccessoryCreate(dst, &status);
            nanoemMutableDocumentAccessorySetAddBlendEnabled(a, nanoemDocumentAccessoryIsAddBlendEnabled(accessory));
            nanoemMutableDocumentAccessorySetDrawOrderIndex(a, nanoemDocumentAccessoryGetDrawOrderIndex(accessory));
            nanoemMutableDocumentAccessorySetName(a, nanoemDocumentAccessoryGetName(accessory), &status);
            nanoemMutableDocumentAccessorySetOpacity(a, nanoemDocumentAccessoryGetOpacity(accessory));
            nanoemMutableDocumentAccessorySetOrientation(a, nanoemDocumentAccessoryGetOrientation(accessory));
            nanoemMutableDocumentAccessorySetParentModelObject(
                a, find_model(nanoemDocumentAccessoryGetParentModelObject(accessory)));
            nanoemMutableDocumentAccessorySetParentModelBoneName(
                a, nanoemDocumentAccessoryGetParentModelBoneName(accessory), &status);
            nanoemMutableDocumentAccessorySetPath(a, nanoemDocumentAccessoryGetPath(accessory), &status);
            nanoemMutableDocumentAccessorySetScaleFactor(a, nanoemDocumentAccessoryGetScaleFactor(accessory));
            nanoemMutableDocumentAccessorySetShadowEnabled(a, nanoemDocumentAccessoryIsShadowEnabled(accessory));
            nanoemMutableDocumentAccessorySetTranslation(a, nanoemDocumentAccessoryGetTranslation(accessory));
            nanoemMutableDocumentAccessorySetVisible(a, nanoemDocumentAccessoryIsVisible(accessory));
            nanoemMutableDocumentInsertAccessoryObject(dst, a, -1, &status);
            nanoemMutableDocumentAccessoryDestroy(a);
        }
        nanoem_rsize_t num_new_accessories;
        nanoem_document_accessory_t *const *new_accessories =
            nanoemDocumentGetAllAccessoryObjects(nanoemMutableDocumentGetOrigin(dst), &num_new_accessories);
        for (nanoem_rsize_t i = 0; i < num_accessories; i++) {
            const nanoem_document_accessory_t *accessory = accessories[i];
            nanoem_mutable_document_accessory_t *a =
                nanoemMutableDocumentAccessoryCreateAsReference(dst, new_accessories[i], &status);
            nanoem_rsize_t num_keyframes;
            nanoem_document_accessory_keyframe_t *const *keyframes =
                nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(accessory, &num_keyframes);
            for (nanoem_rsize_t j = 0; j < num_keyframes; j++) {
                const nanoem_document_accessory_keyframe_t *keyframe = keyframes[j];
                nanoem_mutable_document_accessory_keyframe_t *k =
                    nanoemMutableDocumentAccessoryKeyframeCreate(a, &status);
                nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                    nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(keyframe));
                nanoemMutableDocumentAccessoryKeyframeSetOpacity(
                    k, nanoemDocumentAccessoryKeyframeGetOpacity(keyframe));
                nanoemMutableDocumentAccessoryKeyframeSetOrientation(
                    k, nanoemDocumentAccessoryKeyframeGetOrientation(keyframe));
                nanoemMutableDocumentAccessoryKeyframeSetParentModelObject(
                    k, find_model(nanoemDocumentAccessoryKeyframeGetParentModelObject(keyframe)));
                nanoemMutableDocumentAccessoryKeyframeSetParentModelBoneName(
                    k, nanoemDocumentAccessoryKeyframeGetParentModelBoneName(keyframe), &status);
                nanoemMutableDocumentAccessoryKeyframeSetScaleFactor(
                    k, nanoemDocumentAccessoryKeyframeGetScaleFactor(keyframe));
                nanoemMutableDocumentAccessoryKeyframeSetShadowEnabled(
                    k, nanoemDocumentAccessoryKeyframeIsShadowEnabled(keyframe));
                nanoemMutableDocumentAccessoryKeyframeSetTranslation(
                    k, nanoemDocumentAccessoryKeyframeGetTranslation(keyframe));
                nanoemMutableDocumentAccessoryKeyframeSetVisible(k, nanoemDocumentAccessoryKeyframeIsVisible(keyframe));
                nanoemMutableDocumentAccessoryAddAccessoryKeyframeObject(a, k, frame_index, &status);
                nanoemMutableDocumentAccessoryKeyframeDestroy(k);
            }
        }
    }
    auto loadModel = [factory](const char *type, std::string path, nanoem_model_t *&model) {
        const char *path_ptr = 0;
#ifndef _WIN32
        // std::replace(path.begin(), path.end(), '\\', '/');
        auto pos = path.find("UserFile");
        path_ptr = &path[pos];
#else
        std::string s2;
        nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateString(
            factory, reinterpret_cast<const nanoem_u8_t *>(path.c_str()), path.size(), nullptr);
        nanoem_rsize_t length;
        nanoem_u8_t *bytes =
            nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, s, &length, NANOEM_CODEC_TYPE_SJIS, nullptr);
        s2 = std::string(reinterpret_cast<const char *>(bytes), length);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
        nanoemUnicodeStringFactoryDestroyString(factory, s);
        path_ptr = s2.c_str();
#endif
        fprintf(stderr, "[%s:%s]\n", type, path_ptr);
        nanoem_bool_t result = false;
        if (FILE *fp = fopen(path_ptr, "rb")) {
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            nanoem_u8_t *data = new nanoem_u8_t[size];
            fread(data, size, 1, fp);
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
            model = nanoemModelCreate(factory, &status);
            nanoemModelLoadFromBuffer(model, buffer, &status);
            nanoemBufferDestroy(buffer);
            delete[] data;
            fclose(fp);
        }
        return result;
    };
#if 0
    for (const auto &it : model_bone_map) {
        nanoem_model_t *model;
        if (loadModel("BONE", it.first, model)) {
            nanoem_rsize_t num_bones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model, &num_bones);
            std::vector<std::string> bone_set;
            for (nanoem_rsize_t j = 0; j < num_bones; j++) {
                const nanoem_model_bone_t *bone = bones[j];
                nanoem_rsize_t length;
                nanoem_u8_t *bone_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
                    nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                bone_set.push_back(reinterpret_cast<const char *>(bone_name));
                nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
            }
            size_t offset = 0;
            for (const auto &it2 : it.second) {
                if (it2 != bone_set[offset++]) {
                    fprintf(stderr, "%zu:%s\n", offset, it2.c_str());
                }
            }
        }
        nanoemModelDestroy(model);
    }
    for (const auto &it : model_morph_map) {
        nanoem_model_t *model;
        if (loadModel("MORPH", it.first, model)) {
            nanoem_rsize_t num_morphs;
            nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model, &num_morphs);
            std::vector<std::string> morph_set;
            for (nanoem_rsize_t j = 0; j < num_morphs; j++) {
                const nanoem_model_morph_t *morph = morphs[j];
                nanoem_rsize_t length;
                nanoem_u8_t *morph_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
                    nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, NANOEM_CODEC_TYPE_UTF8,
                    nullptr);
                morph_set.push_back(reinterpret_cast<const char *>(morph_name));
                nanoemUnicodeStringFactoryDestroyByteArray(factory, morph_name);
            }
            size_t offset = 0;
            for (const auto &it2 : it.second) {
                if (it2 != morph_set[offset++]) {
                    fprintf(stderr, "%zu:%s\n", offset, it2.c_str());
                }
            }
        }
        nanoemModelDestroy(model);
    }
#endif
    for (const auto &it : model_constraint_map) {
        nanoem_model_t *model = nullptr;
        if (loadModel("CONSTRAINT", it.first, model)) {
            nanoem_rsize_t num_constraints;
            nanoem_model_constraint_t *const *constraints = nanoemModelGetAllConstraintObjects(model, &num_constraints);
            std::vector<std::string> constraint_set;
            for (nanoem_rsize_t j = 0; j < num_constraints; j++) {
                const nanoem_model_constraint_t *constraint = constraints[j];
                nanoem_rsize_t length;
                nanoem_u8_t *constraint_name = nanoemUnicodeStringFactoryGetByteArrayEncoding(factory,
                    nanoemModelBoneGetName(
                        nanoemModelConstraintGetTargetBoneObject(constraint), NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
                    &length, NANOEM_CODEC_TYPE_UTF8, nullptr);
                constraint_set.push_back(reinterpret_cast<const char *>(constraint_name));
                nanoemUnicodeStringFactoryDestroyByteArray(factory, constraint_name);
            }
            size_t offset = 0;
            for (const auto &it2 : it.second) {
                if (it2 != constraint_set[offset++]) {
                    fprintf(stderr, "%zu:%s\n", offset, it2.c_str());
                }
            }
        }
        nanoemModelDestroy(model);
    }
}

struct document_compare_t {
    std::vector<std::vector<std::string>> all_model_bones;
    std::vector<std::vector<std::string>> all_model_morphs;
    std::vector<std::vector<std::pair<std::string, nanoem_frame_index_t>>> all_model_bone_keyframes;
    std::vector<std::vector<std::pair<std::string, nanoem_frame_index_t>>> all_model_morph_keyframes;
    static void
    getAllStringList(nanoem_unicode_string_factory_t *factory, nanoem_unicode_string_t *const *names,
        nanoem_rsize_t size, std::vector<std::string> &sl)
    {
        for (nanoem_rsize_t i = 0; i < size; i++) {
            sl.push_back(getString(factory, names[i]));
        }
    }
    static void
    getAllBoneKeyframeList(nanoem_unicode_string_factory_t *factory,
        nanoem_document_model_bone_keyframe_t *const *keyframes, nanoem_rsize_t size,
        std::vector<std::pair<std::string, nanoem_frame_index_t>> &sl)
    {
        for (nanoem_rsize_t i = 0; i < size; i++) {
            const nanoem_document_model_bone_keyframe_t *keyframe = keyframes[i];
            nanoem_frame_index_t frame_index =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe));
            const nanoem_unicode_string_t *name = nanoemDocumentModelBoneKeyframeGetName(keyframe);
            sl.push_back(std::make_pair(getString(factory, name), frame_index));
        }
    }
    static void
    getAllMorphKeyframeList(nanoem_unicode_string_factory_t *factory,
        nanoem_document_model_morph_keyframe_t *const *keyframes, nanoem_rsize_t size,
        std::vector<std::pair<std::string, nanoem_frame_index_t>> &sl)
    {
        for (nanoem_rsize_t i = 0; i < size; i++) {
            const nanoem_document_model_morph_keyframe_t *keyframe = keyframes[i];
            nanoem_frame_index_t frame_index = nanoemDocumentBaseKeyframeGetFrameIndex(
                nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe));
            const nanoem_unicode_string_t *name = nanoemDocumentModelMorphKeyframeGetName(keyframe);
            sl.push_back(std::make_pair(getString(factory, name), frame_index));
        }
    }
    void
    take(nanoem_unicode_string_factory_t *factory, const nanoem_document_t *document)
    {
        nanoem_rsize_t num_models;
        nanoem_document_model_t *const *all_models = nanoemDocumentGetAllModelObjects(document, &num_models);
        all_model_bones.resize(num_models);
        all_model_morphs.resize(num_models);
        all_model_bone_keyframes.resize(num_models);
        all_model_morph_keyframes.resize(num_models);
        for (nanoem_rsize_t i = 0; i < num_models; i++) {
            const nanoem_document_model_t *model = all_models[i];
            const std::string &model_name =
                getString(factory, nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_JAPANESE));
            auto &all_bones = all_model_bones[i];
            auto &all_morphs = all_model_morphs[i];
            auto &all_bone_keyframes = all_model_bone_keyframes[i];
            auto &all_morph_keyframes = all_model_morph_keyframes[i];
            nanoem_rsize_t num_bones;
            nanoem_unicode_string_t *const *bone_names = nanoemDocumentModelGetAllBoneNameObjects(model, &num_bones);
            getAllStringList(factory, bone_names, num_bones, all_bones);
            nanoem_rsize_t num_bone_keyframes;
            nanoem_document_model_bone_keyframe_t *const *bone_keyframes =
                nanoemDocumentModelGetAllBoneKeyframeObjects(model, &num_bone_keyframes);
            getAllBoneKeyframeList(factory, bone_keyframes, num_bone_keyframes, all_bone_keyframes);
            nanoem_rsize_t num_morphs;
            nanoem_unicode_string_t *const *morph_names = nanoemDocumentModelGetAllMorphNameObjects(model, &num_morphs);
            getAllStringList(factory, morph_names, num_morphs, all_morphs);
            nanoem_rsize_t num_morph_keyframes;
            nanoem_document_model_morph_keyframe_t *const *morph_keyframes =
                nanoemDocumentModelGetAllMorphKeyframeObjects(model, &num_morph_keyframes);
            getAllMorphKeyframeList(factory, morph_keyframes, num_morph_keyframes, all_morph_keyframes);
        }
    }
};

static void
compareDocument(nanoem_unicode_string_factory_t *factory, const nanoem_document_t *left, const nanoem_document_t *right)
{
    document_compare_t lv, rv;
    lv.take(factory, left);
    rv.take(factory, right);
}

int
main(int argc, char **argv)
{
    nanoem_u8_t path[255] = { 0 };
    const char *filename = argc > 1 ? argv[1] : "test.pmm";
    strcpy(reinterpret_cast<char *>(path), filename);
    replacePathSeparator(path);
    if (FILE *fp = fopen(filename, "rb")) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        nanoem_u8_t *data = new nanoem_u8_t[size];
        fread(data, size, 1, fp);
        fclose(fp);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
        nanoem_document_t *document = nanoemDocumentCreate(factory, &status);
        nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
        nanoemDocumentSetParseModelCallback(document, callback);
        nanoemDocumentSetParseModelCallbackUserData(document, path);
        nanoemDocumentLoadFromBuffer(document, buffer, &status);
        if (status == NANOEM_STATUS_SUCCESS) {
            static bool s_get_all_bone_keyframe_names = false;
            if (s_get_all_bone_keyframe_names) {
                nanoem_rsize_t num_objects;
                nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(document, &num_objects);
                nanoem_document_model_bone_keyframe_t *const *keyframes =
                    nanoemDocumentModelGetAllBoneKeyframeObjects(models[0], &num_objects);
                if (num_objects > 0) {
                    nanoemDocumentModelBoneKeyframeGetName(keyframes[0]);
                }
            }
            static bool s_write_document = false;
            if (s_write_document) {
                nanoem_mutable_document_t *new_document = nanoemMutableDocumentCreateAsReference(document, &status);
                nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(&status);
                nanoem_status_t status;
                static bool s_enable_malloc_debug = false;
                if (s_enable_malloc_debug) {
                    nanoem_global_allocator_t allocator = {};
                    allocator.malloc = [](void *, nanoem_rsize_t size, const char *file, int line) {
                        fprintf(stdout, "[malloc] %ld bytes at %s:%d\n", size, file, line);
                        return ::malloc(size);
                    };
                    allocator.calloc = [](void *, nanoem_rsize_t count, nanoem_rsize_t size, const char *file,
                                           int line) {
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
                    nanoemMutableDocumentSaveToBuffer(new_document, mutable_buffer, &status);
                    nanoemGlobalSetCustomAllocator(0);
                }
                else {
                    nanoemMutableDocumentSaveToBuffer(new_document, mutable_buffer, &status);
                }
                nanoem_buffer_t *new_buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
                fp = fopen("output.pmm", "wb");
                fwrite(nanoemBufferGetDataPtr(new_buffer), nanoemBufferGetLength(new_buffer), 1, fp);
                fclose(fp);
                {
                    nanoem_status_t seekStatus = NANOEM_STATUS_SUCCESS, test_status;
                    nanoem_document_t *test_document = nanoemDocumentCreate(factory, &status);
                    nanoemBufferSeek(new_buffer, 0, &seekStatus);
                    fprintf(stdout, "[W] status = %d, size = %ld, origin = %ld\n", status,
                        nanoemBufferGetLength(new_buffer), nanoemBufferGetLength(buffer));
                    nanoemDocumentLoadFromBuffer(test_document, new_buffer, &test_status);
                    nanoemDocumentDestroy(test_document);
                    fprintf(stdout, "[R] status = %d, offset = %ld\n", test_status, nanoemBufferGetOffset(new_buffer));
                }
                nanoemBufferDestroy(new_buffer);
                nanoemMutableDocumentDestroy(new_document);
                nanoemMutableBufferDestroy(mutable_buffer);
            }
            static bool s_copy_document = true;
            if (s_copy_document) {
                nanoem_mutable_document_t *new_document = nanoemMutableDocumentCreate(factory, &status);
                copyDocument(factory, document, new_document);
                compareDocument(factory, document, nanoemMutableDocumentGetOrigin(new_document));
                nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(&status);
                nanoemMutableDocumentSaveToBuffer(new_document, mutable_buffer, &status);
                nanoemMutableDocumentDestroy(new_document);
                nanoem_buffer_t *new_buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
                fp = fopen("copy.pmm", "wb");
                fwrite(nanoemBufferGetDataPtr(new_buffer), nanoemBufferGetLength(new_buffer), 1, fp);
                fclose(fp);
                fprintf(stdout, "[C] status = %d, size = %ld, offset = %ld\n", status,
                    nanoemBufferGetLength(new_buffer), nanoemBufferGetOffset(new_buffer));
                nanoem_status_t test_status;
                nanoem_document_t *test_document = nanoemDocumentCreate(factory, &status);
                nanoemBufferSeek(new_buffer, 0, &test_status);
                nanoemDocumentLoadFromBuffer(test_document, new_buffer, &test_status);
                nanoemDocumentDestroy(test_document);
                fprintf(stdout, "[R] status = %d, offset = %ld\n", test_status, nanoemBufferGetOffset(new_buffer));
                nanoemMutableBufferDestroy(mutable_buffer);
            }
        }
        nanoemDocumentDestroy(document);
        nanoemBufferDestroy(buffer);
        nanoemUnicodeStringFactoryDestroyEXT(factory);
        delete[] data;
    }
    return 0;
}
