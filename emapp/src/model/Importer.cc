/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Importer.h"

#include "emapp/Error.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Label.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/vector_query.hpp"

#include "nanodxm/nanodxm.h"
#include "nanomqo/nanomqo.h"

extern "C" {
#include "tinyobjloader-c/tinyobj_loader_c.h"
}

namespace nanoem {
namespace model {
namespace {

struct VertexUnit {
    VertexUnit(const Vector3 &origin, const Vector2 &uv, nanoem_rsize_t i)
        : m_origin(origin)
        , m_uv(glm::fract(uv))
        , m_index(i)
    {
    }
    static inline Vector2
    mqoUV(const nanoem_f32_t *uvs, nanoem_rsize_t offset)
    {
        return uvs ? glm::make_vec2(&uvs[offset]) : Vector2();
    }
    static inline Vector2
    dxmUV(const nanodxm_texcoord_t *uvs, nanoem_rsize_t index)
    {
        return uvs ? Vector2(uvs[index].u, 1 - uvs[index].v) : Vector2();
    }
    const Vector3 m_origin;
    const Vector2 m_uv;
    const nanoem_rsize_t m_index;
};
typedef tinystl::vector<VertexUnit, TinySTLAllocator> VertexUnitList;
typedef tinystl::vector<Vector3, TinySTLAllocator> Vector3List;
typedef tinystl::unordered_map<nanoem_rsize_t, Vector3List, TinySTLAllocator> FaceNormalList;
typedef tinystl::vector<nanoem_u32_t, TinySTLAllocator> IndexList;
typedef tinystl::unordered_map<nanoem_rsize_t, nanoem_rsize_t, TinySTLAllocator> VertexMap;

} /* namespace anonymous */

Importer::Importer(Model *model)
    : m_model(model)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_t *m = nanoemMutableModelCreateAsReference(model->data(), &status);
    nanoemMutableModelSetCodecType(m, NANOEM_CODEC_TYPE_UTF16);
    nanoemMutableModelSetFormatType(m, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
    nanoemMutableModelDestroy(m);
}

Importer::~Importer() NANOEM_DECL_NOEXCEPT
{
}

bool
Importer::execute(const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error)
{
    bool result = false;
    switch (desc.m_fileType) {
    case Model::ImportDescription::kFileTypeWaveFrontObj: {
        result = handleWavefrontObjDocument(bytes, length, desc, error);
        break;
    }
    case Model::ImportDescription::kFileTypeDirectX: {
        result = handleDirectXMeshDocument(bytes, length, desc, error);
        break;
    }
    case Model::ImportDescription::kFileTypeMetasequoia: {
        result = handleMetasequoiaDocument(bytes, length, desc, error);
        break;
    }
    case Model::ImportDescription::kFileTypeNone:
    default:
        error = Error("Not supported file type", nullptr, Error::kDomainTypeApplication);
        break;
    }
    return result;
}

bool
Importer::handleWavefrontObjDocument(
    const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error)
{
    BX_UNUSED_4(bytes, length, desc, error);
    tinyobj_attrib_t attr;
    tinyobj_shape_t *shapes = nullptr;
    tinyobj_material_t *materials = nullptr;
    nanoem_rsize_t numShapes, numMaterials;
    int ret = tinyobj_parse_obj(&attr, &shapes, &numShapes, &materials, &numMaterials,
        desc.m_fileURI.absolutePathConstString(), handleLoadingTinyOBJCallback, this, TINYOBJ_FLAG_TRIANGULATE);
    bool result = false;
    if (ret == TINYOBJ_SUCCESS) {
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(m_model->data(), &status);
        nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
        nanoem_model_t *originModel = nanoemMutableModelGetOriginObject(mutableModel);
        const bool hasNormals = attr.num_normals > 0 && attr.num_vertices == attr.num_normals,
                   hasTexCoords = attr.num_texcoords > 0 && attr.num_vertices == attr.num_texcoords;
        for (nanoem_rsize_t i = 0, numVertices = attr.num_vertices; i < numVertices; i++) {
            const nanoem_f32_t *originPtr = &attr.vertices[i * 3];
            const Vector4 origin(originPtr[0], originPtr[1], originPtr[2], 1);
            nanoem_mutable_model_vertex_t *vertexPtr = nanoemMutableModelVertexCreate(originModel, &status);
            nanoemMutableModelVertexSetOrigin(vertexPtr, glm::value_ptr(origin));
            if (hasNormals) {
                const nanoem_f32_t *normalPtr = &attr.normals[i * 3];
                const Vector4 normal(normalPtr[0], normalPtr[1], normalPtr[2], 0);
                nanoemMutableModelVertexSetNormal(vertexPtr, glm::value_ptr(normal));
            }
            if (hasTexCoords) {
                const nanoem_f32_t *texCoordPtr = &attr.texcoords[i * 2];
                const Vector4 texCoord(texCoordPtr[0], texCoordPtr[1], 0, 0);
                nanoemMutableModelVertexSetTexCoord(vertexPtr, glm::value_ptr(texCoord));
            }
            nanoemMutableModelInsertVertexObject(mutableModel, vertexPtr, -1, &status);
            nanoemMutableModelVertexDestroy(vertexPtr);
        }
        const nanoem_rsize_t numFaces = attr.num_faces;
        IndexList indices(numFaces);
        for (nanoem_rsize_t i = 0; i < numFaces; i++) {
            const tinyobj_vertex_index_t &face = attr.faces[i];
            indices[i] = face.v_idx;
        }
        nanoemMutableModelSetVertexIndices(mutableModel, indices.data(), indices.size(), &status);
        {
            nanoem_mutable_model_material_t *materialPtr = nanoemMutableModelMaterialCreate(originModel, &status);
            nanoemMutableModelMaterialSetAmbientColor(materialPtr, glm::value_ptr(Vector4(1)));
            nanoemMutableModelMaterialSetDiffuseColor(materialPtr, glm::value_ptr(Vector4(1)));
            nanoemMutableModelMaterialSetSpecularColor(materialPtr, glm::value_ptr(Vector4(1)));
            nanoemMutableModelMaterialSetDiffuseOpacity(materialPtr, 1.0f);
            nanoemMutableModelMaterialSetSpecularPower(materialPtr, 1.0f);
            nanoemMutableModelMaterialSetNumVertexIndices(materialPtr, numFaces);
            nanoemMutableModelInsertMaterialObject(mutableModel, materialPtr, -1, &status);
            nanoemMutableModelMaterialDestroy(materialPtr);
        }
        setupRootParentBoneAndLabel(mutableModel, factory, &status);
        bindAllVerticesWithRootParentBone(mutableModel, &status);
        nanoemMutableModelDestroy(mutableModel);
        result = status == NANOEM_STATUS_SUCCESS;
    }
    return result;
}

bool
Importer::handleDirectXMeshDocument(
    const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error)
{
    nanodxm_document_t *document = nanodxmDocumentCreate();
    nanodxm_buffer_t *buffer = nanodxmBufferCreate(bytes, length);
    nanodxm_status_t nanodxmStatus = nanodxmDocumentParse(document, buffer);
    if (nanodxmStatus == NANODXM_STATUS_SUCCESS) {
        typedef tinystl::unordered_map<const nanodxm_material_t *, VertexUnitList, TinySTLAllocator>
            MaterialVertexListMap;
        nanodxm_rsize_t numVertices, numTexCoords, numMaterials, numVertexFaces, numMaterialIndices;
        const nanodxm_vector3_t *vertices = nanodxmDocumentGetVertices(document, &numVertices);
        const nanodxm_texcoord_t *texcoords = nanodxmDocumentGetTexCoords(document, &numTexCoords);
        nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(document, &numMaterials);
        const nanodxm_face_t *vertexFaces = nanodxmDocumentGetVertexFaces(document, &numVertexFaces);
        const int *materialIndices = nanodxmDocumentGetFaceMaterialIndices(document, &numMaterialIndices);
        MaterialVertexListMap materialMap;
        FaceNormalList faceNormalList;
        for (nanomqo_rsize_t j = 0; j < numVertexFaces; j++) {
            const nanodxm_face_t &vertexFace = vertexFaces[j];
            nanoem_rsize_t materialIndex = materialIndices[j];
            if (materialIndex < numMaterials) {
                const int *indices = vertexFace.indices;
                const nanodxm_material_t *material = materials[materialIndex];
                VertexUnitList &vl = materialMap[material];
                switch (vertexFace.num_indices) {
                case 4: {
                    nanomqo_rsize_t index0 = indices[0], index1 = indices[1], index2 = indices[2], index3 = indices[3];
                    if (index0 < numVertices && index1 < numVertices && index2 < numVertices && index3 < numVertices) {
                        const Vector3 o0(glm::make_vec3(&vertices[index0].x)), o1(glm::make_vec3(&vertices[index1].x)),
                            o2(glm::make_vec3(&vertices[index2].x)), o3(glm::make_vec3(&vertices[index3].x));
                        vl.push_back(
                            VertexUnit(o0, VertexUnit::dxmUV(texcoords, index0), Inline::saturateInt32(index0)));
                        vl.push_back(
                            VertexUnit(o1, VertexUnit::dxmUV(texcoords, index1), Inline::saturateInt32(index1)));
                        vl.push_back(
                            VertexUnit(o2, VertexUnit::dxmUV(texcoords, index2), Inline::saturateInt32(index2)));
                        vl.push_back(
                            VertexUnit(o0, VertexUnit::dxmUV(texcoords, index0), Inline::saturateInt32(index0)));
                        vl.push_back(
                            VertexUnit(o2, VertexUnit::dxmUV(texcoords, index2), Inline::saturateInt32(index2)));
                        vl.push_back(
                            VertexUnit(o3, VertexUnit::dxmUV(texcoords, index3), Inline::saturateInt32(index3)));
                        const Vector3 n0(glm::normalize(glm::cross(o1 - o0, o2 - o1)));
                        if (!glm::isNull(n0, glm::epsilon<Vector3::value_type>())) {
                            faceNormalList[index0].push_back(n0);
                            faceNormalList[index1].push_back(n0);
                            faceNormalList[index2].push_back(n0);
                        }
                        const Vector3 n1(glm::normalize(glm::cross(o2 - o0, o3 - o2)));
                        if (!glm::isNull(n1, glm::epsilon<Vector3::value_type>())) {
                            faceNormalList[index0].push_back(n1);
                            faceNormalList[index2].push_back(n1);
                            faceNormalList[index3].push_back(n1);
                        }
                    }
                    break;
                }
                case 3: {
                    nanomqo_rsize_t index0 = indices[0], index1 = indices[1], index2 = indices[2];
                    if (index0 < numVertices && index1 < numVertices && index2 < numVertices) {
                        const Vector3 o0(glm::make_vec3(&vertices[index0].x)), o1(glm::make_vec3(&vertices[index1].x)),
                            o2(glm::make_vec3(&vertices[index2].x));
                        vl.push_back(
                            VertexUnit(o0, VertexUnit::dxmUV(texcoords, index0), Inline::saturateInt32(index0)));
                        vl.push_back(
                            VertexUnit(o1, VertexUnit::dxmUV(texcoords, index1), Inline::saturateInt32(index1)));
                        vl.push_back(
                            VertexUnit(o2, VertexUnit::dxmUV(texcoords, index2), Inline::saturateInt32(index2)));
                        const Vector3 normal(glm::normalize(glm::cross(o1 - o0, o2 - o1)));
                        if (!glm::isNull(normal, glm::epsilon<Vector3::value_type>())) {
                            faceNormalList[index0].push_back(normal);
                            faceNormalList[index1].push_back(normal);
                            faceNormalList[index2].push_back(normal);
                        }
                    }
                    break;
                }
                case 2: {
                    nanomqo_rsize_t index0 = indices[0], index1 = indices[1];
                    if (index0 < numVertices && index1 < numVertices) {
                        const Vector3 o0(glm::make_vec3(&vertices[index0].x)), o1(glm::make_vec3(&vertices[index1].x));
                        vl.push_back(
                            VertexUnit(o0, VertexUnit::dxmUV(texcoords, index0), Inline::saturateInt32(index0)));
                        vl.push_back(
                            VertexUnit(o1, VertexUnit::dxmUV(texcoords, index1), Inline::saturateInt32(index1)));
                        vl.push_back(
                            VertexUnit(o0, VertexUnit::dxmUV(texcoords, index0), Inline::saturateInt32(index0)));
                    }
                    break;
                }
                default:
                    error = Error("Polygon is not supported (only line/triangle/quad primitive is supported)", nullptr,
                        Error::kDomainTypeApplication);
                    j = numVertexFaces;
                    break;
                }
            }
            else {
                error = Error("Out of material index", nullptr, Error::kDomainTypeApplication);
                j = numVertexFaces;
            }
        }
        if (!error.hasReason()) {
            IndexList indices;
            VertexMap vertexMap;
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(m_model->data(), &status);
            Project *project = m_model->project();
            nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
            nanoem_rsize_t vertexOffset = 0;
            StringUtils::UnicodeStringScope us(factory);
            setupModelNameAndComment(mutableModel, factory, desc, &status);
            Vector3List normals;
            normals.resize(faceNormalList.size());
            for (FaceNormalList::const_iterator it = faceNormalList.begin(), end = faceNormalList.end(); it != end;
                 ++it) {
                Vector3 normal(Constants::kZeroV3);
                const Vector3List &vn = it->second;
                for (Vector3List::const_iterator it2 = vn.begin(), end2 = vn.end(); it2 != end2; ++it2) {
                    normal += *it2;
                }
                if (!glm::isNull(normal, glm::epsilon<Vector3::value_type>())) {
                    normals[it->first] = glm::normalize(normal);
                }
            }
            int materialIndex = 1;
            for (MaterialVertexListMap::const_iterator it = materialMap.begin(), end = materialMap.end(); it != end;
                 ++it) {
                const nanodxm_material_t *m = it->first;
                nanoem_mutable_model_material_t *mutableMaterialPtr =
                    nanoemMutableModelMaterialCreate(m_model->data(), &status);
                const VertexUnitList &vl = it->second;
                for (VertexUnitList::const_iterator it3 = vl.begin(), end3 = vl.end(); it3 != end3; ++it3) {
                    const VertexUnit &v = *it3;
                    VertexMap::const_iterator it = vertexMap.find(v.m_index);
                    if (it == vertexMap.end()) {
                        const Vector4 origin(desc.m_transform * Vector4(v.m_origin, 1)), uv(v.m_uv, 0, 0);
                        nanoem_mutable_model_vertex_t *mutableVertexPtr =
                            nanoemMutableModelVertexCreate(m_model->data(), &status);
                        nanoemMutableModelVertexSetOrigin(mutableVertexPtr, glm::value_ptr(origin));
                        nanoemMutableModelVertexSetNormal(
                            mutableVertexPtr, glm::value_ptr(Vector4(normals[v.m_index], 0)));
                        nanoemMutableModelVertexSetTexCoord(mutableVertexPtr, glm::value_ptr(uv));
                        nanoemMutableModelVertexSetType(mutableVertexPtr, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                        nanoemMutableModelInsertVertexObject(mutableModel, mutableVertexPtr, -1, &status);
                        nanoemMutableModelVertexDestroy(mutableVertexPtr);
                        it = vertexMap.insert(tinystl::make_pair(v.m_index, vertexOffset++)).first;
                    }
                    indices.push_back(Inline::saturateInt32(it->second));
                }
                String name; //(nanodxmMaterialGetName(m));
                StringUtils::format(name, "Material%d", materialIndex++);
                if (nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory,
                        reinterpret_cast<const nanoem_u8_t *>(name.c_str()), name.size(), NANOEM_CODEC_TYPE_SJIS,
                        &status)) {
                    nanoemMutableModelMaterialSetName(mutableMaterialPtr, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
                    nanoemUnicodeStringFactoryDestroyString(factory, s);
                }
                if (const nanodxm_uint8_t *texturePathPtr = nanodxmMaterialGetTextureFilename(m)) {
                    if (nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory,
                            texturePathPtr, StringUtils::length(reinterpret_cast<const char *>(texturePathPtr)),
                            NANOEM_CODEC_TYPE_SJIS, &status)) {
                        nanoem_mutable_model_texture_t *mutableTexturePtr =
                            nanoemMutableModelTextureCreate(m_model->data(), &status);
                        nanoemMutableModelTextureSetPath(mutableTexturePtr, s, &status);
                        setupMaterialTexture(mutableMaterialPtr, mutableTexturePtr,
                            reinterpret_cast<const char *>(texturePathPtr), &status);
                        nanoemMutableModelTextureDestroy(mutableTexturePtr);
                        nanoemUnicodeStringFactoryDestroyString(factory, s);
                    }
                }
                const nanodxm_color_t &ac = nanodxmMaterialGetEmissive(m), &dc = nanodxmMaterialGetDiffuse(m),
                                      &sc = nanodxmMaterialGetSpecular(m);
                const Vector4 ambient(ac.r, ac.g, ac.b, ac.a), diffuse(dc.r, dc.g, dc.b, dc.a),
                    specular(sc.r, sc.g, sc.b, sc.a);
                nanoemMutableModelMaterialSetAmbientColor(mutableMaterialPtr, glm::value_ptr(ambient));
                nanoemMutableModelMaterialSetDiffuseColor(mutableMaterialPtr, glm::value_ptr(diffuse));
                nanoemMutableModelMaterialSetDiffuseOpacity(mutableMaterialPtr, diffuse.w);
                nanoemMutableModelMaterialSetSpecularColor(mutableMaterialPtr, glm::value_ptr(specular));
                nanoemMutableModelMaterialSetSpecularPower(mutableMaterialPtr, nanodxmMaterialGetShininess(m));
                nanoemMutableModelMaterialSetCullingDisabled(mutableMaterialPtr, 0);
                nanoemMutableModelInsertMaterialObject(mutableModel, mutableMaterialPtr, -1, &status);
                nanoemMutableModelMaterialSetNumVertexIndices(mutableMaterialPtr, vl.size());
                nanoemMutableModelMaterialDestroy(mutableMaterialPtr);
            }
            nanoemMutableModelSetVertexIndices(mutableModel, indices.data(), indices.size(), &status);
            setupRootParentBoneAndLabel(mutableModel, factory, &status);
            bindAllVerticesWithRootParentBone(mutableModel, &status);
            nanoemMutableModelDestroy(mutableModel);
        }
    }
    else {
        error = Error("Invalid DirectX mesh document format", nanodxmStatus, Error::kDomainTypeNanodxm);
    }
    nanodxmDocumentDestroy(document);
    nanodxmBufferDestroy(buffer);
    return !error.hasReason();
}

bool
Importer::handleMetasequoiaDocument(
    const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error)
{
    nanomqo_document_t *document = nanomqoDocumentCreate();
    nanomqo_buffer_t *buffer = nanomqoBufferCreate(bytes, length);
    nanomqo_status_t nanomqoStatus = nanomqoDocumentParse(document, buffer);
    if (nanomqoStatus == NANOMQO_STATUS_SUCCESS) {
        typedef tinystl::unordered_map<const nanomqo_material_t *, VertexUnitList, TinySTLAllocator>
            MaterialVertexListMap;
        typedef tinystl::vector<MaterialVertexListMap, TinySTLAllocator> ObjectList;
        typedef tinystl::unordered_map<const nanomqo_object_t *, FaceNormalList, TinySTLAllocator> ObjectFaceNormalList;
        ObjectFaceNormalList faceNormalMap;
        nanomqo_rsize_t numObjects, numMaterials;
        nanomqo_object_t *const *objects = nanomqoDocumentGetAllObjects(document, &numObjects);
        nanomqo_material_t *const *materials = nanomqoDocumentGetAllMaterials(document, &numMaterials);
        ObjectList map;
        map.resize(numObjects);
        for (nanomqo_rsize_t i = 0; i < numObjects; i++) {
            const nanomqo_object_t *o = objects[i];
            if (!nanomqoObjectIsVisible(o)) {
                continue;
            }
            nanomqo_rsize_t numVertices, numFaces;
            nanomqo_vertex_t *const *vertices = nanomqoObjectGetAllVertices(o, &numVertices);
            nanomqo_face_t *const *faces = nanomqoObjectGetAllFaces(o, &numFaces);
            MaterialVertexListMap &materialMap = map[i];
            for (nanomqo_rsize_t j = 0; j < numFaces; j++) {
                const nanomqo_face_t *face = faces[j];
                int numIndices;
                nanomqo_rsize_t materialIndex = nanomqoFaceGetMaterialIndex(face);
                if (materialIndex < numMaterials) {
                    const int *indices = nanomqoFaceGetVertexIndices(face, &numIndices);
                    const nanomqo_float32_t *uvs = nanomqoFaceGetUVs(face);
                    const nanomqo_material_t *material = materials[materialIndex];
                    VertexUnitList &vl = materialMap[material];
                    switch (numIndices) {
                    case 4: {
                        nanomqo_rsize_t index0 = indices[0], index1 = indices[1], index2 = indices[2],
                                        index3 = indices[3];
                        if (index0 < numVertices && index1 < numVertices && index2 < numVertices &&
                            index3 < numVertices) {
                            const Vector3 o0(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index0]))),
                                o1(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index1]))),
                                o2(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index2]))),
                                o3(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index3])));
                            vl.push_back(VertexUnit(o0, VertexUnit::mqoUV(uvs, 0), index0));
                            vl.push_back(VertexUnit(o1, VertexUnit::mqoUV(uvs, 2), index1));
                            vl.push_back(VertexUnit(o2, VertexUnit::mqoUV(uvs, 4), index2));
                            vl.push_back(VertexUnit(o0, VertexUnit::mqoUV(uvs, 0), index0));
                            vl.push_back(VertexUnit(o2, VertexUnit::mqoUV(uvs, 4), index2));
                            vl.push_back(VertexUnit(o3, VertexUnit::mqoUV(uvs, 6), index3));
                            const Vector3 n0(glm::normalize(glm::cross(o1 - o0, o2 - o1)));
                            FaceNormalList &faceNormalList = faceNormalMap[o];
                            if (!glm::isNull(n0, glm::epsilon<Vector3::value_type>())) {
                                faceNormalList[index0].push_back(n0);
                                faceNormalList[index1].push_back(n0);
                                faceNormalList[index2].push_back(n0);
                            }
                            const Vector3 n1(glm::normalize(glm::cross(o2 - o0, o3 - o2)));
                            if (!glm::isNull(n1, glm::epsilon<Vector3::value_type>())) {
                                faceNormalList[index0].push_back(n1);
                                faceNormalList[index2].push_back(n1);
                                faceNormalList[index3].push_back(n1);
                            }
                        }
                        break;
                    }
                    case 3: {
                        nanomqo_rsize_t index0 = indices[0], index1 = indices[1], index2 = indices[2];
                        if (index0 < numVertices && index1 < numVertices && index2 < numVertices) {
                            const Vector3 o0(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index0]))),
                                o1(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index1]))),
                                o2(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index2])));
                            vl.push_back(VertexUnit(o0, VertexUnit::mqoUV(uvs, 0), index0));
                            vl.push_back(VertexUnit(o1, VertexUnit::mqoUV(uvs, 2), index1));
                            vl.push_back(VertexUnit(o2, VertexUnit::mqoUV(uvs, 4), index2));
                            const Vector3 normal(glm::normalize(glm::cross(o1 - o0, o2 - o1)));
                            if (!glm::isNull(normal, glm::epsilon<Vector3::value_type>())) {
                                FaceNormalList &faceNormalList = faceNormalMap[o];
                                faceNormalList[index0].push_back(normal);
                                faceNormalList[index1].push_back(normal);
                                faceNormalList[index2].push_back(normal);
                            }
                        }
                        break;
                    }
                    case 2: {
                        nanomqo_rsize_t index0 = indices[0], index1 = indices[1];
                        if (index0 < numVertices && index1 < numVertices) {
                            const Vector3 o0(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index0]))),
                                o1(glm::make_vec3(nanomqoVertexGetOrigin(vertices[index1])));
                            vl.push_back(VertexUnit(o0, glm::make_vec2(&uvs[0]), index0));
                            vl.push_back(VertexUnit(o1, glm::make_vec2(&uvs[2]), index1));
                            vl.push_back(VertexUnit(o0, glm::make_vec2(&uvs[0]), index0));
                        }
                        break;
                    }
                    default:
                        error = Error("Polygon is not supported (only line/triangle/quad primitive is supported)",
                            nullptr, Error::kDomainTypeApplication);
                        i = numObjects;
                        break;
                    }
                }
                else {
                    error = Error("Out of material index", nullptr, Error::kDomainTypeApplication);
                    i = numObjects;
                }
            }
        }
        if (!error.hasReason()) {
            IndexList indices;
            VertexMap vertexMap;
            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
            nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(m_model->data(), &status);
            nanoem_rsize_t vertexOffset = 0;
            Project *project = m_model->project();
            nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
            StringUtils::UnicodeStringScope us(factory);
            setupModelNameAndComment(mutableModel, factory, desc, &status);
            for (ObjectList::const_iterator it = map.begin(), end = map.end(); it != end; ++it) {
                const nanomqo_object_t *o = objects[it - map.begin()];
                const MaterialVertexListMap &materialMap = *it;
                const FaceNormalList &faceNormalList = faceNormalMap[o];
                Vector3List normals;
                normals.resize(faceNormalList.size());
                for (FaceNormalList::const_iterator it2 = faceNormalList.begin(), end2 = faceNormalList.end();
                     it2 != end2; ++it2) {
                    Vector3 normal(Constants::kZeroV3);
                    const Vector3List &vn = it2->second;
                    for (Vector3List::const_iterator it3 = vn.begin(), end3 = vn.end(); it3 != end3; ++it3) {
                        normal += *it3;
                    }
                    if (!glm::isNull(normal, glm::epsilon<Vector3::value_type>())) {
                        normals[it2->first] = glm::normalize(normal);
                    }
                }
                for (MaterialVertexListMap::const_iterator it2 = materialMap.begin(), end2 = materialMap.end();
                     it2 != end2; ++it2) {
                    const nanomqo_material_t *m = it2->first;
                    nanoem_mutable_model_material_t *mutableMaterialPtr =
                        nanoemMutableModelMaterialCreate(m_model->data(), &status);
                    const VertexUnitList &vl = it2->second;
                    for (VertexUnitList::const_iterator it3 = vl.begin(), end3 = vl.end(); it3 != end3; ++it3) {
                        const VertexUnit &v = *it3;
                        VertexMap::const_iterator it = vertexMap.find(v.m_index);
                        if (it == vertexMap.end()) {
                            const glm::quat orientation(glm::make_vec3(nanomqoObjectGetOrientation(o)));
                            const Vector4 translation(glm::make_vec4(nanomqoObjectGetTranslation(o))),
                                scale(glm::make_vec3(nanomqoObjectGetScale(o)), 1),
                                origin(desc.m_transform *
                                    (orientation * ((Vector4(v.m_origin, 1) + translation) * scale))),
                                uv(v.m_uv, 0, 0);
                            nanoem_mutable_model_vertex_t *mutableVeretxPtr =
                                nanoemMutableModelVertexCreate(m_model->data(), &status);
                            nanoemMutableModelVertexSetOrigin(
                                mutableVeretxPtr, glm::value_ptr(origin * Vector4(1, 1, -1, 1)));
                            nanoemMutableModelVertexSetNormal(
                                mutableVeretxPtr, glm::value_ptr(Vector4(normals[v.m_index], 0)));
                            nanoemMutableModelVertexSetTexCoord(mutableVeretxPtr, glm::value_ptr(uv));
                            nanoemMutableModelVertexSetType(mutableVeretxPtr, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                            nanoemMutableModelInsertVertexObject(mutableModel, mutableVeretxPtr, -1, &status);
                            nanoemMutableModelVertexDestroy(mutableVeretxPtr);
                            it = vertexMap.insert(tinystl::make_pair(v.m_index, vertexOffset++)).first;
                        }
                        indices.push_back(Inline::saturateInt32(it->second));
                    }
                    String name;
                    if (StringUtils::equals(nanomqoObjectGetName(o), nanomqoMaterialGetName(m))) {
                        name = nanomqoObjectGetName(o);
                    }
                    else {
                        StringUtils::format(name, "%s/%s", nanomqoObjectGetName(o), nanomqoMaterialGetName(m));
                    }
                    if (nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory,
                            reinterpret_cast<const nanoem_u8_t *>(name.c_str()), name.size(), NANOEM_CODEC_TYPE_SJIS,
                            &status)) {
                        nanoemMutableModelMaterialSetName(
                            mutableMaterialPtr, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
                        nanoemUnicodeStringFactoryDestroyString(factory, s);
                    }
                    if (const char *texturePathPtr = nanomqoMaterialGetTexturePath(m)) {
                        if (nanoem_unicode_string_t *s = nanoemUnicodeStringFactoryCreateStringWithEncoding(factory,
                                reinterpret_cast<const nanoem_u8_t *>(texturePathPtr),
                                StringUtils::length(texturePathPtr), NANOEM_CODEC_TYPE_SJIS, &status)) {
                            nanoem_mutable_model_texture_t *mutableTexturePtr =
                                nanoemMutableModelTextureCreate(m_model->data(), &status);
                            nanoemMutableModelTextureSetPath(mutableTexturePtr, s, &status);
                            setupMaterialTexture(mutableMaterialPtr, mutableTexturePtr, texturePathPtr, &status);
                            nanoemMutableModelTextureDestroy(mutableTexturePtr);
                            nanoemUnicodeStringFactoryDestroyString(factory, s);
                        }
                    }
                    const Vector4 baseColor(glm::make_vec4(nanomqoMaterialGetColor(m))),
                        ambient(nanomqoMaterialGetAmbient(m) * baseColor),
                        diffuse(nanomqoMaterialGetDiffuse(m) * baseColor),
                        specular(nanomqoMaterialGetSpecular(m) * baseColor);
                    nanoemMutableModelMaterialSetAmbientColor(mutableMaterialPtr, glm::value_ptr(ambient));
                    nanoemMutableModelMaterialSetDiffuseColor(mutableMaterialPtr, glm::value_ptr(diffuse));
                    nanoemMutableModelMaterialSetDiffuseOpacity(mutableMaterialPtr, baseColor.w);
                    nanoemMutableModelMaterialSetSpecularColor(mutableMaterialPtr, glm::value_ptr(specular));
                    nanoemMutableModelMaterialSetSpecularPower(mutableMaterialPtr, nanomqoMaterialGetPower(m));
                    nanoemMutableModelMaterialSetCullingDisabled(
                        mutableMaterialPtr, nanomqoMaterialIsCullingDisabled(m));
                    nanoemMutableModelInsertMaterialObject(mutableModel, mutableMaterialPtr, -1, &status);
                    nanoemMutableModelMaterialSetNumVertexIndices(mutableMaterialPtr, vl.size());
                    nanoemMutableModelMaterialDestroy(mutableMaterialPtr);
                }
                vertexMap.clear();
            }
            nanoemMutableModelSetVertexIndices(mutableModel, indices.data(), indices.size(), &status);
            setupRootParentBoneAndLabel(mutableModel, factory, &status);
            bindAllVerticesWithRootParentBone(mutableModel, &status);
            nanoemMutableModelDestroy(mutableModel);
            if (status != NANOEM_STATUS_SUCCESS) {
                const char *message = Error::convertStatusToMessage(status, project->translator());
                error = Error(message, status, Error::kDomainTypeNanoem);
            }
        }
    }
    else {
        error = Error("Invalid Metasequoia document format", nanomqoStatus, Error::kDomainTypeNanomqo);
    }
    nanomqoDocumentDestroy(document);
    nanomqoBufferDestroy(buffer);
    return !error.hasReason();
}

void
Importer::handleLoadingTinyOBJCallback(
    void *ctx, const char *filename, const int is_mtl, const char *obj_filename, char **data, size_t *len)
{
    BX_UNUSED_1(obj_filename);
    Importer *self = static_cast<Importer *>(ctx);
    FileReaderScope reader(self->m_model->project()->translator());
    Error error;
    char *bytesPtr = nullptr;
    nanoem_rsize_t length = 0;
    URI fileURI;
#if 0 /* disable loading material file due to infinite loop at hash construction in tinyobjloader */
    if (is_mtl && obj_filename) {
        String path(URI::stringByDeletingLastPathComponent(obj_filename));
        path.append("/");
        path.append(filename);
        fileURI = URI::createFromFilePath(path);
    }
    else
#endif
    {
        fileURI = URI::createFromFilePath(filename);
    }
    if (reader.open(fileURI, error)) {
        ByteArray &bytes = self->m_bytes[fileURI.absolutePath()];
        FileUtils::read(reader, bytes, error);
        bytesPtr = reinterpret_cast<char *>(bytes.data());
        length = bytes.size();
    }
    *data = bytesPtr;
    *len = length;
}

void
Importer::setupModelNameAndComment(nanoem_mutable_model_t *mutableModel, nanoem_unicode_string_factory_t *factory,
    const Model::ImportDescription &desc, nanoem_status_t *status)
{
    StringUtils::UnicodeStringScope us(factory);
    for (int i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
        nanoem_language_type_t language = static_cast<nanoem_language_type_t>(i);
        if (StringUtils::tryGetString(factory, desc.m_name[language], us)) {
            nanoemMutableModelSetName(mutableModel, us.value(), language, status);
        }
        if (StringUtils::tryGetString(factory, desc.m_comment[language], us)) {
            nanoemMutableModelSetComment(mutableModel, us.value(), language, status);
        }
    }
}

void
Importer::setupMaterialTexture(nanoem_mutable_model_material_t *mutableMaterialPtr,
    nanoem_mutable_model_texture_t *texture, const char *texturePathPtr, nanoem_status_t *status)
{
    if (const char *p = StringUtils::indexOf(texturePathPtr, '.')) {
        if (StringUtils::equals(p, ".spa")) {
            nanoemMutableModelMaterialSetSphereMapTextureType(
                mutableMaterialPtr, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD);
            nanoemMutableModelMaterialSetSphereMapTextureObject(mutableMaterialPtr, texture, status);
        }
        else if (StringUtils::equals(p, ".sph")) {
            nanoemMutableModelMaterialSetSphereMapTextureType(
                mutableMaterialPtr, NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY);
            nanoemMutableModelMaterialSetSphereMapTextureObject(mutableMaterialPtr, texture, status);
        }
        else {
            nanoemMutableModelMaterialSetDiffuseTextureObject(mutableMaterialPtr, texture, status);
        }
    }
    else {
        nanoemMutableModelMaterialSetDiffuseTextureObject(mutableMaterialPtr, texture, status);
    }
}

void
Importer::setupRootParentBoneAndLabel(
    nanoem_mutable_model_t *mutableModel, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_model_t *originModel = nanoemMutableModelGetOriginObject(mutableModel);
    nanoem_mutable_model_bone_t *mutableCenterBone = nanoemMutableModelBoneCreate(originModel, status);
    StringUtils::UnicodeStringScope scope(factory);
    {
        if (StringUtils::tryGetString(
                factory, reinterpret_cast<const char *>(model::Bone::kNameRootParentInJapanese), scope)) {
            nanoemMutableModelBoneSetName(mutableCenterBone, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
        }
        if (StringUtils::tryGetString(factory, "Root", scope)) {
            nanoemMutableModelBoneSetName(mutableCenterBone, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
        }
        nanoemMutableModelBoneSetVisible(mutableCenterBone, true);
        nanoemMutableModelBoneSetMovable(mutableCenterBone, true);
        nanoemMutableModelBoneSetRotateable(mutableCenterBone, true);
        nanoemMutableModelBoneSetUserHandleable(mutableCenterBone, true);
        nanoemMutableModelInsertBoneObject(mutableModel, mutableCenterBone, -1, status);
    }
    {
        nanoem_mutable_model_label_t *mutableRootLabel = nanoemMutableModelLabelCreate(originModel, status);
        if (StringUtils::tryGetString(factory, "Root", scope)) {
            nanoemMutableModelLabelSetName(mutableRootLabel, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
            nanoemMutableModelLabelSetName(mutableRootLabel, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
        }
        nanoem_mutable_model_label_item_t *mutableLabelItem = nanoemMutableModelLabelItemCreateFromBoneObject(
            mutableRootLabel, nanoemMutableModelBoneGetOriginObject(mutableCenterBone), status);
        nanoemMutableModelLabelInsertItemObject(mutableRootLabel, mutableLabelItem, -1, status);
        nanoemMutableModelLabelItemDestroy(mutableLabelItem);
        nanoemMutableModelLabelSetSpecial(mutableRootLabel, 1);
        nanoemMutableModelInsertLabelObject(mutableModel, mutableRootLabel, -1, status);
        nanoemMutableModelLabelDestroy(mutableRootLabel);
    }
    {
        nanoem_mutable_model_label_t *mutableExpressionLabel = nanoemMutableModelLabelCreate(originModel, status);
        if (StringUtils::tryGetString(
                factory, reinterpret_cast<const char *>(model::Label::kNameExpressionInJapanese), scope)) {
            nanoemMutableModelLabelSetName(
                mutableExpressionLabel, scope.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, status);
        }
        if (StringUtils::tryGetString(factory, "Expression", scope)) {
            nanoemMutableModelLabelSetName(mutableExpressionLabel, scope.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, status);
        }
        nanoemMutableModelLabelSetSpecial(mutableExpressionLabel, 1);
        nanoemMutableModelInsertLabelObject(mutableModel, mutableExpressionLabel, -1, status);
        nanoemMutableModelLabelDestroy(mutableExpressionLabel);
    }
    nanoemMutableModelBoneDestroy(mutableCenterBone);
}

void
Importer::bindAllVerticesWithRootParentBone(nanoem_mutable_model_t *model, nanoem_status_t *status)
{
    nanoem_rsize_t numBones, numVertices;
    nanoem_model_t *originModel = nanoemMutableModelGetOriginObject(model);
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(originModel, &numBones);
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(originModel, &numVertices);
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        nanoem_model_vertex_t *vertexPtr = vertices[i];
        nanoem_mutable_model_vertex_t *mutableVertexPtr = nanoemMutableModelVertexCreateAsReference(vertexPtr, status);
        nanoemMutableModelVertexSetBoneObject(mutableVertexPtr, bones[0], 0);
        nanoemMutableModelVertexSetType(mutableVertexPtr, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
        nanoemMutableModelVertexDestroy(mutableVertexPtr);
    }
}

} /* namespace model */
} /* namespace nanoem */
