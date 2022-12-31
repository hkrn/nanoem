/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/plugin/MeshSyncPlugin.h"

#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace plugin {

MeshSyncPlugin::MeshSyncPlugin(IEventPublisher *publisher)
    : BasePlugin(publisher)
    , m_server(nullptr)
{
}

MeshSyncPlugin::~MeshSyncPlugin() NANOEM_DECL_NOEXCEPT
{
}

bool
MeshSyncPlugin::load(const URI &fileURI)
{
    bool succeeded = m_handle != nullptr;
    if (!succeeded) {
        bool valid = true;
        if (void *handle = bx::dlopen(fileURI.absolutePath().c_str())) {
            if (valid) {
                Inline::resolveSymbol(handle, "msServerStart", _msServerStart, valid);
                Inline::resolveSymbol(handle, "msServerStop", _msServerStop, valid);
                Inline::resolveSymbol(handle, "msServerGetNumMessages", _msServerGetNumMessages, valid);
                Inline::resolveSymbol(handle, "msServerProcessMessages", _msServerProcessMessages, valid);
                Inline::resolveSymbol(handle, "msServerBeginServe", _msServerBeginServe, valid);
                Inline::resolveSymbol(handle, "msServerEndServe", _msServerEndServe, valid);
                Inline::resolveSymbol(handle, "msServerServeTransform", _msServerServeTransform, valid);
                Inline::resolveSymbol(handle, "msServerServeCamera", _msServerServeCamera, valid);
                Inline::resolveSymbol(handle, "msServerServeLight", _msServerServeLight, valid);
                Inline::resolveSymbol(handle, "msServerServeMesh", _msServerServeMesh, valid);
                Inline::resolveSymbol(handle, "msServerServeMaterial", _msServerServeMaterial, valid);
                Inline::resolveSymbol(handle, "msServerSetScreenshotFilePath", _msServerSetScreenshotFilePath, valid);
                Inline::resolveSymbol(handle, "msGetGetBakeSkin", _msGetGetBakeSkin, valid);
                Inline::resolveSymbol(handle, "msGetGetBakeCloth", _msGetGetBakeCloth, valid);
                Inline::resolveSymbol(handle, "msSetGetSceneData", _msSetGetSceneData, valid);
                Inline::resolveSymbol(handle, "msMaterialCreate", _msMaterialCreate, valid);
                Inline::resolveSymbol(handle, "msMaterialGetID", _msMaterialGetID, valid);
                Inline::resolveSymbol(handle, "msMaterialSetID", _msMaterialSetID, valid);
                Inline::resolveSymbol(handle, "msMaterialGetName", _msMaterialGetName, valid);
                Inline::resolveSymbol(handle, "msMaterialSetName", _msMaterialSetName, valid);
                Inline::resolveSymbol(handle, "msMaterialGetColor", _msMaterialGetColor, valid);
                Inline::resolveSymbol(handle, "msMaterialSetColor", _msMaterialSetColor, valid);
                Inline::resolveSymbol(handle, "msAnimationAsTransform", _msAnimationAsTransform, valid);
                Inline::resolveSymbol(handle, "msAnimationAsCamera", _msAnimationAsCamera, valid);
                Inline::resolveSymbol(handle, "msAnimationAsLight", _msAnimationAsLight, valid);
                Inline::resolveSymbol(
                    handle, "msTransformAGetNumTranslationSamples", _msTransformAGetNumTranslationSamples, valid);
                Inline::resolveSymbol(handle, "msTransformAGetTranslationTime", _msTransformAGetTranslationTime, valid);
                Inline::resolveSymbol(
                    handle, "msTransformAGetTranslationValue", _msTransformAGetTranslationValue, valid);
                Inline::resolveSymbol(
                    handle, "msTransformAGetNumRotationSamples", _msTransformAGetNumRotationSamples, valid);
                Inline::resolveSymbol(handle, "msTransformAGetRotationTime", _msTransformAGetRotationTime, valid);
                Inline::resolveSymbol(handle, "msTransformAGetRotationValue", _msTransformAGetRotationValue, valid);
                Inline::resolveSymbol(handle, "msTransformAGetNumScaleSamples", _msTransformAGetNumScaleSamples, valid);
                Inline::resolveSymbol(handle, "msTransformAGetScaleTime", _msTransformAGetScaleTime, valid);
                Inline::resolveSymbol(handle, "msTransformAGetScaleValue", _msTransformAGetScaleValue, valid);
                Inline::resolveSymbol(
                    handle, "msTransformAGetNumVisibleSamples", _msTransformAGetNumVisibleSamples, valid);
                Inline::resolveSymbol(handle, "msTransformAGetVisibleTime", _msTransformAGetVisibleTime, valid);
                Inline::resolveSymbol(handle, "msTransformAGetVisibleValue", _msTransformAGetVisibleValue, valid);
                Inline::resolveSymbol(handle, "msCameraAGetNumFovSamples", _msCameraAGetNumFovSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFovTime", _msCameraAGetFovTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFovValue", _msCameraAGetFovValue, valid);
                Inline::resolveSymbol(handle, "msCameraAGetNumNearSamples", _msCameraAGetNumNearSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetNearTime", _msCameraAGetNearTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetNearValue", _msCameraAGetNearValue, valid);
                Inline::resolveSymbol(handle, "msCameraAGetNumFarSamples", _msCameraAGetNumFarSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFarTime", _msCameraAGetFarTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFarValue", _msCameraAGetFarValue, valid);
                Inline::resolveSymbol(
                    handle, "msCameraAGetNumHApertureSamples", _msCameraAGetNumHApertureSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetHApertureTime", _msCameraAGetHApertureTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetHApertureValue", _msCameraAGetHApertureValue, valid);
                Inline::resolveSymbol(
                    handle, "msCameraAGetNumVApertureSamples", _msCameraAGetNumVApertureSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetVApertureTime", _msCameraAGetVApertureTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetVApertureValue", _msCameraAGetVApertureValue, valid);
                Inline::resolveSymbol(
                    handle, "msCameraAGetNumFocalLengthSamples", _msCameraAGetNumFocalLengthSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFocalLengthTime", _msCameraAGetFocalLengthTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFocalLengthValue", _msCameraAGetFocalLengthValue, valid);
                Inline::resolveSymbol(
                    handle, "msCameraAGetNumFocusDistanceSamples", _msCameraAGetNumFocusDistanceSamples, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFocusDistanceTime", _msCameraAGetFocusDistanceTime, valid);
                Inline::resolveSymbol(handle, "msCameraAGetFocusDistanceValue", _msCameraAGetFocusDistanceValue, valid);
                Inline::resolveSymbol(handle, "msLightAGetNumColorSamples", _msLightAGetNumColorSamples, valid);
                Inline::resolveSymbol(handle, "msLightAGetColorTime", _msLightAGetColorTime, valid);
                Inline::resolveSymbol(handle, "msLightAGetColorValue", _msLightAGetColorValue, valid);
                Inline::resolveSymbol(handle, "msLightAGetNumIntensitySamples", _msLightAGetNumIntensitySamples, valid);
                Inline::resolveSymbol(handle, "msLightAGetIntensityTime", _msLightAGetIntensityTime, valid);
                Inline::resolveSymbol(handle, "msLightAGetIntensityValue", _msLightAGetIntensityValue, valid);
                Inline::resolveSymbol(handle, "msLightAGetNumRangeSamples", _msLightAGetNumRangeSamples, valid);
                Inline::resolveSymbol(handle, "msLightAGetRangeTime", _msLightAGetRangeTime, valid);
                Inline::resolveSymbol(handle, "msLightAGetRangeValue", _msLightAGetRangeValue, valid);
                Inline::resolveSymbol(handle, "msLightAGetNumSpotAngleSamples", _msLightAGetNumSpotAngleSamples, valid);
                Inline::resolveSymbol(handle, "msLightAGetSpotAngleTime", _msLightAGetSpotAngleTime, valid);
                Inline::resolveSymbol(handle, "msLightAGetSpotAngleValue", _msLightAGetSpotAngleValue, valid);
                Inline::resolveSymbol(handle, "msGetGetFlags", _msGetGetFlags, valid);
                Inline::resolveSymbol(handle, "msDeleteGetNumTargets", _msDeleteGetNumTargets, valid);
                Inline::resolveSymbol(handle, "msDeleteGetPath", _msDeleteGetPath, valid);
                Inline::resolveSymbol(handle, "msDeleteGetID", _msDeleteGetID, valid);
                Inline::resolveSymbol(handle, "msFenceGetType", _msFenceGetType, valid);
                Inline::resolveSymbol(handle, "msTextGetText", _msTextGetText, valid);
                Inline::resolveSymbol(handle, "msTextGetType", _msTextGetType, valid);
                Inline::resolveSymbol(handle, "msTransformCreate", _msTransformCreate, valid);
                Inline::resolveSymbol(handle, "msTransformGetID", _msTransformGetID, valid);
                Inline::resolveSymbol(handle, "msTransformSetID", _msTransformSetID, valid);
                Inline::resolveSymbol(handle, "msTransformGetIndex", _msTransformGetIndex, valid);
                Inline::resolveSymbol(handle, "msTransformSetIndex", _msTransformSetIndex, valid);
                Inline::resolveSymbol(handle, "msTransformGetPath", _msTransformGetPath, valid);
                Inline::resolveSymbol(handle, "msTransformSetPath", _msTransformSetPath, valid);
                Inline::resolveSymbol(handle, "msTransformGetPosition", _msTransformGetPosition, valid);
                Inline::resolveSymbol(handle, "msTransformSetPosition", _msTransformSetPosition, valid);
                Inline::resolveSymbol(handle, "msTransformGetRotation", _msTransformGetRotation, valid);
                Inline::resolveSymbol(handle, "msTransformSetRotation", _msTransformSetRotation, valid);
                Inline::resolveSymbol(handle, "msTransformGetScale", _msTransformGetScale, valid);
                Inline::resolveSymbol(handle, "msTransformSetScale", _msTransformSetScale, valid);
                Inline::resolveSymbol(handle, "msTransformGetVisible", _msTransformGetVisible, valid);
                Inline::resolveSymbol(handle, "msTransformSetVisible", _msTransformSetVisible, valid);
                Inline::resolveSymbol(handle, "msTransformGetReference", _msTransformGetReference, valid);
                Inline::resolveSymbol(handle, "msTransformSetReference", _msTransformSetReference, valid);
                Inline::resolveSymbol(handle, "msTransformGetAnimation", _msTransformGetAnimation, valid);
                Inline::resolveSymbol(handle, "msCameraCreate", _msCameraCreate, valid);
                Inline::resolveSymbol(handle, "msCameraIsOrtho", _msCameraIsOrtho, valid);
                Inline::resolveSymbol(handle, "msCameraSetOrtho", _msCameraSetOrtho, valid);
                Inline::resolveSymbol(handle, "msCameraGetFov", _msCameraGetFov, valid);
                Inline::resolveSymbol(handle, "msCameraSetFov", _msCameraSetFov, valid);
                Inline::resolveSymbol(handle, "msCameraGetNearPlane", _msCameraGetNearPlane, valid);
                Inline::resolveSymbol(handle, "msCameraSetNearPlane", _msCameraSetNearPlane, valid);
                Inline::resolveSymbol(handle, "msCameraGetFarPlane", _msCameraGetFarPlane, valid);
                Inline::resolveSymbol(handle, "msCameraSetFarPlane", _msCameraSetFarPlane, valid);
                Inline::resolveSymbol(handle, "msCameraGetHorizontalAperture", _msCameraGetHorizontalAperture, valid);
                Inline::resolveSymbol(handle, "msCameraSetHorizontalAperture", _msCameraSetHorizontalAperture, valid);
                Inline::resolveSymbol(handle, "msCameraGetVerticalAperture", _msCameraGetVerticalAperture, valid);
                Inline::resolveSymbol(handle, "msCameraSetVerticalAperture", _msCameraSetVerticalAperture, valid);
                Inline::resolveSymbol(handle, "msCameraGetFocalLength", _msCameraGetFocalLength, valid);
                Inline::resolveSymbol(handle, "msCameraSetFocalLength", _msCameraSetFocalLength, valid);
                Inline::resolveSymbol(handle, "msCameraGetFocusDistance", _msCameraGetFocusDistance, valid);
                Inline::resolveSymbol(handle, "msCameraSetFocusDistance", _msCameraSetFocusDistance, valid);
                Inline::resolveSymbol(handle, "msLightCreate", _msLightCreate, valid);
                Inline::resolveSymbol(handle, "msLightGetType", _msLightGetType, valid);
                Inline::resolveSymbol(handle, "msLightSetType", _msLightSetType, valid);
                Inline::resolveSymbol(handle, "msLightGetColor", _msLightGetColor, valid);
                Inline::resolveSymbol(handle, "msLightSetColor", _msLightSetColor, valid);
                Inline::resolveSymbol(handle, "msLightGetIntensity", _msLightGetIntensity, valid);
                Inline::resolveSymbol(handle, "msLightSetIntensity", _msLightSetIntensity, valid);
                Inline::resolveSymbol(handle, "msLightGetRange", _msLightGetRange, valid);
                Inline::resolveSymbol(handle, "msLightSetRange", _msLightSetRange, valid);
                Inline::resolveSymbol(handle, "msLightGetSpotAngle", _msLightGetSpotAngle, valid);
                Inline::resolveSymbol(handle, "msLightSetSpotAngle", _msLightSetSpotAngle, valid);
                Inline::resolveSymbol(handle, "msMeshCreate", _msMeshCreate, valid);
                Inline::resolveSymbol(handle, "msMeshGetFlags", _msMeshGetFlags, valid);
                Inline::resolveSymbol(handle, "msMeshSetFlags", _msMeshSetFlags, valid);
                Inline::resolveSymbol(handle, "msMeshGetNumPoints", _msMeshGetNumPoints, valid);
                Inline::resolveSymbol(handle, "msMeshGetNumIndices", _msMeshGetNumIndices, valid);
                Inline::resolveSymbol(handle, "msMeshGetNumSplits", _msMeshGetNumSplits, valid);
                Inline::resolveSymbol(handle, "msMeshReadPoints", _msMeshReadPoints, valid);
                Inline::resolveSymbol(handle, "msMeshWritePoints", _msMeshWritePoints, valid);
                Inline::resolveSymbol(handle, "msMeshReadNormals", _msMeshReadNormals, valid);
                Inline::resolveSymbol(handle, "msMeshWriteNormals", _msMeshWriteNormals, valid);
                Inline::resolveSymbol(handle, "msMeshReadTangents", _msMeshReadTangents, valid);
                Inline::resolveSymbol(handle, "msMeshWriteTangents", _msMeshWriteTangents, valid);
                Inline::resolveSymbol(handle, "msMeshReadUV0", _msMeshReadUV0, valid);
                Inline::resolveSymbol(handle, "msMeshReadUV1", _msMeshReadUV1, valid);
                Inline::resolveSymbol(handle, "msMeshWriteUV0", _msMeshWriteUV0, valid);
                Inline::resolveSymbol(handle, "msMeshWriteUV1", _msMeshWriteUV1, valid);
                Inline::resolveSymbol(handle, "msMeshReadColors", _msMeshReadColors, valid);
                Inline::resolveSymbol(handle, "msMeshWriteColors", _msMeshWriteColors, valid);
                Inline::resolveSymbol(handle, "msMeshReadIndices", _msMeshReadIndices, valid);
                Inline::resolveSymbol(handle, "msMeshWriteIndices", _msMeshWriteIndices, valid);
                Inline::resolveSymbol(handle, "msMeshWriteSubmeshTriangles", _msMeshWriteSubmeshTriangles, valid);
                Inline::resolveSymbol(handle, "msMeshGetSplit", _msMeshGetSplit, valid);
                Inline::resolveSymbol(handle, "msMeshGetNumSubmeshes", _msMeshGetNumSubmeshes, valid);
                Inline::resolveSymbol(handle, "msMeshGetSubmesh", _msMeshGetSubmesh, valid);
                Inline::resolveSymbol(handle, "msMeshReadWeights4", _msMeshReadWeights4, valid);
                Inline::resolveSymbol(handle, "msMeshWriteWeights4", _msMeshWriteWeights4, valid);
                Inline::resolveSymbol(handle, "msMeshGetNumBones", _msMeshGetNumBones, valid);
                Inline::resolveSymbol(handle, "msMeshGetRootBonePath", _msMeshGetRootBonePath, valid);
                Inline::resolveSymbol(handle, "msMeshSetRootBonePath", _msMeshSetRootBonePath, valid);
                Inline::resolveSymbol(handle, "msMeshGetBonePath", _msMeshGetBonePath, valid);
                Inline::resolveSymbol(handle, "msMeshSetBonePath", _msMeshSetBonePath, valid);
                Inline::resolveSymbol(handle, "msMeshReadBindPoses", _msMeshReadBindPoses, valid);
                Inline::resolveSymbol(handle, "msMeshWriteBindPoses", _msMeshWriteBindPoses, valid);
                Inline::resolveSymbol(handle, "msMeshGetNumBlendShapes", _msMeshGetNumBlendShapes, valid);
                Inline::resolveSymbol(handle, "msMeshGetBlendShapeData", _msMeshGetBlendShapeData, valid);
                Inline::resolveSymbol(handle, "msMeshAddBlendShape", _msMeshAddBlendShape, valid);
                Inline::resolveSymbol(handle, "msMeshSetLocal2World", _msMeshSetLocal2World, valid);
                Inline::resolveSymbol(handle, "msMeshSetWorld2Local", _msMeshSetWorld2Local, valid);
                Inline::resolveSymbol(handle, "msSplitGetNumPoints", _msSplitGetNumPoints, valid);
                Inline::resolveSymbol(handle, "msSplitGetNumIndices", _msSplitGetNumIndices, valid);
                Inline::resolveSymbol(handle, "msSplitGetNumSubmeshes", _msSplitGetNumSubmeshes, valid);
                Inline::resolveSymbol(handle, "msSplitGetSubmesh", _msSplitGetSubmesh, valid);
                Inline::resolveSymbol(handle, "msSubmeshGetNumIndices", _msSubmeshGetNumIndices, valid);
                Inline::resolveSymbol(handle, "msSubmeshGetMaterialID", _msSubmeshGetMaterialID, valid);
                Inline::resolveSymbol(handle, "msSubmeshReadIndices", _msSubmeshReadIndices, valid);
                Inline::resolveSymbol(handle, "msBlendShapeGetName", _msBlendShapeGetName, valid);
                Inline::resolveSymbol(handle, "msBlendShapeGetWeight", _msBlendShapeGetWeight, valid);
                Inline::resolveSymbol(handle, "msBlendShapeGetNumFrames", _msBlendShapeGetNumFrames, valid);
                Inline::resolveSymbol(handle, "msBlendShapeGetFrameWeight", _msBlendShapeGetFrameWeight, valid);
                Inline::resolveSymbol(handle, "msBlendShapeReadPoints", _msBlendShapeReadPoints, valid);
                Inline::resolveSymbol(handle, "msBlendShapeReadNormals", _msBlendShapeReadNormals, valid);
                Inline::resolveSymbol(handle, "msBlendShapeReadTangents", _msBlendShapeReadTangents, valid);
                Inline::resolveSymbol(handle, "msBlendShapeAddFrame", _msBlendShapeAddFrame, valid);
                Inline::resolveSymbol(handle, "msSceneGetName", _msSceneGetName, valid);
                Inline::resolveSymbol(handle, "msSceneGetNumMeshes", _msSceneGetNumMeshes, valid);
                Inline::resolveSymbol(handle, "msSceneGetMeshData", _msSceneGetMeshData, valid);
                Inline::resolveSymbol(handle, "msSceneGetNumTransforms", _msSceneGetNumTransforms, valid);
                Inline::resolveSymbol(handle, "msSceneGetTransformData", _msSceneGetTransformData, valid);
                Inline::resolveSymbol(handle, "msSceneGetNumCameras", _msSceneGetNumCameras, valid);
                Inline::resolveSymbol(handle, "msSceneGetCameraData", _msSceneGetCameraData, valid);
                Inline::resolveSymbol(handle, "msSceneGetNumLights", _msSceneGetNumLights, valid);
                Inline::resolveSymbol(handle, "msSceneGetLightData", _msSceneGetLightData, valid);
                Inline::resolveSymbol(handle, "msSceneGetNumMaterials", _msSceneGetNumMaterials, valid);
                Inline::resolveSymbol(handle, "msSceneGetMaterialData", _msSceneGetMaterialData, valid);
                m_handle = handle;
                m_name = fileURI.lastPathComponent();
                succeeded = true;
            }
            else {
                bx::dlclose(handle);
            }
        }
    }
    return succeeded;
}

void
MeshSyncPlugin::unload()
{
}

bool
MeshSyncPlugin::create()
{
    m_server = _msServerStart(nullptr);
    return m_server != nullptr;
}

void
MeshSyncPlugin::destroy()
{
    _msServerStop(m_server);
    m_server = nullptr;
}

void
MeshSyncPlugin::start()
{
    stop();
    _msServerBeginServe(m_server);
}

void
MeshSyncPlugin::stop()
{
    if (m_server) {
        _msServerEndServe(m_server);
    }
}

const char *
MeshSyncPlugin::failureReason() const NANOEM_DECL_NOEXCEPT
{
    return "";
}

const char *
MeshSyncPlugin::recoverySuggestion() const NANOEM_DECL_NOEXCEPT
{
    return "";
}

} /* namespace plugin */
} /* namespace nanoem */
