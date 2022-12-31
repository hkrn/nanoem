/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/sdk/Importer.h"

#define BX_CONFIG_CRT_FILE_READER_WRITER 1
#include "bx/crtimpl.h"
#include "bx/string.h"
#include "tinystl/allocator.h"
#include "tinystl/unordered_map.h"
#include "tinystl/unordered_set.h"
#include "tinystl/vector.h"

#include "./init.h"
#include "./pugixml.hpp"
#include "nanomqo.h"

using namespace pugi;

struct nanoem_application_plugin_importer_t {
    typedef tinystl::vector<nanoem_u32_t> IndexList;
    typedef tinystl::vector<nanoem_mutable_model_morph_t *> MorphList;
    typedef tinystl::unordered_set<nanoem_u32_t> FaceUIDSet;
    typedef tinystl::unordered_map<int, nanomqo_object_t *> ObjectMap;
    typedef tinystl::unordered_map<int, nanoem_mutable_model_bone_t *> BoneMap;
    typedef tinystl::unordered_map<int, nanoem_mutable_model_label_t *> LabelMap;
    typedef tinystl::unordered_map<int, FaceUIDSet> MaterialFaceUIDSetMap;
    typedef tinystl::unordered_map<int, IndexList> MaterialIndexListMap;
    typedef tinystl::unordered_map<int, const nanomqo_object_t *> UID2ObjectMap;

    static void
    constructVertex(nanoem_model_vertex_t *const *vertices, const nanomqo_face_t *face, int index, bool addIndex,
        MaterialIndexListMap &materialindexListMap)
    {
        int numIndices;
        const int *indicesPtr = nanomqoFaceGetVertexIndices(face, &numIndices);
        const int materialIndex = nanomqoFaceGetMaterialIndex(face), vertexIndex = indicesPtr[index];
        if (const nanomqo_float32_t *uv = nanomqoFaceGetUVs(face)) {
            nanoem_mutable_model_vertex_t *vertex = nanoemMutableModelVertexCreateAsReference(vertices[vertexIndex]);
            nanoemMutableModelVertexSetTexCoord(vertex, &uv[index * 2]);
            nanoemMutableModelVertexDestroy(vertex);
        }
        if (addIndex) {
            materialindexListMap[materialIndex].push_back(vertexIndex);
        }
    }
    static void
    fillVerticesInMorph(const nanomqo_document_t *mqo, const char *target, nanoem_mutable_model_morph_t *morph)
    {
        nanomqo_rsize_t numObjects, numVertexAttributeUids;
        nanomqo_object_t *const *objects = nanomqoDocumentGetAllObjects(mqo, &numObjects);
        for (nanomqo_rsize_t i = 0; i < numObjects; i++) {
            const nanomqo_object_t *object = objects[i];
            const char *name = nanomqoObjectGetName(object);
            if (strcmp(name, target) == 0) {
                nanomqoObjectGetAllVertexAttributeUID(object, &numVertexAttributeUids);
            }
        }
    }
    static void
    setupParentBone(const xml_node &node, const BoneMap &bones)
    {
        nanoem_mutable_model_bone_t *bone = bones.find(node.attribute("id").as_int())->second;
        BoneMap::const_iterator it = bones.find(node.child("P").attribute("id").as_int());
        if (it != bones.end()) {
            const nanoem_model_bone_t *parentBone = nanoemMutableModelBoneGetOriginObject(it->second);
            nanoemMutableModelBoneSetParentBoneObject(bone, parentBone);
            nanoemMutableModelBoneSetTargetBoneObject(bone, parentBone);
        }
    }
    static void
    setupVertexBoneWeights(const xml_node &node, const BoneMap &bones, nanoem_mutable_model_t *model)
    {
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices =
            nanoemModelGetAllVertexObjects(nanoemMutableModelGetOriginObject(model), &numVertices);
        const int boneId = node.attribute("id").as_int();
        nanoem_mutable_model_bone_t *bone = bones.find(boneId)->second;
        const xml_object_range<xml_named_node_iterator> &range = node.children("W");
        for (xml_named_node_iterator weight = range.begin(), end = range.end(); weight != end; ++weight) {
            unsigned int vertexIndex = weight->attribute("vi").as_uint();
            if (vertexIndex < numVertices) {
                nanoem_mutable_model_vertex_t *vertex =
                    nanoemMutableModelVertexCreateAsReference(vertices[vertexIndex]);
                nanoem_f32_t value = weight->attribute("w").as_float();
                if (value > 0 && value < 1) {
                    nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
                    nanoemMutableModelVertexSetBoneObject(vertex, nanoemMutableModelBoneGetOriginObject(bone), 0);
                    nanoemMutableModelVertexSetBoneObject(vertex, 0, 1);
                    nanoemMutableModelVertexSetBoneWeight(vertex, value, 0);
                }
                else {
                    nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                    nanoemMutableModelVertexSetBoneObject(vertex, nanoemMutableModelBoneGetOriginObject(bone), 0);
                }
                nanoemMutableModelVertexDestroy(vertex);
            }
        }
    }
    static void
    setupBoneLabels(const xml_node &node, const BoneMap &bones, const LabelMap &labels)
    {
        nanoem_mutable_status_t status;
        const int boneId = node.attribute("id").as_int();
        nanoem_mutable_model_bone_t *bone = bones.find(boneId)->second;
        LabelMap::const_iterator it = labels.find(node.attribute("group").as_int());
        if (it != labels.end()) {
            nanoem_mutable_model_label_t *label = it->second;
            nanoem_mutable_model_label_item_t *item =
                nanoemMutableModelLabelItemCreateFromBoneObject(label, nanoemMutableModelBoneGetOriginObject(bone));
            nanoemMutableModelLabelInsertItemObject(label, item, -1, &status);
            nanoemMutableModelLabelItemDestroy(item);
        }
    }

    nanoem_application_plugin_importer_t(nanoem_unicode_string_factory_t *factory)
        : m_factory(factory)
        , m_status(NANOEM_MUTABLE_STATUS_UNKNOWN)
    {
    }
    ~nanoem_application_plugin_importer_t()
    {
    }

    int
    setOption(nanoem_u32_t /* key */, const void * /* value */, nanoem_rsize_t /* size */)
    {
        return 0;
    }
    nanoem_model_t *
    importModel(const char *filePath)
    {
        nanoem_model_t *model = NULL;
        bx::CrtFileReader reader;
        if (bx::open(&reader, filePath, &m_error)) {
            nanoem_i32_t size = nanoem_i32_t(bx::getSize(&reader));
            if (size > 0) {
                nanoem_u8_t *data = new nanoem_u8_t[size];
                if (bx::read(&reader, data, size, &m_error) == size) {
                    nanomqo_buffer_t *buffer = nanomqoBufferCreate(data, size);
                    nanomqo_document_t *mqo = nanomqoDocumentCreate();
                    nanomqo_status_t status = nanomqoDocumentParse(mqo, buffer);
                    if (status == NANOMQO_STATUS_SUCCESS) {
                        xml_document root;
                        char xmlPath[1024];
                        bx::strlcpy(xmlPath, filePath, sizeof(xmlPath));
                        if (char *p = strrchr(xmlPath, '.')) {
                            bx::strlcpy(p, ".mqx", xmlPath + sizeof(xmlPath) - p);
                        }
                        if (root.load_file(xmlPath)) {
                            model = convertModel(mqo, root);
                        }
                    }
                    nanomqoBufferDestroy(buffer);
                    delete[] data;
                }
            }
            bx::close(&reader);
        }
        return model;
    }
    const char *
    failureReason() const
    {
        return 0;
    }
    const char *
    recoverySuggestion() const
    {
        return 0;
    }

    void
    constructModelBones(const xml_node &node, nanoem_mutable_model_t *model, BoneMap &bones)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_bone_t *bone = nanoemMutableModelBoneCreate(model);
        const char *name = node.attribute("name").as_string();
        nanoem_bool_t ok;
        nanoem_unicode_string_t *utf8Name = nanoemUnicodeStringFactoryCreateString(m_factory, name, strlen(name), &ok);
        nanoemMutableModelBoneSetName(bone, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoem_f32_t origin[3];
        origin[0] = node.attribute("rtX").as_float();
        origin[1] = node.attribute("rtY").as_float();
        origin[2] = node.attribute("rtZ").as_float();
        nanoemMutableModelBoneSetOrigin(bone, origin);
        const int boneId = node.attribute("id").as_int();
        const bool isDummy = node.attribute("isDummy").as_bool() && boneId != 1;
        nanoemMutableModelBoneSetUserHandleable(bone, isDummy);
        nanoemMutableModelBoneSetRotateable(bone, isDummy ? false : true);
        nanoemMutableModelBoneSetMovable(bone, node.attribute("isMovable").as_bool() && !isDummy);
        nanoemMutableModelBoneSetVisible(bone, node.attribute("isHide").as_bool() || isDummy ? false : true);
        nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
        nanoemMutableModelInsertBoneObject(model, bone, -1, &status);
        bones.insert(tinystl::make_pair(node.attribute("id").as_int(), bone));
    }
    void
    constructModelRootLabel(nanoem_mutable_model_t *model, LabelMap &labels)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_label_t *rootLabel = nanoemMutableModelLabelCreate(model);
        nanoem_bool_t ok;
        const char name[] = "Root";
        nanoem_unicode_string_t *utf8Name = nanoemUnicodeStringFactoryCreateString(m_factory, name, strlen(name), &ok);
        nanoemMutableModelLabelSetName(rootLabel, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
        nanoemMutableModelLabelSetSpecial(rootLabel, true);
        nanoemMutableModelInsertLabelObject(model, rootLabel, 0, &status);
        labels.insert(tinystl::make_pair(0, rootLabel));
    }
    void
    constructModelLabels(const xml_node &node, nanoem_mutable_model_t *model, LabelMap &labels)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_label_t *label = nanoemMutableModelLabelCreate(model);
        const char *name = node.attribute("name").as_string();
        nanoem_bool_t ok;
        nanoem_unicode_string_t *utf8Name = nanoemUnicodeStringFactoryCreateString(m_factory, name, strlen(name), &ok);
        nanoemMutableModelLabelSetName(label, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
        nanoemMutableModelInsertLabelObject(model, label, -1, &status);
        labels.insert(tinystl::make_pair(node.attribute("id").as_int(), label));
    }
    void
    constructModelMorph(
        const nanomqo_document_t *mqo, const xml_node &node, nanoem_mutable_model_t *model, MorphList &morphs)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_morph_t *morph = nanoemMutableModelMorphCreate(model);
        const char *name = node.attribute("name").as_string();
        nanoem_bool_t ok;
        nanoem_unicode_string_t *utf8Name = nanoemUnicodeStringFactoryCreateString(m_factory, name, strlen(name), &ok);
        nanoemMutableModelMorphSetName(morph, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
        const char *type = node.attribute("type").as_string();
        nanoemMutableModelMorphSetType(morph, NANOEM_MODEL_MORPH_TYPE_VERTEX);
        if (strcmp(type, "まゆ") == 0) {
            nanoemMutableModelMorphSetCategory(morph, NANOEM_MODEL_MORPH_CATEGORY_EYEBLOW);
        }
        else if (strcmp(type, "目") == 0) {
            nanoemMutableModelMorphSetCategory(morph, NANOEM_MODEL_MORPH_CATEGORY_EYE);
        }
        else if (strcmp(type, "リップ") == 0) {
            nanoemMutableModelMorphSetCategory(morph, NANOEM_MODEL_MORPH_CATEGORY_LIP);
        }
        else if (strcmp(type, "その他") == 0) {
            nanoemMutableModelMorphSetCategory(morph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
        }
        fillVerticesInMorph(mqo, name, morph);
        nanoemMutableModelInsertMorphObject(model, morph, -1, &status);
        morphs.push_back(morph);
    }
    void
    constructMaterial(const nanomqo_material_t *material, int numVertexIndices, nanoem_mutable_model_t *model)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_material_t *mutableMaterial = nanoemMutableModelMaterialCreate(model);
        const nanoem_f32_t *color = nanomqoMaterialGetColor(material);
        const nanoem_f32_t ambient = nanomqoMaterialGetAmbient(material);
        const nanoem_f32_t diffuse = nanomqoMaterialGetDiffuse(material);
        const nanoem_f32_t specular = nanomqoMaterialGetSpecular(material);
        const nanoem_f32_t ambientColor[3] = { color[0] * ambient, color[1] * ambient, color[2] * ambient },
                           diffuseColor[3] = { color[0] * diffuse, color[1] * diffuse, color[2] * diffuse },
                           specularColor[3] = { color[0] * specular, color[1] * specular, color[2] * specular };
        nanoemMutableModelMaterialSetAmbient(mutableMaterial, ambientColor);
        nanoemMutableModelMaterialSetDiffuse(mutableMaterial, diffuseColor);
        nanoemMutableModelMaterialSetSpecular(mutableMaterial, specularColor);
        nanoemMutableModelMaterialSetDiffuseOpacity(mutableMaterial, color[3]);
        nanoemMutableModelMaterialSetShininess(mutableMaterial, nanomqoMaterialGetPower(material));
        nanoemMutableModelMaterialSetEdgeOpacity(mutableMaterial, 1.0f);
        nanoemMutableModelMaterialSetEdgeSize(mutableMaterial, 1.0f);
        nanoemMutableModelMaterialSetEdgeEnabled(mutableMaterial, false);
        nanoemMutableModelMaterialSetNumVertexIndices(mutableMaterial, numVertexIndices);
        nanoemMutableModelMaterialSetCullingDisabled(mutableMaterial, nanomqoMaterialIsCullingDisabled(material));
        if (const char *path = nanomqoMaterialGetTexturePath(material)) {
            nanoem_mutable_model_texture_t *texture = nanoemMutableModelTextureCreate(model);
            nanoem_bool_t ok;
            nanoem_unicode_string_t *utf8Path =
                nanoemUnicodeStringFactoryCreateString(m_factory, path, strlen(path), &ok);
            nanoemMutableModelTextureSetPath(texture, utf8Path);
            nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Path);
            nanoemMutableModelMaterialSetDiffuseTextureObject(mutableMaterial, texture, &status);
            nanoemMutableModelTextureDestroy(texture);
        }
        const char *name = nanomqoMaterialGetName(material);
        nanoem_bool_t ok;
        nanoem_unicode_string_t *utf8Name = nanoemUnicodeStringFactoryCreateString(m_factory, name, strlen(name), &ok);
        nanoemMutableModelMaterialSetName(mutableMaterial, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
        nanoemMutableModelInsertMaterialObject(model, mutableMaterial, -1, &status);
        nanoemMutableModelMaterialDestroy(mutableMaterial);
    }
    void
    setupMorphLabels(const MorphList &morphs, nanoem_mutable_model_t *model)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_label_t *label = nanoemMutableModelLabelCreate(model);
        for (MorphList::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
            nanoem_mutable_model_label_item_t *item =
                nanoemMutableModelLabelItemCreateFromMorphObject(label, nanoemMutableModelMorphGetOriginObject(*it));
            nanoemMutableModelLabelInsertItemObject(label, item, -1, &status);
            nanoemMutableModelLabelItemDestroy(item);
        }
        nanoem_bool_t ok;
        const char name[] = "Expression";
        nanoem_unicode_string_t *utf8Name = nanoemUnicodeStringFactoryCreateString(m_factory, name, strlen(name), &ok);
        nanoemMutableModelLabelSetName(label, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
        nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
        nanoemMutableModelInsertLabelObject(model, label, 1, &status);
        nanoemMutableModelLabelDestroy(label);
    }
    void
    iterateNodesInBoneSetTag(const xml_node &parent, nanoem_mutable_model_t *model)
    {
        const xml_object_range<xml_node_iterator> &range = parent.children();
        BoneMap bones;
        LabelMap labels;
        for (xml_node_iterator node = range.begin(), end = range.end(); node != end; ++node) {
            if (strcmp(node->name(), "Bone") == 0) {
                constructModelBones(*node, model, bones);
            }
            else if (strcmp(node->name(), "Group") == 0) {
                constructModelLabels(*node, model, labels);
            }
        }
        constructModelRootLabel(model, labels);
        for (xml_node_iterator node = range.begin(), end = range.end(); node != end; ++node) {
            if (strcmp(node->name(), "Bone") == 0) {
                setupParentBone(*node, bones);
                setupVertexBoneWeights(*node, bones, model);
                setupBoneLabels(*node, bones, labels);
            }
        }
        for (BoneMap::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
            nanoemMutableModelBoneDestroy(it->second);
        }
        for (LabelMap::const_iterator it = labels.begin(), end = labels.end(); it != end; ++it) {
            nanoemMutableModelLabelDestroy(it->second);
        }
    }
    void
    iterateNodesInPluginTag(const nanomqo_document_t *mqo, const xml_node &parent, nanoem_mutable_model_t *model)
    {
        if (strncmp(parent.name(), "Plugin", sizeof("Plugin") - 1) == 0) {
            MorphList morphs;
            const xml_object_range<xml_node_iterator> &range = parent.children();
            for (xml_node_iterator node = range.begin(), end = range.end(); node != end; ++node) {
                const char *name = node->name();
                if (strcmp(name, "BoneSet") == 0) {
                    iterateNodesInBoneSetTag(*node, model);
                }
                else if (strcmp(name, "MorphSet") == 0) {
                    const xml_node &targetListNode = node->child("TargetList");
                    if (!targetListNode.empty()) {
                        if (const char *baseName = targetListNode.attribute("base").as_string()) {
                            nanoem_mutable_status_t status;
                            nanoem_mutable_model_morph_t *morph = nanoemMutableModelMorphCreate(model);
                            fillVerticesInMorph(mqo, baseName, morph);
                            nanoemMutableModelMorphSetCategory(morph, NANOEM_MODEL_MORPH_CATEGORY_BASE);
                            nanoemMutableModelMorphSetType(morph, NANOEM_MODEL_MORPH_TYPE_VERTEX);
                            nanoem_bool_t ok;
                            const char morphName[] = "base";
                            nanoem_unicode_string_t *utf8Name =
                                nanoemUnicodeStringFactoryCreateString(m_factory, morphName, strlen(morphName), &ok);
                            nanoemMutableModelMorphSetName(morph, utf8Name, NANOEM_LANGUAGE_TYPE_JAPANESE);
                            nanoemUnicodeStringFactoryDestroyString(m_factory, utf8Name);
                            nanoemMutableModelInsertMorphObject(model, morph, -1, &status);
                            nanoemMutableModelMorphDestroy(morph);
                        }
                        const xml_object_range<xml_node_iterator> &range = parent.children();
                        for (xml_node_iterator node = range.begin(), end = range.end(); node != end; ++node) {
                            constructModelMorph(mqo, *node, model, morphs);
                        }
                    }
                }
            }
            if (!morphs.empty()) {
                setupMorphLabels(morphs, model);
                for (MorphList::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
                    nanoemMutableModelMorphDestroy(*it);
                }
            }
        }
    }
    nanoem_model_t *
    convertModel(nanomqo_document_t *mqo, xml_document &root)
    {
        nanoem_mutable_status_t status;
        nanoem_mutable_model_t *model = nanoemMutableModelCreate(m_factory);
        nanomqo_rsize_t numObjects;
        nanomqo_object_t *const *objects = nanomqoDocumentGetAllObjects(mqo, &numObjects);
        UID2ObjectMap objectUIDMap;
        tinystl::unordered_set<int> vertexAttrUIDMap;
        for (nanomqo_rsize_t i = 0; i < numObjects; i++) {
            const nanomqo_object_t *object = objects[i];
            nanomqo_rsize_t numVertices, numVertexAttrUids;
            nanomqo_vertex_t *const *vertices = nanomqoObjectGetAllVertices(object, &numVertices);
            const int *vertexAttrUIDs = nanomqoObjectGetAllVertexAttributeUID(object, &numVertexAttrUids);
            if (numVertices == numVertexAttrUids) {
                for (nanomqo_rsize_t j = 0; j < numVertices; j++) {
                    int vertexAttrUID = vertexAttrUIDs[j];
                    if (vertexAttrUIDMap.insert(vertexAttrUID).second) {
                        const nanomqo_vertex_t *v = vertices[j];
                        const nanomqo_float32_t *origin = nanomqoVertexGetOrigin(v);
                        nanoem_mutable_model_vertex_t *mutableVertex = nanoemMutableModelVertexCreate(model);
                        nanoemMutableModelVertexSetOrigin(mutableVertex, origin);
                        nanoemMutableModelInsertVertexObject(model, mutableVertex, -1, &status);
                        nanoemMutableModelVertexDestroy(mutableVertex);
                        fprintf(stderr, "%d:%d\n", i, vertexAttrUID);
                    }
                }
                nanoem_rsize_t nv;
                nanoemModelGetAllVertexObjects(nanoemMutableModelGetOriginObject(model), &nv);
                fprintf(stderr, "%d:%d\n", numVertices, nv);
            }
            objectUIDMap.insert(tinystl::make_pair(nanomqoObjectGetUID(object), object));
        }
        MaterialFaceUIDSetMap materialFaceUIDSetMap;
        MaterialIndexListMap materialIndexListMap;
        nanoem_rsize_t numVertices;
        nanoem_model_vertex_t *const *vertices =
            nanoemModelGetAllVertexObjects(nanoemMutableModelGetOriginObject(model), &numVertices);
        for (nanomqo_rsize_t i = 0; i < numObjects; i++) {
            const nanomqo_object_t *object = objects[i];
            nanomqo_rsize_t numFaces;
            nanomqo_face_t *const *faces = nanomqoObjectGetAllFaces(object, &numFaces);
            for (nanomqo_rsize_t j = 0; j < numFaces; j++) {
                int numIndices;
                const nanomqo_face_t *face = faces[j];
                const int faceUID = nanomqoFaceGetUID(face);
                const int materialIndex = nanomqoFaceGetMaterialIndex(face);
                const bool addIndex = materialFaceUIDSetMap[materialIndex].insert(faceUID).second;
                nanomqoFaceGetVertexIndices(face, &numIndices);
                switch (numIndices) {
                case 3: {
                    constructVertex(vertices, face, 0, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 1, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 2, addIndex, materialIndexListMap);
                    break;
                }
                case 4: {
                    constructVertex(vertices, face, 0, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 1, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 2, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 0, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 2, addIndex, materialIndexListMap);
                    constructVertex(vertices, face, 3, addIndex, materialIndexListMap);
                    break;
                }
                default:
                    break;
                }
            }
        }
        nanomqo_rsize_t numMaterials;
        nanomqo_material_t *const *allMaterials = nanomqoDocumentGetAllMaterials(mqo, &numMaterials);
        for (nanomqo_rsize_t i = 0; i < numMaterials; i++) {
            const nanomqo_material_t *material = allMaterials[i];
            MaterialIndexListMap::const_iterator it = materialIndexListMap.find(i);
            if (it != materialIndexListMap.end()) {
                constructMaterial(material, it->second.size(), model);
            }
        }
        IndexList allIndices;
        for (MaterialIndexListMap::const_iterator it = materialIndexListMap.begin(), end = materialIndexListMap.end();
             it != end; ++it) {
            const IndexList &indices = it->second;
            allIndices.insert(allIndices.end(), indices.begin(), indices.end());
        }
        const xml_node &rootNode = root.child("MetasequoiaDocument");
        const xml_object_range<xml_node_iterator> &range = rootNode.children();
        for (xml_node_iterator it = range.begin(), end = range.end(); it != end; ++it) {
            iterateNodesInPluginTag(mqo, *it, model);
        }
        nanoemMutableModelSetCodecType(model, NANOEM_CODEC_TYPE_UTF8);
        nanoemMutableModelSetFormatType(model, NANOEM_MODEL_FORMAT_TYPE_PMX_2_0);
        nanoemMutableModelSetVertexIndices(model, allIndices.data(), allIndices.size(), &status);
        nanoem_mutable_buffer_t *buffer = nanoemMutableBufferCreate();
        nanoemMutableModelSaveToBuffer(model, buffer, &status);
#if 1
        if (FILE *fp = fopen("./output.pmx", "wb")) {
            nanoem_buffer_t *origin = nanoemMutableBufferCreateBufferObject(buffer);
            fwrite(nanoemBufferGetDataPtr(origin), nanoemBufferGetLength(origin), 1, fp);
            fclose(fp);
            nanoemBufferDestroy(origin);
        }
#endif
        nanoem_model_t *newModel = nanoemMutableModelGetOriginObjectReference(model);
        nanoemMutableBufferDestroy(buffer);
        nanoemMutableModelDestroy(model);
        return newModel;
    }

    nanoem_unicode_string_factory_t *m_factory;
    nanoem_mutable_status_t m_status;
    bx::Error m_error;
};

void APIENTRY
nanoemApplicationPluginImporterInitialize(void)
{
}

nanoem_application_plugin_importer_t *APIENTRY
nanoemApplicationPluginImporterCreate(nanoem_unicode_string_factory_t *factory)
{
    return new nanoem_application_plugin_importer_t(factory);
}

int APIENTRY
nanoemApplicationPluginImporterSetOption(
    nanoem_application_plugin_importer_t *plugin, nanoem_u32_t key, const void *value, nanoem_rsize_t size)
{
    return nanoem_is_not_null(plugin) ? plugin->setOption(key, value, size) : 0;
}

nanoem_model_t *APIENTRY
nanoemApplicationPluginImporterImportModelFromFile(nanoem_application_plugin_importer_t *plugin, const char *path)
{
    return nanoem_is_not_null(plugin) ? plugin->importModel(path) : NULL;
}

nanoem_motion_t *APIENTRY
nanoemApplicationPluginImporterImportMotionFromFile(
    nanoem_application_plugin_importer_t * /* plugin */, const char * /* path */)
{
    return NULL;
}

const char *APIENTRY
nanoemApplicationPluginImporterGetFailureReason(const nanoem_application_plugin_importer_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->failureReason() : NULL;
}

const char *APIENTRY
nanoemApplicationPluginImporterGetRecoverySuggestion(const nanoem_application_plugin_importer_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->recoverySuggestion() : NULL;
}

void APIENTRY
nanoemApplicationPluginImporterDestroy(nanoem_application_plugin_importer_t *plugin)
{
    delete plugin;
}
