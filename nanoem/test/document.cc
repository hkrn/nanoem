/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

#include "nanoem/ext/document.h"

using namespace nanoem::test;

class DocumentScope : public BaseScope {
public:
    DocumentScope();
    ~DocumentScope();

    nanoem_mutable_document_t *newDocument();
    nanoem_mutable_document_accessory_t *newAccessory();
    nanoem_mutable_document_model_t *newModel();

private:
    std::vector<nanoem_mutable_document_t *> m_documents;
    std::vector<nanoem_mutable_document_accessory_t *> m_accessories;
    std::vector<nanoem_mutable_document_model_t *> m_models;
};

DocumentScope::DocumentScope()
    : BaseScope()
{
}

DocumentScope::~DocumentScope()
{
    for (auto it : m_accessories) {
        nanoemMutableDocumentAccessoryDestroy(it);
    }
    for (auto it : m_models) {
        nanoemMutableDocumentModelDestroy(it);
    }
    for (auto it : m_documents) {
        nanoemMutableDocumentDestroy(it);
    }
}

nanoem_mutable_document_t *
DocumentScope::newDocument()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_document_t *document = nanoemMutableDocumentCreate(m_factory, &status);
    m_documents.push_back(document);
    return document;
}

nanoem_mutable_document_accessory_t *
DocumentScope::newAccessory()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_document_accessory_t *accessory = nanoemMutableDocumentAccessoryCreate(m_documents.back(), &status);
    m_accessories.push_back(accessory);
    return accessory;
}

nanoem_mutable_document_model_t *
DocumentScope::newModel()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_document_model_t *model = nanoemMutableDocumentModelCreate(m_documents.back(), &status);
    m_models.push_back(model);
    return model;
}

TEST_CASE("null_document", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentGetAccessoryIndexAfterModel(NULL) == -1);
    CHECK(nanoemDocumentGetAllAccessoryObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentGetAllModelObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentGetAudioPath(NULL) == NULL);
    CHECK(nanoemDocumentGetBackgroundImageOffsetX(NULL) == 0);
    CHECK(nanoemDocumentGetBackgroundImageOffsetY(NULL) == 0);
    CHECK(nanoemDocumentGetBackgroundImagePath(NULL) == NULL);
    CHECK(nanoemDocumentGetBackgroundImageScaleFactor(NULL) == 0);
    CHECK(nanoemDocumentGetBackgroundVideoOffsetX(NULL) == 0);
    CHECK(nanoemDocumentGetBackgroundVideoOffsetY(NULL) == 0);
    CHECK(nanoemDocumentGetBackgroundVideoPath(NULL) == NULL);
    CHECK(nanoemDocumentGetBackgroundVideoScaleFactor(NULL) == 0);
    CHECK(nanoemDocumentGetCameraObject(NULL) == NULL);
    CHECK_THAT(nanoemDocumentGetEdgeColor(NULL), EqualsOne(0));
    CHECK(nanoemDocumentGetGravityObject(NULL) == NULL);
    CHECK(nanoemDocumentGetLightObject(NULL) == NULL);
    CHECK(nanoemDocumentGetPhysicsSimulationMode(NULL) == 0);
    CHECK(nanoemDocumentGetPreferredFPS(NULL) == 0);
    CHECK(nanoemDocumentGetSelectedAccessoryIndex(NULL) == -1);
    CHECK(nanoemDocumentGetSelectedModelIndex(NULL) == -1);
    CHECK(nanoemDocumentGetSelfShadowObject(NULL) == NULL);
    CHECK(nanoemDocumentGetTimelineWidth(NULL) == 0);
    CHECK(nanoemDocumentGetViewportHeight(NULL) == 0);
    CHECK(nanoemDocumentGetViewportWidth(NULL) == 0);
}

TEST_CASE("null_document_accessory", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentAccessoryGetDrawOrderIndex(NULL) == -1);
    CHECK(nanoemDocumentAccessoryGetIndex(NULL) == -1);
    CHECK(nanoemDocumentAccessoryGetName(NULL) == NULL);
    CHECK(nanoemDocumentAccessoryGetOpacity(NULL) == 0);
    CHECK_THAT(nanoemDocumentAccessoryGetOrientation(NULL), EqualsOne(0));
    CHECK(nanoemDocumentAccessoryGetParentModelBoneName(NULL) == 0);
    CHECK(nanoemDocumentAccessoryGetParentModelObject(NULL) == 0);
    CHECK(nanoemDocumentAccessoryGetPath(NULL) == 0);
    CHECK(nanoemDocumentAccessoryGetScaleFactor(NULL) == 0);
    CHECK_THAT(nanoemDocumentAccessoryGetTranslation(NULL), EqualsOne(0));
}

TEST_CASE("null_document_accessory_keyframe", "[nanoem]")
{
    CHECK(nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(NULL) == NULL);
    CHECK(nanoemDocumentAccessoryKeyframeGetOpacity(NULL) == 0);
    CHECK_THAT(nanoemDocumentAccessoryKeyframeGetOrientation(NULL), EqualsOne(0));
    CHECK(nanoemDocumentAccessoryKeyframeGetParentModelBoneName(NULL) == NULL);
    CHECK(nanoemDocumentAccessoryKeyframeGetParentModelObject(NULL) == NULL);
    CHECK(nanoemDocumentAccessoryKeyframeGetScaleFactor(NULL) == 0);
    CHECK_THAT(nanoemDocumentAccessoryKeyframeGetTranslation(NULL), EqualsOne(0));
    CHECK_FALSE(nanoemDocumentAccessoryKeyframeIsShadowEnabled(NULL));
    CHECK_FALSE(nanoemDocumentAccessoryKeyframeIsVisible(NULL));
}

TEST_CASE("null_document_camera", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentCameraGetAllCameraKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK_THAT(nanoemDocumentCameraGetAngle(NULL), EqualsOne(0));
    CHECK(nanoemDocumentCameraGetDistance(NULL) == 0);
    CHECK(nanoemDocumentCameraGetFov(NULL) == 0);
    CHECK_THAT(nanoemDocumentCameraGetLookAt(NULL), EqualsOne(0));
    CHECK_THAT(nanoemDocumentCameraGetPosition(NULL), EqualsOne(0));
    CHECK_FALSE(nanoemDocumentCameraIsPerspectiveEnabled(NULL));
}

TEST_CASE("null_document_camera_keyframe", "[nanoem]")
{
    CHECK_THAT(nanoemDocumentCameraKeyframeGetAngle(NULL), EqualsOne(0));
    CHECK(nanoemDocumentCameraKeyframeGetBaseKeyframeObject(NULL) == NULL);
    CHECK(nanoemDocumentCameraKeyframeGetDistance(NULL) == 0);
    CHECK(nanoemDocumentCameraKeyframeGetFov(NULL) == 0);
    CHECK_THAT(nanoemDocumentCameraKeyframeGetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE),
        EqualsU8(20, 20, 107, 107));
    CHECK_THAT(nanoemDocumentCameraKeyframeGetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE),
        EqualsU8(20, 20, 107, 107));
    CHECK_THAT(nanoemDocumentCameraKeyframeGetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV),
        EqualsU8(20, 20, 107, 107));
    CHECK_THAT(nanoemDocumentCameraKeyframeGetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X),
        EqualsU8(20, 20, 107, 107));
    CHECK_THAT(nanoemDocumentCameraKeyframeGetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y),
        EqualsU8(20, 20, 107, 107));
    CHECK_THAT(nanoemDocumentCameraKeyframeGetInterpolation(NULL, NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z),
        EqualsU8(20, 20, 107, 107));
    CHECK_THAT(nanoemDocumentCameraKeyframeGetLookAt(NULL), EqualsOne(0));
    CHECK(nanoemDocumentCameraKeyframeGetParentModelBoneName(NULL) == NULL);
    CHECK(nanoemDocumentCameraKeyframeGetParentModelObject(NULL) == NULL);
}

TEST_CASE("null_document_gravity", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentGravityGetAllGravityKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentGravityGetAcceleration(NULL) == 0);
    CHECK_THAT(nanoemDocumentGravityGetDirection(NULL), EqualsOne(0));
    CHECK(nanoemDocumentGravityGetNoise(NULL) == 0);
}

TEST_CASE("null_document_gravity_keyframe", "[nanoem]")
{
    CHECK(nanoemDocumentGravityKeyframeGetAcceleration(NULL) == 0);
    CHECK(nanoemDocumentGravityKeyframeGetBaseKeyframeObject(NULL) == NULL);
    CHECK_THAT(nanoemDocumentGravityKeyframeGetDirection(NULL), EqualsOne(0));
    CHECK(nanoemDocumentGravityKeyframeGetNoise(NULL) == 0);
}

TEST_CASE("null_document_light", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentLightGetAllLightKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK_THAT(nanoemDocumentLightGetColor(NULL), EqualsOne(0));
    CHECK_THAT(nanoemDocumentLightGetDirection(NULL), EqualsOne(0));
}

TEST_CASE("null_document_light_keyframe", "[nanoem]")
{
    CHECK(nanoemDocumentLightKeyframeGetBaseKeyframeObject(NULL) == NULL);
    CHECK_THAT(nanoemDocumentLightKeyframeGetColor(NULL), EqualsOne(0));
    CHECK_THAT(nanoemDocumentLightKeyframeGetDirection(NULL), EqualsOne(0));
}

TEST_CASE("null_document_model", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentModelGetAllBoneKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllBoneNameObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllBoneStateObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllConstraintStateObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllModelKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllMorphKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllMorphNameObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllMorphStateObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelGetAllOutsideParentStateObjects(NULL, &num_objects) == NULL);
    CHECK(nanoemDocumentModelGetConstraintBoneName(NULL, 0) == NULL);
    CHECK(nanoemDocumentModelGetDrawOrderIndex(NULL) == -1);
    CHECK(nanoemDocumentModelGetEdgeWidth(NULL) == 0);
    CHECK(nanoemDocumentModelGetIndex(NULL) == -1);
    CHECK(nanoemDocumentModelGetLastFrameIndex(NULL) == 0);
    CHECK(nanoemDocumentModelGetName(NULL, NANOEM_LANGUAGE_TYPE_ENGLISH) == NULL);
    CHECK(nanoemDocumentModelGetName(NULL, NANOEM_LANGUAGE_TYPE_JAPANESE) == NULL);
    CHECK(nanoemDocumentModelGetOutsideParentSubjectBoneName(NULL, 0) == NULL);
    CHECK(nanoemDocumentModelGetPath(NULL) == 0);
    CHECK(nanoemDocumentModelGetSelectedBoneName(NULL) == NULL);
    CHECK(nanoemDocumentModelGetSelectedMorphName(NULL, NANOEM_MODEL_MORPH_CATEGORY_BASE) == NULL);
    CHECK(nanoemDocumentModelGetSelectedMorphName(NULL, NANOEM_MODEL_MORPH_CATEGORY_EYE) == NULL);
    CHECK(nanoemDocumentModelGetSelectedMorphName(NULL, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW) == NULL);
    CHECK(nanoemDocumentModelGetSelectedMorphName(NULL, NANOEM_MODEL_MORPH_CATEGORY_LIP) == NULL);
    CHECK(nanoemDocumentModelGetSelectedMorphName(NULL, NANOEM_MODEL_MORPH_CATEGORY_OTHER) == NULL);
    CHECK(nanoemDocumentModelGetTransformOrderIndex(NULL) == -1);
}

TEST_CASE("null_document_model_keyframe", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentModelKeyframeGetAllModelConstraintStateObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentModelKeyframeGetAllOutsideParentObjects(NULL, &num_objects) == NULL);
    CHECK(nanoemDocumentModelKeyframeGetBaseKeyframeObject(NULL) == NULL);
    CHECK_FALSE(nanoemDocumentModelKeyframeIsVisible(NULL));
}

TEST_CASE("null_document_self_shadow", "[nanoem]")
{
    nanoem_rsize_t num_objects;
    CHECK(nanoemDocumentSelfShadowGetAllSelfShadowKeyframeObjects(NULL, &num_objects) == NULL);
    CHECK(num_objects == 0);
    CHECK(nanoemDocumentSelfShadowGetDistance(NULL) == 0);
    CHECK_FALSE(nanoemDocumentSelfShadowIsEnabled(NULL));
}

TEST_CASE("null_document_self_shadow_keyframe", "[nanoem]")
{
    CHECK(nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(NULL) == NULL);
    CHECK(nanoemDocumentSelfShadowKeyframeGetDistance(NULL) == 0);
    CHECK(nanoemDocumentSelfShadowKeyframeGetMode(NULL) == 0);
}

TEST_CASE("mutable_document_accessory_manipulate", "[nanoem]")
{
    DocumentScope scope;
    nanoem_mutable_document_t *document = scope.newDocument();
    nanoem_mutable_document_accessory_t *first_accessory = scope.newAccessory();
    nanoem_mutable_document_accessory_t *second_accessory = scope.newAccessory();
    nanoem_mutable_document_accessory_t *third_accessory = scope.newAccessory();
    nanoem_rsize_t num_accessorys;
    nanoem_document_accessory_t *const *accessorys;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableDocumentInsertAccessoryObject(document, first_accessory, -1, &status);
    nanoemMutableDocumentInsertAccessoryObject(document, second_accessory, 1, &status);
    nanoemMutableDocumentInsertAccessoryObject(document, third_accessory, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableDocumentInsertAccessoryObject(document, first_accessory, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        accessorys = nanoemDocumentGetAllAccessoryObjects(nanoemMutableDocumentGetOrigin(document), &num_accessorys);
        CHECK(num_accessorys == 3);
        CHECK(accessorys[0] == nanoemMutableDocumentAccessoryGetOrigin(third_accessory));
        CHECK(accessorys[1] == nanoemMutableDocumentAccessoryGetOrigin(first_accessory));
        CHECK(accessorys[2] == nanoemMutableDocumentAccessoryGetOrigin(second_accessory));
    }
    SECTION("remove")
    {
        nanoemMutableDocumentRemoveAccessoryObject(document, first_accessory, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        accessorys = nanoemDocumentGetAllAccessoryObjects(nanoemMutableDocumentGetOrigin(document), &num_accessorys);
        CHECK(num_accessorys == 2);
        CHECK(accessorys[0] == nanoemMutableDocumentAccessoryGetOrigin(third_accessory));
        CHECK(accessorys[2] == nanoemMutableDocumentAccessoryGetOrigin(second_accessory));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableDocumentRemoveAccessoryObject(document, first_accessory, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableDocumentRemoveAccessoryObject(document, first_accessory, &status);
        CHECK(status == NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_NOT_FOUND);
    }
}

TEST_CASE("mutable_document_model_manipulate", "[nanoem]")
{
    DocumentScope scope;
    nanoem_mutable_document_t *document = scope.newDocument();
    nanoem_mutable_document_model_t *first_model = scope.newModel();
    nanoem_mutable_document_model_t *second_model = scope.newModel();
    nanoem_mutable_document_model_t *third_model = scope.newModel();
    nanoem_rsize_t num_models;
    nanoem_document_model_t *const *models;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableDocumentInsertModelObject(document, first_model, -1, &status);
    nanoemMutableDocumentInsertModelObject(document, second_model, 1, &status);
    nanoemMutableDocumentInsertModelObject(document, third_model, 0, &status);
    SECTION("duplication should reject")
    {
        nanoemMutableDocumentInsertModelObject(document, first_model, -1, &status);
        CHECK(status == NANOEM_STATUS_ERROR_DOCUMENT_MODEL_ALREADY_EXISTS);
    }
    SECTION("check order")
    {
        models = nanoemDocumentGetAllModelObjects(nanoemMutableDocumentGetOrigin(document), &num_models);
        CHECK(num_models == 3);
        CHECK(models[0] == nanoemMutableDocumentModelGetOrigin(third_model));
        CHECK(models[1] == nanoemMutableDocumentModelGetOrigin(first_model));
        CHECK(models[2] == nanoemMutableDocumentModelGetOrigin(second_model));
    }
    SECTION("remove")
    {
        nanoemMutableDocumentRemoveModelObject(document, first_model, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        models = nanoemDocumentGetAllModelObjects(nanoemMutableDocumentGetOrigin(document), &num_models);
        CHECK(num_models == 2);
        CHECK(models[0] == nanoemMutableDocumentModelGetOrigin(third_model));
        CHECK(models[2] == nanoemMutableDocumentModelGetOrigin(second_model));
    }
    SECTION("duplicated remove should reject")
    {
        nanoemMutableDocumentRemoveModelObject(document, first_model, &status);
        CHECK(status == NANOEM_STATUS_SUCCESS);
        nanoemMutableDocumentRemoveModelObject(document, first_model, &status);
        CHECK(status == NANOEM_STATUS_ERROR_DOCUMENT_MODEL_NOT_FOUND);
    }
}
