/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/project/JSON.h"

#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/ICamera.h"
#include "emapp/ILight.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace project {

JSON::JSON(Project *project)
    : m_project(project)
{
}

JSON::~JSON() NANOEM_DECL_NOEXCEPT
{
}

bool
JSON::load(const JSON_Value *value)
{
    const JSON_Object *root = json_object_get_object(json_object(value), "project");
    const JSON_Object *to = json_object_get_object(root, "types");
    m_project->setEditingMode(static_cast<Project::EditingMode>(int(json_object_get_number(to, "edit"))));
    m_project->setDrawType(static_cast<IDrawable::DrawType>(int(json_object_get_number(to, "draw"))));
    m_project->setLanguage(
        static_cast<ITranslator::LanguageType>(nanoem_u32_t(json_object_get_number(root, "language"))));
    m_project->setCameraShared(json_object_dotget_boolean(root, "camera.shared") > 0);
    m_project->setLoopEnabled(json_object_get_boolean(root, "loop") > 0);
    const JSON_Object *po = json_object_get_object(root, "physics");
    m_project->setPhysicsSimulationEngineDebugFlags(nanoem_u32_t(json_object_get_number(po, "debug")));
    m_project->setPhysicsSimulationMode(json_object_get_boolean(po, "active") != 0
            ? PhysicsEngine::kSimulationModeEnableTracing
            : PhysicsEngine::kSimulationModeDisable);
    m_project->setPreferredMotionFPS(nanoem_u32_t(glm::clamp(json_object_get_number(root, "fps"), 30.0, 60.0)), false);
    const JSON_Object *go = json_object_get_object(root, "grid");
    const JSON_Object *co = json_object_get_object(go, "cell");
    const Vector2 cell(nanoem_f32_t(json_object_get_number(co, "x")), nanoem_f32_t(json_object_get_number(co, "y")));
    Grid *grid = m_project->grid();
    grid->setCell(cell);
    grid->setOpacity(nanoem_f32_t(json_object_get_number(go, "opacity")));
    grid->setVisible(json_object_get_boolean(go, "visible") != 0);
    const JSON_Object *cs = json_object_dotget_object(root, "confirm.seek");
    m_project->setConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE, json_object_get_boolean(cs, "bone") > 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, json_object_get_boolean(cs, "camera") > 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, json_object_get_boolean(cs, "light") > 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, json_object_get_boolean(cs, "model") > 0);
    m_project->setConfirmSeekEnabled(
        NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH, json_object_get_boolean(cs, "morph") > 0);
    m_project->setGroundShadowEnabled(json_object_dotget_boolean(root, "shadow.enabled") != 0);
    m_project->setShadowMapEnabled(json_object_dotget_boolean(root, "shadowmap.enabled") > 0);
    ShadowCamera *shadowCamera = m_project->shadowCamera();
    if (const JSON_Value *distance = json_object_dotget_value(root, "shadowmap.distance")) {
        shadowCamera->setDistance(nanoem_f32_t(json_value_get_number(distance)));
    }
    if (const JSON_Value *mode = json_object_dotget_value(root, "shadowmap.mode")) {
        shadowCamera->setCoverageMode(
            static_cast<ShadowCamera::CoverageModeType>(nanoem_u32_t(json_value_get_number(mode))));
    }
    JSON_Array *colorValue = json_object_dotget_array(root, "screen.color");
    if (json_array_get_count(colorValue) >= 4) {
        const Vector4U8 screenColor(json_array_get_number(colorValue, 0), json_array_get_number(colorValue, 1),
            json_array_get_number(colorValue, 2), json_array_get_number(colorValue, 3));
        m_project->setViewportBackgroundColor(Vector4(screenColor) / Vector4(0xff));
    }
    m_project->setSampleLevel(nanoem_u32_t(json_object_dotget_number(root, "screen.sample")));
    if (const JSON_Value *volume = json_object_dotget_value(root, "audio.volume")) {
        m_project->audioPlayer()->setVolumeGain(nanoem_f32_t(json_value_get_number(volume)));
    }
    if (const JSON_Value *coordination = json_object_dotget_value(root, "coordination")) {
        switch (json_value_get_type(coordination)) {
        case JSONBoolean:
            /* recover coordination value due to saving project bug that produces from 1.15 PR9 */
            m_project->setCoordinationSystem(json_value_get_boolean(coordination) ? GLM_LEFT_HANDED : GLM_RIGHT_HANDED);
            break;
        case JSONNumber:
        default:
            m_project->setCoordinationSystem(
                int(json_value_get_number(coordination)) == GLM_LEFT_HANDED ? GLM_LEFT_HANDED : GLM_RIGHT_HANDED);
            break;
        }
    }
    else {
        m_project->setCoordinationSystem(GLM_RIGHT_HANDED);
    }
    m_project->setMotionMergeEnabled(json_object_dotget_boolean(root, "motion.merge.enabled") > 0);
    m_project->setMultipleBoneSelectionEnabled(json_object_dotget_boolean(root, "selection.bones.enabled") > 0);
    m_project->setEffectPluginEnabled(json_object_dotget_boolean(root, "plugin.effect.enabled") > 0);
    m_project->setBaseDuration(nanoem_frame_index_t(json_object_dotget_number(root, "time.duration")));
    shadowCamera->setDirty(false);
    return true;
}

void
JSON::save(JSON_Value *value)
{
    JSON_Object *root = json_object_get_object(json_object(value), "project");
    json_object_dotset_number(root, "coordination", m_project->coordinationSystem());
    json_object_dotset_number(root, "types.edit", m_project->editingMode());
    json_object_dotset_number(root, "types.draw", m_project->drawType());
    json_object_dotset_number(root, "language", m_project->language());
    json_object_dotset_boolean(root, "loop", m_project->isLoopEnabled());
    PhysicsEngine *engine = m_project->physicsEngine();
    json_object_dotset_number(root, "physics.debug", engine->debugGeometryFlags());
    json_object_dotset_boolean(
        root, "physics.active", engine->simulationMode() != PhysicsEngine::kSimulationModeDisable);
    json_object_dotset_number(root, "fps", m_project->preferredMotionFPS());
    json_object_dotset_number(root, "seek", m_project->currentLocalFrameIndex());
    const Grid *grid = m_project->grid();
    json_object_dotset_number(root, "grid.cell.x", nanoem_f64_t(grid->cell().x));
    json_object_dotset_number(root, "grid.cell.y", nanoem_f64_t(grid->cell().y));
    json_object_dotset_number(root, "grid.opacity", nanoem_f64_t(grid->opacity()));
    json_object_dotset_boolean(root, "grid.visible", grid->isVisible());
    json_object_dotset_boolean(
        root, "confirm.seek.bone", m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE));
    json_object_dotset_boolean(
        root, "confirm.seek.camera", m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA));
    json_object_dotset_boolean(
        root, "confirm.seek.light", m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT));
    json_object_dotset_boolean(
        root, "confirm.seek.model", m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL));
    json_object_dotset_boolean(
        root, "confirm.seek.morph", m_project->isConfirmSeekEnabled(NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH));
    json_object_dotset_boolean(root, "shadow.enabled", m_project->isGroundShadowEnabled());
    json_object_dotset_boolean(root, "shadowmap.enabled", m_project->isShadowMapEnabled());
    const ShadowCamera *shadowCamera = m_project->shadowCamera();
    json_object_dotset_number(root, "shadowmap.distance", shadowCamera->distance());
    json_object_dotset_number(root, "shadowmap.mode", shadowCamera->coverageMode());
    const Vector4U8 color(m_project->viewportBackgroundColor() * Vector4(0xff));
    JSON_Value *colorValue = json_value_init_array();
    JSON_Array *colorArray = json_array(colorValue);
    json_array_append_number(colorArray, color.x);
    json_array_append_number(colorArray, color.y);
    json_array_append_number(colorArray, color.z);
    json_array_append_number(colorArray, color.w);
    json_object_dotset_value(root, "screen.color", colorValue);
    json_object_dotset_number(root, "screen.sample", m_project->sampleLevel());
    json_object_dotset_number(root, "audio.volume", nanoem_f64_t(m_project->audioPlayer()->volumeGain()));
    json_object_dotset_boolean(root, "selection.bones.enabled", m_project->isMultipleBoneSelectionEnabled());
    json_object_dotset_boolean(root, "motion.merge.enabled", m_project->isMotionMergeEnabled());
    json_object_dotset_boolean(root, "plugin.effect.enabled", m_project->isEffectPluginEnabled());
    json_object_dotset_number(root, "time.current", nanoem_f64_t(m_project->currentLocalFrameIndex()));
    json_object_dotset_number(root, "time.duration", nanoem_f64_t(m_project->baseDuration()));
    m_project->globalCamera()->setDirty(false);
    m_project->globalLight()->setDirty(false);
    m_project->shadowCamera()->setDirty(false);
}

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */
