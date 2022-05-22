/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "./nanodxm_p.h"

const nanodxm_global_allocator_t *__nanodxm_global_allocator = NULL;

static const nanodxm_color_t __nanodxm_color_black = { 0, 0, 0, 1 };
static const nanodxm_color_t __nanodxm_color_white = { 1, 1, 1, 1 };

const nanodxm_global_allocator_t *APIENTRY
nanodxmGlobalGetCustomAllocator(void)
{
    return __nanodxm_global_allocator;
}

void APIENTRY
nanodxmGlobalSetCustomAllocator(const nanodxm_global_allocator_t *allocator)
{
    if (nanodxm_is_not_null(allocator)) {
        __nanodxm_global_allocator = allocator;
    }
    else {
        __nanodxm_global_allocator = NULL;
    }
}

nanodxm_buffer_t *APIENTRY
nanodxmBufferCreate(const nanodxm_uint8_t *data, nanodxm_rsize_t length)
{
    nanodxm_buffer_t *buffer;
    buffer = (nanodxm_buffer_t *) nanodxm_calloc(1, sizeof(*buffer));
    if (nanodxm_is_not_null(buffer)) {
        buffer->data = data;
        buffer->length = length;
        buffer->rows = 1;
    }
    return buffer;
}

nanodxm_rsize_t APIENTRY
nanodxmBufferGetLength(const nanodxm_buffer_t *buffer)
{
    return nanodxm_is_not_null(buffer) ? buffer->length : 0;
}

nanodxm_rsize_t APIENTRY
nanodxmBufferGetOffset(const nanodxm_buffer_t *buffer)
{
    return nanodxm_is_not_null(buffer) ? buffer->offset : 0;
}

const nanodxm_uint8_t *APIENTRY
nanodxmBufferGetDataPtr(const nanodxm_buffer_t *buffer)
{
    return (nanodxm_is_not_null(buffer) && buffer->offset < buffer->length) ? (buffer->data + buffer->offset) : NULL;
}

int APIENTRY
nanodxmBufferGetRowOffset(const nanodxm_buffer_t *buffer)
{
    return nanodxm_is_not_null(buffer) ? buffer->rows : 0;
}

int APIENTRY
nanodxmBufferGetColumnOffset(const nanodxm_buffer_t *buffer)
{
    return nanodxm_is_not_null(buffer) ? buffer->columns : 0;
}

nanodxm_bool_t APIENTRY
nanodxmBufferCanReadLength(const nanodxm_buffer_t *buffer, nanodxm_rsize_t size)
{
    return size > 0 && nanodxm_is_not_null(buffer) && buffer->length >= buffer->offset && buffer->length - buffer->offset >= size;
}

void APIENTRY
nanodxmBufferSkip(nanodxm_buffer_t *buffer, nanodxm_rsize_t skip, nanodxm_bool_t *ok)
{
    *ok = nanodxmBufferCanReadLength(buffer, skip);
    if (*ok) {
        buffer->columns += (int) skip;
        buffer->offset += skip;
    }
}

void APIENTRY
nanodxmBufferDestroy(nanodxm_buffer_t *buffer)
{
    if (nanodxm_is_not_null(buffer)) {
        nanodxm_free(buffer);
    }
}

const int *APIENTRY
nanodxmFaceGetIndices(const nanodxm_face_t *face, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(face)) {
        *length = face->num_indices;
        return face->indices;
    }
    else {
        *length = 0;
        return NULL;
    }
}

nanodxm_material_t *
nanodxmMaterialCreate(void)
{
    nanodxm_material_t *material;
    material = (nanodxm_material_t *) nanodxm_calloc(1, sizeof(*material));
    if (nanodxm_is_not_null(material)) {
        material->diffuse = __nanodxm_color_white;
        material->emissive.a = 1.0f;
        material->specular.a = 1.0f;
    }
    return material;
}

void
nanodxmMaterialDestroy(nanodxm_material_t *material)
{
    if (nanodxm_is_not_null(material)) {
        if (material->texture_filename) {
            nanodxm_free(material->texture_filename);
        }
        if (material->normal_map_filename) {
            nanodxm_free(material->normal_map_filename);
        }
        nanodxm_free(material);
    }
}

nanodxm_color_t APIENTRY
nanodxmMaterialGetDiffuse(const nanodxm_material_t *material)
{
    if (nanodxm_is_not_null(material)) {
        return material->diffuse;
    }
    else {
        return __nanodxm_color_white;
    }
}

nanodxm_color_t APIENTRY
nanodxmMaterialGetEmissive(const nanodxm_material_t *material)
{
    if (nanodxm_is_not_null(material)) {
        return material->emissive;
    }
    else {
        return __nanodxm_color_black;
    }
}

nanodxm_color_t APIENTRY
nanodxmMaterialGetSpecular(const nanodxm_material_t *material)
{
    if (nanodxm_is_not_null(material)) {
        return material->specular;
    }
    else {
        return __nanodxm_color_black;
    }
}

nanodxm_float32_t APIENTRY
nanodxmMaterialGetShininess(const nanodxm_material_t *material)
{
    if (nanodxm_is_not_null(material)) {
        return material->shininess;
    }
    else {
        return 0.0f;
    }
}

const nanodxm_uint8_t *APIENTRY
nanodxmMaterialGetTextureFilename(const nanodxm_material_t *material)
{
    return nanodxm_is_not_null(material) ? material->texture_filename : NULL;
}

const nanodxm_uint8_t *APIENTRY
nanodxmMaterialGetNormalMapFilename(const nanodxm_material_t *material)
{
    return nanodxm_is_not_null(material) ? material->normal_map_filename : NULL;
}

nanodxm_document_t *APIENTRY
nanodxmDocumentCreate(void)
{
    nanodxm_document_t *document;
    document = (nanodxm_document_t *) nanodxm_calloc(1, sizeof(*document));
    if (nanodxm_is_not_null(document)) {
        kv_init(document->materials);
    }
    return document;
}

static void
nanodxmBufferSkipBlock(nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    state->depth++;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferSkipUntilEndChunk(buffer, state);
}

static void
nanodxmDocumentParseMeshMaterialList(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_rsize_t i, num_materials, length, value;
    nanodxmBufferSkipLine(buffer);
    state->depth++;
    nanodxmBufferExtractSize(buffer, &num_materials);
    nanodxmBufferSkipSeparator(buffer);
    nanodxmBufferExtractSize(buffer, &document->num_face_material_indices);
    nanodxmBufferSkipSeparator(buffer);
    if (document->face_material_indices) {
        nanodxm_free(document->face_material_indices);
    }
    document->face_material_indices = (int *) nanodxm_calloc(document->num_face_material_indices, sizeof(*document->face_material_indices));
    length = document->num_face_material_indices;
    for (i = 0; i < length; i++) {
        nanodxmBufferExtractSize(buffer, &value);
        if (value < num_materials) {
            document->face_material_indices[i] = (int) value;
        }
        nanodxmBufferSkipSeparator(buffer);
    }
    nanodxmBufferSkipSpaces(buffer);
}

static void
nanodxmDocumentParseMeshNormals(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_face_t *face;
    nanodxm_vector3_t *vertex;
    nanodxm_rsize_t i, j, length;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferExtractSize(buffer, &document->num_normals);
    nanodxmBufferSkipSeparator(buffer);
    state->depth++;
    if (document->normals) {
        nanodxm_free(document->normals);
    }
    document->normals = (nanodxm_vector3_t *) nanodxm_calloc(document->num_normals, sizeof(*document->normals));
    length = document->num_normals;
    for (i = 0; i < length; i++) {
        vertex = &document->normals[i];
        nanodxmBufferExtractFloat(buffer, &vertex->x);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &vertex->y);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &vertex->z);
        nanodxmBufferSkipSeparator(buffer);
    }
    if (document->normal_faces) {
        length = document->num_normal_faces;
        for (i = 0; i < length; i++) {
            nanodxm_free(document->normal_faces[length - i - 1].indices);
        }
        nanodxm_free(document->normal_faces);
    }
    nanodxmBufferExtractSize(buffer, &document->num_normal_faces);
    nanodxmBufferSkipSeparator(buffer);
    document->normal_faces = (nanodxm_face_t *) nanodxm_calloc(document->num_normal_faces, sizeof(*document->normal_faces));
    length = document->num_normal_faces;
    for (i = 0; i < length; i++) {
        face = &document->normal_faces[i];
        nanodxmBufferExtractSize(buffer, &face->num_indices);
        nanodxmBufferSkipSeparator(buffer);
        if (face->num_indices > 0) {
            face->indices = (int *) nanodxm_calloc(face->num_indices, sizeof(*face->indices));
            if (nanodxm_is_not_null(face->indices)) {
                for (j = 0; j < face->num_indices; j++) {
                    nanodxmBufferExtractInt(buffer, &face->indices[j]);
                    nanodxmBufferSkipSeparator(buffer);
                }
            }
            else {
                face->num_indices = 0;
            }
        }
    }
    nanodxmBufferSkipSpaces(buffer);
}

static void
nanodxmDocumentParseMeshTextureCoords(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_rsize_t i, length;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferExtractSize(buffer, &document->num_texcoords);
    nanodxmBufferSkipSeparator(buffer);
    state->depth++;
    if (document->texcoords) {
        nanodxm_free(document->texcoords);
    }
    document->texcoords = (nanodxm_texcoord_t *) nanodxm_calloc(document->num_texcoords, sizeof(*document->texcoords));
    length = document->num_texcoords;
    for (i = 0; i < length; i++) {
        nanodxm_texcoord_t *texcoords = &document->texcoords[i];
        nanodxmBufferExtractFloat(buffer, &texcoords->u);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &texcoords->v);
        nanodxmBufferSkipSeparator(buffer);
    }
    nanodxmBufferSkipUntilEndChunk(buffer, state);
}

static void
nanodxmDocumentParseMeshVertexColors(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_rsize_t i, length, vertex_index;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferExtractSize(buffer, &document->num_vertex_colors);
    nanodxmBufferSkipSeparator(buffer);
    state->depth++;
    if (document->vertex_colors) {
        nanodxm_free(document->vertex_colors);
        document->vertex_colors = NULL;
    }
    document->vertex_colors = (nanodxm_color_t *) nanodxm_calloc(document->num_vertex_colors, sizeof(*document->vertex_colors));
    length = document->num_vertex_colors;
    for (i = 0; i < length; i++) {
        nanodxm_color_t *value = &document->vertex_colors[i];
        nanodxmBufferExtractSize(buffer, &vertex_index);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &value->r);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &value->g);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &value->b);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &value->a);
        nanodxmBufferSkipSeparator(buffer);
    }
    nanodxmBufferSkipUntilEndChunk(buffer, state);
}

static void
nanodxmDocumentParseMesh(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_face_t *face;
    nanodxm_vector3_t *vertex;
    nanodxm_rsize_t i, j, length;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferExtractSize(buffer, &document->num_vertices);
    nanodxmBufferSkipSeparator(buffer);
    state->depth++;
    if (!nanodxm_is_null(document->vertices)) {
        nanodxm_free(document->vertices);
    }
    document->vertices = (nanodxm_vector3_t *) nanodxm_calloc(document->num_vertices, sizeof(*document->vertices));
    length = document->num_vertices;
    for (i = 0; i < length; i++) {
        vertex = &document->vertices[i];
        nanodxmBufferExtractFloat(buffer, &vertex->x);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &vertex->y);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &vertex->z);
        nanodxmBufferSkipSeparator(buffer);
    }
    nanodxmBufferSkipSpaces(buffer);
    if (!nanodxm_is_null(document->vertex_faces)) {
        length = document->num_vertex_faces;
        for (i = 0; i < length; i++) {
            nanodxm_free(document->vertex_faces[length - i - 1].indices);
        }
        nanodxm_free(document->vertex_faces);
    }
    nanodxmBufferExtractSize(buffer, &document->num_vertex_faces);
    nanodxmBufferSkipSeparator(buffer);
    document->vertex_faces = (nanodxm_face_t *) nanodxm_calloc(document->num_vertex_faces, sizeof(*document->vertex_faces));
    length = document->num_vertex_faces;
    for (i = 0; i < length; i++) {
        face = &document->vertex_faces[i];
        nanodxmBufferExtractSize(buffer, &face->num_indices);
        nanodxmBufferSkipSeparator(buffer);
        if (face->num_indices > 0) {
            face->indices = (int *) nanodxm_calloc(face->num_indices, sizeof(*face->indices));
            if (nanodxm_is_not_null(face->indices)) {
                for (j = 0; j < face->num_indices; j++) {
                    nanodxmBufferExtractInt(buffer, &face->indices[j]);
                    nanodxmBufferSkipSeparator(buffer);
                }
            }
            else {
                face->num_indices = 0;
            }
        }
    }
    nanodxmBufferSkipSpaces(buffer);
}

static void
nanodxmDocumentParseMaterial(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_material_t *material = nanodxmMaterialCreate();
    if (nanodxm_is_not_null(material)) {
        nanodxmBufferSkipLine(buffer);
        state->depth++;
        nanodxmBufferSkipSpaces(buffer);
        /* diffuse */
        nanodxmBufferExtractFloat(buffer, &material->diffuse.r);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->diffuse.g);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->diffuse.b);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->diffuse.a);
        nanodxmBufferSkipSeparator(buffer);
        /* specular exponent */
        nanodxmBufferExtractFloat(buffer, &material->shininess);
        nanodxmBufferSkipSeparator(buffer);
        /* specular */
        nanodxmBufferExtractFloat(buffer, &material->specular.r);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->specular.g);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->specular.b);
        nanodxmBufferSkipSeparator(buffer);
        /* emissive */
        nanodxmBufferExtractFloat(buffer, &material->emissive.r);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->emissive.g);
        nanodxmBufferSkipSeparator(buffer);
        nanodxmBufferExtractFloat(buffer, &material->emissive.b);
        nanodxmBufferSkipSeparator(buffer);
        state->current_material = material;
        kv_push(nanodxm_material_t *, document->materials, material);
    }
}

static void
nanodxmDocumentParseTextureFilename(nanodxm_buffer_t *buffer, nanodxm_uint8_t **output)
{
    nanodxm_uint8_t *ptr;
    nanodxm_rsize_t length;
    const char *str_begin, *str_end;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferSkipSpaces(buffer);
    str_begin = (const char *) nanodxmBufferGetDataPtr(buffer);
    if (str_begin && *str_begin == '"') {
        str_begin++;
        if ((str_end = strchr(str_begin, '"')) != NULL) {
            length = str_end - str_begin;
            ptr = (nanodxm_uint8_t *) nanodxm_calloc(length + 1, sizeof(*ptr));
            memcpy(ptr, str_begin, length);
            ptr[length] = '\0';
            nanodxmBufferSkipLine(buffer);
            nanodxmBufferSkipSpaces(buffer);
            *output = ptr;
        }
        else if (output) {
            *output = NULL;
        }
    }
    else if (output) {
        *output = NULL;
    }
}

static void
nanodxmDocumentParseFrame(nanodxm_document_t *document, nanodxm_buffer_t *buffer)
{
    (void) document;
    nanodxmBufferSkipLine(buffer);
    nanodxmBufferSkipSpaces(buffer);
}

static void
nanodxmDocumentParseFrameTransformMatrix(nanodxm_document_t *document, nanodxm_buffer_t *buffer, nanodxm_parser_state_t *state)
{
    nanodxm_float32_t matrix[16];
    int i;
    (void) document;
    nanodxmBufferSkipLine(buffer);
    for (i = 0; i < 16; i++) {
        nanodxmBufferExtractFloat(buffer, &matrix[i]);
        nanodxmBufferSkipSeparator(buffer);
    }
    nanodxmBufferSkipSeparator(buffer);
    nanodxmBufferSkipUntilEndChunk(buffer, state);
}

static nanodxm_status_t
nanodxmDocumentParseASCII(nanodxm_document_t *document, nanodxm_buffer_t *buffer)
{
    nanodxm_material_t *material;
    nanodxm_bool_t is_parsing = nanodxm_true;
    nanodxm_parser_state_t state;
    state.current_material = NULL;
    state.depth = 0;
    nanodxmBufferSkipLine(buffer);
    while (is_parsing) {
        switch (nanodxmBufferGetChar(buffer)) {
        case 'A':
            if (nanodxmBufferTestString(buffer, "AnimationOptions") ||
                nanodxmBufferTestString(buffer, "AnimationKey") ||
                nanodxmBufferTestString(buffer, "AnimationSet") ||
                nanodxmBufferTestString(buffer, "Animation") ||
                nanodxmBufferTestString(buffer, "AnimTicksPerSecond")) {
                nanodxmBufferSkipBlock(buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'F':
            if (nanodxmBufferTestString(buffer, "FrameTransformMatrix")) {
                nanodxmDocumentParseFrameTransformMatrix(document, buffer, &state);
            }
            else if (nanodxmBufferTestString(buffer, "Frame")) {
                nanodxmDocumentParseFrame(document, buffer);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'H':
            if (nanodxmBufferTestString(buffer, "Header")) {
                nanodxmBufferSkipBlock(buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'M':
            if (nanodxmBufferTestString(buffer, "MeshMaterialList")) {
                nanodxmDocumentParseMeshMaterialList(document, buffer, &state);
            }
            else if (nanodxmBufferTestString(buffer, "MeshNormals")) {
                nanodxmDocumentParseMeshNormals(document, buffer, &state);
            }
            else if (nanodxmBufferTestString(buffer, "MeshTextureCords") || nanodxmBufferTestString(buffer, "MeshTextureCoords")) {
                nanodxmDocumentParseMeshTextureCoords(document, buffer, &state);
            }
            else if (nanodxmBufferTestString(buffer, "MeshVertexColors")) {
                nanodxmDocumentParseMeshVertexColors(document, buffer, &state);
            }
            else if (nanodxmBufferTestString(buffer, "Mesh")) {
                nanodxmDocumentParseMesh(document, buffer, &state);
            }
            else if (nanodxmBufferTestString(buffer, "Material")) {
                nanodxmDocumentParseMaterial(document, buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'N':
            if (nanodxmBufferTestString(buffer, "NormalMapFilename")) {
                state.depth++;
                nanodxmDocumentParseTextureFilename(buffer, &state.current_material->normal_map_filename);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'S':
            if (nanodxmBufferTestString(buffer, "SkinWeights")) {
                nanodxmBufferSkipBlock(buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'T':
            if (nanodxmBufferTestString(buffer, "TextureFilename")) {
                state.depth++;
                nanodxmDocumentParseTextureFilename(buffer, &state.current_material->texture_filename);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'V':
            if (nanodxmBufferTestString(buffer, "VertexDuplicationIndices")) {
                nanodxmBufferSkipBlock(buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 'X':
            if (nanodxmBufferTestString(buffer, "XSkinMeshHeader")) {
                nanodxmBufferSkipBlock(buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case 't':
            if (nanodxmBufferTestString(buffer, "template")) {
                nanodxmBufferSkipBlock(buffer, &state);
            }
            else {
                return NANODXM_STATUS_ERROR_INVALID_TOKEN;
            }
            break;
        case '\r':
        case '\n':
        case '\t':
        case ' ':
            nanodxmBufferSkipSpaces(buffer);
            break;
        case '{':
            nanodxmBufferSkipBlock(buffer, &state);
            break;
        case nanodxm_eos:
            is_parsing = nanodxm_false;
            break;
        case '}':
            nanodxmBufferSkipLine(buffer);
            nanodxmBufferSkipSpaces(buffer);
            if (--state.depth < 0) {
                is_parsing = nanodxm_false;
            }
            break;
        case '/':
            if (nanodxmBufferTestString(buffer, "//")) {
                nanodxmBufferSkipLine(buffer);
            }
            break;
        default:
            return NANODXM_STATUS_ERROR_INVALID_TOKEN;
        }
    }
    if (kv_size(document->materials) == 0) {
        material = nanodxmMaterialCreate();
        kv_push(nanodxm_material_t *, document->materials, material);
    }
    return NANODXM_STATUS_SUCCESS;
}

nanodxm_status_t APIENTRY
nanodxmDocumentParse(nanodxm_document_t *document, nanodxm_buffer_t *buffer)
{
    nanodxm_bool_t ok;
    char version_buffer[4], float_size_buffer[4];
    if (nanodxm_is_null(buffer)) {
        return NANODXM_STATUS_ERROR_NULL_BUFFER;
    }
    if (nanodxm_is_null(document)) {
        return NANODXM_STATUS_ERROR_NULL_DOCUMENT;
    }
    if (!nanodxmBufferTestString(buffer, "xof ")) {
        return NANODXM_STATUS_ERROR_INVALID_SIGNATURE;
    }
    nanodxmBufferSkip(buffer, 4, &ok);
    if (!ok || !nanodxmBufferCanReadLength(buffer, 4)) {
        return NANODXM_STATUS_ERROR_INVALID_SIGNATURE;
    }
    memcpy(version_buffer, nanodxmBufferGetDataPtr(buffer), sizeof(version_buffer));
    nanodxmBufferSkip(buffer, 4, &ok);
    document->version_major = (version_buffer[0] - '0') * 10 + version_buffer[1] - '0';
    document->version_minor = (version_buffer[2] - '0') * 10 + version_buffer[3] - '0';
    if (nanodxmBufferTestString(buffer, "txt ")) {
        document->data_type = NANODXM_DATA_TYPE_ASCII;
    }
    else if (nanodxmBufferTestString(buffer, "bin ")) {
        document->data_type = NANODXM_DATA_TYPE_BINARY;
    }
    else if (nanodxmBufferTestString(buffer, "tzip")) {
        document->data_type = NANODXM_DATA_TYPE_ASCII_COMPRESSED;
        return NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE;
    }
    else if (nanodxmBufferTestString(buffer, "bzip")) {
        document->data_type = NANODXM_DATA_TYPE_BINARY_COMPRESSED;
        return NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE;
    }
    else {
        return NANODXM_STATUS_ERROR_INVALID_DATA_TYPE;
    }
    nanodxmBufferSkip(buffer, 4, &ok);
    if (!ok || !nanodxmBufferCanReadLength(buffer, 4)) {
        return NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE;
    }
    memcpy(float_size_buffer, nanodxmBufferGetDataPtr(buffer), sizeof(float_size_buffer));
    nanodxmBufferSkip(buffer, 4, &ok);
    document->binary_float_size = (float_size_buffer[0] - '0') * 1000 +
        (float_size_buffer[1] - '0') * 100 +
        (float_size_buffer[2] - '0') * 10 +
        (float_size_buffer[3] - '0');
    switch (document->binary_float_size) {
    case 32:
    case 64:
        break;
    default:
        return NANODXM_STATUS_ERROR_INVALID_FLOAT_SIZE;
    }
    if (document->data_type == NANODXM_DATA_TYPE_ASCII) {
        return nanodxmDocumentParseASCII(document, buffer);
    }
    else if (document->data_type == NANODXM_DATA_TYPE_BINARY) {
    }
    return NANODXM_STATUS_ERROR_NOT_SUPPORTED_DATA_TYPE;
}

nanodxm_float32_t APIENTRY
nanodxmDocumentGetVersion(const nanodxm_document_t *document)
{
    return nanodxm_is_not_null(document) ? document->version_major + document->version_minor * 0.1f : 0;
}

const nanodxm_vector3_t *APIENTRY
nanodxmDocumentGetVertices(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_vertices;
        return document->vertices;
    }
    else {
        *length = 0;
        return NULL;
    }
}

const nanodxm_color_t *APIENTRY
nanodxmDocumentGetColors(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_vertex_colors;
        return document->vertex_colors;
    }
    else {
        *length = 0;
        return NULL;
    }
}

const nanodxm_vector3_t *APIENTRY
nanodxmDocumentGetNormals(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_normals;
        return document->normals;
    }
    else {
        *length = 0;
        return NULL;
    }
}

const nanodxm_texcoord_t *APIENTRY
nanodxmDocumentGetTexCoords(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_texcoords;
        return document->texcoords;
    }
    else {
        *length = 0;
        return NULL;
    }
}

const nanodxm_face_t *APIENTRY
nanodxmDocumentGetVertexFaces(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_vertex_faces;
        return document->vertex_faces;
    }
    else {
        *length = 0;
        return NULL;
    }
}

const nanodxm_face_t *APIENTRY
nanodxmDocumentGetNormalFaces(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_normal_faces;
        return document->normal_faces;
    }
    else {
        *length = 0;
        return NULL;
    }
}

NANODXM_DECL_API nanodxm_material_t *const *APIENTRY
nanodxmDocumentGetMaterials(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = kv_size(document->materials);
        return document->materials.a;
    }
    else {
        *length = 0;
        return NULL;
    }
}

NANODXM_DECL_API const int *APIENTRY
nanodxmDocumentGetFaceMaterialIndices(const nanodxm_document_t *document, nanodxm_rsize_t *length)
{
    if (nanodxm_is_not_null(document)) {
        *length = document->num_face_material_indices;
        return document->face_material_indices;
    }
    else {
        *length = 0;
        return NULL;
    }
}

void APIENTRY
nanodxmDocumentDestroy(nanodxm_document_t *document)
{
    nanodxm_rsize_t i, length;
    if (nanodxm_is_not_null(document)) {
        length = kv_size(document->materials);
        for (i = 0; i < length; i++) {
            nanodxmMaterialDestroy(kv_A(document->materials, length - i - 1));
        }
        kv_destroy(document->materials);
        if (nanodxm_is_not_null(document->vertices)) {
            nanodxm_free(document->vertices);
        }
        if (nanodxm_is_not_null(document->vertex_colors)) {
            nanodxm_free(document->vertex_colors);
        }
        if (nanodxm_is_not_null(document->normals)) {
            nanodxm_free(document->normals);
        }
        if (nanodxm_is_not_null(document->texcoords)) {
            nanodxm_free(document->texcoords);
        }
        if (nanodxm_is_not_null(document->face_material_indices)) {
            nanodxm_free(document->face_material_indices);
        }
        if (nanodxm_is_not_null(document->normal_faces)) {
            length = document->num_normal_faces;
            for (i = 0; i < length; i++) {
                nanodxm_free(document->normal_faces[length - i - 1].indices);
            }
            nanodxm_free(document->normal_faces);
        }
        if (nanodxm_is_not_null(document->vertex_faces)) {
            length = document->num_vertex_faces;
            for (i = 0; i < length; i++) {
                nanodxm_free(document->vertex_faces[length - i - 1].indices);
            }
            nanodxm_free(document->vertex_faces);
        }
        nanodxm_free(document);
    }
}
