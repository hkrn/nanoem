/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Effect.h"
#include "emapp/ICamera.h"
#include "emapp/ILight.h"
#include "emapp/Model.h"
#include "emapp/Progress.h"

#include "glm/gtc/matrix_inverse.hpp"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

namespace {

static const Matrix4x4 kWorldMatrix = Matrix4x4(0.5);

static void
effectAccessoryEffectPass(const char *path, ProjectPtr &o, Effect::NamedByteArrayMap &uniformBuffer)
{
    Project *project = o.get()->m_project;
    Accessory *accessory = o->createAccessory();
    project->addAccessory(accessory);
    Effect *effect = o->createSourceEffect(accessory, path, true);
    Progress progress(project, 0);
    Error error;
    effect->upload(effect::kAttachmentTypeNone, progress, error);
    nanoem_rsize_t numMaterials;
    nanodxm_material_t *const *materials = nanodxmDocumentGetMaterials(accessory->data(), &numMaterials);
    ITechnique *technique = effect->findTechnique(Effect::kPassTypeObject, materials[0], 0, numMaterials, accessory);
    IPass *pass = technique->execute(accessory, false);
    pass->setGlobalParameters(accessory, project);
    pass->setCameraParameters(project->globalCamera(), kWorldMatrix);
    pass->setLightParameters(project->globalLight(), false);
    pass->setAllAccessoryParameters(accessory, project);
    pass->setMaterialParameters(materials[0]);
    pass->setShadowMapParameters(project->shadowCamera(), Matrix4x4(1));
    Effect::PassUniformBufferMap passUniformBuffer;
    effect->getPassUniformBuffer(passUniformBuffer);
    uniformBuffer = passUniformBuffer["P"];
}

static void
effectModelEffectPass(const char *path, ProjectPtr &o, Effect::NamedByteArrayMap &uniformBuffer)
{
    Project *project = o.get()->m_project;
    Model *model = o->createModel();
    project->addModel(model);
    Effect *effect = o->createSourceEffect(model, path, true);
    Progress progress(project, 0);
    Error error;
    effect->upload(effect::kAttachmentTypeNone, progress, error);
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    ITechnique *technique = effect->findTechnique(Effect::kPassTypeObject, materials[0], 0, numMaterials, model);
    IPass *pass = technique->execute(model, false);
    pass->setGlobalParameters(model, project);
    pass->setCameraParameters(project->globalCamera(), kWorldMatrix);
    pass->setLightParameters(project->globalLight(), false);
    pass->setAllModelParameters(model, project);
    pass->setMaterialParameters(materials[0]);
    pass->setShadowMapParameters(project->shadowCamera(), Matrix4x4(1));
    Effect::PassUniformBufferMap passUniformBuffer;
    effect->getPassUniformBuffer(passUniformBuffer);
    uniformBuffer = passUniformBuffer["P"];
}

static nanoem_f32_t
extractFloat(const Effect::NamedByteArrayMap &uniformBuffer, const char *name)
{
    Effect::NamedByteArrayMap::const_iterator it = uniformBuffer.find(name);
    return it != uniformBuffer.end() ? *reinterpret_cast<const nanoem_f32_t *>(it->second.data()) : FLT_MAX;
}

static Vector3
extractVector3(const Effect::NamedByteArrayMap &uniformBuffer, const char *name)
{
    Effect::NamedByteArrayMap::const_iterator it = uniformBuffer.find(name);
    return it != uniformBuffer.end() ? glm::make_vec3(reinterpret_cast<const nanoem_f32_t *>(it->second.data()))
                                     : Vector3(FLT_MAX);
}

static Vector4
extractVector4(const Effect::NamedByteArrayMap &uniformBuffer, const char *name)
{
    Effect::NamedByteArrayMap::const_iterator it = uniformBuffer.find(name);
    return it != uniformBuffer.end() ? glm::make_vec4(reinterpret_cast<const nanoem_f32_t *>(it->second.data()))
                                     : Vector4(FLT_MAX);
}

static Matrix4x4
extractMatrix(const Effect::NamedByteArrayMap &uniformBuffer, const char *name)
{
    Effect::NamedByteArrayMap::const_iterator it = uniformBuffer.find(name);
    return it != uniformBuffer.end() ? glm::make_mat4(reinterpret_cast<const nanoem_f32_t *>(it->second.data()))
                                     : Matrix4x4(FLT_MAX);
}

} /* namespace anonymous */

TEST_CASE("effect_parameters_camera_matrix", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        ICamera *camera = o.get()->m_project->globalCamera();
        Matrix4x4 view, projection;
        camera->getViewTransform(view, projection);
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/camera/matrix.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 6);
        CHECK(extractMatrix(uniformBuffer, "camera_world") == kWorldMatrix);
        CHECK(extractMatrix(uniformBuffer, "camera_view") == view);
        CHECK(extractMatrix(uniformBuffer, "camera_projection") == projection);
        CHECK(extractMatrix(uniformBuffer, "camera_world_view") == view * kWorldMatrix);
        CHECK(extractMatrix(uniformBuffer, "camera_view_projection") == projection * view);
        CHECK(extractMatrix(uniformBuffer, "camera_world_view_projection") == projection * view * kWorldMatrix);
    }
}

TEST_CASE("effect_parameters_camera_transpose_matrix", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        ICamera *camera = o.get()->m_project->globalCamera();
        Matrix4x4 view, projection;
        camera->getViewTransform(view, projection);
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/camera/matrix_transpose.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 6);
        CHECK(extractMatrix(uniformBuffer, "camera_transpose_world") == glm::transpose(kWorldMatrix));
        CHECK(extractMatrix(uniformBuffer, "camera_transpose_view") == glm::transpose(view));
        CHECK(extractMatrix(uniformBuffer, "camera_transpose_projection") == glm::transpose(projection));
        CHECK(extractMatrix(uniformBuffer, "camera_transpose_world_view") == glm::transpose(view * kWorldMatrix));
        CHECK(extractMatrix(uniformBuffer, "camera_transpose_view_projection") == glm::transpose(projection * view));
        CHECK(extractMatrix(uniformBuffer, "camera_transpose_world_view_projection") ==
            glm::transpose(projection * view * kWorldMatrix));
    }
}

TEST_CASE("effect_parameters_camera_inverse_matrix", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        ICamera *camera = o.get()->m_project->globalCamera();
        Matrix4x4 view, projection;
        camera->getViewTransform(view, projection);
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/camera/matrix_inverse.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 6);
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_world") == glm::inverse(kWorldMatrix));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_view") == glm::inverse(view));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_projection") == glm::inverse(projection));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_world_view") == glm::inverse(view * kWorldMatrix));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_view_projection") == glm::inverse(projection * view));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_world_view_projection") ==
            glm::inverse(projection * view * kWorldMatrix));
    }
}

TEST_CASE("effect_parameters_camera_inverse_transpose_matrix", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        ICamera *camera = o.get()->m_project->globalCamera();
        Matrix4x4 view, projection;
        camera->getViewTransform(view, projection);
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/camera/matrix_inverse_transpose.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 6);
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_transpose_world") ==
            glm::transpose(glm::inverse(kWorldMatrix)));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_transpose_view") == glm::transpose(glm::inverse(view)));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_transpose_projection") ==
            glm::transpose(glm::inverse(projection)));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_transpose_world_view") ==
            glm::transpose(glm::inverse(view * kWorldMatrix)));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_transpose_view_projection") ==
            glm::transpose(glm::inverse(projection * view)));
        CHECK(extractMatrix(uniformBuffer, "camera_inverse_transpose_world_view_projection") ==
            glm::transpose(glm::inverse(projection * view * kWorldMatrix)));
    }
}

TEST_CASE("effect_parameters_camera_position_direction", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        ICamera *camera = o.get()->m_project->globalCamera();
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/camera/position_direction.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 4);
        CHECK_THAT(extractVector3(uniformBuffer, "camera_position_3"), Equals(camera->position()));
        CHECK_THAT(extractVector4(uniformBuffer, "camera_position_4"), Equals(Vector4(camera->position(), 1)));
        CHECK_THAT(extractVector3(uniformBuffer, "camera_direction_3"), Equals(camera->direction()));
        CHECK_THAT(extractVector4(uniformBuffer, "camera_direction_4"), Equals(Vector4(camera->direction(), 0)));
    }
}

TEST_CASE("effect_parameters_light_position_direction", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        ILight *light = o.get()->m_project->globalLight();
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/light/position_direction.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 4);
        const Vector3 position(-light->direction());
        const Vector3 normalizedDirection(glm::normalize(light->direction()));
        CHECK_THAT(extractVector3(uniformBuffer, "light_position_3"), Equals(position));
        CHECK_THAT(extractVector4(uniformBuffer, "light_position_4"), Equals(Vector4(position, 0)));
        CHECK_THAT(extractVector3(uniformBuffer, "light_direction_3"), Equals(normalizedDirection));
        CHECK_THAT(extractVector4(uniformBuffer, "light_direction_4"), Equals(Vector4(normalizedDirection, 0)));
    }
}

TEST_CASE("effect_parameters_controlobjects_accessory", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Effect::NamedByteArrayMap uniformBuffer;
        effectAccessoryEffectPass("effects/parameters/controlobjects/accessory.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 15);
        CHECK(extractFloat(uniformBuffer, "accessory_visible") == 1.0f);
        CHECK(extractFloat(uniformBuffer, "accessory_scaling") == 10.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "accessory_offset_3"), Equals(Vector3(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "accessory_offset_4"), Equals(Vector4(0, 0, 0, 1)));
        // CHECK(extractMatrix(uniformBuffer, "accessory_world_matrix") == Matrix4x4(10));
        CHECK(extractFloat(uniformBuffer, "accessory_x") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "accessory_y") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "accessory_z") == 0.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "accessory_xyz"), Equals(Vector3(0)));
        CHECK(extractFloat(uniformBuffer, "accessory_rx") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "accessory_ry") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "accessory_rz") == 0.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "accessory_rxyz"), Equals(Vector3(0)));
        CHECK(extractFloat(uniformBuffer, "accessory_si") == 10.0f);
        CHECK(extractFloat(uniformBuffer, "accessory_tr") == 1.0f);
    }
}

TEST_CASE("effect_parameters_controlobjects_model", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/controlobjects/model.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 9);
        CHECK(extractFloat(uniformBuffer, "model_visible") == 1.0f);
        CHECK(extractFloat(uniformBuffer, "model_scaling") == 1.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "model_offset_3"), Equals(Vector3(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "model_offset_4"), Equals(Vector4(0, 0, 0, 1)));
        CHECK(extractMatrix(uniformBuffer, "model_world_matrix") == Matrix4x4(1));
        CHECK_THAT(extractVector4(uniformBuffer, "model_bone_3"), Equals(Vector4(0, 0, 0, 1)));
        CHECK_THAT(extractVector4(uniformBuffer, "model_bone_4"), Equals(Vector4(0, 0, 0, 1)));
        CHECK(extractMatrix(uniformBuffer, "model_bone_4x4") == Matrix4x4(1));
        CHECK(extractFloat(uniformBuffer, "model_morph") == 0.0f);
    }
}

TEST_CASE("effect_parameters_controlobjects_no_such_accessory", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Effect::NamedByteArrayMap uniformBuffer;
        effectAccessoryEffectPass("effects/parameters/controlobjects/no_such_accessory.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 17);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_visible") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_scaling") == 10.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_accessory_offset_3"), Equals(Vector3(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "no_such_accessory_offset_4"), Equals(Vector4(0, 0, 0, 1)));
        CHECK(extractMatrix(uniformBuffer, "no_such_accessory_world_matrix") == glm::scale(Matrix4x4(1), Vector3(10)));
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_accessory_bone_3"), Equals(Vector3(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "no_such_accessory_bone_4"), Equals(Vector4(0, 0, 0, 1)));
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_x") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_y") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_z") == 0.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_accessory_xyz"), Equals(Vector3(0)));
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_rx") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_ry") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_rz") == 0.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_accessory_rxyz"), Equals(Vector3(0)));
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_si") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_accessory_tr") == 0.0f);
    }
}

TEST_CASE("effect_parameters_controlobjects_no_such_model", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/controlobjects/no_such_model.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 19);
        CHECK(extractFloat(uniformBuffer, "no_such_model_visible") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_scaling") == 1.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_model_offset_3"), Equals(Vector3(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "no_such_model_offset_4"), Equals(Vector4(0, 0, 0, 1)));
        CHECK(extractMatrix(uniformBuffer, "no_such_model_world_matrix") == Matrix4x4(1));
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_model_bone_3"), Equals(Vector3(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "no_such_model_bone_4"), Equals(Vector4(0, 0, 0, 1)));
        CHECK(extractMatrix(uniformBuffer, "no_such_model_bone_4x4") == Matrix4x4(1));
        CHECK(extractFloat(uniformBuffer, "no_such_model_morph") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_x") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_y") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_z") == 0.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_model_xyz"), Equals(Vector3(0)));
        CHECK(extractFloat(uniformBuffer, "no_such_model_rx") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_ry") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_rz") == 0.0f);
        CHECK_THAT(extractVector3(uniformBuffer, "no_such_model_rxyz"), Equals(Vector3(0)));
        CHECK(extractFloat(uniformBuffer, "no_such_model_si") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "no_such_model_tr") == 0.0f);
    }
}

TEST_CASE("effect_parameters_application", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        o.get()->m_project->setUptimeSeconds(40);
        o.get()->m_project->setUptimeSeconds(42);
        o.get()->m_project->seek(63, true);
        o.get()->m_project->seek(84, true);
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/application.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 9);
        CHECK_THAT(extractVector4(uniformBuffer, "viewport_size"), Equals(Vector4(960, 480, 0, 0)));
        CHECK(extractFloat(uniformBuffer, "time") == 42.0f);
        CHECK(extractFloat(uniformBuffer, "elapsed_time") == 2.0f);
        CHECK(extractFloat(uniformBuffer, "time_sync_in_edit_mode") == 2.8f);
        CHECK(extractFloat(uniformBuffer, "elapsed_time_sync_in_edit_mode") == 0.7f);
        CHECK_THAT(extractVector4(uniformBuffer, "mouse_position"), Equals(Vector4(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "left_mouse_down"), Equals(Vector4(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "middle_mouse_down"), Equals(Vector4(0)));
        CHECK_THAT(extractVector4(uniformBuffer, "right_mouse_down"), Equals(Vector4(0)));
    }
}

TEST_CASE("effect_parameters_builtin_variables", "[emapp][effect]")
{
    TestScope scope;
    {
        ProjectPtr o = scope.createProject();
        Effect::NamedByteArrayMap uniformBuffer;
        effectModelEffectPass("effects/parameters/builtin_variables.fx", o, uniformBuffer);
        CHECK(uniformBuffer.size() == 10);
        CHECK(extractFloat(uniformBuffer, "parthf") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "spadd") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "transp") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "use_texture") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "use_spheremap") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "use_subtexture") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "use_toon") == 1.0f);
        CHECK(extractFloat(uniformBuffer, "opadd") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "VertexCount") == 0.0f);
        CHECK(extractFloat(uniformBuffer, "SubsetCount") == 1.0f);
    }
}
