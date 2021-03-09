/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#include <memory>

#include "emapp/BaseApplicationService.h"
#include "emapp/Error.h"

#include "glm/gtc/epsilon.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

#include "bx/commandline.h"

#define CATCH_CONFIG_FAST_COMPILE
#include "catch2/catch.hpp"

namespace nanoem {
class DefaultTranslator;
namespace internal {
class StubEventPublisher;
} /* namespace internal */
} /* namespace nanoem */

namespace Catch {

template <> struct StringMaker<nanoem::String> {
    static std::string convert(const nanoem::String &value);
};
template <> struct StringMaker<nanoem::Quaternion> {
    static std::string convert(const nanoem::Quaternion &value);
};
template <> struct StringMaker<nanoem::Vector2> {
    static std::string convert(const nanoem::Vector2 &value);
};
template <> struct StringMaker<nanoem::Vector3> {
    static std::string convert(const nanoem::Vector3 &value);
};
template <> struct StringMaker<nanoem::Vector4> {
    static std::string convert(const nanoem::Vector4 &value);
};
template <> struct StringMaker<glm::u8vec4> {
    static std::string convert(const glm::u8vec4 &value);
};
template <> struct StringMaker<const nanoem_motion_outside_parent_t *> {
    static std::string convert(const nanoem_motion_outside_parent_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_accessory_keyframe_t *> {
    static std::string convert(const nanoem_motion_accessory_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_bone_keyframe_t *> {
    static std::string convert(const nanoem_motion_bone_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_camera_keyframe_t *> {
    static std::string convert(const nanoem_motion_camera_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_light_keyframe_t *> {
    static std::string convert(const nanoem_motion_light_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_model_keyframe_t *> {
    static std::string convert(const nanoem_motion_model_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_morph_keyframe_t *> {
    static std::string convert(const nanoem_motion_morph_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_self_shadow_keyframe_t *> {
    static std::string convert(const nanoem_motion_self_shadow_keyframe_t *const &value);
};

} /* namespace Catch */

namespace test {

namespace matcher {

struct EqualsVec2 : Catch::Matchers::Impl::MatcherBase<nanoem::Vector2> {
    EqualsVec2(const nanoem::Vector2 &v);
    EqualsVec2(const EqualsVec2 &v);
    bool match(const nanoem::Vector2 &v) const override;
    std::string describe() const override;
    nanoem::Vector2 m_data;
};

struct EqualsVec3 : Catch::Matchers::Impl::MatcherBase<nanoem::Vector3> {
    EqualsVec3(const nanoem::Vector3 &v);
    EqualsVec3(const EqualsVec3 &v);
    bool match(const nanoem::Vector3 &v) const override;
    std::string describe() const override;
    nanoem::Vector3 m_data;
};

struct EqualsVec4 : Catch::Matchers::Impl::MatcherBase<nanoem::Vector4> {
    EqualsVec4(const nanoem::Vector4 &v);
    EqualsVec4(const EqualsVec4 &v);
    bool match(const nanoem::Vector4 &v) const override;
    std::string describe() const override;
    nanoem::Vector4 m_data;
};

struct EqualsU8Vec4 : Catch::Matchers::Impl::MatcherBase<glm::u8vec4> {
    EqualsU8Vec4(const glm::u8vec4 &v);
    EqualsU8Vec4(const EqualsU8Vec4 &v);
    bool match(const glm::u8vec4 &v) const override;
    std::string describe() const override;
    glm::u8vec4 m_data;
};

struct EqualsQuat : Catch::Matchers::Impl::MatcherBase<nanoem::Quaternion> {
    EqualsQuat(const nanoem::Quaternion &v);
    EqualsQuat(const EqualsQuat &v);
    bool match(const nanoem::Quaternion &v) const override;
    std::string describe() const override;
    nanoem::Quaternion m_data;
};

} /* namespace matcher */

static inline matcher::EqualsVec2
Equals(const nanoem::Vector2 &v)
{
    return matcher::EqualsVec2(v);
}

static inline matcher::EqualsVec3
Equals(const nanoem::Vector3 &v)
{
    return matcher::EqualsVec3(v);
}

static inline matcher::EqualsVec3
EqualsRadians(const nanoem::Vector3 &v)
{
    return matcher::EqualsVec3(glm::radians(v));
}

static inline matcher::EqualsVec4
Equals(const nanoem::Vector4 &v)
{
    return matcher::EqualsVec4(v);
}

static inline matcher::EqualsU8Vec4
Equals(const glm::u8vec4 &v)
{
    return matcher::EqualsU8Vec4(v);
}

static inline matcher::EqualsQuat
Equals(const nanoem::Quaternion &v)
{
    return matcher::EqualsQuat(v);
}

class Application : public nanoem::BaseApplicationService {
public:
    Application(const JSON_Value *root);
    virtual ~Application();

    nanoem::Error lastError() const;
    bool hasAnyError() const;

private:
    void sendEventMessage(const Nanoem__Application__Event *event) override;

    bx::CommandLine m_applicationArguments;
    std::unique_ptr<nanoem::IFileManager> m_defaultFileManager;
    std::unique_ptr<nanoem::internal::StubEventPublisher> m_publisher;
};

class TestScope {
public:
    struct Object {
        Object(nanoem::BaseApplicationService *applicationPtr);
        ~Object();

        nanoem::Accessory *createAccessory(const char *filename = "test.x");
        nanoem::Model *createModel(const char *filename = "test.pmx");
        nanoem::Effect *createBinaryEffect(nanoem::IDrawable *drawable, const char *filename = "test.fxn");
        nanoem::Effect *createSourceEffect(nanoem::IDrawable *drawable, const char *filename, bool inspection = false);
        nanoem::Project::AccessoryList allAccessories();
        nanoem::Project::ModelList allModels();

        nanoem_rsize_t countAllAccessoryKeyframes(const nanoem::Accessory *accessory) const;
        nanoem_rsize_t countAllBoneKeyframes(const nanoem::Model *model) const;
        nanoem_rsize_t countAllModelKeyframes(const nanoem::Model *model) const;
        nanoem_rsize_t countAllMorphKeyframes(const nanoem::Model *model) const;
        nanoem_rsize_t countAllMorphKeyframesByCategory(
            const nanoem::Model *model, nanoem_model_morph_category_t category) const;
        nanoem_frame_index_t motionDuration(const nanoem::IDrawable *drawable) const;
        const nanoem_motion_accessory_keyframe_t *findAccessoryKeyframe(
            const nanoem::Accessory *accessory, nanoem_frame_index_t frameIndex) const;
        const nanoem_motion_bone_keyframe_t *findBoneKeyframe(
            const nanoem::Model *model, const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const;
        const nanoem_motion_bone_keyframe_t *findBoneKeyframe(const nanoem::Model *model) const;
        const nanoem_motion_bone_keyframe_t *findBoneKeyframe(
            const nanoem::Model *model, nanoem_frame_index_t frameIndex) const;
        const nanoem_motion_bone_keyframe_t *findBoneKeyframe(
            const nanoem::Model *model, const nanoem_model_bone_t *bonePtr, nanoem_frame_index_t frameIndex) const;
        const nanoem_motion_morph_keyframe_t *findMorphKeyframe(
            const nanoem::Model *model, const nanoem_unicode_string_t *name, nanoem_frame_index_t frameIndex) const;
        const nanoem_motion_morph_keyframe_t *findMorphKeyframe(
            const nanoem::Model *model, const nanoem_model_morph_t *morphPtr, nanoem_frame_index_t frameIndex) const;
        const nanoem_model_morph_t *findFirstMorph(
            const nanoem::Model *model, nanoem_model_morph_category_t category) const;
        nanoem_f32_t findFirstMorphKeyframeWeight(
            const nanoem::Model *model, nanoem_model_morph_category_t category) const;

        nanoem::Project *withRecoverable(const char *path = "test.redo");
        std::string toString(const nanoem_unicode_string_t *value) const;

        nanoem::BaseApplicationService *m_applicationPtr;
        nanoem::Project *m_project;
    };
    using ProjectPtr = std::unique_ptr<Object>;

    static const nanoem_model_bone_t *findFirstBone(const nanoem::Model *model);
    static const nanoem_model_morph_t *findFirstMorph(const nanoem::Model *model);
    static const nanoem_model_bone_t *findRandomBone(const nanoem::Model *model);
    static const nanoem_model_morph_t *findRandomMorph(const nanoem::Model *model);
    static nanoem::Accessory *createAccessory(nanoem::Project *project, const char *filename);
    static nanoem::Model *createModel(nanoem::Project *project, const char *filename);
    static nanoem::Effect *createBinaryEffect(
        nanoem::Project *project, nanoem::IDrawable *drawable, const char *filename);
    static nanoem::Effect *createSourceEffect(
        nanoem::Project *project, nanoem::IDrawable *drawable, const char *filename, bool inspection = false);
    static nanoem::Project::AccessoryList allAccessories(nanoem::Project *project);
    static nanoem::Project::ModelList allModels(nanoem::Project *project);

    TestScope();
    ~TestScope();

    void recover(nanoem::Project *project, const char *path = "test.redo");
    void deleteFile(const char *path);

    Application *application();
    ProjectPtr createProject();
    bool hasAnyError() const;

private:
    JSON_Value *m_config;
    Application *m_application;
};

using ProjectPtr = TestScope::ProjectPtr;

}; /* namespace test */
