#include "nanoem/ext/document.h"
#include "nanoem/nanoem.h"
#include "xlsxwriter.h"

#include <vector>
#if !defined(PATH_MAX) && defined(MAX_PATH)
#define PATH_MAX MAX_PATH
#endif

#if defined(NANOEM_ENABLE_ICU)
#include "nanoem/ext/icu.h"
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateICU
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyICU
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackICU
#elif defined(NANOEM_ENABLE_MBWC)
#include "nanoem/ext/mbwc.h"
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateMBWC
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyMBWC
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackMBWC
#elif defined(__APPLE__)
#include "nanoem/ext/cfstring.h"
#if defined(NANOEM_ENABLE_CFSTRING)
#define nanoemUnicodeStringFactoryCreateEXT nanoemUnicodeStringFactoryCreateCF
#define nanoemUnicodeStringFactoryDestroyEXT nanoemUnicodeStringFactoryDestroyCF
#define nanoemUnicodeStringFactoryToUtf8OnStackEXT nanoemUnicodeStringFactoryToUtf8OnStackCF
#endif
#else

static nanoem_unicode_string_factory_t *
nanoemUnicodeStringFactoryCreateEXT(void)
{
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreate();
    return factory;
}

static void
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroy(factory);
}

static char *
nanoemUnicodeStringFactoryToUtf8EXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string,
    nanoem_rsize_t *length, nanoem_bool_t *ok)
{
    nanoem_mark_unused(factory);
    nanoem_mark_unused(string);
    *length = 0;
    *ok = nanoem_true;
    return NULL;
}

static void
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory,
    const nanoem_unicode_string_t *string, nanoem_rsize_t *length, char *buffer, size_t capacity, nanoem_bool_t *ok)
{
    char *s = nanoemUnicodeStringFactoryGetByteArray(factory, string, length, ok);
    strncpy(buffer, s, capacity);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, s);
}

#endif /* NANOEM_ENABLE_EXT */

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
    if (const char *ptr = strstr(reinterpret_cast<const char *>(s), needle)) {
        const char *p = ptr + sizeof(needle) - 1;
        char full_path[PATH_MAX] = { 0 };
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
            nanoem_buffer_t *buffer = nanoemBufferCreate(data, size);
            model = nanoemModelCreate(factory);
            nanoemModelLoadFromBuffer(model, buffer, status);
            nanoemBufferDestroy(buffer);
            delete[] data;
        }
    }
    nanoemUnicodeStringFactoryDestroyByteArray(factory, s);
    return model;
}

static bool
loadDocument(nanoem_u8_t *path, nanoem_unicode_string_factory_t *factory, nanoem_document_t **documentPtr)
{
    bool result = false;
    if (const char *p = strrchr(reinterpret_cast<const char *>(path), '.')) {
        if (strcmp(p, ".pmm") == 0) {
            if (FILE *fp = fopen(reinterpret_cast<const char *>(path), "rb")) {
                fseek(fp, 0, SEEK_END);
                long size = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                nanoem_u8_t *data = new nanoem_u8_t[size];
                fread(data, size, 1, fp);
                fclose(fp);
                nanoem_document_t *document = nanoemDocumentCreate(factory);
                nanoem_buffer_t *buffer = nanoemBufferCreate(data, size);
                nanoem_status_t status;
                nanoemDocumentSetParseModelCallback(document, callback);
                nanoemDocumentSetParseModelCallbackUserData(document, path);
                nanoemDocumentLoadFromBuffer(document, buffer, &status);
                delete[] data;
                *documentPtr = document;
                result = status == NANOEM_STATUS_SUCCESS;
            }
        }
    }
    return result;
}

static void
writeCameraSheet(const nanoem_document_t *document, nanoem_unicode_string_factory_t *factory, lxw_workbook *book)
{
    lxw_worksheet *sheet = workbook_add_worksheet(book, "Camera");
    nanoem_rsize_t num_keyframes;
    const nanoem_document_camera_t *camera = nanoemDocumentGetCameraObject(document);
    nanoem_document_camera_keyframe_t *const *keyframes =
        nanoemDocumentCameraGetAllCameraKeyframeObjects(camera, &num_keyframes);
    std::vector<nanoem_document_camera_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [](const nanoem_document_camera_keyframe_t *left, const nanoem_document_camera_keyframe_t *right) {
            nanoem_frame_index_t lf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentCameraKeyframeGetBaseKeyframeObject(left));
            nanoem_frame_index_t rf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentCameraKeyframeGetBaseKeyframeObject(right));
            return lf < rf;
        });
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_camera_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentCameraKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = i + 1;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        const nanoem_f32_t *angle = nanoemDocumentCameraKeyframeGetAngle(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, angle[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, angle[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, angle[2], nullptr);
        const nanoem_f32_t *lookat = nanoemDocumentCameraKeyframeGetLookAt(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, lookat[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, lookat[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, lookat[2], nullptr);
        nanoem_f32_t fov = nanoemDocumentCameraKeyframeGetFov(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, fov, nullptr);
        nanoem_f32_t distance = nanoemDocumentCameraKeyframeGetDistance(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, distance, nullptr);
        nanoem_bool_t perspective = nanoemDocumentCameraKeyframeIsPerspectiveView(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, perspective, nullptr);
        nanoem_rsize_t length;
        nanoem_u8_t *model_name = nanoemUnicodeStringFactoryGetByteArray(factory,
            nanoemDocumentModelGetName(
                nanoemDocumentCameraKeyframeGetParentModelObject(keyframe), NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
            &length, nullptr);
        worksheet_write_string(sheet, row_index, column_index++, reinterpret_cast<const char *>(model_name), nullptr);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
        nanoem_u8_t *bone_name = nanoemUnicodeStringFactoryGetByteArray(
            factory, nanoemDocumentCameraKeyframeGetParentModelBoneName(keyframe), &length, nullptr);
        worksheet_write_string(sheet, row_index, column_index++, reinterpret_cast<const char *>(bone_name), nullptr);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
        for (nanoem_rsize_t j = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             j < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
            const nanoem_u8_t *interpolation = nanoemDocumentCameraKeyframeGetInterpolation(
                keyframe, static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(j));
            worksheet_write_number(sheet, row_index, column_index++, interpolation[0], nullptr);
            worksheet_write_number(sheet, row_index, column_index++, interpolation[1], nullptr);
            worksheet_write_number(sheet, row_index, column_index++, interpolation[2], nullptr);
            worksheet_write_number(sheet, row_index, column_index++, interpolation[3], nullptr);
        }
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Angle(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Angle(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Angle(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "LookAt(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "LookAt(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "LookAt(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Fov", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Distance", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Perspective", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ParentModel", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ParentModelBone", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookX:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookX:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookX:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookX:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookY:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookY:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookY:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookY:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookZ:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookZ:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookZ:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(LookZ:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Angle:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Angle:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Angle:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Angle:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Fov:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Fov:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Fov:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Fov:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Distance:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Distance:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Distance:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Distance:Y1)", nullptr);
    worksheet_autofilter(sheet, 0, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeGravitySheet(
    const nanoem_document_t *document, nanoem_unicode_string_factory_t * /* factory */, lxw_workbook *book)
{
    lxw_worksheet *sheet = workbook_add_worksheet(book, "Gravity");
    nanoem_rsize_t num_keyframes;
    const nanoem_document_gravity_t *gravity = nanoemDocumentGetGravityObject(document);
    nanoem_document_gravity_keyframe_t *const *keyframes =
        nanoemDocumentGravityGetAllGravityKeyframeObjects(gravity, &num_keyframes);
    std::vector<nanoem_document_gravity_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [](const nanoem_document_gravity_keyframe_t *left, const nanoem_document_gravity_keyframe_t *right) {
            nanoem_frame_index_t lf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentGravityKeyframeGetBaseKeyframeObject(left));
            nanoem_frame_index_t rf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentGravityKeyframeGetBaseKeyframeObject(right));
            return lf < rf;
        });
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_gravity_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentGravityKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = i + 1;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        const nanoem_f32_t acceleration = nanoemDocumentGravityKeyframeGetAcceleration(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, acceleration, nullptr);
        const nanoem_f32_t *direction = nanoemDocumentGravityKeyframeGetDirection(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, direction[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, direction[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, direction[2], nullptr);
        int noise = nanoemDocumentGravityKeyframeGetNoise(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, noise, nullptr);
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Acceleration", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Direction(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Direction(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Direction(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Noise", nullptr);
    worksheet_autofilter(sheet, 0, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeLightSheet(const nanoem_document_t *document, nanoem_unicode_string_factory_t * /* factory */, lxw_workbook *book)
{
    lxw_worksheet *sheet = workbook_add_worksheet(book, "Light");
    nanoem_rsize_t num_keyframes;
    const nanoem_document_light_t *light = nanoemDocumentGetLightObject(document);
    nanoem_document_light_keyframe_t *const *keyframes =
        nanoemDocumentLightGetAllLightKeyframeObjects(light, &num_keyframes);
    std::vector<nanoem_document_light_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [](const nanoem_document_light_keyframe_t *left, const nanoem_document_light_keyframe_t *right) {
            nanoem_frame_index_t lf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentLightKeyframeGetBaseKeyframeObject(left));
            nanoem_frame_index_t rf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentLightKeyframeGetBaseKeyframeObject(right));
            return lf < rf;
        });
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_light_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentLightKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = i + 1;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        const nanoem_f32_t *angle = nanoemDocumentLightKeyframeGetColor(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, angle[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, angle[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, angle[2], nullptr);
        const nanoem_f32_t *lookat = nanoemDocumentLightKeyframeGetDirection(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, lookat[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, lookat[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, lookat[2], nullptr);
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Color(R)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Color(G)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Color(B)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Direction(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Direction(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Direction(Z)", nullptr);
    worksheet_autofilter(sheet, 0, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeSelfShadowSheet(
    const nanoem_document_t *document, nanoem_unicode_string_factory_t * /* factory */, lxw_workbook *book)
{
    lxw_worksheet *sheet = workbook_add_worksheet(book, "SelfShadow");
    nanoem_rsize_t num_keyframes;
    const nanoem_document_self_shadow_t *self_shadow = nanoemDocumentGetSelfShadowObject(document);
    nanoem_document_self_shadow_keyframe_t *const *keyframes =
        nanoemDocumentSelfShadowGetAllSelfShadowKeyframeObjects(self_shadow, &num_keyframes);
    std::vector<nanoem_document_self_shadow_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [](const nanoem_document_self_shadow_keyframe_t *left, const nanoem_document_self_shadow_keyframe_t *right) {
            nanoem_frame_index_t lf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(left));
            nanoem_frame_index_t rf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(right));
            return lf < rf;
        });
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_self_shadow_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = i + 1;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        const nanoem_f32_t distance = nanoemDocumentSelfShadowKeyframeGetDistance(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, distance, nullptr);
        const nanoem_u32_t mode = nanoemDocumentSelfShadowKeyframeGetMode(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, mode, nullptr);
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Distance", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Mode", nullptr);
    worksheet_autofilter(sheet, 0, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeAccessorySheet(
    const nanoem_document_accessory_t *accessory, nanoem_unicode_string_factory_t *factory, lxw_worksheet *sheet)
{
    nanoem_rsize_t num_keyframes;
    nanoem_document_accessory_keyframe_t *const *keyframes =
        nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(accessory, &num_keyframes);
    std::vector<nanoem_document_accessory_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [](const nanoem_document_accessory_keyframe_t *left, const nanoem_document_accessory_keyframe_t *right) {
            nanoem_frame_index_t lf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(left));
            nanoem_frame_index_t rf =
                nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(right));
            return lf < rf;
        });
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_accessory_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = i + 1;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        const nanoem_f32_t *translation = nanoemDocumentAccessoryKeyframeGetTranslation(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, translation[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, translation[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, translation[2], nullptr);
        const nanoem_f32_t *orientation = nanoemDocumentAccessoryKeyframeGetOrientation(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, orientation[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, orientation[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, orientation[2], nullptr);
        const nanoem_f32_t opacity = nanoemDocumentAccessoryKeyframeGetOpacity(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, opacity, nullptr);
        const nanoem_f32_t scale_factor = nanoemDocumentAccessoryKeyframeGetScaleFactor(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, scale_factor, nullptr);
        const nanoem_bool_t is_shadow_enabled = nanoemDocumentAccessoryKeyframeIsShadowEnabled(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, is_shadow_enabled, nullptr);
        const nanoem_bool_t is_visible = nanoemDocumentAccessoryKeyframeIsVisible(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, is_visible, nullptr);
        nanoem_rsize_t length;
        nanoem_u8_t *model_name = nanoemUnicodeStringFactoryGetByteArray(factory,
            nanoemDocumentModelGetName(
                nanoemDocumentAccessoryKeyframeGetParentModelObject(keyframe), NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
            &length, nullptr);
        worksheet_write_string(sheet, row_index, column_index++, reinterpret_cast<const char *>(model_name), nullptr);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
        nanoem_u8_t *bone_name = nanoemUnicodeStringFactoryGetByteArray(
            factory, nanoemDocumentAccessoryKeyframeGetParentModelBoneName(keyframe), &length, nullptr);
        worksheet_write_string(sheet, row_index, column_index++, reinterpret_cast<const char *>(bone_name), nullptr);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Translation(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Translation(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Translation(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Orientation(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Orientation(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Orientation(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Opacity", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ScaleFactor", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Shadow", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Visible", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ParentModel", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ParentModelBone", nullptr);
    worksheet_autofilter(sheet, 0, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeAllAccessorySheets(const nanoem_document_t *document, nanoem_unicode_string_factory_t *factory, lxw_workbook *book)
{
    nanoem_rsize_t num_accessories;
    nanoem_document_accessory_t *const *accessories = nanoemDocumentGetAllAccessoryObjects(document, &num_accessories);
    nanoem_rsize_t length;
    for (nanoem_rsize_t i = 0; i < num_accessories; i++) {
        const nanoem_document_accessory_t *accessory = accessories[i];
        nanoem_u8_t *accessory_name = nanoemUnicodeStringFactoryGetByteArray(
            factory, nanoemDocumentAccessoryGetName(accessory), &length, nullptr);
        if (lxw_worksheet *sheet = workbook_add_worksheet(book, reinterpret_cast<const char *>(accessory_name))) {
            nanoemUnicodeStringFactoryDestroyByteArray(factory, accessory_name);
            writeAccessorySheet(accessory, factory, sheet);
        }
        else {
            char buffer[32];
            int index = nanoemDocumentAccessoryGetIndex(accessory);
            snprintf(buffer, sizeof(buffer), "%s_%d", accessory_name, index);
            if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
                writeAccessorySheet(accessory, factory, sheet);
            }
            nanoemUnicodeStringFactoryDestroyByteArray(factory, accessory_name);
        }
    }
}

static void
writeModelAllBonesSheet(
    const nanoem_document_model_t *model, nanoem_unicode_string_factory_t *factory, lxw_worksheet *sheet)
{
    nanoem_rsize_t num_keyframes;
    nanoem_document_model_bone_keyframe_t *const *keyframes =
        nanoemDocumentModelGetAllBoneKeyframeObjects(model, &num_keyframes);
    std::vector<nanoem_document_model_bone_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [factory](
            const nanoem_document_model_bone_keyframe_t *left, const nanoem_document_model_bone_keyframe_t *right) {
            const nanoem_document_base_keyframe_t *lk = nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(left);
            const nanoem_document_base_keyframe_t *rk = nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(right);
            nanoem_frame_index_t lf = nanoemDocumentBaseKeyframeGetFrameIndex(lk);
            nanoem_frame_index_t rf = nanoemDocumentBaseKeyframeGetFrameIndex(rk);
            if (lf == rf) {
                const nanoem_unicode_string_t *ln = nanoemDocumentModelBoneKeyframeGetName(left);
                const nanoem_unicode_string_t *rn = nanoemDocumentModelBoneKeyframeGetName(right);
                return nanoemUnicodeStringFactoryCompareString(factory, ln, rn) < 0;
            }
            else {
                return lf < rf;
            }
        });
    nanoem_rsize_t offset = 1;
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_model_bone_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = offset++;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        nanoem_rsize_t length;
        nanoem_u8_t *bone_name = nanoemUnicodeStringFactoryGetByteArray(
            factory, nanoemDocumentModelBoneKeyframeGetName(keyframe), &length, nullptr);
        worksheet_write_string(sheet, row_index, column_index++, reinterpret_cast<const char *>(bone_name), nullptr);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
        const nanoem_f32_t *translation = nanoemDocumentModelBoneKeyframeGetTranslation(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, translation[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, translation[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, translation[2], nullptr);
        const nanoem_f32_t *orientation = nanoemDocumentModelBoneKeyframeGetOrientation(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, orientation[0], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, orientation[1], nullptr);
        worksheet_write_number(sheet, row_index, column_index++, orientation[2], nullptr);
        nanoem_bool_t simulation = nanoemDocumentModelBoneKeyframeIsPhysicsSimulationDisabled(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, simulation, nullptr);
        for (nanoem_rsize_t k = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             k < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; k++) {
            const nanoem_u8_t *interpolation = nanoemDocumentModelBoneKeyframeGetInterpolation(
                keyframe, static_cast<nanoem_motion_bone_keyframe_interpolation_type_t>(k));
            worksheet_write_number(sheet, row_index, column_index++, interpolation[0], nullptr);
            worksheet_write_number(sheet, row_index, column_index++, interpolation[1], nullptr);
            worksheet_write_number(sheet, row_index, column_index++, interpolation[2], nullptr);
            worksheet_write_number(sheet, row_index, column_index++, interpolation[3], nullptr);
        }
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Name", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Translation(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Translation(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Translation(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Orientation(X)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Orientation(Y)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Orientation(Z)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "DisablePhysicsSimulation", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationX:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationX:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationX:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationX:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationY:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationY:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationY:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationY:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationZ:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationZ:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationZ:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(TranslationZ:Y1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Orientaiton:X0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Orientaiton:Y0)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Orientaiton:X1)", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Interpolation(Orientaiton:Y1)", nullptr);
    worksheet_autofilter(sheet, 0, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeModelAllMorphsSheet(
    const nanoem_document_model_t *model, nanoem_unicode_string_factory_t *factory, lxw_worksheet *sheet)
{
    nanoem_rsize_t num_keyframes;
    nanoem_document_model_morph_keyframe_t *const *keyframes =
        nanoemDocumentModelGetAllMorphKeyframeObjects(model, &num_keyframes);
    std::vector<nanoem_document_model_morph_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [factory](
            const nanoem_document_model_morph_keyframe_t *left, const nanoem_document_model_morph_keyframe_t *right) {
            const nanoem_document_base_keyframe_t *lk = nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(left);
            const nanoem_document_base_keyframe_t *rk = nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(right);
            nanoem_frame_index_t lf = nanoemDocumentBaseKeyframeGetFrameIndex(lk);
            nanoem_frame_index_t rf = nanoemDocumentBaseKeyframeGetFrameIndex(rk);
            if (lf == rf) {
                const nanoem_unicode_string_t *ln = nanoemDocumentModelMorphKeyframeGetName(left);
                const nanoem_unicode_string_t *rn = nanoemDocumentModelMorphKeyframeGetName(right);
                return nanoemUnicodeStringFactoryCompareString(factory, ln, rn) < 0;
            }
            else {
                return lf < rf;
            }
        });
    nanoem_rsize_t offset = 1;
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_model_morph_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(keyframe));
        lxw_row_t row_index = offset++;
        lxw_col_t column_index = 0;
        worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
        nanoem_rsize_t length;
        nanoem_u8_t *morph_name = nanoemUnicodeStringFactoryGetByteArray(
            factory, nanoemDocumentModelMorphKeyframeGetName(keyframe), &length, nullptr);
        worksheet_write_string(sheet, row_index, column_index++, reinterpret_cast<const char *>(morph_name), nullptr);
        nanoemUnicodeStringFactoryDestroyByteArray(factory, morph_name);
        const nanoem_f32_t weight = nanoemDocumentModelMorphKeyframeGetWeight(keyframe);
        worksheet_write_number(sheet, row_index, column_index++, weight, nullptr);
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Name", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Weight", nullptr);
    worksheet_autofilter(sheet, header_row_index, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeModelAllKeyframesSheet(
    const nanoem_document_model_t *model, nanoem_unicode_string_factory_t *factory, lxw_worksheet *sheet)
{
    nanoem_rsize_t num_keyframes;
    nanoem_document_model_keyframe_t *const *keyframes =
        nanoemDocumentModelGetAllModelKeyframeObjects(model, &num_keyframes);
    std::vector<nanoem_document_model_keyframe_t *> new_keyframes(keyframes, keyframes + num_keyframes);
    std::sort(new_keyframes.begin(), new_keyframes.end(),
        [factory](const nanoem_document_model_keyframe_t *left, const nanoem_document_model_keyframe_t *right) {
            const nanoem_document_base_keyframe_t *lk = nanoemDocumentModelKeyframeGetBaseKeyframeObject(left);
            const nanoem_document_base_keyframe_t *rk = nanoemDocumentModelKeyframeGetBaseKeyframeObject(right);
            nanoem_frame_index_t lf = nanoemDocumentBaseKeyframeGetFrameIndex(lk);
            nanoem_frame_index_t rf = nanoemDocumentBaseKeyframeGetFrameIndex(rk);
            return lf < rf;
        });
    lxw_row_t row_index = 1;
    for (nanoem_rsize_t i = 0; i < num_keyframes; i++) {
        const nanoem_document_model_keyframe_t *keyframe = new_keyframes[i];
        nanoem_frame_index_t frame_index =
            nanoemDocumentBaseKeyframeGetFrameIndex(nanoemDocumentModelKeyframeGetBaseKeyframeObject(keyframe));
        nanoem_rsize_t num_states = 0, num_state_skipped = 0;
        nanoem_document_model_constraint_state_t *const *states =
            nanoemDocumentModelKeyframeGetAllModelConstraintStateObjects(keyframe, &num_states);
        bool visible = nanoemDocumentModelKeyframeIsVisible(keyframe);
        if (num_states > 0) {
            for (nanoem_rsize_t j = 0; j < num_states; j++) {
                const nanoem_document_model_constraint_state_t *state = states[j];
                lxw_col_t column_index = 0;
                worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
                worksheet_write_number(sheet, row_index, column_index++, visible, nullptr);
                nanoem_rsize_t length;
                nanoem_u8_t *bone_name = nanoemUnicodeStringFactoryGetByteArray(
                    factory, nanoemDocumentModelConstraintStateGetName(state), &length, nullptr);
                if (bone_name && strlen(reinterpret_cast<const char *>(bone_name)) > 0) {
                    worksheet_write_string(
                        sheet, row_index, column_index++, reinterpret_cast<const char *>(bone_name), nullptr);
                    worksheet_write_number(
                        sheet, row_index, column_index++, nanoemDocumentModelConstraintStateIsEnabled(state), nullptr);
                    worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
                    worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
                    row_index++;
                }
                else {
                    num_state_skipped++;
                }
                nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
            }
        }
        nanoem_rsize_t num_parents = 0, num_parent_skipped = 0;
        nanoem_document_outside_parent_t *const *parents =
            nanoemDocumentModelKeyframeGetAllOutsideParentObjects(keyframe, &num_parents);
        if (num_parents > 0) {
            for (nanoem_rsize_t k = 0; k < num_parents; k++) {
                const nanoem_document_outside_parent_t *parent = parents[k];
                nanoem_rsize_t length;
                nanoem_u8_t *model_name = nanoemUnicodeStringFactoryGetByteArray(factory,
                    nanoemDocumentModelGetName(
                        nanoemDocumentOutsideParentGetModelObject(parent), NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
                    &length, nullptr);
                nanoem_u8_t *bone_name = nanoemUnicodeStringFactoryGetByteArray(
                    factory, nanoemDocumentOutsideParentGetBoneName(parent), &length, nullptr);
                if (model_name && strlen(reinterpret_cast<const char *>(model_name)) > 0 && bone_name &&
                    strlen(reinterpret_cast<const char *>(bone_name)) > 0) {
                    lxw_col_t column_index = 0;
                    worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
                    worksheet_write_number(sheet, row_index, column_index++, visible, nullptr);
                    worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
                    worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
                    worksheet_write_string(
                        sheet, row_index, column_index++, reinterpret_cast<const char *>(model_name), nullptr);
                    worksheet_write_string(
                        sheet, row_index, column_index++, reinterpret_cast<const char *>(bone_name), nullptr);
                    row_index++;
                }
                else {
                    num_parent_skipped++;
                }
                nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
                nanoemUnicodeStringFactoryDestroyByteArray(factory, bone_name);
            }
        }
        if (num_states == num_state_skipped && num_parents == num_parent_skipped) {
            lxw_col_t column_index = 0;
            worksheet_write_number(sheet, row_index, column_index++, frame_index, nullptr);
            worksheet_write_number(sheet, row_index, column_index++, visible, nullptr);
            worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
            worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
            worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
            worksheet_write_string(sheet, row_index, column_index++, "", nullptr);
        }
    }
    lxw_row_t header_row_index = 0;
    lxw_col_t header_column_index = 0;
    worksheet_write_string(sheet, header_row_index, header_column_index++, "FrameIndex", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "Visible", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ConstraintBoneName", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ConstraintEnabled", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ParentModel", nullptr);
    worksheet_write_string(sheet, header_row_index, header_column_index++, "ParentModelBone", nullptr);
    worksheet_autofilter(sheet, header_row_index, 0, num_keyframes + 1, header_column_index - 1);
}

static void
writeAllModelSheets(const nanoem_document_t *document, nanoem_unicode_string_factory_t *factory, lxw_workbook *book)
{
    nanoem_rsize_t num_models;
    nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(document, &num_models);
    nanoem_rsize_t length;
    for (nanoem_rsize_t i = 0; i < num_models; i++) {
        const nanoem_document_model_t *model = models[i];
        char buffer[32];
        int index = nanoemDocumentModelGetIndex(model);
        nanoem_u8_t *model_name = nanoemUnicodeStringFactoryGetByteArray(
            factory, nanoemDocumentModelGetName(model, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, nullptr);
        snprintf(buffer, sizeof(buffer), "%s(B)", model_name);
        if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
            writeModelAllBonesSheet(model, factory, sheet);
        }
        else {
            snprintf(buffer, sizeof(buffer), "%s_%d(B)", model_name, index);
            if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
                writeModelAllBonesSheet(model, factory, sheet);
            }
        }
        snprintf(buffer, sizeof(buffer), "%s(E)", model_name);
        if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
            writeModelAllMorphsSheet(model, factory, sheet);
        }
        else {
            snprintf(buffer, sizeof(buffer), "%s_%d(E)", model_name, index);
            if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
                writeModelAllMorphsSheet(model, factory, sheet);
            }
        }
        snprintf(buffer, sizeof(buffer), "%s(M)", model_name);
        if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
            writeModelAllKeyframesSheet(model, factory, sheet);
        }
        else {
            snprintf(buffer, sizeof(buffer), "%s_%d(M)", model_name, index);
            if (lxw_worksheet *sheet = workbook_add_worksheet(book, buffer)) {
                writeModelAllKeyframesSheet(model, factory, sheet);
            }
        }
        nanoemUnicodeStringFactoryDestroyByteArray(factory, model_name);
    }
}

static void
writeBook(const nanoem_document_t *document, nanoem_unicode_string_factory_t *factory, lxw_workbook *book)
{
    writeCameraSheet(document, factory, book);
    writeGravitySheet(document, factory, book);
    writeLightSheet(document, factory, book);
    writeSelfShadowSheet(document, factory, book);
    writeAllAccessorySheets(document, factory, book);
    writeAllModelSheets(document, factory, book);
}

static bool
slurpFile(const char *path, nanoem_u8_t *&data, size_t &size)
{
    bool result = false;
    if (FILE *fp = fopen(path, "rb")) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = new nanoem_u8_t[size];
        fread(data, 1, size, fp);
        fclose(fp);
        result = true;
    }
    return result;
}

static const char *
toString(nanoem_codec_type_t type)
{
    switch (type) {
    case NANOEM_CODEC_TYPE_SJIS:
        return "SJIS";
    case NANOEM_CODEC_TYPE_UTF8:
        return "UTF-8";
    case NANOEM_CODEC_TYPE_UTF16:
        return "UTF-16";
    default:
        return "(Unknown)";
    }
}

static const char *
toString(nanoem_model_format_type_t type)
{
    switch (type) {
    case NANOEM_MODEL_FORMAT_TYPE_PMD_1_0:
        return "PMD 1.0";
    case NANOEM_MODEL_FORMAT_TYPE_PMX_2_0:
        return "PMX 2.0";
    case NANOEM_MODEL_FORMAT_TYPE_PMX_2_1:
        return "PMX 2.1";
    default:
        return "(Unknown)";
    }
}

static void
validateModel(nanoem_unicode_string_factory_t *factory, const char *path, lxw_worksheet *sheet, lxw_row_t row)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_model_t *model = nanoemModelCreate(factory);
        nanoem_buffer_t *buffer = nanoemBufferCreate(data, size);
        if (nanoemModelLoadFromBuffer(model, buffer, &status)) {
            lxw_col_t column = 0;
            worksheet_write_string(sheet, row, column++, path, nullptr);
            worksheet_write_number(sheet, row, column++, nanoemModelGetAdditionalUVSize(model), nullptr);
            worksheet_write_string(sheet, row, column++, toString(nanoemModelGetCodecType(model)), nullptr);
            worksheet_write_string(sheet, row, column++, toString(nanoemModelGetFormatType(model)), nullptr);
            {
                nanoem_rsize_t num_objects = 0, num_bdef1 = 0, num_bdef2 = 0, num_bdef4 = 0, num_sdef = 0, num_qdef = 0;
                nanoem_model_vertex_t *const *items = nanoemModelGetAllVertexObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_vertex_t *item = items[i];
                    switch (nanoemModelVertexGetType(item)) {
                    case NANOEM_MODEL_VERTEX_TYPE_BDEF1:
                        num_bdef1++;
                        break;
                    case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
                        num_bdef2++;
                        break;
                    case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
                        num_bdef4++;
                        break;
                    case NANOEM_MODEL_VERTEX_TYPE_SDEF:
                        num_sdef++;
                        break;
                    case NANOEM_MODEL_VERTEX_TYPE_QDEF:
                        num_qdef++;
                        break;
                    default:
                        break;
                    }
                }
                worksheet_write_number(sheet, row, column++, num_bdef1, nullptr);
                worksheet_write_number(sheet, row, column++, num_bdef2, nullptr);
                worksheet_write_number(sheet, row, column++, num_bdef4, nullptr);
                worksheet_write_number(sheet, row, column++, num_sdef, nullptr);
                worksheet_write_number(sheet, row, column++, num_qdef, nullptr);
            }
            {
                nanoem_rsize_t num_objects = 0, num_add = 0, num_mul = 0, num_none = 0, num_sub = 0, num_csm = 0,
                               num_cs = 0, num_cd = 0, num_edge = 0, num_line = 0, num_point = 0, num_sm = 0,
                               num_ts = 0, num_vc = 0;
                nanoem_model_material_t *const *items = nanoemModelGetAllMaterialObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_material_t *item = items[i];
                    switch (nanoemModelMaterialGetSphereMapTextureType(item)) {
                    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD:
                        num_add++;
                        break;
                    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY:
                        num_mul++;
                        break;
                    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE:
                        num_none++;
                        break;
                    case NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_SUB_TEXTURE:
                        num_sub++;
                        break;
                    default:
                        break;
                    }
                    if (nanoemModelMaterialIsCastingShadowEnabled(item)) {
                        num_cs++;
                    }
                    if (nanoemModelMaterialIsCastingShadowMapEnabled(item)) {
                        num_csm++;
                    }
                    if (nanoemModelMaterialIsCullingDisabled(item)) {
                        num_cd++;
                    }
                    if (nanoemModelMaterialIsEdgeEnabled(item)) {
                        num_edge++;
                    }
                    if (nanoemModelMaterialIsLineDrawEnabled(item)) {
                        num_line++;
                    }
                    if (nanoemModelMaterialIsPointDrawEnabled(item)) {
                        num_point++;
                    }
                    if (nanoemModelMaterialIsShadowMapEnabled(item)) {
                        num_sm++;
                    }
                    if (nanoemModelMaterialIsToonShared(item)) {
                        num_ts++;
                    }
                    if (nanoemModelMaterialIsVertexColorEnabled(item)) {
                        num_vc++;
                    }
                }
                worksheet_write_number(sheet, row, column++, num_add, nullptr);
                worksheet_write_number(sheet, row, column++, num_mul, nullptr);
                worksheet_write_number(sheet, row, column++, num_none, nullptr);
                worksheet_write_number(sheet, row, column++, num_sub, nullptr);
                worksheet_write_number(sheet, row, column++, num_cs, nullptr);
                worksheet_write_number(sheet, row, column++, num_csm, nullptr);
                worksheet_write_number(sheet, row, column++, num_cd, nullptr);
                worksheet_write_number(sheet, row, column++, num_edge, nullptr);
                worksheet_write_number(sheet, row, column++, num_line, nullptr);
                worksheet_write_number(sheet, row, column++, num_point, nullptr);
                worksheet_write_number(sheet, row, column++, num_sm, nullptr);
                worksheet_write_number(sheet, row, column++, num_ts, nullptr);
                worksheet_write_number(sheet, row, column++, num_vc, nullptr);
            }
            {
                nanoem_rsize_t num_objects = 0, num_dest = 0, num_rot = 0, num_mov = 0, num_visible = 0, num_handle = 0,
                               num_const = 0, num_lh = 0, num_inh_tr = 0, num_inh_or = 0, num_fixed = 0, num_la = 0,
                               num_physics = 0, num_ext = 0;
                nanoem_model_bone_t *const *items = nanoemModelGetAllBoneObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_bone_t *item = items[i];
                    if (nanoemModelBoneHasDestinationBone(item)) {
                        num_dest++;
                    }
                    if (nanoemModelBoneIsRotateable(item)) {
                        num_rot++;
                    }
                    if (nanoemModelBoneIsMovable(item)) {
                        num_mov++;
                    }
                    if (nanoemModelBoneIsVisible(item)) {
                        num_visible++;
                    }
                    if (nanoemModelBoneIsUserHandleable(item)) {
                        num_handle++;
                    }
                    if (nanoemModelBoneHasConstraint(item)) {
                        num_const++;
                    }
                    if (nanoemModelBoneHasLocalInherent(item)) {
                        num_lh++;
                    }
                    if (nanoemModelBoneHasInherentTranslation(item)) {
                        num_inh_tr++;
                    }
                    if (nanoemModelBoneHasInherentOrientation(item)) {
                        num_inh_or++;
                    }
                    if (nanoemModelBoneHasFixedAxis(item)) {
                        num_fixed++;
                    }
                    if (nanoemModelBoneHasLocalAxes(item)) {
                        num_la++;
                    }
                    if (nanoemModelBoneIsAffectedByPhysicsSimulation(item)) {
                        num_physics++;
                    }
                    if (nanoemModelBoneHasExternalParentBone(item)) {
                        num_ext++;
                    }
                }
                worksheet_write_number(sheet, row, column++, num_dest, nullptr);
                worksheet_write_number(sheet, row, column++, num_rot, nullptr);
                worksheet_write_number(sheet, row, column++, num_mov, nullptr);
                worksheet_write_number(sheet, row, column++, num_visible, nullptr);
                worksheet_write_number(sheet, row, column++, num_handle, nullptr);
                worksheet_write_number(sheet, row, column++, num_const, nullptr);
                worksheet_write_number(sheet, row, column++, num_lh, nullptr);
                worksheet_write_number(sheet, row, column++, num_inh_tr, nullptr);
                worksheet_write_number(sheet, row, column++, num_inh_or, nullptr);
                worksheet_write_number(sheet, row, column++, num_fixed, nullptr);
                worksheet_write_number(sheet, row, column++, num_la, nullptr);
                worksheet_write_number(sheet, row, column++, num_physics, nullptr);
                worksheet_write_number(sheet, row, column++, num_ext, nullptr);
            }
            {
                nanoem_rsize_t num_objects = 0, num_bones = 0, num_flips = 0, num_groups = 0, num_impulsed = 0,
                               num_materials = 0, num_textures = 0, num_uv1 = 0, num_uv2 = 0, num_uv3 = 0, num_uv4 = 0,
                               num_vertices = 0;
                nanoem_model_morph_t *const *items = nanoemModelGetAllMorphObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_morph_t *item = items[i];
                    switch (nanoemModelMorphGetType(item)) {
                    case NANOEM_MODEL_MORPH_TYPE_BONE:
                        num_bones++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_FLIP:
                        num_flips++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_GROUP:
                        num_groups++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_IMPULUSE:
                        num_impulsed++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_MATERIAL:
                        num_materials++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_TEXTURE:
                        num_textures++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_UVA1:
                        num_uv1++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_UVA2:
                        num_uv2++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_UVA3:
                        num_uv3++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_UVA4:
                        num_uv4++;
                        break;
                    case NANOEM_MODEL_MORPH_TYPE_VERTEX:
                        num_vertices++;
                        break;
                    default:
                        break;
                    }
                }
                worksheet_write_number(sheet, row, column++, num_bones, nullptr);
                worksheet_write_number(sheet, row, column++, num_flips, nullptr);
                worksheet_write_number(sheet, row, column++, num_groups, nullptr);
                worksheet_write_number(sheet, row, column++, num_impulsed, nullptr);
                worksheet_write_number(sheet, row, column++, num_materials, nullptr);
                worksheet_write_number(sheet, row, column++, num_textures, nullptr);
                worksheet_write_number(sheet, row, column++, num_uv1, nullptr);
                worksheet_write_number(sheet, row, column++, num_uv2, nullptr);
                worksheet_write_number(sheet, row, column++, num_uv3, nullptr);
                worksheet_write_number(sheet, row, column++, num_uv4, nullptr);
                worksheet_write_number(sheet, row, column++, num_vertices, nullptr);
            }
            {
                nanoem_rsize_t num_objects = 0;
                nanoem_model_label_t *const *items = nanoemModelGetAllLabelObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_label_t *item = items[i];
                }
            }
            {
                nanoem_rsize_t num_objects = 0, num_dynamic = 0, num_kinematic = 0, num_static = 0, num_box = 0,
                               num_capsule = 0, num_sphere = 0;
                nanoem_model_rigid_body_t *const *items = nanoemModelGetAllRigidBodyObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_rigid_body_t *item = items[i];
                    switch (nanoemModelRigidBodyGetObjectType(item)) {
                    case NANOEM_MODEL_RIGID_BODY_OBJECT_TYPE_DYNAMIC:
                        num_dynamic++;
                        break;
                    case NANOEM_MODEL_RIGID_BODY_OBJECT_TYPE_KINEMATIC:
                        num_kinematic++;
                        break;
                    case NANOEM_MODEL_RIGID_BODY_OBJECT_TYPE_STATIC:
                        num_static++;
                        break;
                    default:
                        break;
                    }
                    switch (nanoemModelRigidBodyGetShapeType(item)) {
                    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_BOX:
                        num_box++;
                        break;
                    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_CAPSULE:
                        num_capsule++;
                        break;
                    case NANOEM_MODEL_RIGID_BODY_SHAPE_TYPE_SPHERE:
                        num_sphere++;
                        break;
                    default:
                        break;
                    }
                }
                worksheet_write_number(sheet, row, column++, num_dynamic, nullptr);
                worksheet_write_number(sheet, row, column++, num_kinematic, nullptr);
                worksheet_write_number(sheet, row, column++, num_static, nullptr);
                worksheet_write_number(sheet, row, column++, num_box, nullptr);
                worksheet_write_number(sheet, row, column++, num_capsule, nullptr);
                worksheet_write_number(sheet, row, column++, num_sphere, nullptr);
            }
            {
                nanoem_rsize_t num_objects = 0, num_cone = 0, num_gen_6dof = 0, num_6dof = 0, num_hinge = 0,
                               num_p2p = 0, num_slider = 0;
                nanoem_model_joint_t *const *items = nanoemModelGetAllJointObjects(model, &num_objects);
                worksheet_write_number(sheet, row, column++, num_objects, nullptr);
                for (nanoem_rsize_t i = 0; i < num_objects; i++) {
                    const nanoem_model_joint_t *item = items[i];
                    switch (nanoemModelJointGetType(item)) {
                    case NANOEM_MODEL_JOINT_TYPE_CONE_TWIST_CONSTRAINT:
                        num_cone++;
                        break;
                    case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_CONSTRAINT:
                        num_gen_6dof++;
                        break;
                    case NANOEM_MODEL_JOINT_TYPE_GENERIC_6DOF_SPRING_CONSTRAINT:
                        num_6dof++;
                        break;
                    case NANOEM_MODEL_JOINT_TYPE_HINGE_CONSTRAINT:
                        num_hinge++;
                        break;
                    case NANOEM_MODEL_JOINT_TYPE_POINT2POINT_CONSTRAINT:
                        num_p2p++;
                        break;
                    case NANOEM_MODEL_JOINT_TYPE_SLIDER_CONSTRAINT:
                        num_slider++;
                        break;
                    default:
                        break;
                    }
                }
                worksheet_write_number(sheet, row, column++, num_cone, nullptr);
                worksheet_write_number(sheet, row, column++, num_gen_6dof, nullptr);
                worksheet_write_number(sheet, row, column++, num_6dof, nullptr);
                worksheet_write_number(sheet, row, column++, num_hinge, nullptr);
                worksheet_write_number(sheet, row, column++, num_p2p, nullptr);
                worksheet_write_number(sheet, row, column++, num_slider, nullptr);
            }
        }
        nanoemBufferDestroy(buffer);
        nanoemModelDestroy(model);
        delete[] data;
    }
}

static void
validateMotion(nanoem_unicode_string_factory_t *factory, const char *path, lxw_worksheet *sheet, lxw_row_t row)
{
    nanoem_u8_t *data = 0;
    size_t size = 0;
    if (slurpFile(path, data, size)) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_motion_t *motion = nanoemMotionCreate(factory);
        nanoem_buffer_t *buffer = nanoemBufferCreate(data, size);
        if (nanoemMotionLoadFromBuffer(motion, buffer, 0, &status)) {
            lxw_col_t column = 0;
            worksheet_write_string(sheet, row, column++, path, nullptr);
            nanoem_rsize_t length;
            nanoem_u8_t *target =
                nanoemUnicodeStringFactoryGetByteArray(factory, nanoemMotionGetTargetModelName(motion), &length, 0);
            worksheet_write_string(sheet, row, column++, reinterpret_cast<const char *>(target), nullptr);
            nanoemUnicodeStringFactoryDestroyByteArray(factory, target);
            worksheet_write_number(sheet, row, column++, nanoemMotionGetMaxFrameIndex(motion), nullptr);
            nanoem_rsize_t num_objects = 0;
            nanoemMotionGetAllBoneKeyframeObjects(motion, &num_objects);
            worksheet_write_number(sheet, row, column++, num_objects, nullptr);
            nanoemMotionGetAllMorphKeyframeObjects(motion, &num_objects);
            worksheet_write_number(sheet, row, column++, num_objects, nullptr);
            nanoemMotionGetAllLightKeyframeObjects(motion, &num_objects);
            worksheet_write_number(sheet, row, column++, num_objects, nullptr);
            nanoemMotionGetAllSelfShadowKeyframeObjects(motion, &num_objects);
            worksheet_write_number(sheet, row, column++, num_objects, nullptr);
            nanoemMotionGetAllModelKeyframeObjects(motion, &num_objects);
            worksheet_write_number(sheet, row, column++, num_objects, nullptr);
        }
        nanoemBufferDestroy(buffer);
        nanoemMotionDestroy(motion);
        delete[] data;
    }
}

static void
processFile(nanoem_unicode_string_factory_t *factory, const char *path, lxw_worksheet *model_sheet,
    lxw_worksheet *motion_sheet, lxw_row_t &model_row, lxw_row_t &motion_row)
{
    if (const char *p = strrchr(path, '.')) {
        if (strcmp(p, ".pmd") == 0 || strcmp(p, ".pmx") == 0) {
            validateModel(factory, path, model_sheet, model_row++);
        }
        else if (strcmp(p, ".vmd") == 0) {
            validateMotion(factory, path, motion_sheet, motion_row++);
        }
    }
}

int
main(int argc, char *argv[])
{
    nanoem_u8_t path[PATH_MAX] = { 0 };
    const char *filename = argc > 1 ? argv[1] : "test.pmm";
    strcpy(reinterpret_cast<char *>(path), filename);
    nanoem_document_t *document;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT();
    if (loadDocument(path, factory, &document)) {
        lxw_workbook *book = workbook_new("nanoem.xlsx");
        writeBook(document, factory, book);
        lxw_error error = workbook_close(book);
        if (error) {
            printf("ERROR: %s\n", lxw_strerror(error));
        }
    }
    else if (FILE *fp = fopen(reinterpret_cast<char *>(path), "rb")) {
        char buffer[PATH_MAX];
        lxw_workbook *book = workbook_new("nanoem.xlsx");
        lxw_worksheet *model_sheet = workbook_add_worksheet(book, "Model");
        lxw_worksheet *motion_sheet = workbook_add_worksheet(book, "Motion");
        lxw_row_t model_row = 1, motion_row = 1;
        while (!feof(fp)) {
            fgets(buffer, sizeof(buffer), fp);
            buffer[strcspn(buffer, "\r\n")] = 0;
            processFile(factory, buffer, model_sheet, motion_sheet, model_row, motion_row);
        }
        fclose(fp);
        lxw_col_t column = 0;
        worksheet_write_string(model_sheet, 0, column++, "Path", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "UVA", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Codec", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Format", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Vertices", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "V:BDEF1", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "V:BDEF2", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "V:BDEF4", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "V:SDEF", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "V:QDEF", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Materials", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:SphereTypeAdd", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:SphereTypeMul", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:SphereTypeNone", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:SphereTypeSub", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:CastingShadow", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:CastingShadowMap", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:CullingDisabled", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:EdgeEnabled", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:LineDraw", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:PointDraw", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:ShadowMap", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:SharedToon", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:VertexColor", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Bones", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:BoneDestination", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:Rotateable", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:Movable", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:Visible", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:UserHandleable", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:Constraint", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:LocalInherent", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:InherentTranslation", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:InherentOrientation", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:FixedAxis", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:LocalAxes", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:AffectedByPhysics", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "B:ExternalParent", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Morphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:BoneMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:FlipMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:GroupMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:ImpulseMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:MaterialMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:TextureMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:UV1Morphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:UV2Morphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:UV3Morphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:UV4Morphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "M:VertexMorphs", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Labels", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "RigidBodies", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "R:Dynamic", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "R:Kinematic", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "R:Static", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "R:Box", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "R:Capsule", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "R:Sphere", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "Joints", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "J:ConeTwist", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "J:Generic6DOFSpring", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "J:6DOFSpring", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "J:Hinge", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "J:Point2Point", nullptr);
        worksheet_write_string(model_sheet, 0, column++, "J:Slider", nullptr);
        worksheet_autofilter(model_sheet, 0, 0, model_row, column - 1);
        column = 0;
        worksheet_write_string(motion_sheet, 0, column++, "Path", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "Target", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "Duration", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "BoneKeyframes", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "MorphKeyframes", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "LightKeyframes", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "SelfShadowKeyframes", nullptr);
        worksheet_write_string(motion_sheet, 0, column++, "ModelKeyframes", nullptr);
        worksheet_autofilter(motion_sheet, 0, 0, motion_row, column - 1);
        lxw_error error = workbook_close(book);
        if (error) {
            printf("ERROR: %s\n", lxw_strerror(error));
        }
    }
    nanoemDocumentDestroy(document);
    nanoemUnicodeStringFactoryDestroyEXT(factory);
    return 0;
}
