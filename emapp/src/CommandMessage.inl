/*
   Copyright (c) 2015-2020 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "./protoc/application.pb-c.h"

#include "emapp/IEventPublisher.h"
#include "emapp/Model.h"
#include "emapp/FileUtils.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace {

class CommandMessageUtil NANOEM_DECL_SEALED : private NonCopyable  {
public:
    static inline void
    setVector(const Vector3 &v, Nanoem__Common__Vector3 *&s)
    {
        s = nanoem_new(Nanoem__Common__Vector3);
        nanoem__common__vector3__init(s);
        s->x = v.x;
        s->y = v.y;
        s->z = v.z;
    }
    static inline void
    getVector(const Nanoem__Common__Vector3 *s, Vector3 &v)
    {
        v.x = s->x;
        v.y = s->y;
        v.z = s->z;
    }
    static inline void
    getVector(const Nanoem__Common__Vector3 *s, Vector4 &v)
    {
        v.x = s->x;
        v.y = s->y;
        v.z = s->z;
        v.w = 1.0f;
    }
    static inline void
    releaseVector(Nanoem__Common__Vector3 *s)
    {
        nanoem_delete(s);
    }
    static inline void
    setColor(const Vector4 &v, Nanoem__Common__Color *&s)
    {
        s = nanoem_new(Nanoem__Common__Color);
        nanoem__common__color__init(s);
        s->red = v.x;
        s->green = v.y;
        s->blue = v.z;
        s->alpha = v.w;
    }
    static inline void
    getColor(const Nanoem__Common__Color *s, Vector3 &v)
    {
        v.x = s->red;
        v.y = s->green;
        v.z = s->blue;
    }
    static inline void
    getColor(const Nanoem__Common__Color *s, Vector4 &v)
    {
        v.x = s->red;
        v.y = s->green;
        v.z = s->blue;
        v.w = s->alpha;
    }
    static inline void
    releaseColor(Nanoem__Common__Color *s)
    {
        nanoem_delete(s);
    }
    static inline void
    releaseVector(Nanoem__Common__Color *s)
    {
        nanoem_delete(s);
    }
    static inline void
    setQuaternion(const Quaternion &q, Nanoem__Common__Quaternion *&s)
    {
        s = nanoem_new(Nanoem__Common__Quaternion);
        nanoem__common__quaternion__init(s);
        s->x = q.x;
        s->y = q.y;
        s->z = q.z;
        s->w = q.w;
    }
    static inline void
    getQuaternion(const Nanoem__Common__Quaternion *s, Quaternion &q)
    {
        q.x = s->x;
        q.y = s->y;
        q.z = s->z;
        q.w = s->w;
    }
    static inline void
    releaseQuaternion(Nanoem__Common__Quaternion *q)
    {
        nanoem_delete(q);
    }
    static inline void
    setInterpolation(const Vector4U8 &v, Nanoem__Common__Interpolation *&i)
    {
        i = nanoem_new(Nanoem__Common__Interpolation);
        nanoem__common__interpolation__init(i);
        i->x0 = v.x;
        i->y0 = v.y;
        i->x1 = v.z;
        i->y1 = v.w;
    }
    static inline void
    getInterpolation(const Nanoem__Common__Interpolation *i, Vector4U8 &v)
    {
        v.x = i->x0;
        v.y = i->y0;
        v.z = i->x1;
        v.w = i->y1;
    }
    static inline void
    releaseInterpolation(Nanoem__Common__Interpolation *i)
    {
        nanoem_delete(i);
    }
    static inline void
    setOutsideParent(const StringPair &parent, const Project *project, Nanoem__Application__OutsideParent *&op)
    {
        op = nanoem_new(Nanoem__Application__OutsideParent);
        nanoem__application__outside_parent__init(op);
        if (const Model *model = project->findModelByName(parent.first)) {
            op->has_parent_model_handle = 1;
            op->parent_model_handle = model->handle();
            if (const nanoem_model_bone_t *bonePtr = model->findBone(parent.second)) {
                op->has_parent_model_bone_index = 1;
                op->parent_model_bone_index = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
            }
        }
    }
    static inline void
    setOutsideParent(const nanoem_model_bone_t *bonePtr, const StringPair &parent, const Project *project, Nanoem__Application__OutsideParent *&op)
    {
        op = nanoem_new(Nanoem__Application__OutsideParent);
        nanoem__application__outside_parent__init(op);
        op->has_bone_index = 1;
        op->bone_index = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
        if (const Model *model = project->findModelByName(parent.first)) {
            op->has_parent_model_handle = 1;
            op->parent_model_handle = model->handle();
            if (const nanoem_model_bone_t *bonePtr = model->findBone(parent.second)) {
                op->has_parent_model_bone_index = 1;
                op->parent_model_bone_index = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
            }
        }
    }
    static inline void
    releaseOutsideParent(Nanoem__Application__OutsideParent *op)
    {
        nanoem_delete(op);
    }
    static inline nanoem_frame_index_t
    saturatedFrameIndex(uint64_t value)
    {
        return nanoem_frame_index_t(glm::clamp(value, uint64_t(0), uint64_t(Motion::kMaxFrameIndex)));
    }
};

} /* namespace anonymous */
} /* namespace nanoem */
