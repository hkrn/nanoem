/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ImGuiWindow.h"

#include "../protoc/plugin.pb-c.h"
#include "emapp/Accessory.h"
#include "emapp/ApplicationPreference.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/IModalDialog.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/IState.h"
#include "emapp/ITrack.h"
#include "emapp/ListUtils.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Model.h"
#include "emapp/PerspectiveCamera.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/ResourceBundle.h"
#include "emapp/StringUtils.h"
#include "emapp/command/ModelObjectCommand.h"
#include "emapp/command/UpdateCameraCommand.h"
#include "emapp/internal/AccessoryValueState.h"
#include "emapp/internal/BoneValueState.h"
#include "emapp/internal/CameraValueState.h"
#include "emapp/internal/DraggingMorphState.h"
#include "emapp/internal/ImGuiApplicationMenuBuilder.h"
#include "emapp/internal/LightValueState.h"
#include "emapp/internal/MotionKeyframeSelection.h"
#include "emapp/internal/imgui/AccessoryOutsideParentDialog.h"
#include "emapp/internal/imgui/ActiveAccessorySelector.h"
#include "emapp/internal/imgui/ActiveModelSelector.h"
#include "emapp/internal/imgui/BoneBiasDialog.h"
#include "emapp/internal/imgui/BoneCorrectionDialog.h"
#include "emapp/internal/imgui/BoneKeyframeInterpolationCurveGraphDialog.h"
#include "emapp/internal/imgui/BoneParameterDialog.h"
#include "emapp/internal/imgui/CameraCorrectionDialog.h"
#include "emapp/internal/imgui/CameraKeyframeInterpolationCurveGraphDialog.h"
#include "emapp/internal/imgui/CameraParametersDialog.h"
#include "emapp/internal/imgui/CameraParentModelBoneSelector.h"
#include "emapp/internal/imgui/CameraParentModelSelector.h"
#include "emapp/internal/imgui/ConstraintSelector.h"
#include "emapp/internal/imgui/EffectParameterDialog.h"
#include "emapp/internal/imgui/GizmoController.h"
#include "emapp/internal/imgui/LazyPublishCanCopyEventCommand.h"
#include "emapp/internal/imgui/LazyPublishCanPasteEventCommand.h"
#include "emapp/internal/imgui/LazyReloadEffectCommand.h"
#include "emapp/internal/imgui/LazySetActiveAccessoryCommand.h"
#include "emapp/internal/imgui/LazySetActiveModelBoneCommand.h"
#include "emapp/internal/imgui/LazySetActiveModelCommand.h"
#include "emapp/internal/imgui/LazySetActiveModelMorphCommand.h"
#include "emapp/internal/imgui/LazySetCameraOutsideParentCommand.h"
#include "emapp/internal/imgui/LazySetFrameIndexCommand.h"
#include "emapp/internal/imgui/ModelDrawOrderDialog.h"
#include "emapp/internal/imgui/ModelEdgeDialog.h"
#include "emapp/internal/imgui/ModelEditCommandDialog.h"
#include "emapp/internal/imgui/ModelIOPluginDialog.h"
#include "emapp/internal/imgui/ModelKeyframeSelector.h"
#include "emapp/internal/imgui/ModelOutsideParentDialog.h"
#include "emapp/internal/imgui/ModelParameterDialog.h"
#include "emapp/internal/imgui/ModelTransformOrderDialog.h"
#include "emapp/internal/imgui/MorphCorrectionDialog.h"
#include "emapp/internal/imgui/MorphSelector.h"
#include "emapp/internal/imgui/MotionIOPluginDialog.h"
#include "emapp/internal/imgui/PhysicsSimulationDialog.h"
#include "emapp/internal/imgui/PreferenceDialog.h"
#include "emapp/internal/imgui/ProjectKeyframeSelector.h"
#include "emapp/internal/imgui/ScaleAllSelectedKeyframesDialog.h"
#include "emapp/internal/imgui/SelfShadowConfigurationDialog.h"
#include "emapp/internal/imgui/UVEditDialog.h"
#include "emapp/internal/imgui/UberMotionKeyframeSelection.h"
#include "emapp/internal/imgui/VertexWeightPainter.h"
#include "emapp/internal/imgui/ViewportSettingDialog.h"
#include "emapp/model/IGizmo.h"
#include "emapp/private/CommonInclude.h"

#include "glm/gtx/matrix_query.hpp"
#include "sokol/sokol_time.h"

#define PAR_SHAPES_T uint32_t
#include "par/par_shapes.h"

#define SOKOL_GFX_INCLUDED /* stub */
#include "sokol/util/sokol_gfx_imgui.h"

namespace nanoem {
namespace internal {
namespace {

#include "emapp/private/shaders/ui_fs_glsl_core33.h"
#include "emapp/private/shaders/ui_fs_glsl_es3.h"
#include "emapp/private/shaders/ui_fs_msl_macos.h"
#include "emapp/private/shaders/ui_fs_spirv.h"
#include "emapp/private/shaders/ui_ps_dxbc.h"
#include "emapp/private/shaders/ui_vs_dxbc.h"
#include "emapp/private/shaders/ui_vs_glsl_core33.h"
#include "emapp/private/shaders/ui_vs_glsl_es3.h"
#include "emapp/private/shaders/ui_vs_msl_macos.h"
#include "emapp/private/shaders/ui_vs_spirv.h"

BX_ALIGN_DECL_16(struct)
UniformBlock
{
    ImVec4 m_viewportRect;
    ImVec4 m_scaleFactor;
    ImVec4 m_mul;
    ImVec4 m_add;
};

static const Vector4 kColorRed(1, 0, 0, 1);
static const Vector4 kColorGreen(0, 1, 0, 1);
static const Vector4 kColorBlue(0, 0, 1, 1);
static const Vector4 kColorTranslucentRed(1, 0, 0, 0.5f);
static const Vector4 kColorTranslucentGreen(0, 1, 0, 0.5f);
static const Vector4 kColorTranslucentBlue(0, 0, 1, 0.5f);
static const Vector4 kColorGray(0.5f, 0.5f, 0.5f, 1);

static bool
isSelectingBoneHandle(const IModelObjectSelection *selection, Project::RectangleType type) NANOEM_DECL_NOEXCEPT
{
    const bool movable = selection->areAllBonesMovable(), rotateable = selection->areAllBonesRotateable();
    return (movable && type >= Project::kRectangleTranslateX && type <= Project::kRectangleTranslateZ) ||
        (rotateable && type >= Project::kRectangleOrientateX && type <= Project::kRectangleOrientateZ);
}

static bool
isDraggingBoneAxisAlignedState(const IState *state) NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (state && state->isGrabbingHandle()) {
        switch (state->type()) {
        case IState::kTypeDraggingBoneAxisAlignedOrientateActiveBoneState:
        case IState::kTypeDraggingBoneAxisAlignedTranslateActiveBoneState:
            result = true;
            break;
        default:
            break;
        }
    }
    return result;
}

class VertexWeightPainterUtils : private NonCopyable {
public:
    static const char *selectedVertexType(const nanoem_model_vertex_type_t type) NANOEM_DECL_NOEXCEPT;
    static const char *selectedVertexWeightPainterType(
        const model::IVertexWeightPainter::Type type, const ITranslator *translator) NANOEM_DECL_NOEXCEPT;
    static void layoutSelectBoneComboBox(
        Model *activeModel, nanoem_rsize_t offset, model::IVertexWeightPainter *painter);
};

const char *
VertexWeightPainterUtils::selectedVertexType(const nanoem_model_vertex_type_t type) NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
        return "BDEF1";
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF2: {
        return "BDEF2";
    }
    case NANOEM_MODEL_VERTEX_TYPE_BDEF4: {
        return "BDEF4";
    }
    case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
        return "SDEF";
    }
    case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
        return "QDEF";
    }
    default:
        return "(Unknown)";
    }
}

const char *
VertexWeightPainterUtils::selectedVertexWeightPainterType(
    const model::IVertexWeightPainter::Type type, const ITranslator *translator) NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case model::IVertexWeightPainter::kTypeAirBrush:
        return translator->translate("nanoem.gui.panel.model.edit.weight-paint.painter-type.air-brush");
    case model::IVertexWeightPainter::kTypeBaseBrush:
        return translator->translate("nanoem.gui.panel.model.edit.weight-paint.painter-type.base-brush");
    default:
        return "(Unknown)";
    }
}

void
VertexWeightPainterUtils::layoutSelectBoneComboBox(
    Model *activeModel, nanoem_rsize_t offset, model::IVertexWeightPainter *painter)
{
    char buffer[Inline::kNameStackBufferSize];
    const nanoem_model_bone_t *activeBonePtr = painter->vertexBone(offset);
    const model::Bone *activeBone = model::Bone::cast(activeBonePtr);
    StringUtils::format(buffer, sizeof(buffer), "##bones[%jd]", offset);
    if (ImGui::BeginCombo(buffer, activeBone ? activeBone->nameConstString() : "(null)")) {
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(activeModel->data(), &numBones);
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            const nanoem_model_bone_t *bonePtr = bones[i];
            if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                const bool selected = bonePtr == activeBonePtr;
                StringUtils::format(buffer, sizeof(buffer), "%s##bones[%jd][%jd]", bone->nameConstString(), offset, i);
                if (ImGui::Selectable(buffer, selected)) {
                    painter->setVertexBone(bonePtr, offset);
                }
            }
        }
        ImGui::EndCombo();
    }
}

struct WaveFormPanelDrawer {
    typedef nanoem_f32_t (*CallbackHandler)(void *data, int idx);

    static nanoem_f32_t callback(void *userData, int offset) NANOEM_DECL_NOEXCEPT;
    WaveFormPanelDrawer(const Project *project, nanoem_u32_t numVisibleMarkers);

    nanoem_f32_t plotSample32(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t plotSample24(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t plotSample16(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;
    nanoem_f32_t plotSample8(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;
    nanoem_rsize_t sampleBytesOffset(nanoem_rsize_t offset) const NANOEM_DECL_NOEXCEPT;

    const ByteArray *m_linearPCMSamplesPtr;
    nanoem_rsize_t m_base;
    nanoem_rsize_t m_length;
    nanoem_rsize_t m_bytesPerSample;
};

nanoem_f32_t
WaveFormPanelDrawer::callback(void *userData, int offset) NANOEM_DECL_NOEXCEPT
{
    const WaveFormPanelDrawer *drawer = static_cast<const WaveFormPanelDrawer *>(userData);
    nanoem_f32_t value = 0.0f;
    switch (drawer->m_bytesPerSample) {
    case 4: {
        value = drawer->plotSample32(offset);
        break;
    }
    case 3: {
        value = drawer->plotSample24(offset);
        break;
    }
    case 2: {
        value = drawer->plotSample16(offset);
        break;
    }
    case 1: {
        value = drawer->plotSample8(offset);
        break;
    }
    default:
        break;
    }
    return glm::clamp(value, -1.0f, 1.0f);
}

WaveFormPanelDrawer::WaveFormPanelDrawer(const Project *project, nanoem_u32_t numVisibleMarkers)
    : m_linearPCMSamplesPtr(nullptr)
    , m_base(0)
    , m_length(0)
    , m_bytesPerSample(0)
{
    const IAudioPlayer *player = project->audioPlayer();
    if (player->isLoaded()) {
        const nanoem_f64_t denominator(project->baseFPS());
        const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
        const nanoem_u32_t bytesPerSample = player->bitsPerSample() / 8,
                           bytesPerSecond = player->sampleRate() * player->numChannels() * bytesPerSample,
                           length = static_cast<nanoem_u32_t>((numVisibleMarkers / denominator) * bytesPerSecond),
                           base = static_cast<nanoem_u32_t>((frameIndex / denominator) * bytesPerSecond);
        m_linearPCMSamplesPtr = player->linearPCMSamples();
        m_length = length - length % bytesPerSample;
        m_base = base - base % bytesPerSample;
        m_bytesPerSample = bytesPerSample;
    }
}

nanoem_f32_t
WaveFormPanelDrawer::plotSample32(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t value = 0;
    const nanoem_rsize_t offset = sampleBytesOffset(index);
    if (m_linearPCMSamplesPtr && offset < m_linearPCMSamplesPtr->size() - m_bytesPerSample) {
        value = *reinterpret_cast<const nanoem_i32_t *>(m_linearPCMSamplesPtr->data() + offset) / 2147483647.0f;
    }
    return value;
}

nanoem_f32_t
WaveFormPanelDrawer::plotSample24(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t value = 0;
    const nanoem_rsize_t offset = sampleBytesOffset(index);
    if (m_linearPCMSamplesPtr && offset < m_linearPCMSamplesPtr->size() - m_bytesPerSample) {
        const nanoem_u8_t *ptr = m_linearPCMSamplesPtr->data() + offset;
        value = Inline::readI24(ptr) / 8388607.0f;
    }
    return value;
}

nanoem_f32_t
WaveFormPanelDrawer::plotSample16(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t value = 0;
    const nanoem_rsize_t offset = sampleBytesOffset(index);
    if (m_linearPCMSamplesPtr && offset < m_linearPCMSamplesPtr->size() - m_bytesPerSample) {
        value = *reinterpret_cast<const nanoem_i16_t *>(m_linearPCMSamplesPtr->data() + offset) / 32767.0f;
    }
    return value;
}

nanoem_f32_t
WaveFormPanelDrawer::plotSample8(nanoem_rsize_t index) const NANOEM_DECL_NOEXCEPT
{
    nanoem_f32_t value = 0;
    const nanoem_rsize_t offset = sampleBytesOffset(index);
    if (m_linearPCMSamplesPtr && offset < m_linearPCMSamplesPtr->size() - m_bytesPerSample) {
        value = *reinterpret_cast<const char *>(m_linearPCMSamplesPtr->data() + offset) / 127.0f;
    }
    return value;
}

nanoem_rsize_t
WaveFormPanelDrawer::sampleBytesOffset(nanoem_rsize_t sample) const NANOEM_DECL_NOEXCEPT
{
    return m_base + sample * m_bytesPerSample;
}

class BasePrimitiveDialog : public imgui::BaseNonModalDialogWindow {
protected:
    BasePrimitiveDialog(BaseApplicationService *applicationPtr, const Vector3 &t, const Vector3 &r, const Vector3 &s);

    void layoutTransform();
    void applyTransform(par_shapes_mesh *mesh);
    void createUndoCommand(Project *project, const par_shapes_mesh *mesh);

private:
    Vector3 m_translation;
    Vector3 m_rotation;
    Vector3 m_scale;
};

BasePrimitiveDialog::BasePrimitiveDialog(
    BaseApplicationService *applicationPtr, const Vector3 &t, const Vector3 &r, const Vector3 &s)
    : imgui::BaseNonModalDialogWindow(applicationPtr)
    , m_translation(t)
    , m_rotation(r)
    , m_scale(s)
{
}

void
BasePrimitiveDialog::layoutTransform()
{
    ImGui::TextUnformatted(
        tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.translation"));
    ImGui::DragFloat3("##translation", glm::value_ptr(m_translation));
    ImGui::TextUnformatted(
        tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.rotation"));
    ImGui::DragFloat3("##rotation", glm::value_ptr(m_rotation), 1.0f, -180.0f, 180.0f);
    ImGui::TextUnformatted(
        tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.scale"));
    ImGui::DragFloat3("##scale", glm::value_ptr(m_scale), 1.0f, 0.0001f, 10000.0f);
}

void
BasePrimitiveDialog::applyTransform(par_shapes_mesh *mesh)
{
    const Quaternion q(glm::radians(m_rotation));
    par_shapes_scale(mesh, m_scale.x, m_scale.y, m_scale.z);
    par_shapes_rotate(mesh, glm::angle(q), glm::value_ptr(glm::axis(q)));
    par_shapes_translate(mesh, m_translation.x, m_translation.y, m_translation.z);
}

void
BasePrimitiveDialog::createUndoCommand(Project *project, const par_shapes_mesh *mesh)
{
    command::CreateMaterialCommand::MutableVertexList vertices;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Model *activeModel = project->activeModel();
    for (nanoem_rsize_t i = 0, numPoints = mesh->npoints; i < numPoints; i++) {
        nanoem_mutable_model_vertex_t *vertexPtr = nanoemMutableModelVertexCreate(activeModel->data(), &status);
        {
            const Vector4 position(glm::make_vec3(&mesh->points[i * 3]), 1);
            nanoemMutableModelVertexSetOrigin(vertexPtr, glm::value_ptr(position));
        }
        if (mesh->normals) {
            const Vector4 normal(glm::make_vec3(&mesh->normals[i * 3]), 0);
            nanoemMutableModelVertexSetNormal(vertexPtr, glm::value_ptr(normal));
        }
        if (mesh->tcoords) {
            const Vector4 texcoord(glm::make_vec2(&mesh->tcoords[i * 2]), 0, 0);
            nanoemMutableModelVertexSetTexCoord(vertexPtr, glm::value_ptr(texcoord));
        }
        vertices.push_back(vertexPtr);
    }
    VertexIndexList indices;
    for (nanoem_rsize_t i = 0, numTriangles = mesh->ntriangles; i < numTriangles; i++) {
        const PAR_SHAPES_T *triangles = &mesh->triangles[i * 3];
        indices.push_back(*triangles);
        indices.push_back(*(triangles + 1));
        indices.push_back(*(triangles + 2));
    }
    undo_command_t *command = command::CreateMaterialCommand::create(activeModel, String(), vertices, indices);
    activeModel->pushUndo(command);
}

class CreateConePrimitiveDialog : public BasePrimitiveDialog {
public:
    static const char *const kIdentifier;

    CreateConePrimitiveDialog(BaseApplicationService *applicationPtr);

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;

private:
    int m_slices;
    int m_stacks;
};

const char *const CreateConePrimitiveDialog::kIdentifier = "dialog.model.primitive.cone";

CreateConePrimitiveDialog::CreateConePrimitiveDialog(BaseApplicationService *service)
    : BasePrimitiveDialog(service, Constants::kZeroV3, Vector3(-90, 0, 0), Vector3(5))
    , m_slices(16)
    , m_stacks(10)
{
}

bool
CreateConePrimitiveDialog::draw(Project *project)
{
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 10;
    bool visible = project->isModelEditingEnabled();
    if (open("Create Cone", kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        layoutTransform();
        addSeparator();
        ImGui::DragInt("##slices", &m_slices, 1.0f, 3, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.slices"));
        ImGui::DragInt("##stacks", &m_stacks, 1.0f, 1, 10000,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.stacks"));
        addSeparator();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            par_shapes_mesh *mesh = par_shapes_create_cone(m_slices, m_stacks);
            applyTransform(mesh);
            createUndoCommand(project, mesh);
            par_shapes_free_mesh(mesh);
            break;
        }
        case kResponseTypeCancel: {
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

class CreateCubePrimitiveDialog : public BasePrimitiveDialog {
public:
    static const char *const kIdentifier;

    CreateCubePrimitiveDialog(BaseApplicationService *applicationPtr);

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;
};

const char *const CreateCubePrimitiveDialog::kIdentifier = "dialog.model.primitive.cube";

CreateCubePrimitiveDialog::CreateCubePrimitiveDialog(BaseApplicationService *service)
    : BasePrimitiveDialog(service, Vector3(-2.5f, 0, -2.5f), Constants::kZeroV3, Vector3(5))
{
}

bool
CreateCubePrimitiveDialog::draw(Project *project)
{
    BX_UNUSED_1(project);
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 6;
    bool visible = project->isModelEditingEnabled();
    if (open("Create Cube", kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        layoutTransform();
        addSeparator();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            par_shapes_mesh *mesh = par_shapes_create_cube();
            applyTransform(mesh);
            createUndoCommand(project, mesh);
            par_shapes_free_mesh(mesh);
            break;
        }
        case kResponseTypeCancel: {
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

class CreateCylinderPrimitiveDialog : public BasePrimitiveDialog {
public:
    static const char *const kIdentifier;

    CreateCylinderPrimitiveDialog(BaseApplicationService *applicationPtr);

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;

private:
    int m_slices;
    int m_stacks;
};

const char *const CreateCylinderPrimitiveDialog::kIdentifier = "dialog.model.primitive.cylinder";

CreateCylinderPrimitiveDialog::CreateCylinderPrimitiveDialog(BaseApplicationService *service)
    : BasePrimitiveDialog(service, Constants::kZeroV3, Vector3(-90, 0, 0), Vector3(5))
    , m_slices(16)
    , m_stacks(10)
{
}

bool
CreateCylinderPrimitiveDialog::draw(Project *project)
{
    BX_UNUSED_1(project);
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 8;
    bool visible = project->isModelEditingEnabled();
    if (open("Create Cylinder", kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        layoutTransform();
        addSeparator();
        ImGui::DragInt("##slices", &m_slices, 1.0f, 3, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.slices"));
        ImGui::DragInt("##stacks", &m_stacks, 1.0f, 1, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.stacks"));
        addSeparator();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            par_shapes_mesh *mesh = par_shapes_create_cylinder(m_slices, m_stacks);
            applyTransform(mesh);
            createUndoCommand(project, mesh);
            par_shapes_free_mesh(mesh);
            break;
        }
        case kResponseTypeCancel: {
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

class CreateSpherePrimitiveDialog : public BasePrimitiveDialog {
public:
    static const char *const kIdentifier;

    CreateSpherePrimitiveDialog(BaseApplicationService *applicationPtr);

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;

private:
    int m_slices;
    int m_stacks;
};

const char *const CreateSpherePrimitiveDialog::kIdentifier = "dialog.model.primitive.sphere";

CreateSpherePrimitiveDialog::CreateSpherePrimitiveDialog(BaseApplicationService *service)
    : BasePrimitiveDialog(service, Vector3(0, 5, 0), Constants::kZeroV3, Vector3(5))
    , m_slices(16)
    , m_stacks(10)
{
}

bool
CreateSpherePrimitiveDialog::draw(Project *project)
{
    BX_UNUSED_1(project);
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 8;
    bool visible = project->isModelEditingEnabled();
    if (open("Create Sphere", kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        layoutTransform();
        addSeparator();
        ImGui::DragInt("##slices", &m_slices, 1.0f, 3, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.slices"));
        ImGui::DragInt("##stacks", &m_stacks, 1.0f, 1, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.stacks"));
        addSeparator();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            par_shapes_mesh *mesh = par_shapes_create_parametric_sphere(m_slices, m_stacks);
            applyTransform(mesh);
            createUndoCommand(project, mesh);
            par_shapes_free_mesh(mesh);
            break;
        }
        case kResponseTypeCancel: {
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

class CreateTorusPrimitiveDialog : public BasePrimitiveDialog {
public:
    static const char *const kIdentifier;

    CreateTorusPrimitiveDialog(BaseApplicationService *applicationPtr);
    ~CreateTorusPrimitiveDialog() NANOEM_DECL_NOEXCEPT;

    bool draw(Project *project) NANOEM_DECL_OVERRIDE;

private:
    nanoem_f32_t m_radius;
    int m_slices;
    int m_stacks;
};

const char *const CreateTorusPrimitiveDialog::kIdentifier = "dialog.model.primitive.torus";

CreateTorusPrimitiveDialog::CreateTorusPrimitiveDialog(BaseApplicationService *service)
    : BasePrimitiveDialog(service, Constants::kZeroV3, Vector3(90, 0, 0), Vector3(5))
    , m_radius(0.5f)
    , m_slices(16)
    , m_stacks(10)
{
}

CreateTorusPrimitiveDialog::~CreateTorusPrimitiveDialog() NANOEM_DECL_NOEXCEPT
{
}

bool
CreateTorusPrimitiveDialog::draw(Project *project)
{
    BX_UNUSED_1(project);
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 9;
    bool visible = project->isModelEditingEnabled();
    if (open("Create Torus", kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        layoutTransform();
        addSeparator();
        ImGui::DragInt("##slices", &m_slices, 1.0f, 3, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.slices"));
        ImGui::DragInt("##stacks", &m_stacks, 1.0f, 3, 256,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.stacks"));
        ImGui::SliderFloat("##radius", &m_radius, 0.1f, 1.0f,
            tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.parameters.radius"));
        addSeparator();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            par_shapes_mesh *mesh = par_shapes_create_torus(m_slices, m_stacks, m_radius);
            applyTransform(mesh);
            createUndoCommand(project, mesh);
            par_shapes_free_mesh(mesh);
            break;
        }
        case kResponseTypeCancel: {
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

class DrawUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    static const Vector3 kColorRed;
    static const Vector3 kColorGreen;
    static const Vector3 kColorBlue;
    static const Vector3 kColorYellow;
    static const nanoem_f32_t kKappa;

    static Vector2SI32 deviceScaleCursorActiveBoneInViewport(const Project *project) NANOEM_DECL_NOEXCEPT;
    static bool isDraggingBoneState(const IState *value) NANOEM_DECL_NOEXCEPT;
    static void drawBoneMoveHandle(IPrimitive2D *primitive, const Model *model, bool isGrabbingHandle);
    static void drawBoneRotateHandle(IPrimitive2D *primitive, const Model *model, bool isGrabbingHandle);
    static void drawActiveBonePoint(IPrimitive2D *primitive, const Vector4 &activeBoneColor, const Project *project);
    static void drawCameraLookAtPoint(IPrimitive2D *primitive, const ICamera *camera, const Project *project);
    static void drawVertexWeightPainter(
        IPrimitive2D *primitive, const model::IVertexWeightPainter *brush, const Project *project);
};

const Vector3 DrawUtils::kColorRed = Vector3(1, 0, 0);
const Vector3 DrawUtils::kColorGreen = Vector3(0, 1, 0);
const Vector3 DrawUtils::kColorBlue = Vector3(0, 0, 1);
const Vector3 DrawUtils::kColorYellow = Vector3(1, 1, 0);
const nanoem_f32_t DrawUtils::kKappa = 0.5522847498307933984022516322796f;

Vector2SI32
DrawUtils::deviceScaleCursorActiveBoneInViewport(const Project *project) NANOEM_DECL_NOEXCEPT
{
    Vector2SI32 deviceScaleCursor(0);
    if (const Model *model = project->activeModel()) {
        if (const model::Bone *bone = model::Bone::cast(model->activeBone())) {
            deviceScaleCursor =
                project->activeCamera()->toDeviceScreenCoordinateInViewport(bone->worldTransformOrigin());
        }
    }
    return deviceScaleCursor;
}

bool
DrawUtils::isDraggingBoneState(const IState *value) NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (value) {
        switch (value->type()) {
        case IState::kTypeDraggingBoneAxisAlignedOrientateActiveBoneState:
        case IState::kTypeDraggingBoneAxisAlignedTranslateActiveBoneState:
        case IState::kTypeDraggingBoneOrientateActiveBoneState:
        case IState::kTypeDraggingBoneTranslateActiveBoneState:
            result = true;
            break;
        default:
            break;
        }
    }
    return result;
}

void
DrawUtils::drawBoneMoveHandle(IPrimitive2D *primitive, const Model *activeModel, bool isGrabbingHandle)
{
    const Project *project = activeModel->project();
    const Model::AxisType type = activeModel->transformAxisType();
    const Vector2 center(deviceScaleCursorActiveBoneInViewport(project));
    const nanoem_f32_t length = project->deviceScaleCircleRadius() * 10.0f,
                       thickness = project->logicalScaleCircleRadius();
    primitive->strokeLine(center, Vector2(center.x + length, center.y),
        Vector4(kColorRed, type == Model::kAxisTypeX || !isGrabbingHandle ? 1.0 : 0.25), thickness);
    primitive->strokeLine(center, Vector2(center.x, center.y - length),
        Vector4(kColorGreen, type == Model::kAxisTypeY || !isGrabbingHandle ? 1.0 : 0.25), thickness);
}

void
DrawUtils::drawBoneRotateHandle(IPrimitive2D *primitive, const Model *activeModel, bool isGrabbingHandle)
{
    const Project *project = activeModel->project();
    const Model::AxisType type = activeModel->transformAxisType();
    const Vector2 center(deviceScaleCursorActiveBoneInViewport(project));
    nanoem_f32_t radius = project->deviceScaleCircleRadius() * 7.5f,
                 thickness = project->logicalScaleCircleRadius() * 1.5f;
    const nanoem_f32_t x1 = center.x - radius;
    const nanoem_f32_t y1 = center.y - radius;
    const nanoem_f32_t x2 = center.x + radius;
    const nanoem_f32_t y2 = center.y + radius;
    primitive->strokeLine(Vector2(center.x - radius, center.y), Vector2(center.x + radius, center.y),
        Vector4(kColorBlue, type == Model::kAxisTypeZ || !isGrabbingHandle ? 1.0 : 0.25), thickness);
    primitive->strokeLine(Vector2(center.x, center.y - radius), Vector2(center.x, center.y + radius),
        Vector4(kColorRed, type == Model::kAxisTypeX || !isGrabbingHandle ? 1.0 : 0.25), thickness);
    const Vector4 green(kColorGreen, type == Model::kAxisTypeY || !isGrabbingHandle ? 1.0 : 0.25);
    primitive->strokeCurve(Vector2(x1, center.y + 1), Vector2(x1, center.y - radius * kKappa),
        Vector2(center.x - radius * kKappa, y1), Vector2(center.x + 1, y1), green, thickness);
    primitive->strokeCurve(Vector2(center.x - 1, y1), Vector2(center.x + radius * kKappa, y1),
        Vector2(x2, center.y - radius * kKappa), Vector2(x2, center.y + 1), green, thickness);
    primitive->strokeCurve(Vector2(x2, center.y - 1), Vector2(x2, center.y + radius * kKappa),
        Vector2(center.x + radius * kKappa, y2), Vector2(center.x - 1, y2), green, thickness);
    primitive->strokeCurve(Vector2(center.x + 1, y2), Vector2(center.x - radius * kKappa, y2),
        Vector2(x1, center.y + radius * kKappa), Vector2(x1, center.y - 1), green, thickness);
}

void
DrawUtils::drawActiveBonePoint(IPrimitive2D *primitive, const Vector4 &activeBoneColor, const Project *project)
{
    const Vector2SI32 center(deviceScaleCursorActiveBoneInViewport(project));
    if (center.x != 0 && center.y != 0) {
        const nanoem_f32_t radius = project->deviceScaleCircleRadius(), extent = radius * 2;
        primitive->fillCircle(Vector4(center.x - radius, center.y - radius, extent, extent), activeBoneColor);
    }
}

void
DrawUtils::drawCameraLookAtPoint(IPrimitive2D *primitive, const ICamera *camera, const Project *project)
{
    const Vector2 center(camera->toDeviceScreenCoordinateInViewport(camera->boundLookAt()));
    const nanoem_f32_t circleRadius = project->deviceScaleCircleRadius();
    const nanoem_f32_t innerRadius = circleRadius * 0.65f;
    primitive->strokeCircle(
        Vector4(center.x - circleRadius, center.y - circleRadius, circleRadius * 2, circleRadius * 2),
        Vector4(1, 0, 0, 1), 2.0f);
    primitive->fillCircle(
        Vector4(center.x - innerRadius, center.y - innerRadius, innerRadius * 2, innerRadius * 2), Vector4(1, 0, 0, 1));
}

void
DrawUtils::drawVertexWeightPainter(
    IPrimitive2D *primitive, const model::IVertexWeightPainter *brush, const Project *project)
{
    const Vector4UI16 layoutRect(project->deviceScaleUniformedViewportLayoutRect());
    const Vector2SI32 center(project->deviceScaleMovingCursorPosition() - Vector2SI32(layoutRect));
    const nanoem_f32_t radius = brush->radius(), extent = radius * 2;
    primitive->fillCircle(Vector4(center.x - radius, center.y - radius, extent, extent), Vector4(kColorRed, 0.5));
}

} /* namespace anonymous */

using namespace imgui;

const Vector2UI16 ImGuiWindow::kMinimumMainWindowSize = Vector2UI16(1080, 700);
const Vector2UI16 ImGuiWindow::kMinimumViewportWindowSize = Vector2UI16(720, 480);
const nanoem_f32_t ImGuiWindow::kFontSize = 16.0f;
const nanoem_f32_t ImGuiWindow::kWindowRounding = 5.0f;
const nanoem_f32_t ImGuiWindow::kLeftPaneWidth = 150.0f;
const nanoem_f32_t ImGuiWindow::kTranslationStepFactor = 0.1f;
const nanoem_f32_t ImGuiWindow::kOrientationStepFactor = 0.1f;
const nanoem_f32_t ImGuiWindow::kTimelineDefaultWidthRatio = 0.3f;
const nanoem_f32_t ImGuiWindow::kTimelineMinWidthRatio = 0.25f;
const nanoem_f32_t ImGuiWindow::kTimelineMaxWidthRatio = 0.75f;
const nanoem_f32_t ImGuiWindow::kTimelineSnapGridRatio = 0.02f;
const nanoem_f32_t ImGuiWindow::kTimelineSeekerPanelWidth = 240.0f;
const nanoem_f32_t ImGuiWindow::kTimelineUndoPanelWidth = 180.0f;
const nanoem_f32_t ImGuiWindow::kTimelineTrackMaxWidth = 150.0f;
const nanoem_f32_t ImGuiWindow::kTimelineKeyframeActionPanelWidth = 320.0f;
const nanoem_f32_t ImGuiWindow::kModelEditCommandWidth = 270.0f;
const nanoem_f32_t ImGuiWindow::kDrawCircleSegmentCount = 24.0f;
const ImGuiDataType ImGuiWindow::kFrameIndexDataType = ImGuiDataType_U32;
const ImU32 ImGuiWindow::kMainWindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus;
const ImU32 ImGuiWindow::kColorWindowBg = IM_COL32(45, 45, 45, 255);
const ImU32 ImGuiWindow::kColorMenuBarBg = IM_COL32(45, 45, 45, 255);
const ImU32 ImGuiWindow::kColorTitleBg = IM_COL32(40, 40, 40, 255);
const ImU32 ImGuiWindow::kColorTitleBgActive = IM_COL32(65, 65, 65, 255);
const ImU32 ImGuiWindow::kColorTab = IM_COL32(40, 40, 40, 255);
const ImU32 ImGuiWindow::kColorTabActive = IM_COL32(65, 65, 65, 255);
const ImU32 ImGuiWindow::kColorTabHovered = IM_COL32(80, 80, 80, 255);
const ImU32 ImGuiWindow::kColorResizeGrip = IM_COL32(65, 65, 65, 255);
const ImU32 ImGuiWindow::kColorResizeGripHovered = IM_COL32(88, 88, 88, 255);
const ImU32 ImGuiWindow::kColorResizeGripActive = IM_COL32(127, 127, 127, 255);
const ImU32 ImGuiWindow::kColorChildBg = IM_COL32(45, 45, 45, 255);
const ImU32 ImGuiWindow::kColorPopupBg = IM_COL32(45, 45, 45, 255);
const ImU32 ImGuiWindow::kColorBorder = IM_COL32(65, 65, 65, 255);
const ImU32 ImGuiWindow::kColorText = IM_COL32(175, 175, 175, 255);
const ImU32 ImGuiWindow::kColorTextDisabled = IM_COL32(88, 88, 88, 255);
const ImU32 ImGuiWindow::kColorButton = IM_COL32(50, 50, 50, 255);
const ImU32 ImGuiWindow::kColorButtonActive = IM_COL32(35, 35, 35, 255);
const ImU32 ImGuiWindow::kColorButtonHovered = IM_COL32(40, 40, 40, 255);
const ImU32 ImGuiWindow::kColorFrameBg = IM_COL32(50, 50, 50, 255);
const ImU32 ImGuiWindow::kColorFrameBgActive = IM_COL32(35, 35, 35, 255);
const ImU32 ImGuiWindow::kColorFrameBgHovered = IM_COL32(40, 40, 40, 255);
const ImU32 ImGuiWindow::kColorHeader = IM_COL32(50, 50, 50, 255);
const ImU32 ImGuiWindow::kColorHeaderActive = IM_COL32(35, 35, 35, 255);
const ImU32 ImGuiWindow::kColorHeaderHovered = IM_COL32(40, 40, 40, 255);
const ImU32 ImGuiWindow::kColorCheckMark = IM_COL32(150, 150, 150, 255);
const ImU32 ImGuiWindow::kColorSliderGrab = IM_COL32(100, 100, 100, 255);
const ImU32 ImGuiWindow::kColorSliderGrabActive = IM_COL32(150, 150, 150, 255);
const ImU32 ImGuiWindow::kColorScrollbarBg = IM_COL32(40, 40, 40, 255);
const ImU32 ImGuiWindow::kColorScrollbarGrab = IM_COL32(100, 100, 100, 255);
const ImU32 ImGuiWindow::kColorScrollbarGrabActive = IM_COL32(150, 150, 150, 255);
const ImU32 ImGuiWindow::kColorScrollbarGrabHovered = IM_COL32(120, 120, 120, 255);
const ImU32 ImGuiWindow::kColorRhombusSelect = IM_COL32(255, 127, 0, 255);
const ImU32 ImGuiWindow::kColorRhombusDefault = IM_COL32(175, 175, 175, 255);
const ImU32 ImGuiWindow::kColorRhombusMoving = IM_COL32(255, 127, 0, 127);
const ImU32 ImGuiWindow::kColorInterpolationCurveBezierLine = IM_COL32(255, 127, 0, 255);
const ImU32 ImGuiWindow::kColorInterpolationCurveControlLine = IM_COL32(0, 255, 0, 127);
const ImU32 ImGuiWindow::kColorInterpolationCurveControlPoint = IM_COL32(255, 0, 0, 255);
const ImU32 ImGuiWindow::kColorSelectedModelObject = IM_COL32(255, 127, 0, 255);
const nanoem_u8_t ImGuiWindow::kFAArrows[] = { 0xef, 0x81, 0x87, 0 };
const nanoem_u8_t ImGuiWindow::kFARefresh[] = { 0xef, 0x80, 0xa1, 0 };
const nanoem_u8_t ImGuiWindow::kFAZoom[] = { 0xef, 0x80, 0x82, 0 };
const nanoem_u8_t ImGuiWindow::kFAMagicWand[] = { 0xef, 0x83, 0x90, 0 };
const nanoem_u8_t ImGuiWindow::kFATHList[] = { 0xef, 0x80, 0x8b, 0 };
const nanoem_u8_t ImGuiWindow::kFAPencil[] = { 0xef, 0x81, 0x80, 0 };
const nanoem_u8_t ImGuiWindow::kFAForward[] = { 0xef, 0x81, 0x8e, 0 };
const nanoem_u8_t ImGuiWindow::kFABackward[] = { 0xef, 0x81, 0x8a, 0 };
const nanoem_u8_t ImGuiWindow::kFACogs[] = { 0xef, 0x82, 0x85, 0 };
const nanoem_u8_t ImGuiWindow::kFAPlus[] = { 0xef, 0x81, 0xa7, 0 };
const nanoem_u8_t ImGuiWindow::kFAMinus[] = { 0xef, 0x81, 0xa8, 0 };
const nanoem_u8_t ImGuiWindow::kFAArrowUp[] = { 0xef, 0x81, 0xa2, 0x0 };
const nanoem_u8_t ImGuiWindow::kFAArrowDown[] = { 0xef, 0x81, 0xa3, 0x0 };
const nanoem_u8_t ImGuiWindow::kFAFolderOpen[] = { 0xef, 0x84, 0xba, 0 };
const nanoem_u8_t ImGuiWindow::kFAFolderClose[] = { 0xef, 0x84, 0xb8, 0 };
const nanoem_u8_t ImGuiWindow::kFACircle[] = { 0xef, 0x84, 0x91, 0 };
const nanoem_u8_t ImGuiWindow::kFALink[] = { 0xef, 0x83, 0x81, 0 };
const nanoem_u8_t ImGuiWindow::kFAExpand[] = { 0xef, 0x81, 0xa5, 0 };

bool
ImGuiWindow::handleButton(const char *label)
{
    return handleButton(label, 0, true);
}

bool
ImGuiWindow::handleButton(const char *label, nanoem_f32_t width, bool enabled)
{
    const ImVec2 size(width, 0);
    bool pressed = false;
    if (enabled) {
        pressed = ImGui::Button(label, size);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, kColorTextDisabled);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, kColorFrameBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kColorFrameBg);
        ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
    }
    return pressed;
}

bool
ImGuiWindow::handleArrowButton(const char *label, ImGuiDir dir, bool enabled)
{
    bool pressed = false;
    if (enabled) {
        pressed = ImGui::ArrowButton(label, dir);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, kColorTextDisabled);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, kColorButton);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kColorButton);
        ImGui::ArrowButton(label, dir);
        ImGui::PopStyleColor(3);
    }
    return pressed;
}

bool
ImGuiWindow::handleRadioButton(const char *label, bool value, bool enabled)
{
    bool pressed = false;
    if (enabled) {
        pressed = ImGui::RadioButton(label, value);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, kColorTextDisabled);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kColorFrameBg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kColorFrameBg);
        ImGui::RadioButton(label, value);
        ImGui::PopStyleColor(3);
    }
    return pressed;
}

bool
ImGuiWindow::handleCheckBox(const char *label, bool *value, bool enabled)
{
    bool pressed = false;
    if (enabled) {
        pressed = ImGui::Checkbox(label, value);
    }
    else {
        bool disabled = value ? *value : false;
        ImGui::PushStyleColor(ImGuiCol_Text, kColorTextDisabled);
        ImGui::PushStyleColor(ImGuiCol_CheckMark, kColorTextDisabled);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kColorFrameBg);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kColorFrameBg);
        ImGui::Checkbox(label, &disabled);
        ImGui::PopStyleColor(4);
    }
    return pressed;
}

bool
ImGuiWindow::handleDragFloat3(const char *label, nanoem_f32_t *value, bool enabled, nanoem_f32_t factor,
    nanoem_f32_t min, nanoem_f32_t max, const char *format, ImGuiSliderFlags flags)
{
    return handleDragScalarN(label, ImGuiDataType_Float, value, 3, enabled, factor, &min, &max, format, flags);
}

bool
ImGuiWindow::handleDragFloat(const char *label, nanoem_f32_t *value, bool enabled, nanoem_f32_t factor,
    nanoem_f32_t min, nanoem_f32_t max, const char *format, ImGuiSliderFlags flags)
{
    return handleDragScalarN(label, ImGuiDataType_Float, value, 1, enabled, factor, &min, &max, format, flags);
}

bool
ImGuiWindow::handleDragScalarN(const char *label, ImGuiDataType dataType, void *value, int numComponents, bool enabled,
    nanoem_f32_t factor, const void *min, const void *max, const char *format, ImGuiSliderFlags flags)
{
    bool reacted = false;
    if (enabled) {
        reacted = ImGui::DragScalarN(label, dataType, value, numComponents, factor, min, max, format, flags);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, kColorTextDisabled);
        ImGui::InputScalarN(
            label, dataType, value, numComponents, nullptr, nullptr, format, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }
    return reacted;
}

bool
ImGuiWindow::handleSliderFloat3(const char *label, nanoem_f32_t *value, bool enabled, nanoem_f32_t min,
    nanoem_f32_t max, const char *format, ImGuiSliderFlags flags)
{
    return handleSliderScalarN(label, ImGuiDataType_Float, value, 3, enabled, &min, &max, format, flags);
}

bool
ImGuiWindow::handleSliderFloat(const char *label, nanoem_f32_t *value, bool enabled, nanoem_f32_t min, nanoem_f32_t max,
    const char *format, ImGuiSliderFlags flags)
{
    return handleSliderScalarN(label, ImGuiDataType_Float, value, 1, enabled, &min, &max, format, flags);
}

bool
ImGuiWindow::handleSliderInt(
    const char *label, int *value, bool enabled, int min, int max, const char *format, ImGuiSliderFlags flags)
{
    return handleSliderScalarN(label, ImGuiDataType_S32, value, 1, enabled, &min, &max, format, flags);
}

bool
ImGuiWindow::handleSliderScalarN(const char *label, ImGuiDataType dataType, void *value, int numComponents,
    bool enabled, const void *min, const void *max, const char *format, ImGuiSliderFlags flags)
{
    bool reacted = false;
    if (enabled) {
        reacted = ImGui::SliderScalarN(label, dataType, value, numComponents, min, max, format, flags);
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, kColorTextDisabled);
        ImGui::InputScalarN(
            label, dataType, value, numComponents, nullptr, nullptr, format, ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }
    return reacted;
}

void
ImGuiWindow::getImageCoordinate(ImVec2 &uv0, ImVec2 &uv1) NANOEM_DECL_NOEXCEPT
{
    if (sg::query_features().origin_top_left) {
        uv0 = ImVec2(0, 0);
        uv1 = ImVec2(1, 1);
    }
    else {
        uv0 = ImVec2(0, 1);
        uv1 = ImVec2(1, 0);
    }
}

void
ImGuiWindow::saveDefaultStyle(nanoem_f32_t deviceScaleRatio)
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, kColorWindowBg);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, kColorMenuBarBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBg, kColorTitleBg);
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, kColorTitleBgActive);
    ImGui::PushStyleColor(ImGuiCol_Tab, kColorTab);
    ImGui::PushStyleColor(ImGuiCol_TabActive, kColorTabActive);
    ImGui::PushStyleColor(ImGuiCol_TabHovered, kColorTabHovered);
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, kColorResizeGrip);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, kColorResizeGripHovered);
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, kColorResizeGripActive);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, kColorChildBg);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, kColorPopupBg);
    ImGui::PushStyleColor(ImGuiCol_Border, kColorBorder);
    ImGui::PushStyleColor(ImGuiCol_Text, kColorText);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, kColorTextDisabled);
    ImGui::PushStyleColor(ImGuiCol_Button, kColorButton);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, kColorButtonActive);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, kColorButtonHovered);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, kColorFrameBg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, kColorFrameBgActive);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, kColorFrameBgHovered);
    ImGui::PushStyleColor(ImGuiCol_Header, kColorHeader);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, kColorHeaderActive);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, kColorHeaderHovered);
    ImGui::PushStyleColor(ImGuiCol_CheckMark, kColorCheckMark);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, kColorSliderGrab);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, kColorSliderGrabActive);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, kColorScrollbarBg);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, kColorScrollbarGrab);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, kColorScrollbarGrabActive);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, kColorScrollbarGrabHovered);
    const nanoem_f32_t rounding = 3 * deviceScaleRatio;
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, deviceScaleRatio);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0);
}

void
ImGuiWindow::restoreDefaultStyle()
{
    ImGui::PopStyleVar(5);
    ImGui::PopStyleColor(31);
}

ImGuiWindow::ImGuiWindow(BaseApplicationService *application)
    : m_applicationPtr(application)
    , m_menu(nullptr)
    , m_gizmoController(nullptr)
    , m_cameraLookAtVectorValueState(nullptr)
    , m_cameraAngleVectorValueState(nullptr)
    , m_cameraDistanceVectorValueState(nullptr)
    , m_cameraFovVectorValueState(nullptr)
    , m_lightColorVectorValueState(nullptr)
    , m_lightDirectionVectorValueState(nullptr)
    , m_accessoryTranslationVectorValueState(nullptr)
    , m_accessoryOrientationVectorValueState(nullptr)
    , m_accessoryScaleFactorValueState(nullptr)
    , m_accessoryOpacityValueState(nullptr)
    , m_boneTranslationValueState(nullptr)
    , m_boneOrientationValueState(nullptr)
    , m_draggingMorphSliderState(nullptr)
    , m_requestedScrollHereTrack(nullptr)
    , m_context(nullptr)
    , m_debugger(nullptr)
    , m_draggingMarkerPanelRect(Constants::kZeroV4)
    , m_elapsedTime(0)
    , m_currentMemoryBytes(0)
    , m_maxMemoryBytes(0)
    , m_currentCPUPercentage(0)
    , m_scrollTimelineY(0)
    , m_timelineWidth(0)
    , m_timelineWidthRatio(kTimelineDefaultWidthRatio)
    , m_movingAllSelectedKeyframesIndexDelta(0)
    , m_modalDialogPressedButton(0)
    , m_projectKeyframeSelectorIndex(0)
    , m_modelKeyframeSelectorIndex(0)
    , m_requestClearAllModalDialogs(false)
    , m_draggingTimelineScrollBar(false)
    , m_lastKeyframeSelection(false)
    , m_lastKeyframeCopied(false)
    , m_editingAccessoryName(false)
    , m_editingModelName(false)
    , m_visible(true)
{
    IMGUI_CHECKVERSION();
    Inline::clearZeroMemory(m_bindings);
    Inline::clearZeroMemory(m_basePipelineDescription);
    m_atlasImage = { SG_INVALID_ID };
    m_transparentTileImage = { SG_INVALID_ID };
}

ImGuiWindow::~ImGuiWindow()
{
    if (m_debugger) {
        nanoem_delete(static_cast<sg_imgui_t *>(m_debugger));
        m_debugger = nullptr;
    }
    for (LazyExecutionCommandList::const_iterator it = m_lazyExecutionCommands.begin(),
                                                  end = m_lazyExecutionCommands.end();
         it != end; ++it) {
        nanoem_delete(*it);
    }
    m_lazyExecutionCommands.clear();
    for (ModalDialogList::const_iterator it = m_lazyModalDialogs.begin(), end = m_lazyModalDialogs.end(); it != end;
         ++it) {
        nanoem_delete(*it);
    }
    m_lazyModalDialogs.clear();
    for (ModalDialogList::const_iterator it = m_allModalDialogs.begin(), end = m_allModalDialogs.end(); it != end;
         ++it) {
        nanoem_delete(*it);
    }
    m_allModalDialogs.clear();
    nanoem_delete_safe(m_menu);
    nanoem_delete_safe(m_gizmoController);
}

bool
ImGuiWindow::handleTranslatedButton(const char *id)
{
    char buffer[Inline::kLongNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##%s", tr(id), id);
    return handleButton(buffer);
}

bool
ImGuiWindow::handleTranslatedButton(const char *id, nanoem_f32_t width, bool enabled)
{
    char buffer[Inline::kLongNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##%s", tr(id), id);
    return handleButton(buffer, width, enabled);
}

void
ImGuiWindow::resetSelectionIndex()
{
    m_projectKeyframeSelectorIndex = 0;
    m_modelKeyframeSelectorIndex = 0;
}

void
ImGuiWindow::cancelAllDraggingStates()
{
    nanoem_delete_safe(m_cameraLookAtVectorValueState);
    nanoem_delete_safe(m_cameraAngleVectorValueState);
    nanoem_delete_safe(m_cameraDistanceVectorValueState);
    nanoem_delete_safe(m_cameraFovVectorValueState);
    nanoem_delete_safe(m_lightColorVectorValueState);
    nanoem_delete_safe(m_lightDirectionVectorValueState);
    nanoem_delete_safe(m_accessoryTranslationVectorValueState);
    nanoem_delete_safe(m_accessoryOrientationVectorValueState);
    nanoem_delete_safe(m_accessoryScaleFactorValueState);
    nanoem_delete_safe(m_accessoryOpacityValueState);
    nanoem_delete_safe(m_boneTranslationValueState);
    nanoem_delete_safe(m_boneOrientationValueState);
    nanoem_delete_safe(m_draggingMorphSliderState);
}

void
ImGuiWindow::reloadActiveEffect(IDrawable *value)
{
    addLazyExecutionCommand(nanoem_new(LazyReloadEffectCommand(value)));
}

void
ImGuiWindow::addLazyExecutionCommand(ILazyExecutionCommand *command)
{
    m_lazyExecutionCommands.push_back(command);
}

void
ImGuiWindow::drawTextTooltip(const char *text)
{
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(text);
    ImGui::EndTooltip();
}

void
ImGuiWindow::openBoneKeyframeBezierInterpolationCurveGraphDialog(Project *project)
{
    if (m_dialogWindows.find(BoneKeyframeInterpolationCurveGraphDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog =
                nanoem_new(BoneKeyframeInterpolationCurveGraphDialog(activeModel, project, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(BoneKeyframeInterpolationCurveGraphDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openCameraKeyframeBezierInterpolationCurveGraphDialog(Project *project)
{
    if (m_dialogWindows.find(CameraKeyframeInterpolationCurveGraphDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog =
            nanoem_new(CameraKeyframeInterpolationCurveGraphDialog(project, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(CameraKeyframeInterpolationCurveGraphDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openModelOutsideParentDialog(Project *project)
{
    if (m_dialogWindows.find(ModelOutsideParentDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(ModelOutsideParentDialog(project, m_applicationPtr, this));
        m_dialogWindows.insert(tinystl::make_pair(ModelOutsideParentDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openAccessoryOutsideParentDialog(Project *project)
{
    if (m_dialogWindows.find(AccessoryOutsideParentDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Accessory *activeAccessory = project->activeAccessory()) {
            INoModalDialogWindow *dialog =
                nanoem_new(AccessoryOutsideParentDialog(activeAccessory, m_applicationPtr, this));
            m_dialogWindows.insert(tinystl::make_pair(AccessoryOutsideParentDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openSelfShadowConfigurationDialog()
{
    if (m_dialogWindows.find(SelfShadowConfigurationDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(SelfShadowConfigurationDialog(m_applicationPtr, this));
        m_dialogWindows.insert(tinystl::make_pair(SelfShadowConfigurationDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openUVEditDialog(const nanoem_model_material_t *materialPtr, Model *activeModel)
{
    if (m_dialogWindows.find(UVEditDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(UVEditDialog(materialPtr, activeModel, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(UVEditDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::initialize(nanoem_f32_t windowDevicePixelRatio, nanoem_f32_t viewportDevicePixelRatio)
{
    SG_PUSH_GROUP("ImGuiWindow::initialize()");
    ImGuiContext *context = m_context = ImGui::CreateContext(&m_atlas);
    ImGui::SetCurrentContext(context);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.FontGlobalScale = windowDevicePixelRatio;
    io.DisplayFramebufferScale = ImVec2(windowDevicePixelRatio, windowDevicePixelRatio);
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    ImGui::StyleColorsDark(&m_style);
    m_style.AntiAliasedFill = m_style.AntiAliasedLines = viewportDevicePixelRatio > 1.0f;
    ImGui::GetStyle() = m_style;
    ImGui::GetStyle().ScaleAllSizes(windowDevicePixelRatio);
    sg_shader_desc sd;
    Inline::clearZeroMemory(sd);
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        sd.vs.bytecode.ptr = g_nanoem_ui_vs_dxbc_data;
        sd.vs.bytecode.size = g_nanoem_ui_vs_dxbc_size;
        sd.fs.bytecode.ptr = g_nanoem_ui_ps_dxbc_data;
        sd.fs.bytecode.size = g_nanoem_ui_ps_dxbc_size;
    }
    else if (sg::is_backend_metal(backend)) {
        sd.vs.bytecode.ptr = g_nanoem_ui_vs_msl_macos_data;
        sd.vs.bytecode.size = g_nanoem_ui_vs_msl_macos_size;
        sd.fs.bytecode.ptr = g_nanoem_ui_fs_msl_macos_data;
        sd.fs.bytecode.size = g_nanoem_ui_fs_msl_macos_size;
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        sd.vs.source = reinterpret_cast<const char *>(g_nanoem_ui_vs_glsl_core33_data);
        sd.fs.source = reinterpret_cast<const char *>(g_nanoem_ui_fs_glsl_core33_data);
    }
    else if (backend == SG_BACKEND_GLES3) {
        sd.vs.source = reinterpret_cast<const char *>(g_nanoem_ui_vs_glsl_es3_data);
        sd.fs.source = reinterpret_cast<const char *>(g_nanoem_ui_fs_glsl_es3_data);
    }
    sd.vs.entry = "nanoemVSMain";
    sd.fs.uniform_blocks[0].size = sd.vs.uniform_blocks[0].size = sizeof(UniformBlock);
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
    sd.fs.images[0] = sg_shader_image_desc { "SPIRV_Cross_Combined", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    sd.fs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "_37", SG_UNIFORMTYPE_FLOAT4, 4 };
    sd.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "_19", SG_UNIFORMTYPE_FLOAT4, 4 };
#else
    sd.fs.images[0] =
        sg_shader_image_desc { "SPIRV_Cross_Combinedu_textureu_textureSampler", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    sd.fs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "ui_parameters_t", SG_UNIFORMTYPE_FLOAT4, 4 };
    sd.vs.uniform_blocks[0].uniforms[0] = sg_shader_uniform_desc { "ui_parameters_t", SG_UNIFORMTYPE_FLOAT4, 4 };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
    sd.fs.entry = "nanoemPSMain";
    sd.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
    sd.attrs[1] = sg_shader_attr_desc {
        "a_normal",
        "NORMAL",
        0,
    };
    sd.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
    sd.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
    sd.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
    sd.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
    sd.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
    sd.attrs[7] = sg_shader_attr_desc { "a_color0", "COLOR", 0 };
    m_basePipelineDescription.shader = sg::make_shader(&sd);
    nanoem_assert(sg::query_shader_state(m_basePipelineDescription.shader) == SG_RESOURCESTATE_VALID,
        "shader buffer must be valid");
#if defined(NANOEM_ENABLE_IMGUI_INDEX32)
    m_basePipelineDescription.index_type = SG_INDEXTYPE_UINT32;
#else
    m_basePipelineDescription.index_type = SG_INDEXTYPE_UINT16;
#endif
    sg_layout_desc &ld = m_basePipelineDescription.layout;
    ld.buffers[0].stride = sizeof(VertexUnit);
    ld.attrs[0] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[1] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[2] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_uv), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[3] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_uv), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[4] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_uv), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[5] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_uv), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[6] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_uv), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[7] = sg_vertex_attr_desc { 0, offsetof(VertexUnit, m_color), SG_VERTEXFORMAT_UBYTE4N };
    m_basePipelineDescription.depth.write_enabled = false;
    m_basePipelineDescription.sample_count = 1;
    sg_blend_state &bs = m_basePipelineDescription.colors[0].blend;
    bs.enabled = true;
    bs.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    bs.src_factor_alpha = SG_BLENDFACTOR_SRC_ALPHA;
    bs.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    bs.dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    nanoem_u32_t key = nanoem_u32_t(m_basePipelineDescription.sample_count);
    char label[Inline::kMarkerStringLength];
    if (Inline::isDebugLabelEnabled()) {
        StringUtils::format(label, sizeof(label), "@nanoem/ImGuiWindow/%d", Inline::saturateInt32(m_pipelines.size()));
        m_basePipelineDescription.label = label;
    }
    sg_pipeline pipeline = sg::make_pipeline(&m_basePipelineDescription);
    nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline buffer must be valid");
    SG_LABEL_PIPELINE(pipeline, label);
    m_pipelines.insert(tinystl::make_pair(key, pipeline));
    const nanoem_f32_t pointSize = kFontSize;
    setFontPointSize(pointSize);
    if (BaseApplicationClient *client = m_applicationPtr->menubarApplicationClient()) {
        IEventPublisher *eventPublisher = m_applicationPtr->eventPublisher();
        IFileManager *fileManager = m_applicationPtr->fileManager();
        const ITranslator *translator = m_applicationPtr->translator();
        const ApplicationPreference preference(m_applicationPtr);
        m_menu = nanoem_new(ImGuiApplicationMenuBuilder(
            client, eventPublisher, fileManager, translator, preference.isModelEditingEnabled()));
        m_menu->build();
    }
    nanoem_u32_t buffer[256];
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int index = y * 16 + x;
            nanoem_u8_t color = 0;
            if (y < 8) {
                color = x < 8 ? 0xff : 0xcf;
            }
            else {
                color = x < 8 ? 0xcf : 0xff;
            }
            buffer[index] = 0xff << 24 | color << 16 | color << 8 | color << 0;
        }
    }
    sg_image_desc id;
    Inline::clearZeroMemory(id);
    id.width = 16;
    id.height = 16;
    id.pixel_format = SG_PIXELFORMAT_RGBA8;
    sg_range &content = id.data.subimage[0][0];
    content.ptr = buffer;
    content.size = sizeof(buffer);
    m_transparentTileImage = sg::make_image(&id);
#if defined(NANOEM_ENABLE_DEBUG_LABEL)
#if !defined(__has_feature)
    setSGXDebbugerEnabled(true);
#elif !__has_feature(address_sanitizer)
    setSGXDebbugerEnabled(true);
#endif /* __has_feature */
#endif
    SG_POP_GROUP();
}

void
ImGuiWindow::reset(Project *project)
{
    if (!m_lazyExecutionCommands.empty()) {
        for (LazyExecutionCommandList::const_iterator it = m_lazyExecutionCommands.begin(),
                                                      end = m_lazyExecutionCommands.end();
             it != end; ++it) {
            ILazyExecutionCommand *command = *it;
            command->destroy(project);
            nanoem_delete(command);
        }
        m_lazyExecutionCommands.clear();
    }
    if (!m_dialogWindows.empty()) {
        for (NoModalDialogWindowList::const_iterator it = m_dialogWindows.begin(), end = m_dialogWindows.end();
             it != end; ++it) {
            INoModalDialogWindow *window = it->second;
            window->destroy(project);
            nanoem_delete(window);
        }
        m_dialogWindows.clear();
    }
    m_requestClearAllModalDialogs = false;
    cancelAllDraggingStates();
    resetSelectionIndex();
}

void
ImGuiWindow::destroy() NANOEM_DECL_NOEXCEPT
{
    if (m_debugger) {
        sg_imgui_discard(static_cast<sg_imgui_t *>(m_debugger));
    }
    for (PipelineMap::iterator it = m_pipelines.begin(), end = m_pipelines.end(); it != end; ++it) {
        sg::destroy_pipeline(it->second);
    }
    m_pipelines.clear();
    sg::destroy_image(m_atlasImage);
    m_atlasImage = { SG_INVALID_ID };
    sg::destroy_image(m_transparentTileImage);
    m_transparentTileImage = { SG_INVALID_ID };
    ImGui::DestroyContext(m_context);
    m_context = nullptr;
    m_atlas.Clear();
}

void
ImGuiWindow::setFontPointSize(nanoem_f32_t pointSize)
{
    m_atlas.Clear();
    nanoem_u8_t *pixels;
    int width, height;
    resources::initializeTextFont(&m_atlas, pointSize, &m_textFontRanges);
    resources::initializeIconFont(&m_atlas, pointSize);
    ApplicationPreference preference(m_applicationPtr);
    if (const char *fontPath = preference.extraFontPath()) {
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(m_atlas.GetGlyphRangesCyrillic());
        builder.AddRanges(m_atlas.GetGlyphRangesChineseSimplifiedCommon());
        builder.AddRanges(m_atlas.GetGlyphRangesJapanese());
        builder.AddRanges(m_atlas.GetGlyphRangesKorean());
        builder.AddRanges(m_atlas.GetGlyphRangesThai());
        builder.AddRanges(m_atlas.GetGlyphRangesVietnamese());
        ImVector<ImWchar> ranges;
        builder.BuildRanges(&ranges);
        ImFontConfig config;
        config.MergeMode = true;
        m_atlas.AddFontFromFileTTF(fontPath, pointSize, &config, ranges.Data);
        resources::initializeIconFont(&m_atlas, pointSize);
        m_atlas.GetTexDataAsRGBA32(&pixels, &width, &height);
        const sg_limits limits = sg::query_limits();
        if (width >= limits.max_image_size_2d || height >= limits.max_image_size_2d) {
            m_atlas.Clear();
            resources::initializeTextFont(&m_atlas, pointSize, &m_textFontRanges);
            resources::initializeIconFont(&m_atlas, pointSize);
        }
    }
    m_atlas.GetTexDataAsRGBA32(&pixels, &width, &height);
    sg_image_desc id;
    Inline::clearZeroMemory(id);
    id.width = width;
    id.height = height;
    id.pixel_format = SG_PIXELFORMAT_RGBA8;
    id.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
    id.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
    sg_range &content = id.data.subimage[0][0];
    content.ptr = pixels;
    content.size = nanoem_rsize_t(4) * width * height;
    sg::destroy_image(m_atlasImage);
    m_atlasImage = sg::make_image(&id);
    nanoem_assert(sg::query_image_state(m_atlasImage) == SG_RESOURCESTATE_VALID, "font atlas must be valid");
}

void
ImGuiWindow::setKeyPressed(BaseApplicationService::KeyType key)
{
    ImGuiIO &io = ImGui::GetIO();
    io.AddKeyEvent(mapKey(key), true);
}

void
ImGuiWindow::setKeyReleased(BaseApplicationService::KeyType key)
{
    ImGuiIO &io = ImGui::GetIO();
    io.AddKeyEvent(mapKey(key), false);
}

void
ImGuiWindow::setUnicodePressed(nanoem_u32_t key)
{
    ImGuiIO &io = ImGui::GetIO();
    io.AddInputCharacter(key);
}

void
ImGuiWindow::setCurrentCPUPercentage(nanoem_f32_t value)
{
    m_currentCPUPercentage = value;
}

void
ImGuiWindow::setCurrentMemoryBytes(nanoem_u64_t value)
{
    m_currentMemoryBytes = value;
}

void
ImGuiWindow::setMaxMemoryBytes(nanoem_u64_t value)
{
    m_maxMemoryBytes = value;
}

void
ImGuiWindow::setScreenCursorPress(int type, int x, int y, int modifiers)
{
    m_screenCursor.assign(x, y, modifiers, type, true);
}

void
ImGuiWindow::setScreenCursorMove(int x, int y, int modifiers)
{
    m_screenCursor.assign(x, y, modifiers);
}

void
ImGuiWindow::setScreenCursorRelease(int type, int x, int y, int modifiers)
{
    m_screenCursor.assign(x, y, modifiers, type, false);
}

void
ImGuiWindow::openBoneParametersDialog(Project *project)
{
    if (m_dialogWindows.find(BoneParameterDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog = nanoem_new(BoneParameterDialog(activeModel, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(BoneParameterDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openBoneCorrectionDialog(Project *project)
{
    if (m_dialogWindows.find(BoneCorrectionDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog = nanoem_new(BoneCorrectionDialog(activeModel, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(BoneCorrectionDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openBoneBiasDialog(Project *project)
{
    if (m_dialogWindows.find(BoneBiasDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog = nanoem_new(BoneBiasDialog(activeModel, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(BoneBiasDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openCameraParametersDialog(Project *project)
{
    if (m_dialogWindows.find(CameraParametersDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(CameraParametersDialog(project, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(CameraParametersDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openCameraCorrectionDialog(Project *project)
{
    BX_UNUSED_1(project);
    if (m_dialogWindows.find(CameraCorrectionDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(CameraCorrectionDialog(m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(CameraCorrectionDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openMorphCorrectionDialog(Project *project)
{
    if (m_dialogWindows.find(MorphCorrectionDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog = nanoem_new(MorphCorrectionDialog(activeModel, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(MorphCorrectionDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openViewportDialog(Project *project)
{
    if (m_dialogWindows.find(ViewportSettingDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(ViewportSettingDialog(project, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(ViewportSettingDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openModelDrawOrderDialog(Project *project)
{
    if (m_dialogWindows.find(ModelDrawOrderDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(ModelDrawOrderDialog(project, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(ModelDrawOrderDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openModelTransformOrderDialog(Project *project)
{
    if (m_dialogWindows.find(ModelTransformOrderDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(ModelTransformOrderDialog(project, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(ModelTransformOrderDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openPhysicsEngineDialog(Project *project)
{
    if (m_dialogWindows.find(PhysicsEngineDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(PhysicsEngineDialog(project, m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(PhysicsEngineDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openModelAllObjectsDialog()
{
}

void
ImGuiWindow::openModelEdgeDialog(Project *project)
{
    if (m_dialogWindows.find(ModelEdgeDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog = nanoem_new(ModelEdgeDialog(activeModel, project, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(ModelEdgeDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openScaleAllSelectedKeyframesDialog()
{
    if (m_dialogWindows.find(ScaleAllSelectedKeyframesDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(ScaleAllSelectedKeyframesDialog(m_applicationPtr));
        m_dialogWindows.insert(tinystl::make_pair(ScaleAllSelectedKeyframesDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openEffectParameterDialog(Project *project)
{
    BX_UNUSED_1(project);
    if (m_dialogWindows.find(EffectParameterDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(EffectParameterDialog(m_applicationPtr, this));
        m_dialogWindows.insert(tinystl::make_pair(EffectParameterDialog::kIdentifier, dialog));
    }
}

void
ImGuiWindow::openModelParameterDialog(Project *project, Error &error)
{
    if (project->isDirty()) {
        const ITranslator *translator = project->translator();
        error =
            Error(translator->translate("nanoem.error.project.open-model.reason"), "", Error::kDomainTypeApplication);
    }
    else if (m_dialogWindows.find(ModelParameterDialog::kIdentifier) == m_dialogWindows.end()) {
        if (Model *activeModel = project->activeModel()) {
            INoModalDialogWindow *dialog =
                nanoem_new(ModelParameterDialog(activeModel, project, m_applicationPtr, this));
            m_dialogWindows.insert(tinystl::make_pair(ModelParameterDialog::kIdentifier, dialog));
        }
    }
}

void
ImGuiWindow::openPreferenceDialog()
{
    if (m_dialogWindows.find(PreferenceDialog::kIdentifier) == m_dialogWindows.end()) {
        INoModalDialogWindow *dialog = nanoem_new(PreferenceDialog(m_applicationPtr, this));
        m_dialogWindows.insert(tinystl::make_pair(PreferenceDialog::kIdentifier, dialog));
    }
}

bool
ImGuiWindow::openModelIOPluginDialog(
    Project *project, plugin::ModelIOPlugin *plugin, const ByteArray &input, const ByteArray &layout, int functionIndex)
{
    bool result = false;
    if (Nanoem__Application__Plugin__UIWindow *window =
            nanoem__application__plugin__uiwindow__unpack(g_protobufc_allocator, layout.size(), layout.data())) {
        if (m_dialogWindows.find(window->id) == m_dialogWindows.end()) {
            ModelIOPluginDialog *dialog =
                nanoem_new(ModelIOPluginDialog(functionIndex, input, plugin, project, window, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(dialog->windowId(), static_cast<INoModalDialogWindow *>(dialog)));
        }
        else {
            nanoem__application__plugin__uiwindow__free_unpacked(window, g_protobufc_allocator);
        }
        result = true;
    }
    return result;
}

bool
ImGuiWindow::openMotionIOPluginDialog(Project *project, plugin::MotionIOPlugin *plugin, const ByteArray &input,
    const ByteArray &layout, int functionIndex)
{
    bool result = false;
    if (Nanoem__Application__Plugin__UIWindow *window =
            nanoem__application__plugin__uiwindow__unpack(g_protobufc_allocator, layout.size(), layout.data())) {
        if (m_dialogWindows.find(window->id) == m_dialogWindows.end()) {
            MotionIOPluginDialog *dialog =
                nanoem_new(MotionIOPluginDialog(functionIndex, input, plugin, project, window, m_applicationPtr));
            m_dialogWindows.insert(tinystl::make_pair(dialog->windowId(), static_cast<INoModalDialogWindow *>(dialog)));
        }
        else {
            nanoem__application__plugin__uiwindow__free_unpacked(window, g_protobufc_allocator);
        }
        result = true;
    }
    return result;
}

void
ImGuiWindow::openSaveProjectDialog(Project *project)
{
    if (m_menu) {
        m_menu->openSaveProjectDialog(project);
    }
}

void
ImGuiWindow::resizeDevicePixelWindowSize(const Vector2UI16 &value)
{
    ImGuiIO &io = ImGui::GetIO();
    const nanoem_u16_t minimumWindowWidth = kMinimumMainWindowSize.x * io.DisplayFramebufferScale.x,
                       minimumWindowHeight = kMinimumMainWindowSize.y * io.DisplayFramebufferScale.y;
    io.DisplaySize.x = glm::max(value.x, minimumWindowWidth);
    io.DisplaySize.y = glm::max(value.y, minimumWindowHeight);
    m_timelineWidth = m_lastTimelineWidth = m_defaultTimelineWidth = 0;
}

void
ImGuiWindow::setDevicePixelRatio(float value)
{
    float devicePixelRatio = glm::max(value, 1.0f);
    ImGuiIO &io = ImGui::GetIO();
    io.DisplayFramebufferScale = ImVec2(devicePixelRatio, devicePixelRatio);
    io.FontGlobalScale = 1.0f;
    ImGui::GetStyle() = m_style;
    ImGui::GetStyle().ScaleAllSizes(devicePixelRatio);
}

void
ImGuiWindow::setAntiAliasEnabled(bool value)
{
    ImGuiStyle &style = ImGui::GetStyle();
    style.AntiAliasedFill = style.AntiAliasedLines = value;
}

void
ImGuiWindow::drawAllWindows(Project *project, IState *state, nanoem_u32_t flags)
{
    BX_UNUSED_2(project, flags);
    sg_pass_action pa;
    sg::PassBlock pb;
    Inline::clearZeroMemory(pa);
    pa.colors[0].action = pa.depth.action = pa.stencil.action = SG_ACTION_CLEAR;
    pa.colors[0].value.r = 0.0f;
    pa.colors[0].value.g = 0.5f;
    pa.colors[0].value.b = 0.7f;
    pa.colors[0].value.a = 1.0f;
    if (isVisible()) {
        SG_PUSH_GROUP("ImGuiWindow::drawAllWindows");
        const ImGuiIO &io = ImGui::GetIO();
        const Vector2UI16 deviceScaleWindowSize(io.DisplaySize.x, io.DisplaySize.y);
        int sampleCount = 1;
        m_applicationPtr->beginDefaultPass(0, pa, deviceScaleWindowSize.x, deviceScaleWindowSize.y, sampleCount);
        setupDeviceInput(project);
        ImGui::NewFrame();
        if (m_gizmoController) {
            m_gizmoController->begin();
        }
        bool seekable = false;
        drawMainWindow(deviceScaleWindowSize, project, state, flags, seekable);
        drawAllNonModalWindows(project);
        handleModalDialogWindow(deviceScaleWindowSize, project);
#if !defined(NDEBUG)
        ImGui::ShowDemoWindow();
#endif
        if (sg_imgui_t *debugger = static_cast<sg_imgui_t *>(m_debugger)) {
            sg_imgui_draw(debugger);
        }
        ImGui::Render();
        Buffer *buffer = &m_buffers[0];
        renderDrawList(project, ImGui::GetDrawData(), sampleCount, buffer, m_bindings, pb);
        m_applicationPtr->endDefaultPass();
        batchLazySetAllObjects(project, seekable);
        SG_POP_GROUP();
#if defined(IMGUI_HAS_VIEWPORT)
        if (EnumUtils::isEnabledT<ImGuiConfigFlags>(ImGui::GetIO().ConfigFlags, ImGuiConfigFlags_ViewportsEnable)) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault(nullptr, this);
        }
#endif /* IMGUI_HAS_VIEWPORT */
    }
}

void
ImGuiWindow::drawWindow(Project *project, ImDrawData *drawData, bool load)
{
#if defined(IMGUI_HAS_VIEWPORT)
    SG_PUSH_GROUPF("ImGuiWindow::drawWindow(ID=%u, x=%.1f, y=%.1f, width=%.1f, height=%.1f)",
        drawData->OwnerViewport->ID, drawData->OwnerViewport->Pos.x, drawData->OwnerViewport->Pos.y,
        drawData->OwnerViewport->Size.x, drawData->OwnerViewport->Size.y);
    sg_pass_action pa;
    sg::PassBlock pb;
    Inline::clearZeroMemory(pa);
    pa.colors[0].action = pa.depth.action = pa.stencil.action = load ? SG_ACTION_LOAD : SG_ACTION_CLEAR;
    pa.colors[0].value.r = 0.0f;
    pa.colors[0].value.g = 0.5f;
    pa.colors[0].value.b = 0.7f;
    pa.colors[0].value.a = 1.0f;
    ImGuiID id = drawData->OwnerViewport->ID;
    int sampleCount = 1;
    m_applicationPtr->beginDefaultPass(id, pa, drawData->DisplaySize.x, drawData->DisplaySize.y, sampleCount);
    Buffer *buffer = &m_buffers[drawData->OwnerViewport->ID];
    renderDrawList(project, drawData, sampleCount, buffer, m_bindings, pb);
    m_applicationPtr->endDefaultPass();
    SG_POP_GROUP();
#else
    BX_UNUSED_3(project, drawData, load);
#endif /* IMGUI_HAS_VIEWPORT */
}

const IModalDialog *
ImGuiWindow::currentModalDialog() const NANOEM_DECL_NOEXCEPT
{
    return m_allModalDialogs.empty() ? nullptr : m_allModalDialogs.front();
}

IModalDialog *
ImGuiWindow::currentModalDialog() NANOEM_DECL_NOEXCEPT
{
    return m_allModalDialogs.empty() ? nullptr : m_allModalDialogs.front();
}

void
ImGuiWindow::addModalDialog(IModalDialog *value)
{
    if (value) {
        if (m_requestClearAllModalDialogs) {
            m_lazyModalDialogs.insert(m_lazyModalDialogs.begin(), value);
        }
        else {
            m_allModalDialogs.insert(m_allModalDialogs.begin(), value);
        }
        m_applicationPtr->eventPublisher()->publishAddModalDialogEvent();
    }
}

void
ImGuiWindow::clearAllModalDialogs()
{
    m_requestClearAllModalDialogs = true;
}

bool
ImGuiWindow::hasModalDialog() const NANOEM_DECL_NOEXCEPT
{
    return !m_allModalDialogs.empty();
}

void
ImGuiWindow::swapModalDialog(IModalDialog *value)
{
    if (value) {
        m_allModalDialogs.insert(m_allModalDialogs.begin(), value);
    }
    if (!m_allModalDialogs.empty()) {
        nanoem_delete(m_allModalDialogs.back());
        m_allModalDialogs.pop_back();
        if (m_allModalDialogs.empty()) {
            m_applicationPtr->eventPublisher()->publishClearModalDialogEvent();
        }
    }
}

bool
ImGuiWindow::isVisible() const NANOEM_DECL_NOEXCEPT
{
    return m_visible;
}

void
ImGuiWindow::setVisible(bool value)
{
    m_visible = value;
}

IPrimitive2D *
ImGuiWindow::primitiveContext() NANOEM_DECL_NOEXCEPT
{
    return &m_primitive2D;
}

bool
ImGuiWindow::isSGXDebuggerEnabled() const NANOEM_DECL_NOEXCEPT
{
    return m_debugger != nullptr;
}

void
ImGuiWindow::setSGXDebbugerEnabled(bool value)
{
    if (value && !m_debugger) {
        sg_imgui_t *debugger = nanoem_new(sg_imgui_t);
        sg_imgui_init(debugger);
        m_debugger = debugger;
    }
    else if (!value && m_debugger) {
        sg_imgui_t *debugger = static_cast<sg_imgui_t *>(m_debugger);
        m_debugger = nullptr;
        sg_imgui_discard(debugger);
        nanoem_delete(debugger);
    }
}

ImGuiKey
ImGuiWindow::mapKey(BaseApplicationService::KeyType key) NANOEM_DECL_NOEXCEPT
{
    switch (key) {
    case BaseApplicationService::kKeyType_SPACE:
        return ImGuiKey_Space;
    case BaseApplicationService::kKeyType_APOSTROPHE:
        return ImGuiKey_Apostrophe;
    case BaseApplicationService::kKeyType_COMMA:
        return ImGuiKey_Comma;
    case BaseApplicationService::kKeyType_MINUS:
        return ImGuiKey_Minus;
    case BaseApplicationService::kKeyType_PERIOD:
        return ImGuiKey_Period;
    case BaseApplicationService::kKeyType_SLASH:
        return ImGuiKey_Slash;
    case BaseApplicationService::kKeyType_0:
        return ImGuiKey_0;
    case BaseApplicationService::kKeyType_1:
        return ImGuiKey_1;
    case BaseApplicationService::kKeyType_2:
        return ImGuiKey_2;
    case BaseApplicationService::kKeyType_3:
        return ImGuiKey_3;
    case BaseApplicationService::kKeyType_4:
        return ImGuiKey_4;
    case BaseApplicationService::kKeyType_5:
        return ImGuiKey_5;
    case BaseApplicationService::kKeyType_6:
        return ImGuiKey_6;
    case BaseApplicationService::kKeyType_7:
        return ImGuiKey_7;
    case BaseApplicationService::kKeyType_8:
        return ImGuiKey_8;
    case BaseApplicationService::kKeyType_9:
        return ImGuiKey_9;
    case BaseApplicationService::kKeyType_SEMICOLON:
        return ImGuiKey_Semicolon;
    case BaseApplicationService::kKeyType_EQUAL:
        return ImGuiKey_Equal;
    case BaseApplicationService::kKeyType_A:
        return ImGuiKey_A;
    case BaseApplicationService::kKeyType_B:
        return ImGuiKey_B;
    case BaseApplicationService::kKeyType_C:
        return ImGuiKey_C;
    case BaseApplicationService::kKeyType_D:
        return ImGuiKey_D;
    case BaseApplicationService::kKeyType_E:
        return ImGuiKey_E;
    case BaseApplicationService::kKeyType_F:
        return ImGuiKey_F;
    case BaseApplicationService::kKeyType_G:
        return ImGuiKey_G;
    case BaseApplicationService::kKeyType_H:
        return ImGuiKey_H;
    case BaseApplicationService::kKeyType_I:
        return ImGuiKey_I;
    case BaseApplicationService::kKeyType_J:
        return ImGuiKey_J;
    case BaseApplicationService::kKeyType_K:
        return ImGuiKey_K;
    case BaseApplicationService::kKeyType_L:
        return ImGuiKey_L;
    case BaseApplicationService::kKeyType_M:
        return ImGuiKey_M;
    case BaseApplicationService::kKeyType_N:
        return ImGuiKey_N;
    case BaseApplicationService::kKeyType_O:
        return ImGuiKey_O;
    case BaseApplicationService::kKeyType_P:
        return ImGuiKey_P;
    case BaseApplicationService::kKeyType_Q:
        return ImGuiKey_Q;
    case BaseApplicationService::kKeyType_R:
        return ImGuiKey_R;
    case BaseApplicationService::kKeyType_S:
        return ImGuiKey_S;
    case BaseApplicationService::kKeyType_T:
        return ImGuiKey_T;
    case BaseApplicationService::kKeyType_U:
        return ImGuiKey_U;
    case BaseApplicationService::kKeyType_V:
        return ImGuiKey_V;
    case BaseApplicationService::kKeyType_W:
        return ImGuiKey_W;
    case BaseApplicationService::kKeyType_X:
        return ImGuiKey_X;
    case BaseApplicationService::kKeyType_Y:
        return ImGuiKey_Y;
    case BaseApplicationService::kKeyType_Z:
        return ImGuiKey_Z;
    case BaseApplicationService::kKeyType_LEFT_BRACKET:
        return ImGuiKey_LeftBracket;
    case BaseApplicationService::kKeyType_BACKSLASH:
        return ImGuiKey_Backslash;
    case BaseApplicationService::kKeyType_RIGHT_BRACKET:
        return ImGuiKey_RightBracket;
    case BaseApplicationService::kKeyType_GRAVE_ACCENT:
        return ImGuiKey_GraveAccent;
    case BaseApplicationService::kKeyType_ESCAPE:
        return ImGuiKey_Escape;
    case BaseApplicationService::kKeyType_ENTER:
        return ImGuiKey_Enter;
    case BaseApplicationService::kKeyType_TAB:
        return ImGuiKey_Tab;
    case BaseApplicationService::kKeyType_BACKSPACE:
        return ImGuiKey_Backspace;
    case BaseApplicationService::kKeyType_INSERT:
        return ImGuiKey_Insert;
    case BaseApplicationService::kKeyType_DELETE:
        return ImGuiKey_Delete;
    case BaseApplicationService::kKeyType_RIGHT:
        return ImGuiKey_RightArrow;
    case BaseApplicationService::kKeyType_LEFT:
        return ImGuiKey_LeftArrow;
    case BaseApplicationService::kKeyType_DOWN:
        return ImGuiKey_DownArrow;
    case BaseApplicationService::kKeyType_UP:
        return ImGuiKey_UpArrow;
    case BaseApplicationService::kKeyType_PAGE_UP:
        return ImGuiKey_PageUp;
    case BaseApplicationService::kKeyType_PAGE_DOWN:
        return ImGuiKey_PageDown;
    case BaseApplicationService::kKeyType_HOME:
        return ImGuiKey_Home;
    case BaseApplicationService::kKeyType_END:
        return ImGuiKey_End;
    case BaseApplicationService::kKeyType_CAPS_LOCK:
        return ImGuiKey_CapsLock;
    case BaseApplicationService::kKeyType_SCROLL_LOCK:
        return ImGuiKey_ScrollLock;
    case BaseApplicationService::kKeyType_NUM_LOCK:
        return ImGuiKey_NumLock;
    case BaseApplicationService::kKeyType_PRINT_SCREEN:
        return ImGuiKey_PrintScreen;
    case BaseApplicationService::kKeyType_PAUSE:
        return ImGuiKey_Pause;
    case BaseApplicationService::kKeyType_F1:
        return ImGuiKey_F1;
    case BaseApplicationService::kKeyType_F2:
        return ImGuiKey_F2;
    case BaseApplicationService::kKeyType_F3:
        return ImGuiKey_F3;
    case BaseApplicationService::kKeyType_F4:
        return ImGuiKey_F4;
    case BaseApplicationService::kKeyType_F5:
        return ImGuiKey_F5;
    case BaseApplicationService::kKeyType_F6:
        return ImGuiKey_F6;
    case BaseApplicationService::kKeyType_F7:
        return ImGuiKey_F7;
    case BaseApplicationService::kKeyType_F8:
        return ImGuiKey_F8;
    case BaseApplicationService::kKeyType_F9:
        return ImGuiKey_F9;
    case BaseApplicationService::kKeyType_F10:
        return ImGuiKey_F10;
    case BaseApplicationService::kKeyType_F11:
        return ImGuiKey_F11;
    case BaseApplicationService::kKeyType_F12:
        return ImGuiKey_F12;
    case BaseApplicationService::kKeyType_LEFT_SHIFT:
        return ImGuiKey_LeftShift;
    case BaseApplicationService::kKeyType_LEFT_CONTROL:
        return ImGuiKey_LeftCtrl;
    case BaseApplicationService::kKeyType_LEFT_ALT:
        return ImGuiKey_LeftAlt;
    case BaseApplicationService::kKeyType_LEFT_SUPER:
        return ImGuiKey_LeftSuper;
    case BaseApplicationService::kKeyType_RIGHT_SHIFT:
        return ImGuiKey_RightShift;
    case BaseApplicationService::kKeyType_RIGHT_CONTROL:
        return ImGuiKey_RightCtrl;
    case BaseApplicationService::kKeyType_RIGHT_ALT:
        return ImGuiKey_RightAlt;
    case BaseApplicationService::kKeyType_RIGHT_SUPER:
        return ImGuiKey_RightSuper;
    case BaseApplicationService::kKeyType_MENU:
        return ImGuiKey_Menu;
    default:
        return ImGuiKey_None;
    }
}

Vector4
ImGuiWindow::createViewportImageRect(const Project *project, const Vector4 &viewportLayoutRect) NANOEM_DECL_NOEXCEPT
{
    const Vector2 viewportImageSize(project->deviceScaleUniformedViewportImageSize());
    nanoem_f32_t x = viewportLayoutRect.x, y = viewportLayoutRect.y;
    x += (viewportLayoutRect.z - viewportImageSize.x) * 0.5f;
    y += (viewportLayoutRect.w - viewportImageSize.y) * 0.5f;
    return Vector4(x, y, viewportImageSize.x, viewportImageSize.y);
}

const char *
ImGuiWindow::selectBoneInterpolationType(
    const ITranslator *translator, nanoem_motion_bone_keyframe_interpolation_type_t type) NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_X:
        return translator->translate("nanoem.gui.panel.interpolation.bone.translation.x");
    case NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Y:
        return translator->translate("nanoem.gui.panel.interpolation.bone.translation.y");
    case NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_TRANSLATION_Z:
        return translator->translate("nanoem.gui.panel.interpolation.bone.translation.z");
    case NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_ORIENTATION:
        return translator->translate("nanoem.gui.panel.interpolation.bone.orientation");
    default:
        return "(Unknown)";
    }
}

const char *
ImGuiWindow::selectCameraInterpolationType(
    const ITranslator *translator, nanoem_motion_camera_keyframe_interpolation_type_t type) NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_X:
        return translator->translate("nanoem.gui.panel.interpolation.camera.translation.x");
    case NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Y:
        return translator->translate("nanoem.gui.panel.interpolation.camera.translation.y");
    case NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_LOOKAT_Z:
        return translator->translate("nanoem.gui.panel.interpolation.camera.translation.z");
    case NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE:
        return translator->translate("nanoem.gui.panel.interpolation.camera.orientation");
    case NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FOV:
        return translator->translate("nanoem.gui.panel.interpolation.camera.fov");
    case NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_DISTANCE:
        return translator->translate("nanoem.gui.panel.interpolation.camera.distance");
    default:
        return "(Unknown)";
    }
}

nanoem_rsize_t
ImGuiWindow::findTrack(const void *opaque, ITrack::Type targetTrackType, Project::TrackList &tracks)
{
    nanoem_rsize_t i = 0, offset = NANOEM_RSIZE_MAX;
    for (Project::TrackList::const_iterator it = tracks.begin(), end = tracks.end(); it != end; ++it, ++i) {
        const ITrack *track = *it;
        const ITrack::Type type = track->type();
        if (type == targetTrackType) {
            if (track->opaque() == opaque) {
                offset = i;
                break;
            }
        }
    }
    return offset;
}

bool
ImGuiWindow::isRhombusReactionSelectable(RhombusReactionType type) NANOEM_DECL_NOEXCEPT
{
    switch (type) {
    case kRhombusReactionClicked:
    case kRhombusReactionDragged:
        return true;
    default:
        return false;
    }
}

bool
ImGuiWindow::validateCameraState(
    BaseCameraVectorValueState *state, const ICamera *camera, const Project *project) NANOEM_DECL_NOEXCEPT
{
    return state->camera() == camera && state->project() == project;
}

bool
ImGuiWindow::handleVectorValueState(IVectorValueState *state)
{
    bool deletable = true;
    if (ImGui::IsItemDeactivatedAfterEdit() && state) {
        state->commit();
    }
    else if (!ImGui::IsItemDeactivated()) {
        deletable = false;
    }
    return deletable;
}

void
ImGuiWindow::appendDrawFlags(
    const Model *activeModel, const ModelParameterDialog *dialog, nanoem_u32_t &flags) NANOEM_DECL_NOEXCEPT
{
    static const nanoem_u32_t kActiveBoneFlags = IState::kDrawTypeActiveBone | IState::kDrawTypeActiveBoneConnection;
    if (!activeModel->isMorphWeightFocused()) {
        const Project::EditingMode editingMode = activeModel->project()->editingMode();
        switch (editingMode) {
        case Project::kEditingModeSelect: {
            if (dialog && dialog->isActiveBoneShown()) {
                flags |= IState::kDrawTypeActiveBoneConnection;
            }
            else {
                flags |= IState::kDrawTypeAllBoneConnections;
            }
            break;
        }
        case Project::kEditingModeMove: {
            flags |= kActiveBoneFlags | IState::kDrawTypeBoneMoveHandle;
            break;
        }
        case Project::kEditingModeRotate: {
            flags |= kActiveBoneFlags | IState::kDrawTypeBoneRotateHandle;
            break;
        }
        default:
            break;
        }
        if (activeModel->isShowAllVertexWeights()) {
            flags |= IState::kDrawTypeVertexWeightPainter;
        }
    }
}

ITrack *
ImGuiWindow::selectTrack(nanoem_rsize_t offset, ITrack::Type targetTrackType, Project::TrackList &tracks)
{
    const ImGuiIO &io = ImGui::GetIO();
    ITrack *track = nullptr;
    if (ImGui::GetKeyPressedAmount(ImGui::GetKeyIndex(ImGuiKey_UpArrow), io.KeyRepeatDelay, 0.02f) > 0 && offset > 0) {
        for (Project::TrackList::const_iterator it = tracks.begin() + offset - 1, end = tracks.begin(); it != end;
             --it) {
            ITrack *item = *it;
            const ITrack::Type type = item->type();
            if (type == targetTrackType) {
                track = item;
                break;
            }
        }
    }
    else if (ImGui::GetKeyPressedAmount(ImGui::GetKeyIndex(ImGuiKey_DownArrow), io.KeyRepeatDelay, 0.02f) > 0 &&
        offset < tracks.size() - 1) {
        for (Project::TrackList::const_iterator it = tracks.begin() + offset + 1, end = tracks.end(); it != end; ++it) {
            ITrack *item = *it;
            const ITrack::Type type = item->type();
            if (type == targetTrackType) {
                track = item;
                break;
            }
        }
    }
    return track;
}

const char *
ImGuiWindow::tr(const char *id) NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr->translator()->translate(id);
}

ImGuiWindow::RhombusReactionType
ImGuiWindow::handleRhombusButton(const char *buffer, nanoem_f32_t extent, bool enableDraggingRect) NANOEM_DECL_NOEXCEPT
{
    RhombusReactionType type = kRhombusReactionNone;
    if (ImGui::InvisibleButton(buffer, ImVec2(extent, extent))) {
        type = kRhombusReactionClicked;
    }
    else if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        type = kRhombusReactionMoving;
    }
    else if (enableDraggingRect && m_draggingMarkerPanelRect.x > 0 && m_draggingMarkerPanelRect.y > 0 &&
        m_draggingMarkerPanelRect.z > 0 && m_draggingMarkerPanelRect.w > 0) {
        const ImVec2 min(ImGui::GetItemRectMin()), max(ImGui::GetItemRectMax());
        if (max.x > m_draggingMarkerPanelRect.x && max.y > m_draggingMarkerPanelRect.y &&
            min.x < m_draggingMarkerPanelRect.z && min.y < m_draggingMarkerPanelRect.w) {
            type = kRhombusReactionDragged;
        }
    }
    return type;
}

nanoem_f32_t
ImGuiWindow::calculateTimelineWidth(const ImVec2 &size)
{
    nanoem_f32_t timelineWidth;
    if (m_defaultTimelineWidth != 0) {
        timelineWidth = m_timelineWidth;
    }
    else {
        m_defaultTimelineWidth = size.x * kTimelineDefaultWidthRatio;
        timelineWidth = m_timelineWidth = size.x * m_timelineWidthRatio;
    }
    return timelineWidth;
}

void
ImGuiWindow::searchNearestKeyframe(nanoem_frame_index_t frameIndex, const Project *project,
    const nanoem_motion_keyframe_object_t *&pk, const nanoem_motion_keyframe_object_t *&nk) NANOEM_DECL_NOEXCEPT
{
    bool found = false;
    if (const Model *model = project->activeModel()) {
        const nanoem_model_bone_t *bonePtr = model->activeBone();
        const Motion *motion = project->resolveMotion(model);
        if (bonePtr && motion) {
            nanoem_motion_bone_keyframe_t *previousKeyframe, *nextKeyframe;
            nanoemMotionSearchClosestBoneKeyframes(motion->data(),
                nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_JAPANESE), frameIndex, &previousKeyframe,
                &nextKeyframe);
            pk = nanoemMotionBoneKeyframeGetKeyframeObject(previousKeyframe);
            nk = nanoemMotionBoneKeyframeGetKeyframeObject(nextKeyframe);
            found = true;
        }
    }
    if (!found) {
        nanoem_motion_camera_keyframe_t *previousKeyframe, *nextKeyframe;
        nanoemMotionSearchClosestCameraKeyframes(
            project->cameraMotion()->data(), frameIndex, &previousKeyframe, &nextKeyframe);
        pk = nanoemMotionCameraKeyframeGetKeyframeObject(previousKeyframe);
        nk = nanoemMotionCameraKeyframeGetKeyframeObject(nextKeyframe);
    }
}

void
ImGuiWindow::resetBoxSelectionState(Project *project)
{
    if (Model *model = project->activeModel()) {
        model->selection()->setBoxSelectedBoneModeEnabled(false);
    }
}

void
ImGuiWindow::setCameraLookAt(const Vector3 &value, ICamera *camera, Project *project)
{
    if (m_cameraLookAtVectorValueState) {
        if (validateCameraState(m_cameraLookAtVectorValueState, camera, project)) {
            m_cameraLookAtVectorValueState->setValue(glm::value_ptr(value));
        }
        else {
            nanoem_delete_safe(m_cameraLookAtVectorValueState);
        }
    }
    else if (camera && project && !project->isPlaying()) {
        m_cameraLookAtVectorValueState = nanoem_new(CameraLookAtVectorValueState(project, camera));
    }
}

void
ImGuiWindow::setCameraAngle(const Vector3 &value, ICamera *camera, Project *project)
{
    if (m_cameraAngleVectorValueState) {
        if (validateCameraState(m_cameraAngleVectorValueState, camera, project)) {
            m_cameraAngleVectorValueState->setValue(glm::value_ptr(glm::radians(value)));
        }
        else {
            nanoem_delete_safe(m_cameraAngleVectorValueState);
        }
    }
    else if (camera && project && !project->isPlaying()) {
        m_cameraAngleVectorValueState = nanoem_new(CameraAngleVectorValueState(project, camera));
    }
}

void
ImGuiWindow::setCameraDistance(nanoem_f32_t value, ICamera *camera, Project *project)
{
    if (m_cameraDistanceVectorValueState) {
        if (validateCameraState(m_cameraDistanceVectorValueState, camera, project)) {
            m_cameraDistanceVectorValueState->setValue(&value);
        }
        else {
            nanoem_delete_safe(m_cameraDistanceVectorValueState);
        }
    }
    else if (camera && project && !project->isPlaying()) {
        m_cameraDistanceVectorValueState = nanoem_new(CameraDistanceVectorValueState(project, camera));
    }
}

void
ImGuiWindow::setCameraFov(nanoem_f32_t value, ICamera *camera, Project *project)
{
    if (m_cameraFovVectorValueState) {
        if (validateCameraState(m_cameraFovVectorValueState, camera, project)) {
            m_cameraFovVectorValueState->setValue(&value);
        }
        else {
            nanoem_delete_safe(m_cameraFovVectorValueState);
        }
    }
    else if (camera && project && !project->isPlaying()) {
        m_cameraFovVectorValueState = nanoem_new(CameraFovVectorValueState(project, camera));
    }
}

void
ImGuiWindow::setCameraPerspective(bool value, ICamera *camera, Project *project)
{
    BX_UNUSED_1(project);
    camera->setPerspective(value);
    camera->update();
    project->resetAllModelEdges();
}

void
ImGuiWindow::setLightColor(const Vector3 &value, ILight *light, Project *project)
{
    if (m_lightColorVectorValueState) {
        m_lightColorVectorValueState->setValue(glm::value_ptr(value));
    }
    else if (light && project && !project->isPlaying()) {
        m_lightColorVectorValueState = nanoem_new(LightColorVectorValueState(project, light));
    }
}

void
ImGuiWindow::setLightDirection(const Vector3 &value, ILight *light, Project *project)
{
    if (m_lightDirectionVectorValueState) {
        m_lightDirectionVectorValueState->setValue(glm::value_ptr(value));
    }
    else if (light && project && !project->isPlaying()) {
        m_lightDirectionVectorValueState = nanoem_new(LightDirectionVectorValueState(project, light));
    }
}

void
ImGuiWindow::setAccessoryTranslation(const Vector3 &value, Accessory *accessory)
{
    if (m_accessoryTranslationVectorValueState) {
        m_accessoryTranslationVectorValueState->setValue(glm::value_ptr(value));
    }
    else if (accessory && !accessory->project()->isPlaying()) {
        m_accessoryTranslationVectorValueState = nanoem_new(AccessoryTranslationVectorValueState(accessory));
    }
}

void
ImGuiWindow::setAccessoryOrientation(const Vector3 &value, Accessory *accessory)
{
    if (m_accessoryOrientationVectorValueState) {
        m_accessoryOrientationVectorValueState->setValue(glm::value_ptr(value));
    }
    else if (accessory && !accessory->project()->isPlaying()) {
        m_accessoryOrientationVectorValueState = nanoem_new(AccessoryOrientationVectorValueState(accessory));
    }
}

void
ImGuiWindow::setAccessoryScaleFactor(nanoem_f32_t value, Accessory *accessory)
{
    if (m_accessoryScaleFactorValueState) {
        m_accessoryScaleFactorValueState->setValue(&value);
    }
    else if (accessory && !accessory->project()->isPlaying()) {
        m_accessoryScaleFactorValueState = nanoem_new(AccessoryScaleFactorValueState(accessory));
    }
}

void
ImGuiWindow::setAccessoryOpacity(nanoem_f32_t value, Accessory *accessory)
{
    if (m_accessoryOpacityValueState) {
        m_accessoryOpacityValueState->setValue(&value);
    }
    else if (accessory && !accessory->project()->isPlaying()) {
        m_accessoryOpacityValueState = nanoem_new(AccessoryOpacityValueState(accessory));
    }
}

void
ImGuiWindow::setModelBoneTranslation(
    const Vector3 &value, const nanoem_model_bone_t *bonePtr, Model *model, Project *project)
{
    if (m_boneTranslationValueState) {
        m_boneTranslationValueState->setValue(glm::value_ptr(value));
    }
    else if (bonePtr && model && project && !project->isPlaying()) {
        m_boneTranslationValueState = nanoem_new(BoneTranslationValueState(bonePtr, model, project));
    }
}

void
ImGuiWindow::setModelBoneOrientation(
    const Vector3 &value, const nanoem_model_bone_t *bonePtr, Model *model, Project *project)
{
    if (m_boneOrientationValueState) {
        m_boneOrientationValueState->setValue(glm::value_ptr(Quaternion(value)));
    }
    else if (bonePtr && model && project && !project->isPlaying()) {
        m_boneOrientationValueState = nanoem_new(BoneOrientationValueState(bonePtr, model, project));
    }
}

void
ImGuiWindow::setModelMorphWeight(
    nanoem_f32_t value, const nanoem_model_morph_t *morphPtr, Model *model, Project *project)
{
    if (m_draggingMorphSliderState) {
        m_draggingMorphSliderState->deform(value);
    }
    else if (morphPtr && model && project && !project->isPlaying()) {
        const model::Morph *morph = model::Morph::cast(morphPtr);
        const nanoem_f32_t initial = morph->weight();
        m_draggingMorphSliderState = nanoem_new(DraggingMorphSliderState(morphPtr, model, project, initial));
    }
}

void
ImGuiWindow::clearAllKeyframeSelection(Project *project)
{
    if (Motion *activeMotion = project->resolveMotion(project->activeModel())) {
        activeMotion->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
    }
    else {
        project->cameraMotion()->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        project->lightMotion()->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        project->selfShadowMotion()->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        const Project::AccessoryList *accessories = project->allAccessories();
        for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end;
             ++it) {
            Motion *motion = project->resolveMotion(*it);
            motion->selection()->clearAllKeyframes(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL);
        }
    }
}

void
ImGuiWindow::copyAllKeyframeSelection(const IMotionKeyframeSelection *selection, Project *project)
{
    if (Motion *activeMotion = project->resolveMotion(project->activeModel())) {
        IMotionKeyframeSelection *dest = activeMotion->selection();
        Motion::BoneKeyframeList boneKeyframes;
        selection->getAll(boneKeyframes, nullptr);
        for (Motion::BoneKeyframeList::const_iterator it = boneKeyframes.begin(), end = boneKeyframes.end(); it != end;
             ++it) {
            dest->add(*it);
        }
        Motion::ModelKeyframeList modelKeyframes;
        selection->getAll(modelKeyframes, nullptr);
        for (Motion::ModelKeyframeList::const_iterator it = modelKeyframes.begin(), end = modelKeyframes.end();
             it != end; ++it) {
            dest->add(*it);
        }
        Motion::MorphKeyframeList morphKeyframes;
        selection->getAll(morphKeyframes, nullptr);
        for (Motion::MorphKeyframeList::const_iterator it = morphKeyframes.begin(), end = morphKeyframes.end();
             it != end; ++it) {
            dest->add(*it);
        }
    }
    else {
        Motion::CameraKeyframeList cameraKeyframes;
        selection->getAll(cameraKeyframes, nullptr);
        IMotionKeyframeSelection *destCamera = project->cameraMotion()->selection();
        for (Motion::CameraKeyframeList::const_iterator it = cameraKeyframes.begin(), end = cameraKeyframes.end();
             it != end; ++it) {
            destCamera->add(*it);
        }
        Motion::LightKeyframeList lightKeyframes;
        selection->getAll(lightKeyframes, nullptr);
        IMotionKeyframeSelection *destLight = project->lightMotion()->selection();
        for (Motion::LightKeyframeList::const_iterator it = lightKeyframes.begin(), end = lightKeyframes.end();
             it != end; ++it) {
            destLight->add(*it);
        }
        Motion::SelfShadowKeyframeList selfShadowKeyframes;
        selection->getAll(selfShadowKeyframes, nullptr);
        IMotionKeyframeSelection *destSelfShadow = project->selfShadowMotion()->selection();
        for (Motion::SelfShadowKeyframeList::const_iterator it = selfShadowKeyframes.begin(),
                                                            end = selfShadowKeyframes.end();
             it != end; ++it) {
            destSelfShadow->add(*it);
        }
        const Project::AccessoryList *accessories = project->allAccessories();
        for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end;
             ++it) {
            Motion *motion = project->resolveMotion(*it);
            Motion::AccessoryKeyframeList accessoryKeyframes;
            selection->getAll(accessoryKeyframes, nullptr);
            IMotionKeyframeSelection *destAccessory = motion->selection();
            for (Motion::AccessoryKeyframeList::const_iterator it = accessoryKeyframes.begin(),
                                                               end = accessoryKeyframes.end();
                 it != end; ++it) {
                destAccessory->add(*it);
            }
        }
    }
}

void
ImGuiWindow::handleDraggingMorphSliderState()
{
    if (ImGui::IsItemDeactivatedAfterEdit() && m_draggingMorphSliderState) {
        m_draggingMorphSliderState->commit();
        nanoem_delete_safe(m_draggingMorphSliderState);
    }
    else if (ImGui::IsItemDeactivated()) {
        nanoem_delete_safe(m_draggingMorphSliderState);
    }
}

void
ImGuiWindow::setEditingMode(Project *project, Project::EditingMode mode)
{
    project->setEditingMode(mode);
    resetBoxSelectionState(project);
}

void
ImGuiWindow::toggleEditingMode(Project *project, Project::EditingMode mode)
{
    project->setEditingMode(project->editingMode() == mode ? Project::kEditingModeNone : mode);
    resetBoxSelectionState(project);
}

void
ImGuiWindow::drawMainWindow(
    const Vector2 &deviceScaleWindowSize, Project *project, IState *state, nanoem_u32_t flags, bool &seekable)
{
    const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
    saveDefaultStyle(deviceScaleRatio);
#if defined(IMGUI_HAS_VIEWPORT)
    if (EnumUtils::isEnabledT<ImGuiConfigFlags>(ImGui::GetIO().ConfigFlags, ImGuiConfigFlags_ViewportsEnable)) {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::SetNextWindowPos(viewport->Pos);
    }
    else {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
    }
#else
    ImGui::SetNextWindowPos(ImVec2(0, 0));
#endif /* IMGUI_HAS_VIEWPORT */
    ImGui::SetNextWindowSize(ImVec2(deviceScaleWindowSize.x, deviceScaleWindowSize.y));
    ImGui::SetNextWindowSizeConstraints(
        ImVec2(deviceScaleWindowSize.x, deviceScaleWindowSize.y), ImVec2(FLT_MAX, FLT_MAX));
    nanoem_u32_t windowFlags = kMainWindowFlags;
    if (m_menu) {
        windowFlags |= ImGuiWindowFlags_MenuBar;
    }
    ImGui::Begin("main", nullptr, windowFlags);
    seekable = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
    if (m_menu) {
        m_menu->draw(m_debugger);
    }
    if (!project->isModelEditingEnabled()) {
        const ImGuiStyle &style = ImGui::GetStyle();
        const ImVec2 size(ImGui::GetContentRegionAvail());
        const nanoem_f32_t panelHeight =
            (ImGui::GetFrameHeightWithSpacing() * 8 + style.ItemSpacing.y * 6 + style.WindowPadding.y * 2) *
            (1.0f / deviceScaleRatio);
        bool viewportWindowDetached = project->isViewportWindowDetached();
        if (viewportWindowDetached) {
            const ImVec2 minimumViewportSize(
                kMinimumViewportWindowSize.x * deviceScaleRatio, kMinimumViewportWindowSize.y * deviceScaleRatio);
            const nanoem_f32_t timelineWidth = size.x, viewportHeight = size.y - (panelHeight * deviceScaleRatio);
            m_defaultTimelineWidth = size.x;
            drawTimeline(timelineWidth, viewportHeight, project);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, kWindowRounding * deviceScaleRatio);
            ImGui::SetNextWindowSizeConstraints(minimumViewportSize, ImVec2(FLT_MAX, FLT_MAX));
            if (ImGui::Begin(tr("nanoem.gui.viewport.title"), &viewportWindowDetached)) {
                drawViewport(project, state, flags);
            }
            ImGui::End();
            ImGui::PopStyleVar();
            if (!viewportWindowDetached) {
                project->setViewportWindowDetached(viewportWindowDetached);
                m_defaultTimelineWidth = 0;
            }
        }
        else {
            const nanoem_f32_t timelineWidth = calculateTimelineWidth(size),
                               viewportHeight = size.y - (panelHeight * deviceScaleRatio);
            drawTimeline(timelineWidth, viewportHeight, project);
            ImGui::SameLine();
            const ImVec2 viewportFrom(ImGui::GetCursorScreenPos());
            ImGui::BeginChild("viewport", ImVec2(ImGui::GetContentRegionAvail().x, viewportHeight), false);
            drawViewport(project, state, flags);
            ImGui::EndChild();
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
                handleVerticalSplitter(viewportFrom, size, viewportHeight, deviceScaleRatio);
            }
        }
        Model *activeModel = project->activeModel();
        ImGui::BeginChild("panel", ImGui::GetContentRegionAvail(), true);
        const ImVec2 &innerSize = ImGui::GetContentRegionAvail();
        float width = 144.0f * deviceScaleRatio, // (innerSize.x - (style.ItemSpacing.x + style.FramePadding.x +
                                                 // style.WindowPadding.x) * 2) / 7,
            height = innerSize.y;
        const ImVec2 panelSize(width, height);
        activeModel ? drawBoneInterpolationPanel(panelSize, activeModel, project)
                    : drawCameraInterpolationPanel(panelSize, project);
        ImGui::SameLine();
        drawModelPanel(panelSize, project);
        ImGui::SameLine();
        activeModel ? drawBonePanel(panelSize, activeModel, project) : drawCameraPanel(panelSize, project);
        ImGui::SameLine();
        activeModel ? drawMorphPanel(panelSize, activeModel, project) : drawLightPanel(panelSize, project);
        ImGui::SameLine();
        if (!activeModel) {
            drawAccessoryPanel(panelSize, project);
        }
        ImGui::SameLine();
        drawViewPanel(panelSize, project);
        ImGui::SameLine();
        drawPlayPanel(panelSize, project);
        ImGui::EndChild();
    }
    else {
        nanoem_f32_t height = ImGui::GetContentRegionAvail().y;
        drawModelEditPanel(project, height);
        ImGui::SameLine();
        ImGui::BeginChild("viewport", ImGui::GetContentRegionAvail(), false, ImGuiWindowFlags_MenuBar);
        NoModalDialogWindowList::const_iterator it = m_dialogWindows.find(ModelParameterDialog::kIdentifier);
        if (it != m_dialogWindows.end()) {
            ModelParameterDialog *dialog = static_cast<ModelParameterDialog *>(it->second);
            dialog->drawMenuBar(project);
        }
        drawViewport(project, state, flags);
        ImGui::EndChild();
    }
    ImGui::End();
    restoreDefaultStyle();
}

void
ImGuiWindow::drawTimeline(nanoem_f32_t timelineWidth, nanoem_f32_t viewportHeight, Project *project)
{
    ImGui::BeginChild("timeline", ImVec2(timelineWidth, viewportHeight), true);
    const nanoem_f32_t devicePixelRatio = project->windowDevicePixelRatio();
    nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
    bool frameIndexChanged = false, forward = false, backward = false;
    {
        const nanoem_f32_t seekerPanelWidth = kTimelineSeekerPanelWidth * devicePixelRatio;
        nanoem_f32_t padding = 0;
        if (timelineWidth > seekerPanelWidth) {
            padding += (timelineWidth - seekerPanelWidth - 10) * 0.5f;
        }
        drawSeekerPanel(project, padding, frameIndex, frameIndexChanged, forward, backward);
    }
    {
        const nanoem_f32_t undoPanelWidth = kTimelineUndoPanelWidth * devicePixelRatio;
        nanoem_f32_t padding = 0;
        if (timelineWidth > undoPanelWidth) {
            padding += (timelineWidth - undoPanelWidth - 10) * 0.5f;
        }
        drawUndoPanel(project, padding);
    }
    {
        const nanoem_f32_t tracksWidth =
            glm::min(m_defaultTimelineWidth * 0.3f, kTimelineTrackMaxWidth * devicePixelRatio);
        nanoem_u32_t numVisibleMarkers;
        drawWavePanel(tracksWidth, project, numVisibleMarkers);
        const nanoem_f32_t tracksHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() * 4;
        Project::TrackList tracks;
        drawAllTracksPanel(ImVec2(tracksWidth, tracksHeight), tracks, project);
        handleTrackSelection(tracks, project);
        ImGui::SameLine();
        drawAllMarkersPanel(ImVec2(ImGui::GetContentRegionAvail().x, tracksHeight), tracks, numVisibleMarkers, project);
    }
    {
        const nanoem_f32_t keyframeActionPanelWidth = kTimelineKeyframeActionPanelWidth * devicePixelRatio;
        nanoem_f32_t padding = 0;
        if (timelineWidth > keyframeActionPanelWidth) {
            padding = (timelineWidth - keyframeActionPanelWidth - 10) * 0.5f;
            ImGui::Dummy(ImVec2(padding, 0));
            ImGui::SameLine();
        }
        drawKeyframeActionPanel(project, padding);
        drawKeyframeSelectionPanel(project, padding);
    }
    ImGui::EndChild();
    nanoem_frame_index_t newFrameIndex;
    if (frameIndexChanged) {
        addLazyExecutionCommand(nanoem_new(LazySetFrameIndexCommand(frameIndex)));
    }
    else if (forward && Motion::addFrameIndexDelta(1, frameIndex, newFrameIndex)) {
        addLazyExecutionCommand(nanoem_new(LazySetFrameIndexCommand(newFrameIndex)));
    }
    else if (backward && Motion::subtractFrameIndexDelta(1, frameIndex, newFrameIndex)) {
        addLazyExecutionCommand(nanoem_new(LazySetFrameIndexCommand(newFrameIndex)));
    }
}

void
ImGuiWindow::drawSeekerPanel(Project *project, nanoem_f32_t padding, nanoem_frame_index_t &frameIndex,
    bool &frameIndexChanged, bool &forward, bool &backward)
{
    ImGui::Dummy(ImVec2(padding, 0));
    ImGui::SameLine();
    ImGui::PushButtonRepeat(true);
    const nanoem_motion_keyframe_object_t *pk, *nk;
    bool buttonEnabled = !project->isPlaying();
    searchNearestKeyframe(frameIndex, project, pk, nk);
    if (handleButton(reinterpret_cast<const char *>(kFABackward), 0, buttonEnabled && pk != nullptr)) {
        frameIndex = nanoemMotionKeyframeObjectGetFrameIndex(pk);
        frameIndexChanged = true;
    }
    ImGui::SameLine();
    backward = handleArrowButton("<", ImGuiDir_Left, buttonEnabled && frameIndex > 0);
    ImGui::SameLine();
    ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - padding) * 0.65f);
    if (ImGui::InputScalar("##timeline.frame-index", kFrameIndexDataType, &frameIndex, nullptr, nullptr, nullptr,
            ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
        frameIndexChanged = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    forward = handleArrowButton(">", ImGuiDir_Right, buttonEnabled && frameIndex < Motion::kMaxFrameIndex);
    ImGui::SameLine();
    const nanoem_frame_index_t nextKeyframeIndex = nanoemMotionKeyframeObjectGetFrameIndex(nk);
    if (handleButton(reinterpret_cast<const char *>(kFAForward), 0, buttonEnabled && nextKeyframeIndex > frameIndex)) {
        frameIndex = nextKeyframeIndex;
        frameIndexChanged = true;
    }
    ImGui::PopButtonRepeat();
    ImGui::Dummy(ImVec2(padding, 0));
}

void
ImGuiWindow::drawUndoPanel(Project *project, nanoem_f32_t padding)
{
    ImGui::Dummy(ImVec2(padding, 0));
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.undo", (ImGui::GetContentRegionAvail().x - padding) / 2.0f, project->canUndo())) {
        project->handleUndoAction();
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.redo", ImGui::GetContentRegionAvail().x - padding, project->canRedo())) {
        project->handleRedoAction();
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(padding, 0));
}

void
ImGuiWindow::drawWavePanel(nanoem_f32_t tracksWidth, Project *project, nanoem_u32_t &numVisibleMarkers)
{
    nanoem_f32_t histogramHeight = ImGui::GetFrameHeightWithSpacing() * 1.5f;
    nanoem_f32_t numVisibleMarkersWidth =
        (ImGui::GetContentRegionAvail().x - tracksWidth - ImGui::GetStyle().ScrollbarSize) /
        (ImGui::GetTextLineHeight());
    numVisibleMarkers = nanoem_u32_t(numVisibleMarkersWidth) - 1;
    WaveFormPanelDrawer drawer(project, numVisibleMarkers);
    ImGui::PlotHistogram("##timeline.wave", WaveFormPanelDrawer::callback, &drawer,
        Inline::saturateInt32(drawer.m_length), 0, nullptr, -1, 1,
        ImVec2(ImGui::GetContentRegionAvail().x, histogramHeight));
}

void
ImGuiWindow::drawAllTracksPanel(const ImVec2 &panelSize, Project::TrackList &tracks, Project *project)
{
    ImGui::BeginChild("tracks", panelSize, true, ImGuiWindowFlags_NoScrollbar);
    if (ImGui::IsWindowHovered()) {
        m_scrollTimelineY = ImGui::GetScrollY();
    }
    else {
        ImGui::SetScrollY(m_scrollTimelineY);
    }
    const Project::TrackList *allTracks = project->allTracks();
    for (Project::TrackList::const_iterator it = allTracks->begin(), end = allTracks->end(); it != end; ++it) {
        ITrack *track = *it;
        tracks.push_back(track);
        if (track->isExpanded()) {
            Project::TrackList children(track->children());
            for (Project::TrackList::const_iterator it2 = children.begin(), end2 = children.end(); it2 != end2; ++it2) {
                ITrack *track2 = *it2;
                tracks.push_back(track2);
            }
        }
    }
    ImGuiListClipper clipper;
    TrackSet selectedTracks;
    const nanoem_f32_t trackWidth = ImGui::GetContentRegionAvail().x;
    Model *activeModel = project->activeModel();
    clipper.Begin(Inline::saturateInt32(tracks.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            ITrack *track = tracks[i];
            drawTrack(track, i, activeModel, selectedTracks);
            const char *name = track->nameConstString();
            const nanoem_f32_t textWidth = ImGui::CalcTextSize(name).x;
            if (ImGui::IsItemHovered() && textWidth > trackWidth) {
                drawTextTooltip(name);
            }
            if (m_requestedScrollHereTrack == track) {
                ImGui::SetScrollHereY();
                m_requestedScrollHereTrack = nullptr;
            }
        }
    }
    if (!selectedTracks.empty()) {
        for (Project::TrackList::const_iterator it = allTracks->begin(), end = allTracks->end(); it != end; ++it) {
            ITrack *track = *it;
            if (selectedTracks.find(track) == selectedTracks.end()) {
                track->setSelected(false);
            }
            if (track->isExpandable()) {
                Project::TrackList children(track->children());
                for (Project::TrackList::const_iterator it2 = children.begin(), end2 = children.end(); it2 != end2;
                     ++it2) {
                    ITrack *childTrack = *it2;
                    if (selectedTracks.find(childTrack) == selectedTracks.end()) {
                        childTrack->setSelected(false);
                    }
                }
            }
        }
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawTrack(ITrack *track, int i, Model *activeModel, TrackSet &selectedTracks)
{
    const char *name = track->nameConstString();
    const bool expandable = track->isExpandable();
    char labelID[64];
    bool selected = false, expanded = false;
    if (expandable) {
        const char *icon = reinterpret_cast<const char *>(track->isExpanded() ? kFAFolderOpen : kFAFolderClose);
        const nanoem_f32_t iconWidth = ImGui::CalcTextSize(icon).x;
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        StringUtils::format(labelID, sizeof(labelID), "%s/%d", icon, i);
        expanded = ImGui::InvisibleButton(labelID, ImVec2(iconWidth, ImGui::GetTextLineHeight()));
        ImGui::SameLine();
        StringUtils::format(labelID, sizeof(labelID), "%s/%d", name, i);
        const ImVec2 size(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());
        selected = ImGui::InvisibleButton(labelID, size);
        ImGui::SetCursorScreenPos(pos);
        ImGui::TextUnformatted(icon);
        ImGui::SameLine();
    }
    else if (track->isFixed()) {
        const ImVec2 pos = ImGui::GetCursorScreenPos(),
                     size(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());
        StringUtils::format(labelID, sizeof(labelID), "%s/%d", name, i);
        selected = ImGui::InvisibleButton(labelID, size);
        ImGui::SetCursorScreenPos(pos);
        ImGui::TextUnformatted(reinterpret_cast<const char *>(kFACircle));
        ImGui::SameLine();
    }
    else {
        const ImVec2 pos = ImGui::GetCursorScreenPos(),
                     size(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());
        StringUtils::format(labelID, sizeof(labelID), "%s/%d", name, i);
        selected = ImGui::InvisibleButton(labelID, size);
        ImGui::SetCursorScreenPos(pos);
    }
    ImGui::TextColored(track->isSelected() ? ImColor(0xff, 0x7f, 0) : ImColor(0xff, 0xff, 0xff), "%s", name);
    if (expanded && expandable) {
        track->setExpanded(track->isExpanded() ? false : true);
    }
    else if (selected) {
        const ITrack::Type type = track->type();
        if (expandable && activeModel) {
            Project::TrackList children(track->children());
            IModelObjectSelection *selection = activeModel->selection();
            if (!ImGui::GetIO().KeyCtrl) {
                selection->removeAllBones();
            }
            for (Project::TrackList::const_iterator it = children.begin(), end = children.end(); it != end; ++it) {
                ITrack *child = *it;
                const ITrack *childConst = child;
                if (childConst->type() == ITrack::kTypeModelBone) {
                    const nanoem_model_bone_t *bonePtr = static_cast<const nanoem_model_bone_t *>(childConst->opaque());
                    selection->addBone(bonePtr);
                }
                child->setSelected(true);
                selectedTracks.insert(child);
            }
            Project *project = activeModel->project();
            const Project::EditingMode editingMode = project->editingMode();
            if ((editingMode == Project::kEditingModeMove && !selection->areAllBonesMovable()) ||
                (editingMode == Project::kEditingModeRotate && !selection->areAllBonesRotateable())) {
                project->setEditingMode(Project::kEditingModeNone);
            }
        }
        else if (type == ITrack::kTypeAccessory) {
            Accessory *accessory = static_cast<Accessory *>(track->opaque());
            addLazyExecutionCommand(nanoem_new(LazySetActiveAccessoryCommand(accessory, this)));
        }
        else if (type == ITrack::kTypeModelLabel && activeModel) {
            const ITrack *trackConst = track;
            const nanoem_model_label_t *labelPtr = static_cast<const nanoem_model_label_t *>(trackConst->opaque());
            nanoem_rsize_t numLabels;
            nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numLabels);
            if (items && numLabels > 0) {
                const nanoem_model_label_item_t *item = items[0];
                IModelObjectSelection *selection = activeModel->selection();
                if (const nanoem_model_bone_t *bonePtr = nanoemModelLabelItemGetBoneObject(item)) {
                    if (!ImGui::GetIO().KeyCtrl) {
                        selection->removeAllBones();
                    }
                    addLazyExecutionCommand(nanoem_new(LazySetActiveModelBoneCommand(bonePtr)));
                }
                else if (const nanoem_model_morph_t *morphPtr = nanoemModelLabelItemGetMorphObject(item)) {
                    addLazyExecutionCommand(nanoem_new(LazySetActiveModelMorphCommand(morphPtr)));
                }
            }
        }
        else if (type == ITrack::kTypeModelBone && activeModel) {
            const ITrack *trackConst = track;
            const nanoem_model_bone_t *bonePtr = static_cast<const nanoem_model_bone_t *>(trackConst->opaque());
            IModelObjectSelection *selection = activeModel->selection();
            if (!ImGui::GetIO().KeyCtrl) {
                selection->removeAllBones();
            }
            addLazyExecutionCommand(nanoem_new(LazySetActiveModelBoneCommand(bonePtr)));
        }
        else if (type == ITrack::kTypeModelMorph) {
            const ITrack *trackConst = track;
            const nanoem_model_morph_t *morphPtr = static_cast<const nanoem_model_morph_t *>(trackConst->opaque());
            addLazyExecutionCommand(nanoem_new(LazySetActiveModelMorphCommand(morphPtr)));
        }
        track->setSelected(true);
        selectedTracks.insert(track);
    }
}

void
ImGuiWindow::handleTrackSelection(Project::TrackList &tracks, Project *project)
{
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
        if (Model *model = project->activeModel()) {
            nanoem_rsize_t offset = findTrack(model->activeBone(), ITrack::kTypeModelBone, tracks);
            if (offset != NANOEM_RSIZE_MAX) {
                if (ITrack *track = selectTrack(offset, ITrack::kTypeModelBone, tracks)) {
                    const ITrack *constTrack = track;
                    if (const nanoem_model_bone_t *bonePtr =
                            static_cast<const nanoem_model_bone_t *>(constTrack->opaque())) {
                        tracks[offset]->setSelected(false);
                        track->setSelected(true);
                        addLazyExecutionCommand(nanoem_new(LazySetActiveModelBoneCommand(bonePtr)));
                        m_requestedScrollHereTrack = track;
                    }
                }
            }
        }
        else {
            nanoem_rsize_t offset = findTrack(project->activeAccessory(), ITrack::kTypeAccessory, tracks);
            if (offset != NANOEM_RSIZE_MAX) {
                if (ITrack *track = selectTrack(offset, ITrack::kTypeAccessory, tracks)) {
                    if (Accessory *accessory = static_cast<Accessory *>(track->opaque())) {
                        tracks[offset]->setSelected(false);
                        track->setSelected(true);
                        addLazyExecutionCommand(nanoem_new(LazySetActiveAccessoryCommand(accessory, this)));
                        m_requestedScrollHereTrack = track;
                    }
                }
            }
        }
    }
}

void
ImGuiWindow::drawAllMarkersPanel(
    const ImVec2 &panelSize, const Project::TrackList &tracks, nanoem_u32_t numVisibleMarkers, Project *project)
{
    const ImGuiStyle &style = ImGui::GetStyle();
    const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
    ImGui::BeginChild("markers", panelSize, true);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(deviceScaleRatio, style.ItemSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(deviceScaleRatio, style.ItemInnerSpacing.y));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(deviceScaleRatio, style.FramePadding.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(deviceScaleRatio, style.WindowPadding.y));
    ImDrawList *draw = ImGui::GetWindowDrawList();
    const bool isMarkerWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
    if (m_draggingTimelineScrollBar && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        m_draggingTimelineScrollBar = false;
    }
    else if (!m_draggingTimelineScrollBar && ImGui::IsWindowHovered()) {
        const ImGuiIO &io = ImGui::GetIO();
        nanoem_f32_t x = ImGui::GetCursorScreenPos().x + ImGui::GetWindowContentRegionMax().x;
        if (x < io.MousePos.x) {
            m_draggingTimelineScrollBar = true;
        }
    }
    if (m_draggingTimelineScrollBar || isMarkerWindowHovered) {
        m_scrollTimelineY = ImGui::GetScrollY();
    }
    else if (!m_draggingTimelineScrollBar) {
        ImGui::SetScrollY(m_scrollTimelineY);
    }
    nanoem_f32_t extent = ImGui::GetTextLineHeight(), radius = extent * 0.5f;
    const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
    nanoem_u32_t startFrameIndex = 0, endFrameIndex = 0;
    {
        nanoem_u32_t center = static_cast<nanoem_u32_t>(numVisibleMarkers * 0.5f);
        if (frameIndex >= center) {
            startFrameIndex = frameIndex - center;
        }
        else {
            startFrameIndex = 0;
        }
        endFrameIndex = startFrameIndex + numVisibleMarkers;
    }
    const nanoem_f32_t lineWidth = 1.0f * deviceScaleRatio;
    for (nanoem_u32_t i = startFrameIndex; i <= endFrameIndex; i++) {
        bool current = i == frameIndex;
        ImVec2 from = ImGui::GetCursorScreenPos();
        from.y = -0xffff;
        from.x += radius + (extent + deviceScaleRatio) * (i - startFrameIndex);
        ImVec2 to(from);
        to.y = 0xffff;
        nanoem_u32_t color = 0;
        if (current) {
            color = IM_COL32(0, 0xff, 0, 0xff);
        }
        else if (i % 5 == 0) {
            color = IM_COL32(0x7f, 0x7f, 0x7f, 0xff);
        }
        else {
            color = IM_COL32(0x4f, 0x4f, 0x4f, 0xff);
        }
        draw->AddLine(from, to, color, lineWidth);
    }
    ImGuiListClipper clipper;
    UberMotionKeyframeSelection selection;
    nanoem_f32_t margin = radius * 0.25f;
    bool haveAnySelection = false, movingSelection = false;
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - margin);
    clipper.Begin(Inline::saturateInt32(tracks.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            ITrack *track = tracks[i];
            ImVec2 offsetFrom = ImGui::GetCursorScreenPos(), offsetTo(offsetFrom);
            offsetFrom.x = 0;
            offsetTo.x = 0xffff;
            draw->AddLine(offsetFrom, offsetTo, IM_COL32(0x4f, 0x4f, 0x4f, 0xff), lineWidth);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + margin);
            for (nanoem_frame_index_t frameIndex = startFrameIndex; frameIndex < endFrameIndex; frameIndex++) {
                const RhombusReactionType reaction =
                    drawMarkerRhombus(frameIndex, track, extent, radius, &selection, project);
                if (isRhombusReactionSelectable(reaction)) {
                    haveAnySelection |= true;
                }
                else if (reaction == kRhombusReactionMoving) {
                    movingSelection |= true;
                }
                ImGui::SameLine();
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - margin);
            ImGui::NewLine();
        }
    }
    ImVec2 offsetFrom = ImGui::GetCursorScreenPos(), offsetTo(offsetFrom);
    offsetFrom.x = 0;
    offsetTo.x = 0xffff;
    draw->AddLine(offsetFrom, offsetTo, IM_COL32(0x4f, 0x4f, 0x4f, 0xff), lineWidth);
    ImGui::PopStyleVar(4);
    if (ImGui::IsWindowFocused()) {
        m_draggingMarkerPanelRect = Constants::kZeroV4;
        if (m_movingAllSelectedKeyframesIndexDelta != 0 && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            Error error;
            CommandRegistrator registrator(project);
            registrator.registerMoveAllSelectedKeyframesCommand(m_movingAllSelectedKeyframesIndexDelta, error);
            error.notify(project->eventPublisher());
            m_movingAllSelectedKeyframesIndexDelta = 0;
        }
        else if (movingSelection) {
            nanoem_f32_t index = ImGui::GetMouseDragDelta().x / extent, f;
            glm::modf(index, f);
            m_movingAllSelectedKeyframesIndexDelta = static_cast<int>(f);
        }
        else if (!m_draggingTimelineScrollBar && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            const ImVec2 delta(ImGui::GetMouseDragDelta());
            ImVec2 to(ImGui::GetMousePos()), from(to);
            from.x -= delta.x;
            from.y -= delta.y;
            ImGui::GetWindowDrawList()->AddRectFilled(from, to, IM_COL32(0xff, 0x3f, 0, 0x7f));
            m_draggingMarkerPanelRect =
                Vector4(glm::min(from.x, to.x), glm::min(from.y, to.y), glm::max(from.x, to.x), glm::max(from.y, to.y));
        }
    }
    if ((isMarkerWindowHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) || haveAnySelection) {
        if (!ImGui::GetIO().KeyCtrl) {
            clearAllKeyframeSelection(project);
        }
        copyAllKeyframeSelection(&selection, project);
    }
    ImGui::EndChild();
}

ImGuiWindow::RhombusReactionType
ImGuiWindow::drawMarkerRhombus(nanoem_frame_index_t frameIndex, ITrack *track, nanoem_f32_t extent, nanoem_f32_t radius,
    IMotionKeyframeSelection *source, Project *project)
{
    const ImVec2 dummyRhombusSize(extent, 0);
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 offsetFrom = ImGui::GetCursorScreenPos();
    offsetFrom.x += radius;
    offsetFrom.y += radius;
    RhombusReactionType reaction = kRhombusReactionNone;
    nanoem_frame_index_t newFrameIndex;
    char buffer[64];
    StringUtils::format(buffer, sizeof(buffer), "%s/%u", track->nameConstString(), frameIndex);
    switch (track->type()) {
    case ITrack::kTypeCamera: {
        Motion *motion = project->cameraMotion();
        if (const nanoem_motion_camera_keyframe_t *keyframe = motion->findCameraKeyframe(frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, true);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            drawList->AddCircleFilled(offsetFrom, radius,
                motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault, 4);
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findCameraKeyframe(newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeLight: {
        Motion *motion = project->lightMotion();
        if (const nanoem_motion_light_keyframe_t *keyframe = motion->findLightKeyframe(frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, true);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            drawList->AddCircleFilled(offsetFrom, radius,
                motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault, 4);
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findLightKeyframe(newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeSelfShadow: {
        Motion *motion = project->selfShadowMotion();
        if (const nanoem_motion_self_shadow_keyframe_t *keyframe = motion->findSelfShadowKeyframe(frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, true);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            drawList->AddCircleFilled(offsetFrom, radius,
                motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault, 4);
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findSelfShadowKeyframe(newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeAccessory: {
        Accessory *accessory = static_cast<Accessory *>(track->opaque());
        Motion *motion = project->resolveMotion(accessory);
        if (const nanoem_motion_accessory_keyframe_t *keyframe = motion->findAccessoryKeyframe(frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, true);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            drawList->AddCircleFilled(offsetFrom, radius,
                motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault, 4);
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findAccessoryKeyframe(newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeModelRoot: {
        Model *model = static_cast<Model *>(track->opaque());
        Motion *motion = project->resolveMotion(model);
        if (const nanoem_motion_model_keyframe_t *keyframe = motion->findModelKeyframe(frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, false);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            drawList->AddCircleFilled(offsetFrom, radius,
                motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault, 4);
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findModelKeyframe(newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeModelLabel: {
        if (track->isExpanded()) {
            ImGui::Dummy(dummyRhombusSize);
        }
        else {
            const TrackList tracks(track->children());
            const nanoem_motion_bone_keyframe_t *rootBoneKeyframe = nullptr;
            const nanoem_motion_morph_keyframe_t *rootMorphKeyframe = nullptr;
            Model *model = project->activeModel();
            Motion *motion = project->resolveMotion(model);
            bool hasKeyframeRegistered = false, hasKeyframeSelected = false;
            if (const nanoem_model_label_t *labelPtr =
                    static_cast<const nanoem_model_label_t *>(static_cast<const ITrack *>(track)->opaque())) {
                nanoem_rsize_t numLabels;
                nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numLabels);
                if (items && numLabels > 0) {
                    const nanoem_model_label_item_t *item = items[0];
                    if (const nanoem_model_bone_t *bonePtr = nanoemModelLabelItemGetBoneObject(item)) {
                        const nanoem_unicode_string_t *name =
                            nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                        rootBoneKeyframe = motion->findBoneKeyframe(name, frameIndex);
                        hasKeyframeRegistered |= rootBoneKeyframe != nullptr;
                        hasKeyframeSelected |= motion->selection()->contains(rootBoneKeyframe);
                    }
                    else if (const nanoem_model_morph_t *morphPtr = nanoemModelLabelItemGetMorphObject(item)) {
                        const nanoem_unicode_string_t *name =
                            nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                        rootMorphKeyframe = motion->findMorphKeyframe(name, frameIndex);
                        hasKeyframeRegistered |= rootMorphKeyframe != nullptr;
                        hasKeyframeSelected |= motion->selection()->contains(rootMorphKeyframe);
                    }
                }
            }
            else {
                const IMotionKeyframeSelection *selection = motion->selection();
                for (TrackList::const_iterator it = tracks.begin(), end = tracks.end(); it != end; ++it) {
                    const ITrack *child = *it;
                    switch (child->type()) {
                    case ITrack::kTypeModelBone: {
                        const nanoem_model_bone_t *bonePtr = static_cast<const nanoem_model_bone_t *>(child->opaque());
                        const nanoem_unicode_string_t *name =
                            nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                        const nanoem_motion_bone_keyframe_t *keyframe = motion->findBoneKeyframe(name, frameIndex);
                        hasKeyframeRegistered |= keyframe != nullptr;
                        hasKeyframeSelected |= selection->contains(keyframe);
                        break;
                    }
                    case ITrack::kTypeModelMorph: {
                        const nanoem_model_morph_t *morphPtr =
                            static_cast<const nanoem_model_morph_t *>(child->opaque());
                        const nanoem_unicode_string_t *name =
                            nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                        const nanoem_motion_morph_keyframe_t *keyframe = motion->findMorphKeyframe(name, frameIndex);
                        hasKeyframeRegistered |= keyframe != nullptr;
                        hasKeyframeSelected |= selection->contains(keyframe);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
            if (hasKeyframeRegistered) {
                reaction = handleRhombusButton(buffer, extent, true);
                if (isRhombusReactionSelectable(reaction)) {
                    if (rootBoneKeyframe) {
                        source->add(rootBoneKeyframe);
                    }
                    else if (rootMorphKeyframe) {
                        source->add(rootMorphKeyframe);
                    }
                    else {
                        for (TrackList::const_iterator it = tracks.begin(), end = tracks.end(); it != end; ++it) {
                            const ITrack *child = *it;
                            switch (child->type()) {
                            case ITrack::kTypeModelBone: {
                                const nanoem_model_bone_t *bonePtr =
                                    static_cast<const nanoem_model_bone_t *>(child->opaque());
                                const nanoem_unicode_string_t *name =
                                    nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                                source->add(motion->findBoneKeyframe(name, frameIndex));
                            }
                            case ITrack::kTypeModelMorph: {
                                const nanoem_model_morph_t *morph =
                                    static_cast<const nanoem_model_morph_t *>(child->opaque());
                                const nanoem_unicode_string_t *name =
                                    nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                                source->add(motion->findMorphKeyframe(name, frameIndex));
                                break;
                            }
                            default:
                                break;
                            }
                        }
                    }
                }
                drawList->AddCircleFilled(
                    offsetFrom, radius, hasKeyframeSelected ? kColorRhombusSelect : kColorRhombusDefault, 4);
            }
            else {
                ImGui::Dummy(dummyRhombusSize);
            }
        }
        break;
    }
    case ITrack::kTypeModelBone: {
        const ITrack *trackConst = track;
        const nanoem_model_bone_t *bonePtr = static_cast<const nanoem_model_bone_t *>(trackConst->opaque());
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        Model *model = project->activeModel();
        Motion *motion = project->resolveMotion(model);
        if (const nanoem_motion_bone_keyframe_t *keyframe = motion->findBoneKeyframe(name, frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, true);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            const ImColor &color = motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault;
            if (nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(keyframe) && model->isRigidBodyBound(bonePtr)) {
                const nanoem_f32_t thickness = radius * 0.25f;
                drawList->AddLine(ImVec2(offsetFrom.x - radius, offsetFrom.y - radius),
                    ImVec2(offsetFrom.x + radius, offsetFrom.y + radius), color, thickness);
                drawList->AddLine(ImVec2(offsetFrom.x + radius, offsetFrom.y - radius),
                    ImVec2(offsetFrom.x - radius, offsetFrom.y + radius), color, thickness);
            }
            else {
                drawList->AddCircleFilled(offsetFrom, radius, color, 4);
            }
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findBoneKeyframe(name, newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeModelMorph: {
        const ITrack *trackConst = track;
        const nanoem_model_morph_t *morph = static_cast<const nanoem_model_morph_t *>(trackConst->opaque());
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morph, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        Model *model = project->activeModel();
        Motion *motion = project->resolveMotion(model);
        if (const nanoem_motion_morph_keyframe_t *keyframe = motion->findMorphKeyframe(name, frameIndex)) {
            reaction = handleRhombusButton(buffer, extent, true);
            if (isRhombusReactionSelectable(reaction)) {
                source->add(keyframe);
            }
            drawList->AddCircleFilled(offsetFrom, radius,
                motion->selection()->contains(keyframe) ? kColorRhombusSelect : kColorRhombusDefault, 4);
        }
        else {
            ImGui::Dummy(dummyRhombusSize);
            if (Motion::subtractFrameIndexDelta(m_movingAllSelectedKeyframesIndexDelta, frameIndex, newFrameIndex) &&
                motion->selection()->contains(motion->findMorphKeyframe(name, newFrameIndex))) {
                drawList->AddCircleFilled(offsetFrom, radius, kColorRhombusMoving, 4);
            }
        }
        break;
    }
    case ITrack::kTypeGravity:
    case ITrack::kTypeMaxEnum:
    default:
        break;
    }
    return reaction;
}

void
ImGuiWindow::drawKeyframeActionPanel(Project *project, nanoem_f32_t padding)
{
    const bool buttonEnabled = !project->isPlaying(),
               hasKeyframeSelection = buttonEnabled && project->hasKeyframeSelection(),
               hasKeyframeCopied = buttonEnabled && !project->isMotionClipboardEmpty();
    if (hasKeyframeSelection != m_lastKeyframeSelection) {
        addLazyExecutionCommand(nanoem_new(LazyPublishCanCopyEventCommand(hasKeyframeSelection)));
        m_lastKeyframeSelection = hasKeyframeSelection;
    }
    if (hasKeyframeCopied != m_lastKeyframeCopied) {
        addLazyExecutionCommand(nanoem_new(LazyPublishCanPasteEventCommand(hasKeyframeCopied)));
        m_lastKeyframeCopied = hasKeyframeCopied;
    }
    if (handleButton(tr("nanoem.gui.keyframe.copy"), (ImGui::GetContentRegionAvail().x - padding) / 3.0f,
            hasKeyframeSelection)) {
        Error error;
        project->copyAllSelectedKeyframes(error);
        error.notify(project->eventPublisher());
    }
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.keyframe.paste", (ImGui::GetContentRegionAvail().x - padding) / 2.0f, hasKeyframeCopied)) {
        Error error;
        project->pasteAllSelectedKeyframes(project->currentLocalFrameIndex(), error);
        error.notify(project->eventPublisher());
    }
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.keyframe.cut", ImGui::GetContentRegionAvail().x - padding, hasKeyframeSelection)) {
        Error error;
        project->copyAllSelectedKeyframes(error);
        CommandRegistrator registrator(project);
        registrator.registerRemoveAllSelectedKeyframesCommand();
        error.notify(project->eventPublisher());
    }
    if (padding > 0) {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding, 0));
        ImGui::Dummy(ImVec2(padding, 0));
        ImGui::SameLine();
    }
    if (handleButton(tr("nanoem.gui.keyframe.inverse-paste"), (ImGui::GetContentRegionAvail().x - padding) / 3.0f,
            hasKeyframeCopied)) {
        Error error;
        project->symmetricPasteAllSelectedKeyframes(project->currentLocalFrameIndex(), error);
        error.notify(project->eventPublisher());
    }
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.keyframe.select-column", (ImGui::GetContentRegionAvail().x - padding) / 2.0f, buttonEnabled)) {
        project->selectAllKeyframesInColumn();
    }
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.keyframe.remove", ImGui::GetContentRegionAvail().x - padding, hasKeyframeSelection)) {
        CommandRegistrator registrator(project);
        registrator.registerRemoveAllSelectedKeyframesCommand();
    }
    if (padding > 0) {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding, 0));
    }
}

void
ImGuiWindow::drawKeyframeSelectionPanel(Project *project, nanoem_f32_t padding)
{
    if (padding > 0) {
        ImGui::Dummy(ImVec2(padding, 0));
        ImGui::SameLine();
    }
    ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - padding) * 0.4f);
    if (Model *activeModel = project->activeModel()) {
        ModelKeyframeSelector selector(project);
        selector.combo(&m_modelKeyframeSelectorIndex);
        drawKeyframeSelectionPanel(&selector, m_modelKeyframeSelectorIndex, padding, project);
    }
    else {
        ProjectKeyframeSelector selector(project);
        selector.combo(&m_projectKeyframeSelectorIndex);
        drawKeyframeSelectionPanel(&selector, m_projectKeyframeSelectorIndex, padding, project);
    }
}

void
ImGuiWindow::drawKeyframeSelectionPanel(void *selector, int index, nanoem_f32_t padding, Project *project)
{
    IKeyframeSelector *sel = static_cast<IKeyframeSelector *>(selector);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    const nanoem_frame_index_t duration = project->duration();
    const bool playing = project->isPlaying();
    TimelineSegment segment = project->selectionSegment();
    ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - padding) / 2.0f);
    if (handleDragScalarN("##timeline.select.range.from", kFrameIndexDataType, &segment.m_from, 1, !playing, 1.0f,
            nullptr, &segment.m_to, "From: %u", ImGuiSliderFlags_None)) {
        project->setSelectionSegment(segment);
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - padding);
    if (handleDragScalarN("##timeline.select.range.to", kFrameIndexDataType, &segment.m_to, 1, !playing, 1.0f,
            &segment.m_from, &duration, "To: %u", ImGuiSliderFlags_None)) {
        project->setSelectionSegment(segment);
    }
    ImGui::PopItemWidth();
    ImGui::PopItemWidth();
    if (padding > 0) {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding, 0));
        ImGui::Dummy(ImVec2(padding, 0));
        ImGui::SameLine();
    }
    bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton(
            "nanoem.gui.keyframe.select", (ImGui::GetContentRegionAvail().x - padding) / 2.0f, buttonEnabled)) {
        sel->handleAction(segment, index);
    }
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.keyframe.scale", (ImGui::GetContentRegionAvail().x - padding), buttonEnabled)) {
        openScaleAllSelectedKeyframesDialog();
    }
    if (padding > 0) {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(padding, 0));
    }
}

void
ImGuiWindow::drawViewport(Project *project, IState *state, nanoem_u32_t flags)
{
    const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio(),
                       invertDeviceScaleRatio = 1.0f / deviceScaleRatio;
    if (const Model *activeModel = project->activeModel()) {
        ImGui::Text(
            "%s: %s", activeModel->nameConstString(), model::Bone::nameConstString(activeModel->activeBone(), nullptr));
    }
    else if (const Accessory *activeAccessory = project->activeAccessory()) {
        ImGui::Text("%s: %s", tr("nanoem.gui.panel.model.default"), activeAccessory->nameConstString());
    }
    else {
        ImGui::TextUnformatted(tr("nanoem.gui.panel.model.default"));
    }
    // ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeightWithSpacing()));
    const ImVec2 offset(ImGui::GetCursorScreenPos());
    ImVec2 size(ImGui::GetContentRegionAvail()), innerOffset(offset), basePos;
#if defined(IMGUI_HAS_VIEWPORT)
    basePos = ImGui::GetWindowViewport()->Pos;
#endif /* IMGUI_HAS_VIEWPORT */
    innerOffset.x -= basePos.x;
    innerOffset.y -= basePos.y;
    const bool isModelEditingEnabled = project->isModelEditingEnabled();
    if (!isModelEditingEnabled) {
        size.y -= ImGui::GetFrameHeightWithSpacing();
    }
    const Vector4 viewportLayout(innerOffset.x, innerOffset.y, size.x, size.y);
    bool hovered = ImGui::IsWindowHovered();
    project->resizeUniformedViewportLayout(viewportLayout * invertDeviceScaleRatio);
    const Vector4 viewportImageRect(createViewportImageRect(project, viewportLayout));
    const sg_image viewportImageHandle = project->viewportPrimaryImage();
    ImGui::Dummy(size);
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    const ImVec2 a(offset.x, offset.y), b(offset.x + size.x, offset.y + size.y),
        viewportImageFrom(basePos.x + viewportImageRect.x, basePos.y + viewportImageRect.y),
        viewportImageTo(viewportImageFrom.x + viewportImageRect.z, viewportImageFrom.y + viewportImageRect.w);
    const nanoem_f32_t tileSizeRatio = 1.0f / (16.0f * deviceScaleRatio);
    const Vector2UI16 viewportPadding(
        viewportImageFrom.x > a.y ? (viewportImageFrom.x - a.x) * invertDeviceScaleRatio : 0,
        viewportImageFrom.y > a.y ? (viewportImageFrom.y - a.y) * invertDeviceScaleRatio : 0);
    project->setLogicalViewportPadding(viewportPadding);
    drawList->PushClipRect(a, b);
    drawList->AddImage(reinterpret_cast<ImTextureID>(m_transparentTileImage.id), viewportImageFrom, viewportImageTo,
        ImVec2(0, 0), ImVec2(viewportImageRect.z * tileSizeRatio, viewportImageRect.w * tileSizeRatio));
    ImVec2 uv0, uv1;
    if (sg::query_features().origin_top_left) {
        uv1 = ImVec2(1, 1);
    }
    else {
        uv0 = ImVec2(0, 1);
        uv1 = ImVec2(1, 0);
    }
    drawList->AddImage(
        reinterpret_cast<ImTextureID>(viewportImageHandle.id), viewportImageFrom, viewportImageTo, uv0, uv1);
    if (isModelEditingEnabled) {
        if (m_gizmoController) {
            hovered &= m_gizmoController->draw(drawList, offset, size, project);
        }
        else {
            m_gizmoController = nanoem_new(GizmoController);
        }
    }
    {
        m_primitive2D.setBaseOffset(offset);
        m_primitive2D.setScaleFactor(deviceScaleRatio);
        drawPrimitive2D(project, state, flags);
        drawTransformHandleSet(project, state, offset);
        if (project->isFPSCounterEnabled()) {
            drawFPSCounter(project, offset);
        }
        if (project->isPerformanceMonitorEnabled()) {
            drawPerformanceMonitor(project, offset);
        }
        drawBoneTooltip(project, flags);
    }
    drawList->PopClipRect();
    if (!isModelEditingEnabled) {
        drawViewportParameterBox(project);
    }
    project->setViewportHovered(hovered);
}

void
ImGuiWindow::drawViewportParameterBox(Project *project)
{
    const nanoem_f32_t width = ImGui::GetContentRegionAvail().x;
    if (Model *activeModel = project->activeModel()) {
        const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        ImGui::PushItemWidth(width * 0.2f);
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.model.translation"));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(width * 0.3f);
        Vector3 translation(activeBone ? activeBone->localUserTranslation() : Constants::kZeroV3);
        if (handleDragFloat3("##viewport.bone.translation", glm::value_ptr(translation),
                model::Bone::isMovable(activeBonePtr), kTranslationStepFactor, 0, 0, "%.2f", ImGuiSliderFlags_None)) {
            setModelBoneTranslation(translation, activeBonePtr, activeModel, project);
        }
        if (handleVectorValueState(m_boneTranslationValueState)) {
            nanoem_delete_safe(m_boneTranslationValueState);
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(width * 0.2f);
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.model.orientation"));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(width * 0.3f);
        Vector3 orientation(
            activeBone ? glm::degrees(glm::eulerAngles(activeBone->localUserOrientation())) : Constants::kZeroV3);
        if (handleDragFloat3("##viewport.bone.orientation", glm::value_ptr(orientation),
                model::Bone::isRotateable(activeBonePtr), kOrientationStepFactor, -180, 180, "%.1f",
                ImGuiSliderFlags_None)) {
            setModelBoneOrientation(glm::radians(orientation), activeBonePtr, activeModel, project);
        }
        if (handleVectorValueState(m_boneOrientationValueState)) {
            nanoem_delete_safe(m_boneOrientationValueState);
        }
        ImGui::PopItemWidth();
    }
    else {
        const bool playing = project->isPlaying();
        ICamera *camera = project->activeCamera();
        ImGui::PushItemWidth(width * 0.2f);
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.look-at"));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(width * 0.3f);
        Vector3 lookAt(camera->lookAt());
        if (handleDragFloat3("##viewport.camera.look-at", glm::value_ptr(lookAt), !playing, kTranslationStepFactor, 0,
                0, "%.2f", ImGuiSliderFlags_None)) {
            setCameraLookAt(lookAt, camera, project);
        }
        if (handleVectorValueState(m_cameraLookAtVectorValueState)) {
            nanoem_delete_safe(m_cameraLookAtVectorValueState);
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(width * 0.2f);
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.angle"));
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(width * 0.3f);
        Vector3 angle(glm::degrees(camera->angle()));
        if (handleDragFloat3("##viewport.camera.angle", glm::value_ptr(angle), !playing, kOrientationStepFactor, -180,
                180, "%.1f", ImGuiSliderFlags_None)) {
            setCameraAngle(angle, camera, project);
        }
        if (handleVectorValueState(m_cameraAngleVectorValueState)) {
            nanoem_delete_safe(m_cameraAngleVectorValueState);
        }
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(width * 0.2f);
        ImGui::SameLine();
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.distance"));
        ImGui::PopItemWidth();
        ImGui::PushItemWidth(width * 0.2f);
        ImGui::SameLine();
        nanoem_f32_t distance(camera->distance());
        if (handleDragFloat(
                "##viewport.camera.distance", &distance, !playing, 1.0f, 1, 100000.0f, "%.1f", ImGuiSliderFlags_None)) {
            setCameraDistance(distance, camera, project);
        }
        if (handleVectorValueState(m_cameraDistanceVectorValueState)) {
            nanoem_delete_safe(m_cameraDistanceVectorValueState);
        }
        ImGui::PopItemWidth();
    }
}

void
ImGuiWindow::drawCommonInterpolationControls(Project *project)
{
    bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton("nanoem.gui.panel.interpolation.copy", -1, buttonEnabled)) {
        project->copyAllSelectedKeyframeInterpolations();
    }
    if (handleTranslatedButton("nanoem.gui.panel.interpolation.paste", -1,
            buttonEnabled && project->hasSelectedKeyframeInterpolations())) {
        project->pasteAllSelectedKeyframeInterpolations(project->currentLocalFrameIndex());
    }
    if (handleTranslatedButton("nanoem.gui.panel.interpolation.linear", -1,
            buttonEnabled && project->hasSelectedKeyframeInterpolations())) {
        Error error;
        project->makeAllSelectedKeyframeInterpolationsLinear(error);
        error.notify(project->eventPublisher());
    }
    bool enableAdjustment = project->isBezierCurveAjustmentEnabled();
    if (ImGui::Checkbox(tr("nanoem.gui.panel.interpolation.automatic"), &enableAdjustment)) {
        project->setBezierCurveAdjustmentEnabled(enableAdjustment);
    }
}

void
ImGuiWindow::drawBoneInterpolationPanel(const ImVec2 &panelSize, Model *activeModel, Project *project)
{
    static const Vector4U8 kMinCurvePointValue(0), kMaxCurvePointValue(0x7f);
    ImGui::BeginChild("interpolation", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.interpolation.title"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushItemWidth(-1);
    Vector4U8 controlPoint(20, 20, 107, 107);
    nanoem_motion_bone_keyframe_interpolation_type_t type = project->boneKeyframeInterpolationType();
    model::Bone *bone = nullptr;
    if (Motion *activeMotion = project->resolveMotion(activeModel)) {
        Motion::BoneKeyframeList keyframes;
        activeMotion->selection()->getAll(keyframes, nullptr);
        if (keyframes.size() > 0) {
            const nanoem_unicode_string_t *namePtr = nanoemMotionBoneKeyframeGetName(keyframes[0]);
            String name;
            StringUtils::getUtf8String(namePtr, project->unicodeStringFactory(), name);
            bone = model::Bone::cast(activeModel->findBone(name));
            if (bone) {
                controlPoint = bone->bezierControlPoints(type);
            }
        }
    }
    if (handleButton(reinterpret_cast<const char *>(kFACogs), 0, bone != nullptr)) {
        openBoneKeyframeBezierInterpolationCurveGraphDialog(project);
    }
    ImGui::SameLine();
    const ITranslator *translator = m_applicationPtr->translator();
    if (ImGui::BeginCombo("##interpolation.type", selectBoneInterpolationType(translator, type))) {
        for (int i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            nanoem_motion_bone_keyframe_interpolation_type_t item =
                static_cast<nanoem_motion_bone_keyframe_interpolation_type_t>(i);
            if (ImGui::Selectable(selectBoneInterpolationType(translator, item))) {
                project->setBoneKeyframeInterpolationType(item);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    if (handleSliderScalarN("##curve.x0", ImGuiDataType_U8, &controlPoint.x, 1, bone != nullptr,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "X0: %d",
            ImGuiSliderFlags_None)) {
        bone->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (handleSliderScalarN("##curve.y0", ImGuiDataType_U8, &controlPoint.y, 1, bone != nullptr,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "Y0: %d",
            ImGuiSliderFlags_None)) {
        bone->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    if (handleSliderScalarN("##curve.x1", ImGuiDataType_U8, &controlPoint.z, 1, bone != nullptr,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "X1: %d",
            ImGuiSliderFlags_None)) {
        bone->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (handleSliderScalarN("##curve.y1", ImGuiDataType_U8, &controlPoint.w, 1, bone != nullptr,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "Y1: %d",
            ImGuiSliderFlags_None)) {
        bone->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    drawCommonInterpolationControls(project);
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void
ImGuiWindow::drawCameraInterpolationPanel(const ImVec2 &panelSize, Project *project)
{
    static const Vector4U8 kMinCurvePointValue(0), kMaxCurvePointValue(0x7f);
    const bool playing = project->isPlaying();
    ImGui::BeginChild("interpolation", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.interpolation.title"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushItemWidth(-1);
    Vector4U8 controlPoint(20, 20, 107, 107);
    nanoem_motion_camera_keyframe_interpolation_type_t type = project->cameraKeyframeInterpolationType();
    controlPoint = project->globalCamera()->bezierControlPoints(type);
    if (handleButton(reinterpret_cast<const char *>(kFACogs))) {
        openCameraKeyframeBezierInterpolationCurveGraphDialog(project);
    }
    ImGui::SameLine();
    const ITranslator *translator = m_applicationPtr->translator();
    if (ImGui::BeginCombo("##interpolation.type", selectCameraInterpolationType(translator, type))) {
        for (int i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            nanoem_motion_camera_keyframe_interpolation_type_t item =
                static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(i);
            if (ImGui::Selectable(selectCameraInterpolationType(translator, item))) {
                project->setCameraKeyframeInterpolationType(item);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    if (handleSliderScalarN("##curve.x0", ImGuiDataType_U8, &controlPoint.x, 1, !playing,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "X0: %d",
            ImGuiSliderFlags_None)) {
        project->globalCamera()->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (handleSliderScalarN("##curve.y0", ImGuiDataType_U8, &controlPoint.y, 1, !playing,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "Y0: %d",
            ImGuiSliderFlags_None)) {
        project->globalCamera()->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    if (handleSliderScalarN("##curve.x1", ImGuiDataType_U8, &controlPoint.z, 1, !playing,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "X1: %d",
            ImGuiSliderFlags_None)) {
        project->globalCamera()->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (handleSliderScalarN("##curve.y1", ImGuiDataType_U8, &controlPoint.w, 1, !playing,
            glm::value_ptr(kMinCurvePointValue), glm::value_ptr(kMaxCurvePointValue), "Y1: %d",
            ImGuiSliderFlags_None)) {
        project->globalCamera()->setBezierControlPoints(type, controlPoint);
    }
    else if (ImGui::IsItemDeactivatedAfterEdit()) {
    }
    ImGui::PopItemWidth();
    drawCommonInterpolationControls(project);
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void
ImGuiWindow::drawModelPanel(const ImVec2 &panelSize, Project *project)
{
    Model *activeModel = project->activeModel();
    ImGui::BeginChild("model", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.model.title"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushItemWidth(-1);
    if (m_editingModelName) {
        if (activeModel) {
            char buffer[Inline::kMarkerStringLength];
            StringUtils::copyString(buffer, activeModel->nameConstString(), sizeof(buffer));
            if (ImGui::InputText("##name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                activeModel->setName(buffer);
                m_editingModelName = false;
            }
        }
        else {
            m_editingModelName = false;
        }
    }
    else {
        ActiveModelSelector activeModelSelector(project);
        int modelIndex = activeModelSelector.index(activeModel);
        if (activeModelSelector.combo(&modelIndex)) {
            Model *newActiveModel = activeModelSelector.resolve(modelIndex);
            addLazyExecutionCommand(nanoem_new(LazySetActiveModelCommand(newActiveModel, this)));
        }
        else if (activeModel && ImGui::IsItemClicked() && ImGui::GetIO().KeyCtrl) {
            m_editingModelName = true;
        }
    }
    bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton("nanoem.gui.panel.model.load", ImGui::GetContentRegionAvail().x * 0.5f, buttonEnabled)) {
        StringList extensions(Model::loadableExtensions());
        extensions.push_back("txt");
        project->eventPublisher()->publishQueryOpenSingleFileDialogEvent(
            IFileManager::kDialogTypeOpenModelFile, extensions);
    }
    ImGui::SameLine();
    if (handleButton(tr("nanoem.gui.panel.model.delete"), ImGui::GetContentRegionAvail().x, activeModel != nullptr)) {
        m_applicationPtr->addModalDialog(
            ModalDialogFactory::createConfirmDeletingModelDialog(activeModel, m_applicationPtr));
    }
    ImGui::PopItemWidth();
    bool visible = activeModel ? activeModel->isVisible() : false;
    if (handleCheckBox(tr("nanoem.gui.panel.model.visible.short"), &visible, activeModel != nullptr)) {
        activeModel->setVisible(visible);
    }
    if (ImGui::IsItemHovered()) {
        drawTextTooltip(tr("nanoem.gui.panel.model.visible"));
    }
    ImGui::SameLine();
    bool shadow = activeModel ? activeModel->isShadowMapEnabled() : false;
    if (handleCheckBox(tr("nanoem.gui.panel.model.shadow.short"), &shadow, activeModel != nullptr)) {
        activeModel->setShadowMapEnabled(shadow);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tr("nanoem.gui.panel.model.shadow"));
    }
    ImGui::SameLine();
    bool add = activeModel ? activeModel->isAddBlendEnabled() : false;
    if (handleCheckBox(tr("nanoem.gui.panel.model.blend-add.short"), &add, activeModel != nullptr)) {
        activeModel->setAddBlendEnabled(add);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tr("nanoem.gui.panel.model.blend-add"));
    }
    ConstraintSelector constraintSelector(activeModel, project->translator());
    int constraintIndex = constraintSelector.index(activeModel);
    if (constraintSelector.combo(&constraintIndex) && activeModel) {
        activeModel->setActiveConstraint(constraintSelector.activeConstraint(constraintIndex));
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.panel.model.outside-parent", 0, activeModel != nullptr)) {
        openModelOutsideParentDialog(project);
    }
    model::Constraint *constraint = nullptr;
    bool enabled = false;
    if (activeModel) {
        if (const nanoem_model_constraint_t *constraintPtr = activeModel->activeConstraint()) {
            constraint = model::Constraint::cast(constraintPtr);
            enabled = constraint->isEnabled();
        }
    }
    struct SetConstraintEnabledCommand : ImGuiWindow::ILazyExecutionCommand {
        SetConstraintEnabledCommand(model::Constraint *constraint, bool enabled, Model *model)
            : m_model(model)
            , m_constraint(constraint)
            , m_enabled(enabled)
        {
        }
        void
        execute(Project * /* project */) NANOEM_DECL_OVERRIDE
        {
            m_constraint->setEnabled(m_enabled);
            m_model->performAllBonesTransform();
        }
        void
        destroy(Project *project) NANOEM_DECL_OVERRIDE
        {
            BX_UNUSED_1(project);
        }
        Model *m_model;
        model::Constraint *m_constraint;
        const bool m_enabled;
    };
    if (handleRadioButton("ON##constraint.enable", enabled, constraint != nullptr)) {
        addLazyExecutionCommand(nanoem_new(SetConstraintEnabledCommand(constraint, true, activeModel)));
    }
    ImGui::SameLine();
    if (handleRadioButton("OFF##constraint.disable", !enabled, constraint != nullptr)) {
        addLazyExecutionCommand(nanoem_new(SetConstraintEnabledCommand(constraint, false, activeModel)));
    }
    if (handleTranslatedButton("nanoem.gui.panel.model.register", -1, activeModel != nullptr)) {
        CommandRegistrator registrator(project);
        registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(activeModel);
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawCameraPanel(const ImVec2 &panelSize, Project *project)
{
    ICamera *activeCamera = project->activeCamera();
    ImGui::BeginChild("camera", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.camera.title"));
    ImGui::Separator();
    ImGui::Spacing();
    bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton("nanoem.gui.panel.camera.reset", -1, buttonEnabled)) {
        activeCamera->reset();
        activeCamera->update();
        project->resetAllModelEdges();
    }
    bool perspective = activeCamera->isPerspective();
    if (handleCheckBox(tr("nanoem.gui.panel.camera.perspective"), &perspective, buttonEnabled)) {
        setCameraPerspective(perspective, activeCamera, project);
    }
    ImGui::PushItemWidth(-1);
    int fov = activeCamera->fov();
    if (handleSliderInt(
            "##fov", &fov, buttonEnabled, 1, 135, tr("nanoem.gui.panel.camera.fov.format"), ImGuiSliderFlags_None)) {
        setCameraFov(static_cast<nanoem_f32_t>(fov), activeCamera, project);
    }
    if (handleVectorValueState(m_cameraFovVectorValueState)) {
        nanoem_delete_safe(m_cameraFovVectorValueState);
    }
    StringPair pair(activeCamera->outsideParent());
    CameraParentModelSelector modelSelector(project);
    int modelIndex = modelSelector.index(pair);
    if (modelSelector.combo(&modelIndex)) {
        pair.first = modelSelector.resolveName(modelIndex);
        addLazyExecutionCommand(nanoem_new(LazySetCameraOutsideParenthCommand(pair)));
    }
    const Model *model = modelSelector.resolveModel(modelIndex);
    CameraParentModelBoneSelector boneSelector(model, project->translator());
    int boneIndex = boneSelector.index(pair);
    if (boneSelector.combo(&boneIndex)) {
        pair.second = boneSelector.resolveName(boneIndex);
        addLazyExecutionCommand(nanoem_new(LazySetCameraOutsideParenthCommand(pair)));
    }
    ImGui::PopItemWidth();
    if (handleTranslatedButton("nanoem.gui.panel.camera.register", -1, buttonEnabled)) {
        CommandRegistrator registrator(project);
        registrator.registerAddCameraKeyframesCommandByCurrentLocalFrameIndex();
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawLightPanel(const ImVec2 &panelSize, Project *project)
{
    ILight *activeLight = project->activeLight();
    ImGui::BeginChild("light", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.light.title"));
    ImGui::Separator();
    ImGui::Spacing();
    bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton("nanoem.gui.panel.light.reset", -1, buttonEnabled)) {
        activeLight->reset();
    }
    ImGui::PushItemWidth(-1);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.light.color"));
    Vector3 color(activeLight->color());
    if (ImGui::ColorEdit3("##light.color", glm::value_ptr(color))) {
        setLightColor(color, activeLight, project);
    }
    if (handleVectorValueState(m_lightColorVectorValueState)) {
        nanoem_delete_safe(m_lightColorVectorValueState);
    }
    ImGui::TextUnformatted(tr("nanoem.gui.panel.light.direction"));
    Vector3 direction(activeLight->direction());
    if (handleSliderFloat3("##light.direction", glm::value_ptr(direction), buttonEnabled, -1.0f, 1.0f, "%.2f",
            ImGuiSliderFlags_None)) {
        setLightDirection(direction, activeLight, project);
    }
    if (handleVectorValueState(m_lightDirectionVectorValueState)) {
        nanoem_delete_safe(m_lightDirectionVectorValueState);
    }
    ImGui::PopItemWidth();
    if (handleTranslatedButton("nanoem.gui.panel.light.self-shadow", -1, buttonEnabled)) {
        openSelfShadowConfigurationDialog();
    }
    if (handleTranslatedButton("nanoem.gui.panel.light.register", -1, buttonEnabled)) {
        CommandRegistrator registrator(project);
        registrator.registerAddLightKeyframesCommandByCurrentLocalFrameIndex();
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawAccessoryPanel(const ImVec2 &panelSize, Project *project)
{
    Accessory *activeAccessory = project->activeAccessory();
    ImGui::BeginChild("accessory", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.accessory.title"));
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::PushItemWidth(-1);
    if (m_editingAccessoryName) {
        if (activeAccessory) {
            char buffer[Inline::kMarkerStringLength];
            StringUtils::copyString(buffer, activeAccessory->nameConstString(), sizeof(buffer));
            if (ImGui::InputText("##name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                activeAccessory->setName(buffer);
                m_editingAccessoryName = false;
            }
        }
        else {
            m_editingAccessoryName = false;
        }
    }
    else {
        ActiveAccessorySelector activeAccessorySelector(project);
        int accessoryIndex = activeAccessorySelector.index(activeAccessory);
        if (activeAccessorySelector.combo(&accessoryIndex)) {
            Accessory *newActiveAccessory = activeAccessorySelector.resolve(accessoryIndex);
            addLazyExecutionCommand(nanoem_new(LazySetActiveAccessoryCommand(newActiveAccessory, this)));
        }
        else if (activeAccessory && ImGui::IsItemClicked() && ImGui::GetIO().KeyCtrl) {
            m_editingModelName = true;
        }
    }
    ImGui::PopItemWidth();
    const bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton(
            "nanoem.gui.panel.accessory.load", ImGui::GetContentRegionAvail().x * 0.5f, buttonEnabled)) {
        StringList extensions(Accessory::loadableExtensions());
        extensions.push_back("txt");
        project->eventPublisher()->publishQueryOpenSingleFileDialogEvent(
            IFileManager::kDialogTypeOpenModelFile, extensions);
    }
    ImGui::SameLine();
    if (handleButton(
            tr("nanoem.gui.panel.accessory.delete"), ImGui::GetContentRegionAvail().x, activeAccessory != nullptr)) {
        m_applicationPtr->addModalDialog(
            ModalDialogFactory::createConfirmDeletingAccessoryDialog(activeAccessory, m_applicationPtr));
    }
    bool visible = activeAccessory ? activeAccessory->isVisible() : false;
    nanoem_f32_t width = ImGui::GetContentRegionAvail().x;
    if (handleCheckBox(tr("nanoem.gui.panel.accessory.visible"), &visible, activeAccessory != nullptr)) {
        activeAccessory->setVisible(visible);
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - width * 0.5f, 0));
    ImGui::SameLine();
    if (handleButton(
            tr("nanoem.gui.panel.accessory.op"), ImGui::GetContentRegionAvail().x, activeAccessory != nullptr)) {
        openAccessoryOutsideParentDialog(project);
    }
    ImGui::PushItemWidth(-1);
    Vector3 translation(activeAccessory ? activeAccessory->translation() : Constants::kZeroV3);
    if (handleDragFloat3("##accessory.translation", glm::value_ptr(translation), activeAccessory && buttonEnabled,
            kTranslationStepFactor, 0.0f, 0.0f, "%.2f", ImGuiSliderFlags_None)) {
        setAccessoryTranslation(translation, activeAccessory);
    }
    if (handleVectorValueState(m_accessoryTranslationVectorValueState)) {
        nanoem_delete_safe(m_accessoryTranslationVectorValueState);
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        drawTextTooltip(project->translator()->translate("nanoem.gui.panel.accessory.translation"));
    }
    Vector3 orientation(glm::degrees(activeAccessory ? activeAccessory->orientation() : Constants::kZeroV3));
    if (handleDragFloat3("##accessory.orientation", glm::value_ptr(orientation), activeAccessory && buttonEnabled,
            kOrientationStepFactor, -180.0f, 180.0f, "%.1f", ImGuiSliderFlags_None)) {
        setAccessoryOrientation(glm::radians(orientation), activeAccessory);
    }
    if (handleVectorValueState(m_accessoryOrientationVectorValueState)) {
        nanoem_delete_safe(m_accessoryOrientationVectorValueState);
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        drawTextTooltip(project->translator()->translate("nanoem.gui.panel.accessory.orientation"));
    }
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    nanoem_f32_t scaleFactor = activeAccessory ? activeAccessory->scaleFactor() : 0.0f;
    if (handleDragFloat("##accessory.scale", &scaleFactor, activeAccessory && buttonEnabled, 0.0025f, 0.01f, FLT_MAX,
            "Si: %.2f", ImGuiSliderFlags_None) &&
        activeAccessory) {
        setAccessoryScaleFactor(scaleFactor, activeAccessory);
    }
    if (handleVectorValueState(m_accessoryScaleFactorValueState)) {
        nanoem_delete_safe(m_accessoryScaleFactorValueState);
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        drawTextTooltip(project->translator()->translate("nanoem.gui.panel.accessory.scale"));
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    nanoem_f32_t opacity = activeAccessory ? activeAccessory->opacity() : 0.0f;
    if (handleSliderFloat("##accessory.opacity", &opacity, activeAccessory && buttonEnabled, 0.0f, 1.0f, "Tr: %.2f",
            ImGuiSliderFlags_None)) {
        setAccessoryOpacity(opacity, activeAccessory);
    }
    if (handleVectorValueState(m_accessoryOpacityValueState)) {
        nanoem_delete_safe(m_accessoryOpacityValueState);
    }
    if (ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
        drawTextTooltip(project->translator()->translate("nanoem.gui.panel.accessory.opacity"));
    }
    ImGui::PopItemWidth();
    if (handleTranslatedButton("nanoem.gui.panel.accessory.register", -1, activeAccessory != nullptr)) {
        CommandRegistrator registrator(project);
        registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(activeAccessory);
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawBonePanel(const ImVec2 &panelSize, Model *activeModel, Project *project)
{
    IModelObjectSelection *selection = activeModel->selection();
    ImGui::BeginChild("bone", ImVec2(panelSize.x * 1.3f, panelSize.y), true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.bone.title"));
    ImGui::Separator();
    ImGui::Columns(2, nullptr, false);
    const nanoem_model_bone_t *activeBone = activeModel->activeBone();
    const Project::EditingMode editingMode = project->editingMode();
    bool buttonEnabled = !project->isPlaying();
    if (handleRadioButton(tr("nanoem.gui.panel.bone.select"), editingMode == Project::kEditingModeSelect,
            buttonEnabled && model::Bone::isSelectable(activeBone))) {
        toggleEditingMode(project, Project::kEditingModeSelect);
    }
    ImGui::NextColumn();
    if (handleRadioButton(tr("nanoem.gui.panel.bone.rotate"), editingMode == Project::kEditingModeRotate,
            buttonEnabled && model::Bone::isRotateable(activeBone))) {
        toggleEditingMode(project, Project::kEditingModeRotate);
    }
    ImGui::NextColumn();
    if (handleRadioButton(tr("nanoem.gui.panel.bone.move"), editingMode == Project::kEditingModeMove,
            buttonEnabled && model::Bone::isMovable(activeBone))) {
        toggleEditingMode(project, Project::kEditingModeMove);
    }
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    if (handleRadioButton("(none)", editingMode == Project::kEditingModeNone, buttonEnabled)) {
        toggleEditingMode(project, Project::kEditingModeNone);
    }
    ImGui::Columns(1);
    if (handleTranslatedButton(
            "nanoem.gui.panel.bone.select.box", ImGui::GetContentRegionAvail().x / 3.0f, buttonEnabled)) {
        setEditingMode(project, Project::kEditingModeSelect);
        selection->removeAllBones();
        selection->setBoxSelectedBoneModeEnabled(selection->isBoxSelectedBoneModeEnabled() ? false : true);
    }
    ImGui::SameLine();
    if (handleTranslatedButton(
            "nanoem.gui.panel.bone.select.all", ImGui::GetContentRegionAvail().x / 2.0f, buttonEnabled)) {
        setEditingMode(project, Project::kEditingModeSelect);
        selection->addAllBones();
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.panel.bone.select.dirty", ImGui::GetContentRegionAvail().x, buttonEnabled)) {
        setEditingMode(project, Project::kEditingModeSelect);
        selection->addAllDirtyBones();
    }
    if (handleTranslatedButton("nanoem.gui.panel.bone.copy", ImGui::GetContentRegionAvail().x / 2.0f, buttonEnabled)) {
        Error error;
        project->copyAllSelectedBones(error);
        error.notify(project->eventPublisher());
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.panel.bone.paste", ImGui::GetContentRegionAvail().x, buttonEnabled)) {
        Error error;
        project->pasteAllSelectedBones(error);
        error.notify(project->eventPublisher());
    }
    if (handleTranslatedButton(
            "nanoem.gui.panel.bone.inverse-paste", ImGui::GetContentRegionAvail().x / 2.0f, buttonEnabled)) {
        Error error;
        project->symmetricPasteAllSelectedBones(error);
        error.notify(project->eventPublisher());
    }
    ImGui::SameLine();
    bool checked = project->isPhysicsSimulationForBoneKeyframeEnabled();
    if (handleCheckBox(tr("nanoem.gui.panel.bone.physics-simulation"), &checked, buttonEnabled)) {
        project->setPhysicsSimulationForBoneKeyframeEnabled(checked);
    }
    ImGui::Spacing();
    if (handleTranslatedButton("nanoem.gui.panel.bone.register", -1, buttonEnabled)) {
        CommandRegistrator registrator(project);
        registrator.registerAddBoneKeyframesCommandBySelectedBoneSet(activeModel);
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawMorphPanel(const ImVec2 &panelSize, Model *activeModel, Project *project)
{
    ImGui::BeginChild("morph", ImVec2(panelSize.x * 1.7f, panelSize.y), true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.morph.title"));
    ImGui::Separator();
    ImGui::Spacing();
    const ITranslator *translator = project->translator();
    MorphSelector eye(activeModel, translator, NANOEM_MODEL_MORPH_CATEGORY_EYE),
        lip(activeModel, translator, NANOEM_MODEL_MORPH_CATEGORY_LIP);
    {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        int eyeIndex = eye.index(), lipIndex = lip.index();
        if (eye.combo(&eyeIndex)) {
            addLazyExecutionCommand(
                nanoem_new(LazySetActiveModelMorphCommand(eye.activeMorph(eyeIndex), eye.category())));
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (lip.combo(&lipIndex)) {
            addLazyExecutionCommand(
                nanoem_new(LazySetActiveModelMorphCommand(lip.activeMorph(lipIndex), lip.category())));
        }
        ImGui::PopItemWidth();
    }
    {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        const nanoem_model_morph_t *eyeMorphPtr = activeModel->activeMorph(eye.category());
        model::Morph *eyeMorph = model::Morph::cast(eyeMorphPtr);
        nanoem_f32_t eyeMorphWeight = eyeMorph ? eyeMorph->weight() : 0;
        if (eye.slider(&eyeMorphWeight, activeModel)) {
            setModelMorphWeight(eyeMorphWeight, eyeMorphPtr, activeModel, project);
        }
        handleDraggingMorphSliderState();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        const nanoem_model_morph_t *lipMorphPtr = activeModel->activeMorph(lip.category());
        model::Morph *lipMorph = model::Morph::cast(lipMorphPtr);
        nanoem_f32_t lipMorphWeight = lipMorph ? lipMorph->weight() : 0;
        if (lip.slider(&lipMorphWeight, activeModel)) {
            setModelMorphWeight(lipMorphWeight, lipMorphPtr, activeModel, project);
        }
        handleDraggingMorphSliderState();
        ImGui::PopItemWidth();
    }
    {
        if (handleTranslatedButton("nanoem.gui.panel.morph.eye.register", ImGui::GetContentRegionAvail().x * 0.5f,
                eye.canRegister(activeModel))) {
            eye.handleRegisterButton(activeModel, project);
        }
        ImGui::SameLine();
        if (handleTranslatedButton("nanoem.gui.panel.morph.lip.register", ImGui::GetContentRegionAvail().x,
                lip.canRegister(activeModel))) {
            lip.handleRegisterButton(activeModel, project);
        }
    }
    ImGui::Spacing();
    MorphSelector eyebrow(activeModel, translator, NANOEM_MODEL_MORPH_CATEGORY_EYEBROW),
        other(activeModel, translator, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    {
        int eyebrowIndex = eyebrow.index(), otherIndex = other.index();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (eyebrow.combo(&eyebrowIndex)) {
            addLazyExecutionCommand(
                nanoem_new(LazySetActiveModelMorphCommand(eyebrow.activeMorph(eyebrowIndex), eyebrow.category())));
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (other.combo(&otherIndex)) {
            addLazyExecutionCommand(
                nanoem_new(LazySetActiveModelMorphCommand(other.activeMorph(otherIndex), other.category())));
        }
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    }
    {
        const nanoem_model_morph_t *eyebrowMorphPtr = activeModel->activeMorph(eyebrow.category());
        model::Morph *eyebrowMorph = model::Morph::cast(eyebrowMorphPtr);
        nanoem_f32_t eyebrowMorphWeight = eyebrowMorph ? eyebrowMorph->weight() : 0;
        if (eyebrow.slider(&eyebrowMorphWeight, activeModel)) {
            setModelMorphWeight(eyebrowMorphWeight, eyebrowMorphPtr, activeModel, project);
        }
        handleDraggingMorphSliderState();
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        const nanoem_model_morph_t *otherMorphPtr = activeModel->activeMorph(other.category());
        model::Morph *otherMorph = model::Morph::cast(otherMorphPtr);
        nanoem_f32_t otherMorphWeight = otherMorph ? otherMorph->weight() : 0;
        if (other.slider(&otherMorphWeight, activeModel)) {
            setModelMorphWeight(otherMorphWeight, otherMorphPtr, activeModel, project);
        }
        handleDraggingMorphSliderState();
        ImGui::PopItemWidth();
    }
    {
        if (handleTranslatedButton("nanoem.gui.panel.morph.eyebrow.register", ImGui::GetContentRegionAvail().x * 0.5f,
                eyebrow.canRegister(activeModel))) {
            eyebrow.handleRegisterButton(activeModel, project);
        }
        ImGui::SameLine();
        if (handleTranslatedButton("nanoem.gui.panel.morph.other.register", ImGui::GetContentRegionAvail().x,
                other.canRegister(activeModel))) {
            other.handleRegisterButton(activeModel, project);
        }
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawViewPanel(const ImVec2 &panelSize, Project *project)
{
    ImGui::BeginChild("view", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.view.title"));
    ImGui::Separator();
    ImGui::Spacing();
    ICamera *camera = project->activeCamera();
    bool buttonEnabled = !project->isPlaying();
    if (handleTranslatedButton("nanoem.gui.panel.view.front", ImGui::GetContentRegionAvail().x * 0.5f, buttonEnabled)) {
        setCameraAngle(Vector3(0), camera, project);
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.panel.view.back", ImGui::GetContentRegionAvail().x, buttonEnabled)) {
        setCameraAngle(Vector3(0, 180, 0), camera, project);
    }
    if (handleTranslatedButton("nanoem.gui.panel.view.up", ImGui::GetContentRegionAvail().x * 0.5f, buttonEnabled)) {
        setCameraAngle(Vector3(90, 0, 0), camera, project);
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.panel.view.left", ImGui::GetContentRegionAvail().x, buttonEnabled)) {
        setCameraAngle(Vector3(0, -90, 0), camera, project);
    }
    if (handleTranslatedButton("nanoem.gui.panel.view.right", ImGui::GetContentRegionAvail().x * 0.5f, buttonEnabled)) {
        setCameraAngle(Vector3(0, 90, 0), camera, project);
    }
    ImGui::SameLine();
    if (handleTranslatedButton("nanoem.gui.panel.view.bottom", ImGui::GetContentRegionAvail().x, buttonEnabled)) {
        setCameraAngle(Vector3(-90, 0, 0), camera, project);
    }
    const ICamera::FollowingType followingType = camera->followingType();
    bool followingModel = followingType == ICamera::kFollowingTypeModel;
    if (handleCheckBox(tr("nanoem.gui.panel.view.follow.model"), &followingModel, buttonEnabled)) {
        camera->setFollowingType(followingModel ? ICamera::kFollowingTypeModel : ICamera::kFollowingTypeNone);
    }
    bool followingBone = followingType == ICamera::kFollowingTypeBone;
    if (handleCheckBox(tr("nanoem.gui.panel.view.follow.bone"), &followingBone, buttonEnabled)) {
        camera->setFollowingType(followingBone ? ICamera::kFollowingTypeBone : ICamera::kFollowingTypeNone);
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawPlayPanel(const ImVec2 &panelSize, Project *project)
{
    ImGui::BeginChild("play", panelSize, true);
    ImGui::TextUnformatted(tr("nanoem.gui.panel.play.title"));
    ImGui::Separator();
    ImGui::Spacing();
    const bool isPlaying = project->isPlaying();
    const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
    if (isPlaying && handleTranslatedButton("nanoem.gui.panel.play.pause", -1, true)) {
        project->pause(true);
    }
    else if (!isPlaying && frameIndex > 0 && handleTranslatedButton("nanoem.gui.panel.play.resume", -1, true)) {
        project->resume(true);
    }
    else if (!isPlaying && frameIndex == 0 && handleTranslatedButton("nanoem.gui.panel.play.play", -1, true)) {
        project->play();
    }
    bool loop = project->isLoopEnabled();
    if (handleCheckBox(tr("nanoem.gui.panel.play.loop"), &loop, true)) {
        project->setLoopEnabled(loop);
    }
    ImGui::PushItemWidth(-1);
    IAudioPlayer *player = project->audioPlayer();
    int volume = static_cast<int>(player->volumeGain() * 100);
    if (ImGui::SliderInt("##play.volume", &volume, 0, 100, tr("nanoem.gui.panel.play.volume.format"))) {
        player->setVolumeGain(volume * 0.01f);
    }
    ImGui::PopItemWidth();
    TimelineSegment segment(project->playingSegment());
    if (handleCheckBox("##play.start.enabled", &segment.m_enableFrom, !isPlaying)) {
        project->setPlayingSegment(segment);
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    const nanoem_frame_index_t duration = project->duration();
    if (handleDragScalarN("##play.start.value", kFrameIndexDataType, &segment.m_from, 1, !isPlaying, 1.0f, nullptr,
            &segment.m_to, "From: %u", ImGuiSliderFlags_None)) {
        project->setPlayingSegment(segment);
    }
    ImGui::PopItemWidth();
    if (handleCheckBox("##play.end.enabled", &segment.m_enableTo, !isPlaying)) {
        project->setPlayingSegment(segment);
    }
    ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (handleDragScalarN("##play.end.value", kFrameIndexDataType, &segment.m_to, 1, !isPlaying, 1.0f, &segment.m_from,
            &duration, "To: %u", ImGuiSliderFlags_None)) {
        project->setPlayingSegment(segment);
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void
ImGuiWindow::drawModelEditPanel(Project *project, nanoem_f32_t height)
{
    char buffer[Inline::kLongNameStackBufferSize];
    Model *activeModel = project->activeModel();
    const Model::EditActionType editActionType = activeModel->editActionType();
    ImGui::BeginChild("command", ImVec2(kModelEditCommandWidth * project->windowDevicePixelRatio(), height));
    StringUtils::format(buffer, sizeof(buffer), "%s##operation", tr("nanoem.gui.panel.model.edit.operation.title"));
    IModelObjectSelection *selection = activeModel->selection();
    const IModelObjectSelection::ObjectType objectType = selection->objectType();
    if (ImGui::BeginMenu(tr("nanoem.gui.panel.model.edit.operation.action.title"))) {
        if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.camera.title"), nullptr,
                editActionType == Model::kEditActionTypeNone)) {
            ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
            selection->setObjectType(IModelObjectSelection::kObjectTypeNull);
            activeModel->setEditActionType(Model::kEditActionTypeNone);
        }
        if (ImGui::BeginMenu(tr("nanoem.gui.panel.model.edit.operation.action.selection.title"))) {
            bool isSelection = editActionType == Model::kEditActionTypeSelectModelObject;
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.vertex"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeVertex)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeVertex, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.face"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeFace)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeFace, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.material"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeMaterial)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeMaterial, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.bone"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeBone)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeBone, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.rigid-body"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeRigidBody)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeRigidBody, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.joint"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeJoint)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeJoint, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            if (ImGui::MenuItem(tr("nanoem.gui.window.model.menu.soft-body"), nullptr,
                    isSelection && objectType == IModelObjectSelection::kObjectTypeSoftBody)) {
                ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
                ModelEditCommandDialog::afterToggleEditingMode(
                    IModelObjectSelection::kObjectTypeSoftBody, activeModel, project);
                activeModel->setEditActionType(Model::kEditActionTypeSelectModelObject);
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.create-parent-bone.title"), nullptr,
                editActionType == Model::kEditActionTypeCreateParentBone)) {
            ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
            ModelEditCommandDialog::afterToggleEditingMode(
                IModelObjectSelection::kObjectTypeBone, activeModel, project);
            activeModel->setEditActionType(Model::kEditActionTypeCreateParentBone);
        }
        if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.create-target-bone.title"), nullptr,
                editActionType == Model::kEditActionTypeCreateTargetBone)) {
            ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
            ModelEditCommandDialog::afterToggleEditingMode(
                IModelObjectSelection::kObjectTypeBone, activeModel, project);
            activeModel->setEditActionType(Model::kEditActionTypeCreateTargetBone);
        }
        if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.paint-vertex-weight.title"), nullptr,
                editActionType == Model::kEditActionTypePaintVertexWeight)) {
            ModelEditCommandDialog::beforeToggleEditingMode(objectType, activeModel, project);
            ModelEditCommandDialog::afterToggleEditingMode(
                IModelObjectSelection::kObjectTypeVertex, activeModel, project);
            activeModel->setEditActionType(Model::kEditActionTypePaintVertexWeight);
            activeModel->setShowAllVertexWeights(true);
            if (!activeModel->vertexWeightPainter()) {
                activeModel->setVertexWeightPainter(nanoem_new(VertexWeightPainter(activeModel)));
            }
        }
        ImGui::Separator();
        if (ImGui::BeginMenu(tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.title"))) {
            if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.cone")) &&
                m_dialogWindows.find(CreateConePrimitiveDialog::kIdentifier) == m_dialogWindows.end()) {
                INoModalDialogWindow *dialog = nanoem_new(CreateConePrimitiveDialog(m_applicationPtr));
                m_dialogWindows.insert(tinystl::make_pair(CreateConePrimitiveDialog::kIdentifier, dialog));
            }
            if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.cube")) &&
                m_dialogWindows.find(CreateCubePrimitiveDialog::kIdentifier) == m_dialogWindows.end()) {
                INoModalDialogWindow *dialog = nanoem_new(CreateCubePrimitiveDialog(m_applicationPtr));
                m_dialogWindows.insert(tinystl::make_pair(CreateCubePrimitiveDialog::kIdentifier, dialog));
            }
            if (ImGui::MenuItem(
                    tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.cylinder")) &&
                m_dialogWindows.find(CreateCylinderPrimitiveDialog::kIdentifier) == m_dialogWindows.end()) {
                INoModalDialogWindow *dialog = nanoem_new(CreateCylinderPrimitiveDialog(m_applicationPtr));
                m_dialogWindows.insert(tinystl::make_pair(CreateCylinderPrimitiveDialog::kIdentifier, dialog));
            }
            if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.sphere")) &&
                m_dialogWindows.find(CreateSpherePrimitiveDialog::kIdentifier) == m_dialogWindows.end()) {
                INoModalDialogWindow *dialog = nanoem_new(CreateSpherePrimitiveDialog(m_applicationPtr));
                m_dialogWindows.insert(tinystl::make_pair(CreateSpherePrimitiveDialog::kIdentifier, dialog));
            }
            if (ImGui::MenuItem(tr("nanoem.gui.panel.model.edit.operation.action.create-material-primitive.torus")) &&
                m_dialogWindows.find(CreateTorusPrimitiveDialog::kIdentifier) == m_dialogWindows.end()) {
                INoModalDialogWindow *dialog = nanoem_new(CreateTorusPrimitiveDialog(m_applicationPtr));
                m_dialogWindows.insert(tinystl::make_pair(CreateTorusPrimitiveDialog::kIdentifier, dialog));
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }
    const bool isSelectionMode = objectType >= IModelObjectSelection::kObjectTypeVertex &&
        objectType < IModelObjectSelection::kObjectTypeMaxEnum;
    if (isSelectionMode &&
        ImGui::CollapsingHeader(
            tr("nanoem.gui.panel.model.edit.operation.selection.title"), ImGuiTreeNodeFlags_DefaultOpen)) {
        const IModelObjectSelection::TargetModeType targetMode = selection->targetMode();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.operation.selection.circle"),
                targetMode == IModelObjectSelection::kTargetModeTypeCircle, isSelectionMode)) {
            selection->setTargetMode(IModelObjectSelection::kTargetModeTypeCircle);
        }
        ImGui::SameLine();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.operation.selection.rectangle"),
                targetMode == IModelObjectSelection::kTargetModeTypeRectangle, isSelectionMode)) {
            selection->setTargetMode(IModelObjectSelection::kTargetModeTypeRectangle);
        }
        ImGui::SameLine();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.operation.selection.point"),
                targetMode == IModelObjectSelection::kTargetModeTypePoint, isSelectionMode)) {
            selection->setTargetMode(IModelObjectSelection::kTargetModeTypePoint);
        }
        ImGui::Spacing();
    }
    model::IGizmo *gizmo = activeModel->gizmo();
    const bool isGizmoEnabled = gizmo ? !glm::isNull(gizmo->pivotMatrix(), Constants::kEpsilon) : false;
    StringUtils::format(buffer, sizeof(buffer), "%s##gizmo", tr("nanoem.gui.panel.model.edit.gizmo.title"));
    if (isGizmoEnabled && ImGui::CollapsingHeader(buffer, ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextUnformatted(tr("nanoem.gui.panel.model.edit.gizmo.operation.title"));
        model::IGizmo::OperationType op = gizmo->operationType();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.gizmo.operation.translate"),
                op == model::IGizmo::kOperationTypeTranslate, isGizmoEnabled)) {
            gizmo->setOperationType(model::IGizmo::kOperationTypeTranslate);
        }
        ImGui::SameLine();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.gizmo.operation.rotate"),
                op == model::IGizmo::kOperationTypeRotate, isGizmoEnabled)) {
            gizmo->setOperationType(model::IGizmo::kOperationTypeRotate);
        }
        ImGui::SameLine();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.gizmo.operation.scale"),
                op == model::IGizmo::kOperationTypeScale, isGizmoEnabled)) {
            gizmo->setOperationType(model::IGizmo::kOperationTypeScale);
        }
        ImGui::Separator();
        ImGui::TextUnformatted(tr("nanoem.gui.panel.model.edit.gizmo.coordinate.title"));
        model::IGizmo::TransformCoordinateType coord = gizmo->transformCoordinateType();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.gizmo.coordinate.global"),
                coord == model::IGizmo::kTransformCoordinateTypeGlobal, isGizmoEnabled)) {
            gizmo->setTransformCoordinateType(model::IGizmo::kTransformCoordinateTypeGlobal);
        }
        ImGui::SameLine();
        if (handleRadioButton(tr("nanoem.gui.panel.model.edit.gizmo.coordinate.local"),
                coord == model::IGizmo::kTransformCoordinateTypeLocal, isGizmoEnabled)) {
            gizmo->setTransformCoordinateType(model::IGizmo::kTransformCoordinateTypeLocal);
        }
        ImGui::Spacing();
    }
    const bool paintMode = editActionType == Model::kEditActionTypePaintVertexWeight;
    StringUtils::format(
        buffer, sizeof(buffer), "%s##vertex-weight-paint", tr("nanoem.gui.panel.model.edit.weight-paint.title"));
    if (paintMode && ImGui::CollapsingHeader(buffer, ImGuiTreeNodeFlags_DefaultOpen)) {
        const ITranslator *translator = project->translator();
        ImGui::PushItemWidth(-1);
        model::IVertexWeightPainter *painter = activeModel->vertexWeightPainter();
        model::IVertexWeightPainter::Type painterType = painter->type();
        ImGui::TextUnformatted(tr("nanoem.gui.panel.model.edit.weight-paint.painter-type"));
        if (ImGui::BeginCombo(
                "##painter-type", VertexWeightPainterUtils::selectedVertexWeightPainterType(painterType, translator))) {
            for (int i = model::IVertexWeightPainter::kTypeFirstEnum; i < model::IVertexWeightPainter::kTypeMaxEnum;
                 i++) {
                const model::IVertexWeightPainter::Type type = static_cast<model::IVertexWeightPainter::Type>(i);
                if (ImGui::Selectable(VertexWeightPainterUtils::selectedVertexWeightPainterType(type, translator),
                        type == painterType)) {
                    painter->setType(type);
                }
            }
            ImGui::EndCombo();
        }
        switch (painter->type()) {
        case model::IVertexWeightPainter::kTypeBaseBrush: {
            nanoem_model_vertex_type_t vertexType = painter->vertexType();
            ImGui::TextUnformatted(tr("nanoem.gui.panel.model.edit.weight-paint.vertex-type"));
            if (ImGui::BeginCombo("##vertex-type", VertexWeightPainterUtils::selectedVertexType(vertexType))) {
                for (int i = NANOEM_MODEL_VERTEX_TYPE_FIRST_ENUM; i < NANOEM_MODEL_VERTEX_TYPE_MAX_ENUM; i++) {
                    const nanoem_model_vertex_type_t type = static_cast<nanoem_model_vertex_type_t>(i);
                    if (ImGui::Selectable(VertexWeightPainterUtils::selectedVertexType(type), type == vertexType)) {
                        painter->setVertexType(type);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted(tr("nanoem.gui.panel.model.edit.weight-paint.bone"));
            switch (vertexType) {
            case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
                VertexWeightPainterUtils::layoutSelectBoneComboBox(activeModel, 0, painter);
                break;
            }
            case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
            case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
                for (nanoem_rsize_t i = 0; i < 2; i++) {
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                    VertexWeightPainterUtils::layoutSelectBoneComboBox(activeModel, i, painter);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    nanoem_f32_t weight = painter->vertexWeight(i);
                    StringUtils::format(buffer, sizeof(buffer), "##weights[%jd]", i);
                    if (ImGui::SliderFloat(buffer, &weight, 0.0f, 1.0f, "Weight: %.2f")) {
                        painter->setVertexWeight(weight, i);
                    }
                }
                if (ImGui::Button(tr("nanoem.gui.panel.model.edit.weight-paint.normalize"))) {
                    nanoem_f32_t weight = glm::clamp(painter->vertexWeight(0), 0.0f, 1.0f);
                    painter->setVertexWeight(weight, 0);
                    painter->setVertexWeight(1.0f - weight, 1);
                }
                break;
            }
            case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
            case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
                for (nanoem_rsize_t i = 0; i < 4; i++) {
                    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                    VertexWeightPainterUtils::layoutSelectBoneComboBox(activeModel, i, painter);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    nanoem_f32_t weight = painter->vertexWeight(i);
                    StringUtils::format(buffer, sizeof(buffer), "##weights[%jd]", i);
                    if (ImGui::SliderFloat(buffer, &weight, 0.0f, 1.0f, "Weight: %.2f")) {
                        painter->setVertexWeight(weight, i);
                    }
                }
                if (ImGui::Button(tr("nanoem.gui.panel.model.edit.weight-paint.normalize"))) {
                    nanoem_f32_t sum = 0.0f;
                    for (nanoem_rsize_t i = 0; i < 4; i++) {
                        if (painter->vertexBone(i)) {
                            sum += painter->vertexWeight(i);
                        }
                    }
                    for (nanoem_rsize_t i = 0; i < 4; i++) {
                        nanoem_f32_t weight = 0;
                        if (painter->vertexBone(i)) {
                            weight = painter->vertexWeight(i) / sum;
                        }
                        painter->setVertexWeight(weight, i);
                    }
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        case model::IVertexWeightPainter::kTypeAirBrush: {
            ImGui::TextUnformatted(tr("nanoem.gui.panel.model.edit.weight-paint.bone"));
            VertexWeightPainterUtils::layoutSelectBoneComboBox(activeModel, 0, painter);
            nanoem_f32_t radius = painter->radius(), delta = painter->delta();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
            if (ImGui::DragFloat(
                    "##radius", &radius, 0.5f, 0.5f, 100.0f, tr("nanoem.gui.panel.model.edit.weight-paint.radius"))) {
                painter->setRadius(radius);
            }
            ImGui::SameLine();
            if (ImGui::DragFloat(
                    "##delta", &delta, 0.005f, -1.0f, 1.0f, tr("nanoem.gui.panel.model.edit.weight-paint.delta"))) {
                painter->setDelta(delta);
            }
            bool automaticNormalization = painter->isAutomaticNormalizationEnabled();
            StringUtils::format(buffer, sizeof(buffer), "%s##automatic-normalization",
                tr("nanoem.gui.panel.model.edit.weight-paint.automatic-normalization"));
            if (ImGui::Checkbox(buffer, &automaticNormalization)) {
                painter->setAutomaticNormalizationEnabled(automaticNormalization);
            }
            break;
        }
        default:
            break;
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
    }
    StringUtils::format(buffer, sizeof(buffer), "%s##camera", tr("nanoem.gui.panel.camera.title"));
    if (ImGui::CollapsingHeader(buffer)) {
        ICamera *camera = project->activeCamera();
        Vector3 lookAt(camera->lookAt()), angle(glm::degrees(camera->angle()));
        nanoem_f32_t distance = camera->distance();
        int fov = camera->fov();
        bool perspective = camera->isPerspective(), changed = false;
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.look-at"));
        if (ImGui::DragFloat3("##look-at", glm::value_ptr(lookAt))) {
            camera->setLookAt(glm::radians(lookAt));
            changed = true;
        }
        ImGui::TextUnformatted(tr("nanoem.gui.viewport.parameter.camera.angle"));
        if (ImGui::DragFloat3("##angle", glm::value_ptr(angle))) {
            camera->setAngle(glm::radians(angle));
            changed = true;
        }
        StringUtils::format(buffer, sizeof(buffer), "%s: %%.1f", tr("nanoem.gui.viewport.parameter.camera.distance"));
        if (ImGui::DragFloat("##distance", &distance, 0.1f, 0.0f, 0.0f, buffer)) {
            camera->setDistance(distance);
            changed = true;
        }
        if (ImGui::DragInt("##fov", &fov, 0.1f, 1, 135, tr("nanoem.gui.panel.camera.fov.format"))) {
            camera->setFov(fov);
            changed = true;
        }
        if (ImGui::Checkbox("Perspective##perspective", &perspective)) {
            camera->setPerspective(perspective);
            changed = true;
        }
        if (ImGui::Button("Initialize##initialize", ImVec2(-1, 0))) {
            camera->reset();
            changed = true;
        }
        ImGui::PopItemWidth();
        if (changed) {
            camera->update();
            project->resetAllModelEdges();
        }
        ImGui::Spacing();
    }
    StringUtils::format(buffer, sizeof(buffer), "%s##light", tr("nanoem.gui.panel.light.title"));
    if (ImGui::CollapsingHeader(buffer)) {
        ILight *light = project->activeLight();
        Vector3 color(light->color()), direction(light->direction());
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.panel.light.color"));
        if (ImGui::ColorEdit3("##color", glm::value_ptr(color))) {
            light->setColor(color);
        }
        ImGui::TextUnformatted(tr("nanoem.gui.panel.light.direction"));
        if (ImGui::DragFloat3("##direction", glm::value_ptr(direction), 0.01f, -1.0f, 1.0f)) {
            light->setDirection(direction);
        }
        StringUtils::format(buffer, sizeof(buffer), "%s##initialize", tr("nanoem.gui.panel.light.reset"));
        if (ImGui::Button(buffer, ImVec2(-1, 0))) {
            light->reset();
        }
        ImGui::PopItemWidth();
    }
    ImGui::EndChild();
}

void
ImGuiWindow::drawAllNonModalWindows(Project *project)
{
    if (!m_dialogWindows.empty()) {
        typedef tinystl::vector<const char *, TinySTLAllocator> StringConstList;
        StringConstList deletingWindows;
        saveDefaultStyle(project->windowDevicePixelRatio());
        for (NoModalDialogWindowList::const_iterator it = m_dialogWindows.begin(), end = m_dialogWindows.end();
             it != end; ++it) {
            INoModalDialogWindow *window = it->second;
            if (!window->draw(project)) {
                deletingWindows.push_back(it->first);
            }
        }
        if (!deletingWindows.empty()) {
            for (StringConstList::const_iterator it = deletingWindows.begin(), end = deletingWindows.end(); it != end;
                 ++it) {
                NoModalDialogWindowList::const_iterator it2 = m_dialogWindows.find(*it);
                if (it2 != m_dialogWindows.end()) {
                    INoModalDialogWindow *window = it2->second;
                    window->destroy(project);
                    m_dialogWindows.erase(it2);
                    nanoem_delete(window);
                }
            }
        }
        restoreDefaultStyle();
    }
}

void
ImGuiWindow::drawTransformHandleSet(const Vector4UI16 *rects, const ImVec2 &offset, const nanoem_u8_t *icon,
    int baseRectType, int intercectedRectType, bool handleable)
{
    const Vector4 x(rects[baseRectType + 0]), y(rects[baseRectType + 1]), z(rects[baseRectType + 2]);
    const bool intersectsX = baseRectType == intercectedRectType;
    m_primitive2D.fillCircle(x, handleable ? (intersectsX ? kColorRed : kColorTranslucentRed) : kColorGray);
    const bool intersectsY = baseRectType + 1 == intercectedRectType;
    m_primitive2D.fillCircle(y, handleable ? (intersectsY ? kColorGreen : kColorTranslucentGreen) : kColorGray);
    const bool intersectsZ = baseRectType + 2 == intercectedRectType;
    m_primitive2D.fillCircle(z, handleable ? (intersectsZ ? kColorBlue : kColorTranslucentBlue) : kColorGray);
    const char *p = reinterpret_cast<const char *>(icon);
    size_t length = StringUtils::length(p);
    drawTextCentered(offset, x, p, length);
    drawTextCentered(offset, y, p, length);
    drawTextCentered(offset, z, p, length);
}

void
ImGuiWindow::drawPrimitive2D(Project *project, IState *state, nanoem_u32_t flags)
{
    if (state) {
        state->onDrawPrimitive2D(&m_primitive2D);
    }
    Model *activeModel = project->activeModel();
    if (activeModel && activeModel->isVisible()) {
        const Vector2SI32 deviceScaleCursor(project->deviceScaleMovingCursorPosition());
        Vector4 activeBoneColor(DrawUtils::kColorRed, 1);
        NoModalDialogWindowList::const_iterator it = m_dialogWindows.find(ModelParameterDialog::kIdentifier);
        const ModelParameterDialog *dialog = nullptr;
        if (it != m_dialogWindows.end()) {
            dialog = static_cast<ModelParameterDialog *>(it->second);
        }
        appendDrawFlags(activeModel, dialog, flags);
        if (EnumUtils::isEnabled(IState::kDrawTypeAllBoneConnections, flags)) {
            activeModel->drawAllBoneConnections(&m_primitive2D, deviceScaleCursor);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeConstraintConnections, flags)) {
            activeModel->drawConstraintConnections(&m_primitive2D, deviceScaleCursor);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeConstraintHeatmaps, flags)) {
            activeModel->drawConstraintsHeatMap(&m_primitive2D);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeBoneMoveHandle, flags)) {
            activeBoneColor = Vector4(DrawUtils::kColorYellow, 1);
            const bool isGrabbingHandle = DrawUtils::isDraggingBoneState(state) && state->isGrabbingHandle();
            DrawUtils::drawBoneMoveHandle(&m_primitive2D, activeModel, isGrabbingHandle);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeActiveBoneConnection, flags)) {
            const nanoem_model_bone_t *bonePtr = activeModel->activeBone();
            activeModel->drawBoneConnections(&m_primitive2D, bonePtr);
            activeModel->drawBonePoint(&m_primitive2D, bonePtr, deviceScaleCursor);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeBoneRotateHandle, flags)) {
            activeBoneColor = Vector4(DrawUtils::kColorYellow, 1);
            const bool isGrabbingHandle = DrawUtils::isDraggingBoneState(state) && state->isGrabbingHandle();
            DrawUtils::drawBoneRotateHandle(&m_primitive2D, activeModel, isGrabbingHandle);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeActiveBone, flags)) {
            DrawUtils::drawActiveBonePoint(&m_primitive2D, activeBoneColor, project);
        }
        if (EnumUtils::isEnabled(IState::kDrawTypeVertexWeightPainter, flags)) {
            const model::IVertexWeightPainter *painter = activeModel->vertexWeightPainter();
            DrawUtils::drawVertexWeightPainter(&m_primitive2D, painter, project);
        }
    }
    const ICamera *camera = project->activeCamera();
    if (EnumUtils::isEnabled(IState::kDrawTypeCameraLookAt, flags) && camera) {
        DrawUtils::drawCameraLookAtPoint(&m_primitive2D, camera, project);
    }
}

void
ImGuiWindow::drawTransformHandleSet(const Project *project, IState *state, const ImVec2 &offset)
{
    nanoem_parameter_assert(project, "must not be nullptr");
    if (project->isTransformHandleVisible()) {
        Vector4UI16 deviceScaleRects[Project::kRectangleTypeMaxEnum];
        const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
        for (int i = Project::kRectangleTypeFirstEnum; i < Project::kRectangleTypeMaxEnum; i++) {
            deviceScaleRects[i] =
                project->queryDevicePixelRectangle(static_cast<Project::RectangleType>(i), Vector2UI16());
        }
        Project::RectangleType intersectedRectangleType = Project::kRectangleTypeMaxEnum;
        const ImGuiIO &io = ImGui::GetIO();
        ImVec2 mousePos(io.MousePos), basePos;
#if defined(IMGUI_HAS_VIEWPORT)
        basePos = ImGui::GetMainViewport()->Pos;
#endif /* IMGUI_HAS_VIEWPORT */
        mousePos.x -= basePos.x;
        mousePos.y -= basePos.y;
        const ImVec2 scale(io.DisplayFramebufferScale), invertedScale(1.0f / scale.x, 1.0f / scale.y);
        const bool intersected = project->intersectsTransformHandle(
            Vector2(mousePos.x * invertedScale.x, mousePos.y * invertedScale.y), intersectedRectangleType);
        if (const Model *activeModel = project->activeModel()) {
            const IModelObjectSelection *selection = activeModel->selection();
            drawOrientationAxes(intersected, intersectedRectangleType, selection, activeModel, project, state);
            const bool movable = selection->areAllBonesMovable(), rotateable = selection->areAllBonesRotateable();
            drawTransformHandleSet(deviceScaleRects, offset, kFARefresh, Project::kRectangleOrientateX,
                intersectedRectangleType, rotateable);
            drawTransformHandleSet(
                deviceScaleRects, offset, kFAArrows, Project::kRectangleTranslateX, intersectedRectangleType, movable);
            const Vector4 rect(deviceScaleRects[Project::kRectangleTransformCoordinateType]);
            internalFillRect(rect, deviceScaleRatio);
            switch (activeModel->transformCoordinateType()) {
            case Model::kTransformCoordinateTypeLocal: {
                static const char kLocal[] = "Local";
                drawTextCentered(offset, rect, kLocal, sizeof(kLocal) - 1);
                break;
            }
            case Model::kTransformCoordinateTypeGlobal: {
                static const char kGlobal[] = "Global";
                drawTextCentered(offset, rect, kGlobal, sizeof(kGlobal) - 1);
                break;
            }
            default: {
                static const char kUnkwown[] = "(Unknown)";
                drawTextCentered(offset, rect, kUnkwown, sizeof(kUnkwown) - 1);
                break;
            }
            }
        }
        else if (const ICamera *activeCamera = project->activeCamera()) {
            drawTransformHandleSet(
                deviceScaleRects, offset, kFARefresh, Project::kRectangleOrientateX, intersectedRectangleType, true);
            drawTransformHandleSet(
                deviceScaleRects, offset, kFAArrows, Project::kRectangleTranslateX, intersectedRectangleType, true);
            const Vector4 rect(deviceScaleRects[Project::kRectangleTransformCoordinateType]);
            internalFillRect(rect, deviceScaleRatio);
            switch (activeCamera->transformCoordinateType()) {
            case ICamera::kTransformCoordinateTypeLocal: {
                static const char kLocal[] = "Local";
                drawTextCentered(offset, rect, kLocal, sizeof(kLocal) - 1);
                break;
            }
            case ICamera::kTransformCoordinateTypeGlobal: {
                static const char kGlobal[] = "Global";
                drawTextCentered(offset, rect, kGlobal, sizeof(kGlobal) - 1);
                break;
            }
            default: {
                static const char kUnkwown[] = "(Unknown)";
                drawTextCentered(offset, rect, kUnkwown, sizeof(kUnkwown) - 1);
                break;
            }
            }
        }
        {
            const Vector4 rect(project->queryDevicePixelRectangle(Project::kRectangleCameraLookAt, Vector2UI16()));
            internalFillRect(rect, deviceScaleRatio);
            drawTextCentered(offset, rect, reinterpret_cast<const char *>(kFAArrows), sizeof(kFAArrows) - 1);
        }
        {
            const Vector4 rect(project->queryDevicePixelRectangle(Project::kRectangleCameraZoom, Vector2UI16()));
            internalFillRect(rect, deviceScaleRatio);
            drawTextCentered(offset, rect, reinterpret_cast<const char *>(kFAZoom), sizeof(kFAZoom) - 1);
        }
    }
}

void
ImGuiWindow::drawOrientationAxes(bool intersected, nanoem_u32_t rectangleType, const IModelObjectSelection *selection,
    const Model *activeModel, const Project *project, IState *state)
{
    const ImGuiIO &io = ImGui::GetIO();
    const ImVec2 scale(io.DisplayFramebufferScale), invertedScale(1.0f / scale.x, 1.0f / scale.y);
    const nanoem_model_bone_t *activeBonePtr = activeModel->activeBone();
    const model::Bone *activeBone = model::Bone::cast(activeBonePtr);
    bool showXYZAxes = activeBone && !nanoemModelBoneHasFixedAxis(activeBonePtr) &&
        ((intersected && isSelectingBoneHandle(selection, static_cast<Project::RectangleType>(rectangleType))) ||
            isDraggingBoneAxisAlignedState(state));
    NoModalDialogWindowList::const_iterator it = m_dialogWindows.find(ModelParameterDialog::kIdentifier);
    if (it != m_dialogWindows.end()) {
        const ModelParameterDialog *dialog = static_cast<ModelParameterDialog *>(it->second);
        showXYZAxes |= dialog->isLocalAxesShown();
    }
    if (showXYZAxes) {
        Quaternion orientation(Constants::kZeroQ);
        if (activeModel->transformCoordinateType() == Model::kTransformCoordinateTypeLocal) {
            orientation = activeBone->localOrientation() * glm::quat_cast(model::Bone::localAxes(activeBonePtr));
        }
        const Vector3 base(activeBone->worldTransformOrigin()), scaleFactor(3),
            unitX(orientation * Constants::kUnitX * scaleFactor), unitY(orientation * Constants::kUnitY * scaleFactor),
            unitZ(orientation * Constants::kUnitZ * -scaleFactor);
        const ICamera *camera = project->activeCamera();
        const Vector2SI32 p(camera->toDeviceScreenCoordinateInViewport(base)),
            x(camera->toDeviceScreenCoordinateInViewport(base + unitX)),
            y(camera->toDeviceScreenCoordinateInViewport(base + unitY)),
            z(camera->toDeviceScreenCoordinateInViewport(base + unitZ)),
            windowSize(Vector2(project->windowSize()) * project->windowDevicePixelRatio());
        const Vector4SI32 rect(-windowSize, windowSize * 2);
        if (Inline::intersectsRectPoint(rect, p)) {
            const nanoem_f32_t thickness = 3.0f;
            if (Inline::intersectsRectPoint(rect, x)) {
                m_primitive2D.strokeLine(p, x, kColorRed, thickness);
            }
            if (Inline::intersectsRectPoint(rect, y)) {
                m_primitive2D.strokeLine(p, y, kColorGreen, thickness);
            }
            if (Inline::intersectsRectPoint(rect, z)) {
                m_primitive2D.strokeLine(p, z, kColorBlue, thickness);
            }
        }
    }
}

void
ImGuiWindow::drawFPSCounter(const Project *project, const ImVec2 &offset)
{
    char textBuffer[16];
    nanoem_u32_t actualFPS = project->actualFPS();
    if (actualFPS > 0 && !project->isPowerSavingEnabled()) {
        StringUtils::format(textBuffer, sizeof(textBuffer), "%d FPS", actualFPS);
    }
    else {
        bx::strCopy(textBuffer, sizeof(textBuffer), "-- FPS");
    }
    const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
    const Vector4 rect(project->queryDevicePixelRectangle(Project::kRectangleActualFPS, Vector2UI16()));
    internalFillRect(rect, deviceScaleRatio);
    drawTextCentered(offset, rect, textBuffer, Inline::saturateInt32(StringUtils::length(textBuffer)));
}

void
ImGuiWindow::drawPerformanceMonitor(const Project *project, const ImVec2 &offset)
{
    static const nanoem_f32_t kSpacingSize = 10, kMarginSize = 5;
    const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
    char memoryBytesInString[32], usageCPUBuffer[128], usageMemoryBuffer[128];
    bx::prettify(memoryBytesInString, sizeof(memoryBytesInString), m_currentMemoryBytes, bx::Units::Kilo);
    StringUtils::format(usageCPUBuffer, sizeof(usageCPUBuffer), "CPU: %.2f%%", m_currentCPUPercentage);
    StringUtils::format(usageMemoryBuffer, sizeof(usageCPUBuffer), "MEM: %s", memoryBytesInString);
    const nanoem_f32_t offsetX = kSpacingSize * deviceScaleRatio,
                       rectWidth = 115 * deviceScaleRatio + kMarginSize * deviceScaleRatio * 2;
    const Vector4 rect(
        offsetX, offsetX, rectWidth, ImGui::GetTextLineHeightWithSpacing() * 2 + kMarginSize * deviceScaleRatio * 2);
    internalFillRect(rect, deviceScaleRatio);
    ImVec2 localOffset(offset);
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    localOffset.x += offsetX * 2;
    localOffset.y -= offsetX * 0.25f;
    localOffset.y += ImGui::GetTextLineHeightWithSpacing();
    drawList->AddText(localOffset, IM_COL32_WHITE, usageCPUBuffer);
    localOffset.y += ImGui::GetTextLineHeightWithSpacing();
    drawList->AddText(localOffset, IM_COL32_WHITE, usageMemoryBuffer);
}

void
ImGuiWindow::drawTextCentered(const ImVec2 &offset, const Vector4 &rect, const char *text, size_t length, ImU32 color)
{
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    ImVec2 t(ImGui::CalcTextSize(text)),
        c(offset.x + rect.x + (rect.z - t.x) * 0.5f, offset.y + rect.y + (rect.w - t.y) * 0.5f);
    drawList->AddText(c, color, text, text + length);
}

void
ImGuiWindow::drawBoneTooltip(Project *project, nanoem_u32_t flags)
{
    if (Model *activeModel = project->activeModel()) {
        const nanoem_model_bone_t *bonePtr = activeModel->hoveredBone();
        bool enabled = EnumUtils::isEnabled(IState::kDrawTypeBoneTooltip, flags);
        if (activeModel && bonePtr && enabled) {
            activeModel->drawBoneTooltip(primitiveContext(), bonePtr);
        }
    }
}

void
ImGuiWindow::setupDeviceInput(Project *project)
{
    ImGuiIO &io = ImGui::GetIO();
    const Vector2SI32 delta(project->lastScrollDelta());
    io.AddMouseWheelEvent(static_cast<nanoem_f32_t>(delta.x), static_cast<nanoem_f32_t>(delta.y));
    io.DeltaTime = static_cast<nanoem_f32_t>(stm_sec(stm_laptime(&m_elapsedTime)));
#if defined(IMGUI_HAS_VIEWPORT)
    if (EnumUtils::isEnabledT<ImGuiConfigFlags>(io.ConfigFlags, ImGuiConfigFlags_ViewportsEnable)) {
        io.AddMousePosEvent(m_screenCursor.m_x, m_screenCursor.m_y);
        io.AddMouseButtonEvent(0, m_screenCursor.m_pressed[Project::kCursorTypeMouseLeft]);
        io.AddMouseButtonEvent(1, m_screenCursor.m_pressed[Project::kCursorTypeMouseRight]);
        io.AddMouseButtonEvent(2, m_screenCursor.m_pressed[Project::kCursorTypeMouseMiddle]);
        const Project::CursorModifierType modifiers = m_screenCursor.m_modifiers;
        io.AddKeyEvent(ImGuiKey_ModAlt,
            EnumUtils::isEnabledT<Project::CursorModifierType>(modifiers, Project::kCursorModifierTypeAlt));
        io.AddKeyEvent(ImGuiKey_ModCtrl,
            EnumUtils::isEnabledT<Project::CursorModifierType>(modifiers, Project::kCursorModifierTypeControl));
        io.AddKeyEvent(ImGuiKey_ModShift,
            EnumUtils::isEnabledT<Project::CursorModifierType>(modifiers, Project::kCursorModifierTypeShift));
    }
    else
#endif /* IMGUI_HAS_VIEWPORT */
    {
        const Vector2SI32 offset(project->deviceScaleMovingCursorPosition());
        io.AddMousePosEvent(static_cast<nanoem_f32_t>(offset.x), static_cast<nanoem_f32_t>(offset.y));
        io.AddMouseButtonEvent(0, project->isCursorPressed(Project::kCursorTypeMouseLeft));
        io.AddMouseButtonEvent(1, project->isCursorPressed(Project::kCursorTypeMouseRight));
        io.AddMouseButtonEvent(2, project->isCursorPressed(Project::kCursorTypeMouseMiddle));
        const nanoem_u32_t modifiers = project->cursorModifiers();
        io.AddKeyEvent(ImGuiKey_ModAlt, EnumUtils::isEnabled(modifiers, Project::kCursorModifierTypeAlt));
        io.AddKeyEvent(ImGuiKey_ModCtrl, EnumUtils::isEnabled(modifiers, Project::kCursorModifierTypeControl));
        io.AddKeyEvent(ImGuiKey_ModShift, EnumUtils::isEnabled(modifiers, Project::kCursorModifierTypeShift));
    }
}

void
ImGuiWindow::handleVerticalSplitter(
    const ImVec2 &pos, const ImVec2 &size, nanoem_f32_t viewportHeight, nanoem_f32_t deviceScaleRatio)
{
    ImVec2 posFrom(pos);
    nanoem_f32_t thumbWidth = 10 * deviceScaleRatio;
    posFrom.x -= thumbWidth;
    const ImVec2 posTo(posFrom.x + thumbWidth * 0.5f, posFrom.y + viewportHeight);
    if (m_lastTimelineWidth > 0) {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            const nanoem_f32_t snapThreshold = size.x * kTimelineSnapGridRatio;
            nanoem_f32_t width = glm::clamp(m_lastTimelineWidth + ImGui::GetMouseDragDelta().x,
                             size.x * kTimelineMinWidthRatio, size.x * kTimelineMaxWidthRatio),
                         delta = width - m_defaultTimelineWidth;
            if (delta > -snapThreshold && delta < snapThreshold) {
                width = m_defaultTimelineWidth;
            }
            m_timelineWidth = width;
            m_timelineWidthRatio = width / size.x;
            ImGui::GetWindowDrawList()->AddRectFilled(
                posFrom, posTo, ImGui::ColorConvertFloat4ToU32(ImVec4(0.10f, 0.40f, 0.75f, 1.00f)));
        }
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            m_lastTimelineWidth = 0;
        }
    }
    else if (!ImGui::IsAnyItemActive() && ImGui::IsMouseHoveringRect(posFrom, posTo)) {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            m_lastTimelineWidth = m_timelineWidth;
        }
        ImGui::GetWindowDrawList()->AddRectFilled(
            posFrom, posTo, ImGui::ColorConvertFloat4ToU32(ImVec4(0.10f, 0.40f, 0.75f, 0.78f)));
    }
}

void
ImGuiWindow::handleModalDialogWindow(const Vector2 &deviceScaleWindowSize, Project *project)
{
    if (m_requestClearAllModalDialogs) {
        for (ModalDialogList::const_iterator it = m_allModalDialogs.begin(), end = m_allModalDialogs.end(); it != end;
             ++it) {
            nanoem_delete(*it);
        }
        m_allModalDialogs.clear();
        if (!m_lazyModalDialogs.empty()) {
            for (ModalDialogList::const_iterator it = m_lazyModalDialogs.begin(), end = m_lazyModalDialogs.end();
                 it != end; ++it) {
                m_allModalDialogs.push_back(*it);
            }
            m_lazyModalDialogs.clear();
        }
        else {
            m_applicationPtr->eventPublisher()->publishClearModalDialogEvent();
        }
        m_requestClearAllModalDialogs = false;
    }
    else if (IModalDialog *dialog = currentModalDialog()) {
        if (m_modalDialogPressedButton != 0) {
            switch (m_modalDialogPressedButton) {
            case IModalDialog::kButtonTypeCancel: {
                swapModalDialog(dialog->onCancelled(project));
                break;
            }
            case IModalDialog::kButtonTypeDiscard: {
                swapModalDialog(dialog->onDiscarded(project));
                break;
            }
            case IModalDialog::kButtonTypeOk: {
                swapModalDialog(dialog->onAccepted(project));
                break;
            }
            case IModalDialog::kButtonTypeSave: {
                swapModalDialog(dialog->onSaved(project));
                break;
            }
            default:
                break;
            }
            m_modalDialogPressedButton = 0;
        }
        else {
            layoutModalDialogWindow(dialog, project, deviceScaleWindowSize);
        }
    }
}

void
ImGuiWindow::layoutModalDialogWindow(IModalDialog *dialog, Project *project, const Vector2UI16 &deviceScaleWindowSize)
{
    const nanoem_f32_t deviceScaleRatio = project->windowDevicePixelRatio();
    saveDefaultStyle(deviceScaleRatio);
    char buffer[Inline::kLongNameStackBufferSize];
    StringUtils::format(buffer, sizeof(buffer), "%s##modal", dialog->title());
    if (!ImGui::IsPopupOpen(dialog->title())) {
        ImGui::OpenPopup(dialog->title());
    }
    const Vector4 desiredWindowSize(dialog->desiredWindowSize(deviceScaleWindowSize, deviceScaleRatio));
    ImVec2 actualWindowPos(desiredWindowSize.x, desiredWindowSize.y),
        actualWindowSize(desiredWindowSize.z, desiredWindowSize.w);
#if defined(IMGUI_HAS_VIEWPORT)
    const ImVec2 &viewportPosition = ImGui::GetMainViewport()->Pos;
    actualWindowPos.x += viewportPosition.x;
    actualWindowPos.y += viewportPosition.y;
#endif /* IMGUI_HAS_VIEWPORT */
    ImGui::SetNextWindowPos(actualWindowPos);
    ImGui::SetNextWindowSize(actualWindowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, kWindowRounding * deviceScaleRatio);
    if (ImGui::BeginPopupModal(dialog->title(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        const nanoem_u64_t buttons = dialog->buttons();
        dialog->draw(project);
        int numButtons = 0;
        numButtons += EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeCancel);
        numButtons += EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeDiscard);
        numButtons += EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeOk);
        numButtons += EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeSave);
        nanoem_f32_t width = ImGui::GetContentRegionAvail().x, buttonWidth = width * 0.15f;
        ImVec2 avail(ImGui::GetContentRegionAvail());
        if (avail.y > ImGui::GetFrameHeightWithSpacing()) {
            avail.y -= ImGui::GetFrameHeightWithSpacing();
            ImGui::Dummy(avail);
        }
        ImGui::Dummy(ImVec2((width - (buttonWidth * numButtons)) * 0.5f, 0));
        ImGui::SameLine();
        if (EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeOk)) {
            if (handleButton("OK", buttonWidth, dialog->isButtonEnabled(IModalDialog::kButtonTypeOk)) ||
                ImGui::IsKeyPressed(BaseApplicationService::kKeyType_ENTER)) {
                m_modalDialogPressedButton = IModalDialog::kButtonTypeOk;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        }
        if (EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeSave)) {
            if (handleTranslatedButton("nanoem.window.dialog.buttons.save", buttonWidth,
                    dialog->isButtonEnabled(IModalDialog::kButtonTypeSave)) ||
                ImGui::IsKeyPressed(BaseApplicationService::kKeyType_ENTER)) {
                m_modalDialogPressedButton = IModalDialog::kButtonTypeSave;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        }
        if (EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeDiscard)) {
            if (handleTranslatedButton("nanoem.window.dialog.buttons.discard", buttonWidth,
                    dialog->isButtonEnabled(IModalDialog::kButtonTypeDiscard))) {
                m_modalDialogPressedButton = IModalDialog::kButtonTypeDiscard;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
        }
        if (EnumUtils::isEnabled(buttons, IModalDialog::kButtonTypeCancel)) {
            if (handleTranslatedButton("nanoem.window.dialog.buttons.cancel", buttonWidth,
                    dialog->isButtonEnabled(IModalDialog::kButtonTypeCancel)) ||
                ImGui::IsKeyPressed(BaseApplicationService::kKeyType_ESCAPE)) {
                m_modalDialogPressedButton = IModalDialog::kButtonTypeCancel;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
    restoreDefaultStyle();
}

void
ImGuiWindow::batchLazySetAllObjects(Project *project, bool seekable)
{
    if (!m_lazyExecutionCommands.empty()) {
        for (LazyExecutionCommandList::const_iterator it = m_lazyExecutionCommands.begin(),
                                                      end = m_lazyExecutionCommands.end();
             it != end; ++it) {
            ILazyExecutionCommand *command = *it;
            command->execute(project);
            nanoem_delete(command);
        }
        m_lazyExecutionCommands.clear();
    }
    if (seekable && m_allModalDialogs.empty()) {
        const ImGuiIO &io = ImGui::GetIO();
        const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
        if (Inline::isDebuggerPresent()) {
            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow), false) &&
                frameIndex < Motion::kMaxFrameIndex) {
                project->seek(frameIndex + 1, false);
            }
            else if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), false) && frameIndex > 0) {
                project->seek(frameIndex - 1, false);
            }
        }
        else if (ImGui::GetKeyPressedAmount(ImGui::GetKeyIndex(ImGuiKey_RightArrow), io.KeyRepeatDelay, 0.02f) > 0 &&
            frameIndex < Motion::kMaxFrameIndex) {
            project->seek(frameIndex + 1, false);
        }
        else if (ImGui::GetKeyPressedAmount(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), io.KeyRepeatDelay, 0.02f) > 0 &&
            frameIndex > 0) {
            project->seek(frameIndex - 1, false);
        }
    }
}

void
ImGuiWindow::renderDrawList(const Project *project, const ImDrawData *drawData, int sampleCount, Buffer *bufferPtr,
    sg_bindings &bindingsRef, sg::PassBlock &pb)
{
    SG_PUSH_GROUP("ImGuiWindow::renderDrawList()");
    const sg_pixel_format pixelFormat = project->viewportPixelFormat();
    bx::HashMurmur2A v;
    v.begin();
    v.add(pixelFormat);
    v.add(sampleCount);
    nanoem_u32_t hash = v.end();
    PipelineMap::const_iterator it = m_pipelines.find(hash);
    sg_pipeline pipeline = { SG_INVALID_ID };
    if (it != m_pipelines.end()) {
        pipeline = it->second;
    }
    else {
        sg_pipeline_desc pd(m_basePipelineDescription);
        pd.colors[0].pixel_format = pixelFormat;
        if (sg::query_backend() != SG_BACKEND_GLCORE33) {
            pd.sample_count = sampleCount;
        }
        pipeline = sg::make_pipeline(&pd);
        nanoem_assert(sg::query_pipeline_state(pipeline) == SG_RESOURCESTATE_VALID, "pipeline must be valid");
        m_pipelines.insert(tinystl::make_pair(hash, pipeline));
    }
    if (drawData->CmdListsCount > 0) {
#if defined(IMGUI_HAS_VIEWPORT)
        const ImVec2 &displayPos = drawData->DisplayPos, &displaySize = drawData->DisplaySize;
        const nanoem_f32_t normalizedWidth = 1.0f / displaySize.x, normalizedHeight = 1.0f / displaySize.y;
        const ImVec4 viewportRect(
            -displayPos.x * normalizedWidth, -displayPos.y * normalizedHeight, normalizedWidth, normalizedHeight);
#else
        const ImVec2 displayPos, &displaySize = ImGui::GetIO().DisplaySize;
        const ImVec4 viewportRect(displayPos.x, displayPos.y, 1.0f / displaySize.x, 1.0f / displaySize.y);
#endif /* IMGUI_HAS_VIEWPORT */
        const sg_image viewportPrimaryImage = project->viewportPrimaryImage();
        const bool viewportWithOpaque = project->isViewportWithTransparentEnabled() ? false : true;
        const UniformBlock uniformBlockWithTransparentColor = { viewportRect, ImVec4(2, -2, 0, 0), ImVec4(1, 1, 1, 1),
            ImVec4(0, 0, 0, 0) },
                           uniformBlockWithOpaqueColor = { viewportRect, ImVec4(2, -2, 0, 0), ImVec4(1, 1, 1, 0),
                               ImVec4(0, 0, 0, 1) };
        bufferPtr->update(drawData, bindingsRef);
        pb.applyPipeline(pipeline);
        pb.applyUniformBlock(
            SG_SHADERSTAGE_VS, &uniformBlockWithTransparentColor, sizeof(uniformBlockWithTransparentColor));
        bindingsRef.fs_images[0].id = m_atlasImage.id;
        nanoem_u32_t savedTextureID = m_atlasImage.id, vertexBufferOffset = 0, indexBufferOffset = 0;
        for (int i = 0, numCmdLists = drawData->CmdListsCount; i < numCmdLists; i++) {
            const ImDrawList *cmdList = drawData->CmdLists[i];
            SG_PUSH_GROUPF(
                "ImGuiWindow::renderDrawList(numCmdLists=%d, index=%d vertexBufferOffset=%d, indexBufferOffset=%d)",
                numCmdLists, i, vertexBufferOffset, indexBufferOffset);
            bindingsRef.index_buffer_offset = Inline::saturateInt32(indexBufferOffset);
            bindingsRef.vertex_buffer_offsets[0] = Inline::saturateInt32(vertexBufferOffset);
            pb.applyBindings(bindingsRef);
            nanoem_u32_t savedVertexOffset = 0;
            for (ImVector<ImDrawCmd>::const_iterator it = cmdList->CmdBuffer.begin(), end = cmdList->CmdBuffer.end();
                 it != end; ++it) {
                const ImDrawCmd &command = *it;
                if (command.UserCallback) {
                    command.UserCallback(cmdList, &command);
                }
                else {
                    SG_PUSH_GROUPF("ImGuiWindow::renderDrawList(indexOffset=%d, numIndices=%d, vertexOffset=%d)",
                        command.IdxOffset, command.ElemCount, command.VtxOffset);
                    const int sx = static_cast<int>(command.ClipRect.x - displayPos.x),
                              sy = static_cast<int>(command.ClipRect.y - displayPos.y),
                              sw = static_cast<int>(command.ClipRect.z - sx - displayPos.x),
                              sh = static_cast<int>(command.ClipRect.w - sy - displayPos.y);
                    pb.applyScissorRect(sx, sy, sw, sh);
                    const intptr_t id = reinterpret_cast<intptr_t>(command.TextureId);
                    const sg_image image = { static_cast<nanoem_u32_t>(id) };
                    if (viewportWithOpaque && image.id == viewportPrimaryImage.id) {
                        pb.applyUniformBlock(
                            SG_SHADERSTAGE_FS, &uniformBlockWithOpaqueColor, sizeof(uniformBlockWithOpaqueColor));
                    }
                    else {
                        pb.applyUniformBlock(SG_SHADERSTAGE_FS, &uniformBlockWithTransparentColor,
                            sizeof(uniformBlockWithTransparentColor));
                    }
                    if (savedTextureID != image.id || savedVertexOffset != command.VtxOffset) {
                        savedTextureID = image.id;
                        savedVertexOffset = command.VtxOffset;
                        bindingsRef.fs_images[0] = sg::is_valid(image) ? image : m_atlasImage;
                        bindingsRef.vertex_buffer_offsets[0] =
                            vertexBufferOffset + savedVertexOffset * sizeof(ImGuiWindow::VertexUnit);
                        pb.applyBindings(bindingsRef);
                    }
                    pb.draw(Inline::saturateInt32(command.IdxOffset), Inline::saturateInt32(command.ElemCount));
                    SG_POP_GROUP();
                }
            }
            vertexBufferOffset +=
                Inline::saturateInt32U(size_t(cmdList->VtxBuffer.Size) * sizeof(ImGuiWindow::VertexUnit));
            indexBufferOffset +=
                Inline::saturateInt32U(size_t(cmdList->IdxBuffer.Size) * sizeof(cmdList->IdxBuffer[0]));
            SG_POP_GROUP();
        }
    }
    SG_POP_GROUP();
}

void
ImGuiWindow::internalFillRect(const Vector4 &deviceScaleRect, nanoem_f32_t deviceScaleRatio)
{
    m_primitive2D.fillRect(deviceScaleRect, Vector4(0, 0, 0, 0.65f), 4.0f * deviceScaleRatio);
    m_primitive2D.strokeRect(deviceScaleRect, Vector4(0.5f, 0.5f, 0.5f, 1), 4.0f * deviceScaleRatio, 0.5f);
}

ImGuiWindow::Buffer::Buffer()
    : m_vertexDataPtr(nullptr)
    , m_indexDataPtr(nullptr)
    , m_needsUnmapBuffer(false)
{
    m_vertexHandle = { SG_INVALID_ID };
    m_indexHandle = { SG_INVALID_ID };
}

ImGuiWindow::Buffer::~Buffer() NANOEM_DECL_NOEXCEPT
{
}

void
ImGuiWindow::Buffer::update(const ImDrawData *drawData, sg_bindings &bindingsRef)
{
    size_t estimatedNumVertices = 0, estimatedNumIndices = 0, offsetVertex = 0, offsetIndex = 0;
    for (int i = 0, numCmdLists = drawData->CmdListsCount; i < numCmdLists; i++) {
        const ImDrawList *drawList = drawData->CmdLists[i];
        estimatedNumVertices += drawList->VtxBuffer.size();
        estimatedNumIndices += drawList->IdxBuffer.size();
    }
    map(estimatedNumVertices, estimatedNumIndices);
    for (int i = 0, numCmdLists = drawData->CmdListsCount; i < numCmdLists; i++) {
        const ImDrawList *drawList = drawData->CmdLists[i];
        const ImDrawVert *sourceVertices = &drawList->VtxBuffer.front();
        int numVertices = drawList->VtxBuffer.size();
        for (int j = 0; j < numVertices; j++) {
            const ImDrawVert &item = sourceVertices[j];
            ImGuiWindow::VertexUnit &unit = m_vertexDataPtr[offsetVertex + j];
            unit.m_position.x = item.pos.x;
            unit.m_position.y = item.pos.y;
            unit.m_uv.x = item.uv.x;
            unit.m_uv.y = item.uv.y;
            unit.m_color = glm::make_vec4(reinterpret_cast<const nanoem_u8_t *>(&item.col));
        }
        const ImDrawIdx *sourceIndices = &drawList->IdxBuffer.front();
        int numIndices = drawList->IdxBuffer.size();
        for (int j = 0; j < numIndices; j++) {
            m_indexDataPtr[offsetIndex + j] = sourceIndices[j];
        }
        offsetVertex += numVertices;
        offsetIndex += numIndices;
    }
    unmap();
    bindingsRef.vertex_buffers[0] = m_vertexHandle;
    bindingsRef.index_buffer = m_indexHandle;
}

void
ImGuiWindow::Buffer::map(size_t estimatedNumVertices, size_t estimatedNumIndices)
{
    SG_PUSH_GROUP("ImGuiWindow::Buffer::map");
    const bool needsReset = estimatedNumVertices > m_vertexData.size() || estimatedNumIndices > m_indexData.size();
    if (needsReset) {
        m_vertexData.resize(estimatedNumVertices);
        m_indexData.resize(estimatedNumIndices);
        sg_buffer_desc vbd, ibd;
        Inline::clearZeroMemory(vbd);
        vbd.size = m_vertexData.size() * sizeof(m_vertexData[0]);
        vbd.usage = SG_USAGE_STREAM;
        Inline::clearZeroMemory(ibd);
        ibd.size = m_indexData.size() * sizeof(m_indexData[0]);
        ibd.type = SG_BUFFERTYPE_INDEXBUFFER;
        ibd.usage = SG_USAGE_STREAM;
        sg::destroy_buffer(m_vertexHandle);
        sg::destroy_buffer(m_indexHandle);
        if (Inline::isDebugLabelEnabled()) {
            vbd.label = "@nanoem/ImGuiWindow/VertexBuffer";
            ibd.label = "@nanoem/ImGuiWindow/IndexBuffer";
        }
        m_vertexHandle = sg::make_buffer(&vbd);
        nanoem_assert(sg::query_buffer_state(m_vertexHandle) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
        SG_LABEL_BUFFER(m_vertexHandle, vbd.label);
        m_indexHandle = sg::make_buffer(&ibd);
        nanoem_assert(sg::query_buffer_state(m_indexHandle) == SG_RESOURCESTATE_VALID, "index buffer must be valid");
        SG_LABEL_BUFFER(m_indexHandle, ibd.label);
    }
    if (VertexUnit *ptr = static_cast<VertexUnit *>(sg::map_buffer(m_vertexHandle))) {
        m_vertexDataPtr = ptr;
        m_indexDataPtr = static_cast<ImDrawIdx *>(sg::map_buffer(m_indexHandle));
        m_needsUnmapBuffer = true;
    }
    else {
        m_vertexDataPtr = m_vertexData.data();
        m_indexDataPtr = m_indexData.data();
    }
    SG_POP_GROUP();
}

void
ImGuiWindow::Buffer::unmap()
{
    SG_PUSH_GROUP("ImGuiWindow::Buffer::unmap");
    if (m_needsUnmapBuffer) {
        sg::unmap_buffer(m_vertexHandle, m_vertexDataPtr);
        sg::unmap_buffer(m_indexHandle, m_indexDataPtr);
    }
    else {
        int vertexDataSize = Inline::saturateInt32(m_vertexData.size() * sizeof(m_vertexData[0]));
        sg::update_buffer(m_vertexHandle, m_vertexData.data(), vertexDataSize);
        int indexDataSize = Inline::saturateInt32(m_indexData.size() * sizeof(m_indexData[0]));
        sg::update_buffer(m_indexHandle, m_indexData.data(), indexDataSize);
    }
    SG_POP_GROUP();
}

void
ImGuiWindow::Buffer::destroy() NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUP("ImGuiWindow::Buffer::destroy");
    sg::destroy_buffer(m_vertexHandle);
    sg::destroy_buffer(m_indexHandle);
    SG_POP_GROUP();
}

ImGuiWindow::PrimitiveContext::PrimitiveContext()
    : m_scaleFactor(1)
{
}

ImGuiWindow::PrimitiveContext::~PrimitiveContext() NANOEM_DECL_NOEXCEPT
{
}

void
ImGuiWindow::PrimitiveContext::setBaseOffset(const ImVec2 &value)
{
    m_offset = value;
}

void
ImGuiWindow::PrimitiveContext::setScaleFactor(nanoem_f32_t value)
{
    m_scaleFactor = value;
}

void
ImGuiWindow::PrimitiveContext::strokeLine(
    const Vector2 &from, const Vector2 &to, const Vector4 &color, nanoem_f32_t thickness)
{
    const ImU32 c(ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    const ImVec2 a(m_offset.x + from.x, m_offset.y + from.y), b(m_offset.x + to.x, m_offset.y + to.y);
    ImGui::GetWindowDrawList()->AddLine(a, b, c, thickness * m_scaleFactor);
}

void
ImGuiWindow::PrimitiveContext::strokeRect(
    const Vector4 &rect, const Vector4 &color, nanoem_f32_t roundness, nanoem_f32_t thickness)
{
    const ImU32 c(ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    const ImVec2 a(m_offset.x + rect.x, m_offset.y + rect.y), b(a.x + rect.z, a.y + rect.w);
    ImGui::GetWindowDrawList()->AddRect(a, b, c, roundness, ImDrawFlags_RoundCornersAll, thickness * m_scaleFactor);
}

void
ImGuiWindow::PrimitiveContext::fillRect(const Vector4 &rect, const Vector4 &color, nanoem_f32_t roundness)
{
    const ImU32 col(ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    const ImVec2 a(m_offset.x + rect.x, m_offset.y + rect.y), b(a.x + rect.z, a.y + rect.w);
    ImGui::GetWindowDrawList()->AddRectFilled(a, b, col, roundness, ImDrawFlags_RoundCornersAll);
}

void
ImGuiWindow::PrimitiveContext::strokeCircle(const Vector4 &rect, const Vector4 &color, nanoem_f32_t thickness)
{
    static const nanoem_f32_t kScaleFactor = 0.5f / glm::sqrt(2.0f);
    const ImU32 c(ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    const ImVec2 center(m_offset.x + rect.x + rect.z * 0.5f, m_offset.y + rect.y + rect.w * 0.5f);
    const nanoem_f32_t radius = glm::sqrt(rect.z * rect.z + rect.w * rect.w) * kScaleFactor;
    ImGui::GetWindowDrawList()->AddCircle(
        center, radius, c, int(kDrawCircleSegmentCount * m_scaleFactor), thickness * m_scaleFactor);
}

void
ImGuiWindow::PrimitiveContext::fillCircle(const Vector4 &rect, const Vector4 &color)
{
    static const nanoem_f32_t kScaleFactor = 0.5f / glm::sqrt(2.0f);
    const ImU32 c(ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    const ImVec2 center(m_offset.x + rect.x + rect.z * 0.5f, m_offset.y + rect.y + rect.w * 0.5f);
    const nanoem_f32_t radius = glm::sqrt(rect.z * rect.z + rect.w * rect.w) * kScaleFactor;
    ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, c, int(kDrawCircleSegmentCount * m_scaleFactor));
}

void
ImGuiWindow::PrimitiveContext::strokeCurve(const Vector2 &a, const Vector2 &c0, const Vector2 &c1, const Vector2 &b,
    const Vector4 &color, nanoem_f32_t thickness)
{
    const ImU32 c(ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, color.w)));
    ImGui::GetWindowDrawList()->AddBezierCubic(ImVec2(m_offset.x + a.x, m_offset.y + a.y),
        ImVec2(m_offset.x + c0.x, m_offset.y + c0.y), ImVec2(m_offset.x + c1.x, m_offset.y + c1.y),
        ImVec2(m_offset.x + b.x, m_offset.y + b.y), c, thickness * m_scaleFactor);
}

void
ImGuiWindow::PrimitiveContext::drawTooltip(const char *text, size_t length)
{
    ImGui::BeginTooltip();
    ImGui::TextUnformatted(text, text + length);
    ImGui::EndTooltip();
}

} /* namespace internal */
} /* namespace emapp */
