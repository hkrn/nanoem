/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#endif /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nanomqo.h"

#if !defined(_MSC_VER) && !defined(NANOMQO_ENABLE_PEDANTIC_WARNINGS)
#include "assertion-macros/assertion-macros.h"
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#define assert_equal_float(a, b) \
    for (;;) {                   \
        union __float2int_t {    \
            int i;               \
            float f;             \
        } v;                     \
        v.f = (b);               \
        assert_equal((a), v.i);  \
        break;                   \
    }

static char *
nanomqoConvertString(const char *s, char **p)
{
    if (*p) {
        free(*p);
        *p = NULL;
    }
    if (s) {
#ifdef __APPLE__
        CFStringRef cs = CFStringCreateWithBytes(kCFAllocatorDefault,
            (const UInt8 *) s,
            strlen(s),
            kCFStringEncodingMacJapanese,
            FALSE);
        CFIndex length = CFStringGetLength(cs);
        CFIndex max = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
        *p = calloc(max + 1, sizeof(*s));
        CFStringGetBytes(cs, CFRangeMake(0, length), kCFStringEncodingUTF8, '?', FALSE, (UInt8 *) *p, max, NULL);
        CFRelease(cs);
#endif
    }
    return *p;
}

NANOMQO_DECL_INLINE static const char *
nanomqoAxisTypeToString(nanomqo_axis_type_t type)
{
    switch (type) {
    case NANOMQO_AXIS_TYPE_X:
        return "X";
    case NANOMQO_AXIS_TYPE_Y:
        return "Y";
    case NANOMQO_AXIS_TYPE_Z:
        return "Z";
    default:
        return "(Unknown)";
    }
}

NANOMQO_DECL_INLINE static const char *
nanomqoLatheTypeToString(nanomqo_lathe_type_t type)
{
    switch (type) {
    case NANOMQO_LATHE_TYPE_NONE:
        return "NONE";
    case NANOMQO_LATHE_TYPE_TWOSIDES:
        return "TWOSIDES";
    default:
        return "(Unknown)";
    }
}

NANOMQO_DECL_INLINE static const char *
nanomqoMirrorTypeToString(nanomqo_mirror_type_t type)
{
    switch (type) {
    case NANOMQO_MIRROR_TYPE_CONNECTED:
        return "CONNECTED";
    case NANOMQO_MIRROR_TYPE_NONE:
        return "NONE";
    case NANOMQO_MIRROR_TYPE_SEPARATED:
        return "SEPARATED";
    case NANOMQO_MIRROR_TYPE_MAX_ENUM:
    default:
        return "(Unknown)";
    }
}

NANOMQO_DECL_INLINE static const char *
nanomqoPatchTypeToString(nanomqo_patch_type_t type)
{
    switch (type) {
    case NANOMQO_PATCH_TYPE_CATMULL_CLARK:
        return "CATMULL_CLARK";
    case NANOMQO_PATCH_TYPE_OPENSUBDIV:
        return "OPENSUBDIV";
    case NANOMQO_PATCH_TYPE_PLANE:
        return "PLANE";
    case NANOMQO_PATCH_TYPE_SPLINE_TYPE1:
        return "SPLINE_TYPE1";
    case NANOMQO_PATCH_TYPE_SPLINE_TYPE2:
        return "SPLINE_TYPE2";
    case NANOMQO_PATCH_TYPE_MAX_ENUM:
    default:
        return "(Unknown)";
    }
}

NANOMQO_DECL_INLINE static const char *
nanomqoProjectionTypeToString(nanomqo_projection_type_t type)
{
    switch (type) {
    case NANOMQO_PROJECTION_TYPE_CYLINDER:
        return "CYLINDER";
    case NANOMQO_PROJECTION_TYPE_PLANE:
        return "PLANE";
    case NANOMQO_PROJECTION_TYPE_SPHERE:
        return "SPHERE";
    case NANOMQO_PROJECTION_TYPE_UV:
        return "UV";
    case NANOMQO_PROJECTION_TYPE_MAX_ENUM:
    default:
        return "(Unknown)";
    }
}

NANOMQO_DECL_INLINE static const char *
nanomqoShaderTypeToString(nanomqo_shader_type_t type)
{
    switch (type) {
    case NANOMQO_SHADER_TYPE_BLINN:
        return "BLINN";
    case NANOMQO_SHADER_TYPE_CLASSIC:
        return "CLASSIC";
    case NANOMQO_SHADER_TYPE_CONSTANT:
        return "CONSTANT";
    case NANOMQO_SHADER_TYPE_LAMBERT:
        return "LAMBERT";
    case NANOMQO_SHADER_TYPE_PHONG:
        return "PHONG";
    case NANOMQO_SHADER_TYPE_MAX_ENUM:
    default:
        return "(Unknown)";
    }
}

NANOMQO_DECL_INLINE static const char *
nanomqoShadingTypeToString(nanomqo_shading_type_t type)
{
    switch (type) {
    case NANOMQO_SHADING_TYPE_FLAT:
        return "FLAT";
    case NANOMQO_SHADING_TYPE_GOURAUT:
        return "GOURAUT";
    case NANOMQO_SHADING_TYPE_MAX_ENUM:
    default:
        return "(Unknown)";
    }
}

static void
nanomqoTestNull(void)
{
#if !defined(_MSC_VER) && !defined(NANOMQO_ENABLE_PEDANTIC_WARNINGS)
    nanomqo_bool_t ok;
    int nitems;
    assert_equal(nanomqo_false, nanomqoBufferCanReadLength(NULL, 0));
    assert_equal(nanomqo_false, nanomqoBufferCanReadLength(NULL, 1));
    nanomqoBufferDestroy(NULL);
    assert_equal(0, nanomqoBufferGetColumnOffset(NULL));
    assert_null(nanomqoBufferGetDataPtr(NULL));
    assert_equal(0, (int) nanomqoBufferGetLength(NULL));
    assert_equal(0, (int) nanomqoBufferGetOffset(NULL));
    assert_equal(0, nanomqoBufferGetRowOffset(NULL));
    nanomqoBufferSkip(NULL, 0, &ok);
    assert_equal(nanomqo_false, ok);
    nanomqoDocumentDestroy(NULL);
    assert_null(nanomqoDocumentGetMaterials(NULL, &nitems));
    assert_equal(0, nitems);
    assert_null(nanomqoDocumentGetObjects(NULL, &nitems));
    assert_equal(0, nitems);
    assert_null(nanomqoDocumentGetScene(NULL));
    assert_equal(NANOMQO_STATUS_ERROR_NULL_BUFFER, nanomqoDocumentParse(NULL, NULL));
    assert_equal(NANOMQO_STATUS_ERROR_NULL_DOCUMENT, nanomqoDocumentParse(NULL, (nanomqo_buffer_t *) 0xdeadbeaf));
    assert_null(nanomqoFaceGetColors(NULL));
    assert_null(nanomqoFaceGetCreases(NULL));
    assert_equal(0, nanomqoFaceGetMaterialIndex(NULL));
    assert_null(nanomqoFaceGetVertexIndices(NULL, &nitems));
    assert_equal(0, nitems);
    assert_null(nanomqoFaceGetUVs(NULL));
    assert_null(nanomqoMaterialGetAlphaPlanePath(NULL));
    assert_equal_float(0, nanomqoMaterialGetAmbient(NULL));
    assert_null(nanomqoMaterialGetBumpMapPath(NULL));
    assert_not_null(nanomqoMaterialGetColor(NULL));
    assert_equal_float(0, nanomqoMaterialGetDiffuse(NULL));
    assert_equal_float(0, nanomqoMaterialGetEmissive(NULL));
    assert_null(nanomqoMaterialGetName(NULL));
    assert_equal_float(0, nanomqoMaterialGetPower(NULL));
    assert_not_null(nanomqoMaterialGetProjectionAngle(NULL));
    assert_not_null(nanomqoMaterialGetProjectionPosition(NULL));
    assert_not_null(nanomqoMaterialGetProjectionScale(NULL));
    assert_equal(NANOMQO_PROJECTION_TYPE_UV, nanomqoMaterialGetProjectionType(NULL));
    assert_equal_float(0, nanomqoMaterialGetReflect(NULL));
    assert_equal_float(0, nanomqoMaterialGetRefract(NULL));
    assert_equal(NANOMQO_SHADER_TYPE_CLASSIC, nanomqoMaterialGetShaderType(NULL));
    assert_equal_float(0, nanomqoMaterialGetSpecular(NULL));
    assert_null(nanomqoMaterialGetTexturePath(NULL));
    assert_equal(nanomqo_false, nanomqoMaterialHasVertexColor(NULL));
    assert_equal(nanomqo_false, nanomqoMaterialIsCullingDisabled(NULL));
    assert_not_null(nanomqoObjectGetColor(NULL));
    assert_equal(0, nanomqoObjectGetColorType(NULL));
    assert_equal(0, nanomqoObjectGetDepth(NULL));
    assert_null(nanomqoObjectGetFaces(NULL, &nitems));
    assert_equal(0, nitems);
    assert_equal_float(0, nanomqoObjectGetFacet(NULL));
    assert_equal(0, nanomqoObjectGetLatheAxis(NULL));
    assert_equal(NANOMQO_LATHE_TYPE_NONE, nanomqoObjectGetLatheType(NULL));
    assert_equal(0, nanomqoObjectGetMirrorAxis(NULL));
    assert_equal_float(0, nanomqoObjectGetMirrorDistance(NULL));
    assert_equal(NANOMQO_MIRROR_TYPE_NONE, nanomqoObjectGetMirrorType(NULL));
    assert_null(nanomqoObjectGetName(NULL));
    assert_equal(0, nanomqoObjectGetNumLatheSegments(NULL));
    assert_equal(0, nanomqoObjectGetNumSegments(NULL));
    assert_not_null(nanomqoObjectGetOrientation(NULL));
    assert_equal(0, nanomqoObjectGetPatchTriangulationType(NULL));
    assert_equal(NANOMQO_PATCH_TYPE_PLANE, nanomqoObjectGetPatchType(NULL));
    assert_not_null(nanomqoObjectGetScale(NULL));
    assert_equal(NANOMQO_SHADING_TYPE_FLAT, nanomqoObjectGetShadingType(NULL));
    assert_not_null(nanomqoObjectGetTranslation(NULL));
    assert_equal(0, nanomqoObjectGetUid(NULL));
    assert_null(nanomqoObjectGetVertices(NULL, &nitems));
    assert_equal(0, nitems);
    assert_equal(nanomqo_false, nanomqoObjectIsFolding(NULL));
    assert_equal(nanomqo_false, nanomqoObjectIsLocked(NULL));
    assert_equal(nanomqo_false, nanomqoObjectIsVisible(NULL));
    assert_not_null(nanomqoSceneGetAmbient(NULL));
    assert_not_null(nanomqoSceneGetAngle(NULL));
    assert_not_null(nanomqoSceneGetLookAt(NULL));
    assert_equal_float(0, nanomqoSceneGetPerspective(NULL));
    assert_not_null(nanomqoSceneGetPosition(NULL));
    assert_equal_float(0, nanomqoSceneGetZoom(NULL));
    assert_equal_float(0, nanomqoSceneGetZoom2(NULL));
    assert_equal(nanomqo_false, nanomqoSceneIsOrtho(NULL));
    assert_equal(0, nanomqoVertexGetColor(NULL));
    assert_not_null(nanomqoVertexGetOrigin(NULL));
    assert_equal_float(0, nanomqoVertexGetWeight(NULL));
#endif
}

static void
nanomqoLoadFile(const char *filename)
{
    FILE *fp;
    long size;
    nanomqo_object_t *const *objects;
    nanomqo_material_t *const *materials;
    nanomqo_vertex_t *const *vertices;
    nanomqo_face_t *const *faces;
    const nanomqo_object_t *object;
    const nanomqo_material_t *material;
    nanomqo_uint8_t *bytes;
    nanomqo_buffer_t *buffer;
    nanomqo_document_t *document;
    nanomqo_rsize_t read_size;
    nanomqo_status_t rc;
    char *ptr = NULL;
    int nvertices, nfaces, nobjects, nindices, i, j;
    fp = fopen(filename, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        bytes = (nanomqo_uint8_t *) malloc(size);
        read_size = fread(bytes, 1, size, fp);
        fclose(fp);
        buffer = nanomqoBufferCreate(bytes, size);
        document = nanomqoDocumentCreate();
        rc = nanomqoDocumentParse(document, buffer);
        if (rc == NANOMQO_STATUS_SUCCESS) {
            objects = nanomqoDocumentGetObjects(document, &nobjects);
            fprintf(stderr, "row = %d column = %d\n", nanomqoBufferGetRowOffset(buffer), nanomqoBufferGetColumnOffset(buffer));
            for (i = 0; i < nobjects; i++) {
                object = objects[i];
                vertices = nanomqoObjectGetVertices(object, &nvertices);
                faces = nanomqoObjectGetFaces(object, &nfaces);
                if (nvertices > 0 && nfaces > 0) {
                    fprintf(stderr, "  object[%d]: name = %s vertices = %p faces = %p nvertices = %d nfaces = %d\n",
                        i,
                        nanomqoConvertString(nanomqoObjectGetName(object), &ptr),
                        (const void *) vertices,
                        (const void *) faces,
                        nvertices,
                        nfaces);
                    fprintf(stderr, "            : color = (%.2f, %.2f, %.2f) translation = (%.2f, %.2f, %.2f) orientation = (%.2f, %.2f, %.2f)\n",
                        nanomqoObjectGetColor(object)[0],
                        nanomqoObjectGetColor(object)[1],
                        nanomqoObjectGetColor(object)[2],
                        nanomqoObjectGetTranslation(object)[0],
                        nanomqoObjectGetTranslation(object)[1],
                        nanomqoObjectGetTranslation(object)[2],
                        nanomqoObjectGetOrientation(object)[0],
                        nanomqoObjectGetOrientation(object)[1],
                        nanomqoObjectGetOrientation(object)[2]);
                    fprintf(stderr, "            : depth = %d visible = %d locked = %d folding = %d\n",
                        nanomqoObjectGetDepth(object),
                        nanomqoObjectIsVisible(object),
                        nanomqoObjectIsLocked(object),
                        nanomqoObjectIsFolding(object));
                    fprintf(stderr, "            : facet = %.2f axis = %s lathe = %s segments = %d\n",
                        nanomqoObjectGetFacet(object),
                        nanomqoAxisTypeToString(nanomqoObjectGetLatheAxis(object)),
                        nanomqoLatheTypeToString(nanomqoObjectGetLatheType(object)),
                        nanomqoObjectGetNumLatheSegments(object));
                    fprintf(stderr, "            : axis = %s distance = %.2f type = %s\n",
                        nanomqoAxisTypeToString(nanomqoObjectGetMirrorAxis(object)),
                        nanomqoObjectGetMirrorDistance(object),
                        nanomqoMirrorTypeToString(nanomqoObjectGetMirrorType(object)));
                    fprintf(stderr, "            : patch = %s shading = %s\n",
                        nanomqoPatchTypeToString(nanomqoObjectGetPatchType(object)),
                        nanomqoShadingTypeToString(nanomqoObjectGetShadingType(object)));
                    for (j = 0; j < nfaces; j++) {
                        nanomqoFaceGetVertexIndices(faces[j], &nindices);
                    }
                }
            }
            materials = nanomqoDocumentGetMaterials(document, &nobjects);
            for (i = 0; i < nobjects; i++) {
                material = materials[i];
                nanomqoMaterialGetAmbient(material);
                fprintf(stderr, "  material[%04d]: name=%s\n", i,
                    nanomqoConvertString(nanomqoMaterialGetName(material), &ptr));
                fprintf(stderr, "                : color = (%.2f, %.2f, %.2f) ambient = %.2f diffuse = %.2f specular = %.2f power = %.2f\n",
                    nanomqoMaterialGetColor(material)[0],
                    nanomqoMaterialGetColor(material)[1],
                    nanomqoMaterialGetColor(material)[2],
                    nanomqoMaterialGetAmbient(material),
                    nanomqoMaterialGetDiffuse(material),
                    nanomqoMaterialGetSpecular(material),
                    nanomqoMaterialGetPower(material));
                fprintf(stderr, "                : angle = (%.2f, %.2f, %.2f) position = (%.2f, %.2f, %.2f) scale = (%.2f, %.2f, %.2f)\n",
                    nanomqoMaterialGetProjectionAngle(material)[0],
                    nanomqoMaterialGetProjectionAngle(material)[1],
                    nanomqoMaterialGetProjectionAngle(material)[2],
                    nanomqoMaterialGetProjectionPosition(material)[0],
                    nanomqoMaterialGetProjectionPosition(material)[1],
                    nanomqoMaterialGetProjectionPosition(material)[2],
                    nanomqoMaterialGetProjectionScale(material)[0],
                    nanomqoMaterialGetProjectionScale(material)[1],
                    nanomqoMaterialGetProjectionScale(material)[2]);
                fprintf(stderr, "                : reflect = %.2f refract = %.2f vcolor = %d culling = %d\n",
                    nanomqoMaterialGetReflect(material),
                    nanomqoMaterialGetRefract(material),
                    nanomqoMaterialHasVertexColor(material),
                    nanomqoMaterialIsCullingDisabled(material) ? 0 : 1);
                fprintf(stderr, "                : plane = %s bump = %s texture = %s\n",
                    nanomqoConvertString(nanomqoMaterialGetAlphaPlanePath(material), &ptr),
                    nanomqoConvertString(nanomqoMaterialGetBumpMapPath(material), &ptr),
                    nanomqoConvertString(nanomqoMaterialGetTexturePath(material), &ptr));
                fprintf(stderr, "                : projection = %s shader = %s\n",
                    nanomqoProjectionTypeToString(nanomqoMaterialGetProjectionType(material)),
                    nanomqoShaderTypeToString(nanomqoMaterialGetShaderType(material)));
            }
            free(ptr);
        }
        fprintf(stderr, "status = %d size = %ld\n", rc, read_size);
        nanomqoDocumentDestroy(document);
        nanomqoBufferDestroy(buffer);
        free(bytes);
    }
}

static void
nanomqoTestDynamicModelFixtures(FILE *fp)
{
    char path[1024], *p;
    while (fgets(path, sizeof(path), fp) != NULL) {
        p = memchr(path, '\n', sizeof(path));
        if (p) {
            *p = '\0';
        }
        fprintf(stderr, "test: %s\n", path);
        nanomqoLoadFile(path);
    }
    fclose(fp);
}

static void
nanomqoTestStaticModelFixtures(void)
{
    static const char *filenames[] = {
        "testdata/NihonbasiKirin.mqo",
        "testdata/arm.mqo",
        "testdata/bear.mqo",
        "testdata/body.mqo",
        "testdata/cat.mqo",
        "testdata/chair.mqo",
        "testdata/dice.mqo",
        "testdata/fly.mqo",
        "testdata/gourd.mqo",
        "testdata/hand.mqo",
        "testdata/kame.mqo",
        "testdata/meka.mqo",
        "testdata/nas.mqo",
        "testdata/pig.mqo",
        "testdata/sakana.mqo",
        "testdata/shield.mqo",
        "testdata/swan.mqo",
        "testdata/tank.mqo",
        "testdata/train.mqo",
        "testdata/violin.mqo",
        "testdata/witch.mqo",
        /* "testdata/donuts.mqo", */
        NULL
    };
    const char *filename;
    int i;
    nanomqoTestNull();
    for (i = 0; filenames[i]; i++) {
        filename = filenames[i];
        fprintf(stderr, "test: %s\n", filename);
        nanomqoLoadFile(filename);
    }
}
static void *
nanomqoTestMalloc(void *opaque, size_t size)
{
    nanomqo_mark_unused(opaque);
    return malloc(size);
}

static void *
nanomqoTestCalloc(void *opaque, size_t length, size_t size)
{
    nanomqo_mark_unused(opaque);
    return calloc(length, size);
}

static void *
nanomqoTestRealloc(void *opaque, void *ptr, size_t size)
{
    nanomqo_mark_unused(opaque);
    return realloc(ptr, size);
}

static void
nanomqoTestFree(void *opaque, void *ptr)
{
    nanomqo_mark_unused(opaque);
    free(ptr);
}

int
main(void)
{
    FILE *model_fp = NULL;
    nanomqo_global_allocator_t allocator = {
        NULL,
        nanomqoTestMalloc,
        nanomqoTestCalloc,
        nanomqoTestRealloc,
        nanomqoTestFree
    };
    char *value = getenv("NANOMQO_FIXTURES_PATH");
    if (value) {
        model_fp = fopen(value, "r");
    }
    nanomqoGlobalSetCustomAllocator(&allocator);
    if (model_fp) {
        nanomqoTestDynamicModelFixtures(model_fp);
    }
    else {
        nanomqoTestStaticModelFixtures();
    }
    return 0;
}
