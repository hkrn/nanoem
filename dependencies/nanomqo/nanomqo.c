/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "./nanomqo_p.h"

static const char __nanomqo_header_signature[] = "Metasequoia Document";
static const char __nanomqo_header_version_decl[] = "Format Text Ver ";
static const char __nanomqo_material_chunk_decl[] = "Material";
static const char __nanomqo_material_ex2_chunk_decl[] = "MaterialEx2";
static const nanomqo_float32_t __nanomqo_null_vector3[] = { 0, 0, 0, 0 };
static const nanomqo_float32_t __nanomqo_black[] = { 0, 0, 0, 1 };

void APIENTRY
nanomqoGlobalSetCustomAllocator(const nanomqo_global_allocator_t *allocator)
{
    if (nanomqo_is_not_null(allocator)) {
        __nanomqo_global_allocator = allocator;
    }
    else {
        __nanomqo_global_allocator = NULL;
    }
}

nanomqo_buffer_t *APIENTRY
nanomqoBufferCreate(const nanomqo_uint8_t *data, nanomqo_rsize_t length)
{
    nanomqo_buffer_t *buffer;
    buffer = (nanomqo_buffer_t *) nanomqo_calloc(1, sizeof(*buffer));
    if (nanomqo_is_not_null(buffer)) {
        buffer->data = data;
        buffer->length = length;
        buffer->rows = 1;
    }
    return buffer;
}

nanomqo_rsize_t APIENTRY
nanomqoBufferGetLength(const nanomqo_buffer_t *buffer)
{
    return nanomqo_is_not_null(buffer) ? buffer->length : 0;
}

nanomqo_rsize_t APIENTRY
nanomqoBufferGetOffset(const nanomqo_buffer_t *buffer)
{
    return nanomqo_is_not_null(buffer) ? buffer->offset : 0;
}

const nanomqo_uint8_t *APIENTRY
nanomqoBufferGetDataPtr(const nanomqo_buffer_t *buffer)
{
    return (nanomqo_is_not_null(buffer) && buffer->offset < buffer->length) ? (buffer->data + buffer->offset) : NULL;
}

int APIENTRY
nanomqoBufferGetRowOffset(const nanomqo_buffer_t *buffer)
{
    return nanomqo_is_not_null(buffer) ? buffer->rows : 0;
}

int APIENTRY
nanomqoBufferGetColumnOffset(const nanomqo_buffer_t *buffer)
{
    return nanomqo_is_not_null(buffer) ? buffer->columns : 0;
}

nanomqo_bool_t APIENTRY
nanomqoBufferCanReadLength(const nanomqo_buffer_t *buffer, nanomqo_rsize_t size)
{
    return size > 0 && nanomqo_is_not_null(buffer) && buffer->length >= buffer->offset && buffer->length - buffer->offset >= size;
}

void APIENTRY
nanomqoBufferSkip(nanomqo_buffer_t *buffer, nanomqo_rsize_t skip, nanomqo_bool_t *ok)
{
    *ok = nanomqoBufferCanReadLength(buffer, skip);
    if (*ok) {
        buffer->columns += (int) skip;
        buffer->offset += skip;
    }
}

void APIENTRY
nanomqoBufferDestroy(nanomqo_buffer_t *buffer)
{
    if (nanomqo_is_not_null(buffer)) {
        nanomqo_free(buffer);
    }
}

const nanomqo_float32_t *APIENTRY
nanomqoVertexGetOrigin(const nanomqo_vertex_t *vertex)
{
    return nanomqo_is_not_null(vertex) ? vertex->origin : __nanomqo_null_vector3;
}

nanomqo_float32_t APIENTRY
nanomqoVertexGetWeight(const nanomqo_vertex_t *vertex)
{
    return nanomqo_is_not_null(vertex) ? vertex->weight : 0;
}

nanomqo_uint32_t APIENTRY
nanomqoVertexGetColor(const nanomqo_vertex_t *vertex)
{
    return nanomqo_is_not_null(vertex) ? vertex->color : 0;
}

const int *APIENTRY
nanomqoFaceGetVertexIndices(const nanomqo_face_t *face, int *length)
{
    if (nanomqo_is_not_null(face)) {
        *length = face->num_vertex_indices;
        return face->vertex_indices;
    }
    else {
        *length = 0;
        return NULL;
    }
}

int APIENTRY
nanomqoFaceGetMaterialIndex(const nanomqo_face_t *face)
{
    return nanomqo_is_not_null(face) ? face->material_index : 0;
}

const nanomqo_float32_t *APIENTRY
nanomqoFaceGetUVs(const nanomqo_face_t *face)
{
    return nanomqo_is_not_null(face) ? face->uv_values : 0;
}

const nanomqo_float32_t *APIENTRY
nanomqoFaceGetCreases(const nanomqo_face_t *face)
{
    return nanomqo_is_not_null(face) ? face->crease_values : 0;
}

const nanomqo_uint32_t *APIENTRY
nanomqoFaceGetColors(const nanomqo_face_t *face)
{
    return nanomqo_is_not_null(face) ? face->color_values : 0;
}

int APIENTRY
nanomqoFaceGetUID(const nanomqo_face_t *face)
{
    return nanomqo_is_not_null(face) ? face->uid : 0;
}

const char *APIENTRY
nanomqoMaterialGetName(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->name : NULL;
}

const char *APIENTRY
nanomqoMaterialGetAlphaPlanePath(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->alpha_plane_path : NULL;
}

const char *APIENTRY
nanomqoMaterialGetBumpMapPath(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->bump_map_path : NULL;
}

const char *APIENTRY
nanomqoMaterialGetTexturePath(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->texture_path : NULL;
}

const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetProjectionPosition(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->projection_position : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetProjectionScale(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->projection_scale : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetProjectionAngle(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->projection_angle : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetColor(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->color : __nanomqo_black;
}

const nanomqo_float32_t *APIENTRY
nanomqoMaterialGetAmbientColor(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->ambient_color : __nanomqo_black;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetAmbient(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->ambient : 0;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetDiffuse(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->diffusion : 0;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetEmissive(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->emissive : 0;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetSpecular(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->specular : 0;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetPower(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->power : 0;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetReflect(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->reflect : 0;
}

nanomqo_float32_t APIENTRY
nanomqoMaterialGetRefract(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->refract : 0;
}

nanomqo_shader_type_t APIENTRY
nanomqoMaterialGetShaderType(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->shader_type : NANOMQO_SHADER_TYPE_CLASSIC;
}

nanomqo_projection_type_t APIENTRY
nanomqoMaterialGetProjectionType(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->projection_type : NANOMQO_PROJECTION_TYPE_UV;
}

nanomqo_bool_t APIENTRY
nanomqoMaterialHasVertexColor(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->has_vertex_color != 0 : nanomqo_false;
}

nanomqo_bool_t APIENTRY
nanomqoMaterialIsCullingDisabled(const nanomqo_material_t *material)
{
    return nanomqo_is_not_null(material) ? material->disable_culling != 0 : nanomqo_false;
}

const char *APIENTRY
nanomqoObjectGetName(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->name : NULL;
}

const nanomqo_float32_t *APIENTRY
nanomqoObjectGetScale(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->scale : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoObjectGetTranslation(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->translation : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoObjectGetOrientation(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->orientation : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoObjectGetColor(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->color : __nanomqo_black;
}

int APIENTRY
nanomqoObjectGetUID(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->uid : 0;
}

int APIENTRY
nanomqoObjectGetDepth(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->depth : 0;
}

nanomqo_patch_type_t APIENTRY
nanomqoObjectGetPatchType(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->patch_type : NANOMQO_PATCH_TYPE_PLANE;
}

int APIENTRY
nanomqoObjectGetPatchTriangulationType(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->patch_triangulation_type : 0;
}

int APIENTRY
nanomqoObjectGetNumSegments(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->num_segments : 0;
}

nanomqo_shading_type_t APIENTRY
nanomqoObjectGetShadingType(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->shading_type : NANOMQO_SHADING_TYPE_FLAT;
}

nanomqo_float32_t APIENTRY
nanomqoObjectGetFacet(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->facet : 0;
}

int APIENTRY
nanomqoObjectGetColorType(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->color_type : 0;
}

nanomqo_mirror_type_t APIENTRY
nanomqoObjectGetMirrorType(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->mirror_type : NANOMQO_MIRROR_TYPE_NONE;
}

int APIENTRY
nanomqoObjectGetMirrorAxis(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->mirror_axis : 0;
}

nanomqo_float32_t APIENTRY
nanomqoObjectGetMirrorDistance(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->mirror_distance : 0;
}

nanomqo_lathe_type_t APIENTRY
nanomqoObjectGetLatheType(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->lathe_type : NANOMQO_LATHE_TYPE_NONE;
}

nanomqo_axis_type_t APIENTRY
nanomqoObjectGetLatheAxis(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->lathe_axis : NANOMQO_AXIS_TYPE_X;
}

int APIENTRY
nanomqoObjectGetNumLatheSegments(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->num_lathe_segments : 0;
}

nanomqo_bool_t APIENTRY
nanomqoObjectIsVisible(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->is_visible != 0 : nanomqo_false;
}

nanomqo_bool_t APIENTRY
nanomqoObjectIsLocked(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->is_locked != 0 : nanomqo_false;
}

nanomqo_bool_t APIENTRY
nanomqoObjectIsFolding(const nanomqo_object_t *object)
{
    return nanomqo_is_not_null(object) ? object->is_folding != 0 : nanomqo_false;
}

nanomqo_vertex_t *const *APIENTRY
nanomqoObjectGetAllVertices(const nanomqo_object_t *object, nanomqo_rsize_t *nvertices)
{
    if (nanomqo_is_not_null(object)) {
        *nvertices = object->num_vertices;
        return object->vertices;
    }
    else {
        *nvertices = 0;
        return NULL;
    }
}

const int *APIENTRY
nanomqoObjectGetAllVertexAttributeUID(const nanomqo_object_t *object, nanomqo_rsize_t *nuids)
{
    if (nanomqo_is_not_null(object)) {
        *nuids = object->num_vertex_attr_uids;
        return object->vertex_attr_uids;
    }
    else {
        *nuids = 0;
        return NULL;
    }
}

nanomqo_face_t *const *APIENTRY
nanomqoObjectGetAllFaces(const nanomqo_object_t *object, nanomqo_rsize_t *nfaces)
{
    if (nanomqo_is_not_null(object)) {
        *nfaces = object->num_faces;
        return object->faces;
    }
    else {
        *nfaces = 0;
        return NULL;
    }
}

const nanomqo_float32_t *APIENTRY
nanomqoSceneGetAmbient(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->ambient : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoSceneGetPosition(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->position : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoSceneGetLookAt(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->lookat : __nanomqo_null_vector3;
}

const nanomqo_float32_t *APIENTRY
nanomqoSceneGetAngle(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->angle : __nanomqo_null_vector3;
}

nanomqo_float32_t APIENTRY
nanomqoSceneGetZoom(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->zoom : 0;
}

nanomqo_float32_t APIENTRY
nanomqoSceneGetZoom2(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->zoom2 : 0;
}

nanomqo_float32_t APIENTRY
nanomqoSceneGetPerspective(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->perspective : 0;
}

nanomqo_float32_t APIENTRY
nanomqoSceneGetFrontClip(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->frontclip : 0.0f;
}

nanomqo_float32_t APIENTRY
nanomqoSceneGetBackClip(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->backclip : 0.0f;
}

nanomqo_bool_t APIENTRY
nanomqoSceneIsOrtho(const nanomqo_scene_t *scene)
{
    return nanomqo_is_not_null(scene) ? scene->is_ortho != 0 : nanomqo_false;
}

nanomqo_document_t *APIENTRY
nanomqoDocumentCreate(void)
{
    nanomqo_document_t *document;
    document = (nanomqo_document_t *) nanomqo_calloc(1, sizeof(*document));
    if (nanomqo_is_not_null(document)) {
        document->num_allocated_objects = 8;
        document->objects = (nanomqo_object_t **) nanomqo_calloc(document->num_allocated_objects, sizeof(*document->objects));
        document->scene = nanomqoSceneCreate();
    }
    return document;
}

nanomqo_status_t APIENTRY
nanomqoDocumentParse(nanomqo_document_t *document, nanomqo_buffer_t *buffer)
{
    nanomqo_object_t *object;
    nanomqo_uint8_t c;
    nanomqo_bool_t is_parsing = nanomqo_true;
    nanomqo_status_t rc;
    char input[NANOMQO_BUFFER_CAPACITY], *ptr = input;
    int num_materials, i;
    if (nanomqo_is_null(buffer)) {
        return NANOMQO_STATUS_ERROR_NULL_BUFFER;
    }
    if (nanomqo_is_null(document)) {
        return NANOMQO_STATUS_ERROR_NULL_DOCUMENT;
    }
    if (!nanomqoBufferTestString(buffer, __nanomqo_header_signature)) {
        return NANOMQO_STATUS_ERROR_INVALID_SIGNATURE;
    }
    nanomqoBufferSkipLine(buffer);
    nanomqoBufferGetLine(buffer, input, sizeof(input));
    if (!nanomqo_string_equals_const(input, __nanomqo_header_version_decl)) {
        return NANOMQO_STATUS_ERROR_INVALID_VERSION;
    }
    ptr += sizeof(__nanomqo_header_version_decl) - 1;
    document->version = nanomqo_strtof(ptr, &ptr);
    nanomqoBufferSkipSpaces(buffer);
    while (is_parsing) {
        c = nanomqoBufferGetChar(buffer);
        switch (c) {
        case 'B':
            if (nanomqoBufferTestString(buffer, "BackImage")) {
                /* XXX: BackImage */
                buffer->depth++;
                nanomqoBufferSkipLine(buffer);
                nanomqoBufferSkipUntilEndChunk(buffer);
            }
            else if (nanomqoBufferTestString(buffer, "Blob")) {
                /* XXX: Blob */
                buffer->depth++;
                nanomqoBufferSkipLine(buffer);
                nanomqoBufferSkipUntilEndChunk(buffer);
            }
            break;
        case 'E':
            if (nanomqoBufferTestString(buffer, "Eof")) {
                is_parsing = nanomqo_false;
            }
            else {
                return NANOMQO_STATUS_ERROR_INVALID_EOF;
            }
            break;
        case 'I':
            if (nanomqoBufferTestString(buffer, "IncludeXml")) {
                nanomqoBufferSkipLine(buffer);
            }
            break;
        case 'M':
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            if (nanomqo_string_equals_const(input, __nanomqo_material_ex2_chunk_decl)) {
                ptr = input + sizeof(__nanomqo_material_ex2_chunk_decl) - 1;
                nanomqoBufferSkipUntilEndChunk(buffer);
            }
            else if (nanomqo_string_equals_const(input, __nanomqo_material_chunk_decl)) {
                ptr = input + sizeof(__nanomqo_material_chunk_decl) - 1;
                nanomqoBufferExtractInt(buffer, ptr, &num_materials);
                if (num_materials > 0) {
                    document->num_materials = num_materials;
                    document->materials = (nanomqo_material_t **) nanomqo_calloc(num_materials, sizeof(*document->materials));
                    for (i = 0; i < num_materials; i++) {
                        document->materials[i] = nanomqoMaterialCreate();
                        rc = nanomqoMaterialParse(document->materials[i], buffer);
                        if (nanomqo_has_error(rc)) {
                            return rc;
                        }
                    }
                }
                nanomqoBufferSkipUntilEndChunk(buffer);
            }
            break;
        case 'O':
            object = nanomqoObjectCreate();
            rc = nanomqoObjectParse(object, buffer);
            if (nanomqo_has_error(rc)) {
                nanomqoObjectDestroy(object);
                return rc;
            }
            document->num_objects++;
            if (document->num_objects > document->num_allocated_objects) {
                document->num_allocated_objects <<= 1;
                document->objects = (nanomqo_object_t **) nanomqo_realloc(document->objects, document->num_allocated_objects * sizeof(*document->objects));
            }
            if (nanomqo_is_not_null(document->objects) && nanomqo_is_not_null(object)) {
                document->objects[document->num_objects - 1] = object;
            }
            break;
        case 'S':
            rc = nanomqoSceneParse(document->scene, buffer);
            if (nanomqo_has_error(rc)) {
                return rc;
            }
            break;
        case 'T':
            if (nanomqoBufferTestString(buffer, "Thumbnail")) {
                /* XXX: Thumbnail */
                buffer->depth++;
                nanomqoBufferSkipLine(buffer);
                nanomqoBufferSkipUntilEndChunk(buffer);
            }
            break;
        case '\r':
        case '\n':
            nanomqoBufferSkipLine(buffer);
            break;
        default:
            nanomqoBufferGetLine(buffer, input, sizeof(input));
            return NANOMQO_STATUS_ERROR_UNKNOWN_CHUNK_TYPE;
        }
    }
    return NANOMQO_STATUS_SUCCESS;
}

nanomqo_float32_t APIENTRY
nanomqoDocumentGetVersion(const nanomqo_document_t *document)
{
    return nanomqo_is_not_null(document) ? document->version : 0;
}

const nanomqo_scene_t *APIENTRY
nanomqoDocumentGetScene(const nanomqo_document_t *document)
{
    return nanomqo_is_not_null(document) ? document->scene : NULL;
}

nanomqo_material_t *const *APIENTRY
nanomqoDocumentGetAllMaterials(const nanomqo_document_t *document, nanomqo_rsize_t *nmaterials)
{
    if (nanomqo_is_not_null(document)) {
        *nmaterials = document->num_materials;
        return document->materials;
    }
    else {
        *nmaterials = 0;
        return NULL;
    }
}

nanomqo_object_t *const *APIENTRY
nanomqoDocumentGetAllObjects(const nanomqo_document_t *document, nanomqo_rsize_t *nobjects)
{
    if (nanomqo_is_not_null(document)) {
        *nobjects = document->num_objects;
        return document->objects;
    }
    else {
        *nobjects = 0;
        return NULL;
    }
}

void APIENTRY
nanomqoDocumentDestroy(nanomqo_document_t *document)
{
    int num_materials, num_objects, i;
    if (nanomqo_is_not_null(document)) {
        if (nanomqo_is_not_null(document->materials)) {
            num_materials = document->num_materials;
            for (i = 0; i < num_materials; i++) {
                nanomqoMaterialDestroy(document->materials[i]);
            }
            nanomqo_free(document->materials);
        }
        if (nanomqo_is_not_null(document->objects)) {
            num_objects = document->num_objects;
            for (i = 0; i < num_objects; i++) {
                nanomqoObjectDestroy(document->objects[i]);
            }
            nanomqo_free(document->objects);
        }
        nanomqoSceneDestroy(document->scene);
        nanomqo_free(document);
    }
}
