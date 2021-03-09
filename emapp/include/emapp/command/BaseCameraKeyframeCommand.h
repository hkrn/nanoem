/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASECAMERAKEYFRAMECOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASECAMERAKEYFRAMECOMMAND_H_

#include "emapp/command/BaseKeyframeCommand.h"

namespace nanoem {
namespace command {

class BaseCameraKeyframeCommand : public BaseKeyframeCommand {
public:
    virtual ~BaseCameraKeyframeCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct BezierControlPointParameter {
        glm::u8vec4 m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    };
    struct BezierControlPointParameterLookAtOnly {
        glm::u8vec4 m_value[NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_ANGLE];
    };
    typedef tinystl::pair<BezierControlPointParameterLookAtOnly, BezierControlPointParameterLookAtOnly>
        BezierControlPointParameterLookAtOnlyPair;
    struct CameraKeyframe : Keyframe {
        struct State {
            State();
            ~State() NANOEM_DECL_NOEXCEPT;
            void assign(const ICamera *camera);
            void assign(const nanoem_motion_camera_keyframe_t *keyframe);
            Vector4 m_lookAt;
            Vector4 m_angle;
            nanoem_f32_t m_distance;
            BezierControlPointParameter m_parameter;
            nanoem_f32_t m_fov;
            nanoem_u32_t m_stageIndex;
            bool m_perspective;
        };
        CameraKeyframe()
            : Keyframe(0)
            , m_bezierCurveOverrideTargetFrameIndex(Motion::kMaxFrameIndex)
        {
        }
        CameraKeyframe(nanoem_frame_index_t frameIndex)
            : Keyframe(frameIndex)
            , m_bezierCurveOverrideTargetFrameIndex(Motion::kMaxFrameIndex)
        {
        }
        void
        setFrameIndex(const nanoem_motion_t *motion, nanoem_frame_index_t value, bool removing)
        {
            m_frameIndex = value;
            m_updated = !isRemovable(value, removing) && nanoemMotionFindCameraKeyframeObject(motion, value) != NULL;
        }
        tinystl::pair<State, State> m_state;
        nanoem_frame_index_t m_bezierCurveOverrideTargetFrameIndex;
        BezierControlPointParameterLookAtOnlyPair m_lookAtParameters;
    };
    typedef tinystl::vector<CameraKeyframe, nanoem::TinySTLAllocator> CameraKeyframeList;

    BaseCameraKeyframeCommand(const CameraKeyframeList &keyframes, const ICamera *camera, Motion *motion);

    void restoreKeyframeState(
        nanoem_mutable_motion_camera_keyframe_t *keyframe, const CameraKeyframe::State &state, bool &linear);
    void addKeyframe(Error &error);
    void removeKeyframe(Error &error);
    void readMessage(const void *messagePtr, bool removing);
    void writeMessage(void *messagePtr, nanoem_u32_t type);
    void releaseMessage(void *messagePtr);
    void readStateMessage(const void *opaque, CameraKeyframe::State &s);
    void writeStateMessage(const CameraKeyframe::State &s, void *opaque);
    void releaseStateMessage(void *opaque);

    const ICamera *m_camera;
    CameraKeyframeList m_keyframes;
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASECAMERAKEYFRAMECOMMAND_H_ */
