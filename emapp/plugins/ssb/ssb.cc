/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "plugin.pb-c.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifndef NANOEM_DECL_API
#define NANOEM_DECL_API extern "C"
#endif /* NANOEM_DECL_API */
#ifndef APIENTRY
#define APIENTRY
#endif /* APIENTRY */

namespace {

enum nanoem_application_plugin_status_t : int {
    NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON = -3,
    NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION = -2,
    NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT = -1,
    NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS = 0
};

struct SemiStandardBonePlugin {
    using VertexList = std::vector<nanoem_model_vertex_t *>;
    using Callback = std::function<void(const VertexList &vertices, nanoem_status_t &status)>;
    using UIComponentList = std::vector<Nanoem__Application__Plugin__UIComponent *>;

    SemiStandardBonePlugin();
    ~SemiStandardBonePlugin();

    /* plugin interface */
    void setLanguage(nanoem_u32_t value);
    const char *name() const noexcept;
    const char *description() const noexcept;
    const char *version() const noexcept;
    int countAllFunctions() const noexcept;
    const char *functionName(int index) const noexcept;
    void setFunctionIndex(int value);
    void setInputData(const nanoem_u8_t *data, nanoem_u32_t size, nanoem_status_t *status);
    void setComponentLayoutData(const char *name, const nanoem_u8_t *data, nanoem_u32_t size);
    bool loadWindowLayout();
    bool execute();
    nanoem_u32_t outputModelDataSize() const;
    const nanoem_u8_t *outputModelData() const;
    nanoem_u32_t windowLayoutDataSize() const;
    const nanoem_u8_t *windowLayoutData() const;
    const char *failureReason() const noexcept;
    const char *recoverySuggestion() const noexcept;

    /* core */
    void registerCallback(const char *id, const char *textJA, const char *textEN,
        const std::initializer_list<const char *> &required, bool checked, Callback callback);
    void addCheckboxComponent(const char *id, const char *textJA, const char *textEN,
        const std::initializer_list<const char *> &required, bool checked);
    void copyString(char *&dest, const char *text);
    Nanoem__Application__Plugin__UIComponent *createCheckbox(const char *id, const char *text, bool checked);
    void serializeUIComponentList();
    void clearAllUIComponents();
    void destroyUIComponent(Nanoem__Application__Plugin__UIComponent *value);

    /* utility */
    static glm::vec3 boneOrigin(const nanoem_model_bone_t *bone) noexcept;
    static glm::vec3 vertexOrigin(const nanoem_model_vertex_t *vertex) noexcept;
    static int boneIndex(const nanoem_model_bone_t *bone) noexcept;
    static int boneIndex(nanoem_mutable_model_bone_t *bone) noexcept;
    static bool isVertexDedicatedByBone(const nanoem_model_vertex_t *vertex, const nanoem_model_bone_t *bone) noexcept;
    static bool isVertexDedicatedByBone(
        const nanoem_model_vertex_t *vertex, const nanoem_model_bone_t *bone, float threshold) noexcept;
    static bool vertexContainsBone(const nanoem_model_vertex_t *vertex, const nanoem_model_bone_t *bone) noexcept;

    nanoem_model_bone_t *findBone(const char *name) const;
    nanoem_model_label_t *findLabel(const char *name) const;
    nanoem_mutable_model_bone_t *createBone(const char *japanese, const char *english, nanoem_status_t &status);
    nanoem_mutable_model_bone_t *cloneBone(const nanoem_model_bone_t *value, nanoem_status_t &status);
    nanoem_model_label_t *createCenterLabel(nanoem_status_t &status);
    bool findBoneLabel(const nanoem_model_bone_t *base, nanoem_model_label_t *&foundLabel,
        nanoem_model_label_item_t *&foundLabelItem, nanoem_rsize_t &offset) const noexcept;
    bool containsBone(const char *name) const;
    bool isLabelGenerationEnabled() const noexcept;
    bool isCheckboxChecked(const char *id) const noexcept;
    nanoem_unicode_string_t *createUnicodeString(const char *value) const;
    void setBoneName(
        nanoem_mutable_model_bone_t *assignee, const char *japanese, const char *english, nanoem_status_t &status);
    void insertBoneAfter(
        nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status);
    void insertBoneBefore(
        nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status);
    void insertBoneFirst(nanoem_mutable_model_bone_t *assignee, nanoem_status_t &status);
    void insertBoneLast(nanoem_mutable_model_bone_t *assignee, nanoem_status_t &status);
    void insertLabelAfter(
        nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status);
    void insertLabelBefore(
        nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status);
    void insertLabelWithOffset(nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base,
        nanoem_rsize_t offset, nanoem_status_t &status);
    void replaceVertexBone(nanoem_model_vertex_t *assignee, const nanoem_model_bone_t *fromBone,
        nanoem_mutable_model_bone_t *toBone, nanoem_status_t &status);
    void setParentBone(nanoem_model_bone_t *assignee, nanoem_mutable_model_bone_t *value, nanoem_status_t &status);
    void setTargetBone(nanoem_model_bone_t *assignee, nanoem_mutable_model_bone_t *value, nanoem_status_t &status);
    void setRootLabel(nanoem_mutable_model_bone_t *assignee, nanoem_model_bone_t *firstBone, nanoem_status_t &status);
    void replaceBoneInAllRigidBodies(
        const nanoem_model_bone_t *from, nanoem_mutable_model_bone_t *to, nanoem_status_t &status);
    void setRigidBodyBone(
        nanoem_model_rigid_body_t *assignee, nanoem_mutable_model_bone_t *value, nanoem_status_t &status);
    void addTwistBone(const char *nameInJapanese, const char *nameInEnglish, const char *parentTwistBoneName,
        const char *childTwistBoneName, bool enableElbowOffset, const VertexList &vertices, nanoem_status_t &status);
    void addRequiredBoneNotFoundError(const char *name, const char *required);
    const nanoem_model_t *opaque() const;
    nanoem_model_t *opaque();

    std::unordered_map<std::string, Callback> m_callbacks;
    UIComponentList m_checkboxes;
    UIComponentList m_materials;
    std::vector<nanoem_u8_t> m_windowLayoutData;
    std::string m_failureReason;
    std::string m_recoverySuggestion;
    nanoem_unicode_string_factory_t *m_factory = nullptr;
    nanoem_mutable_model_t *m_mutableModel = nullptr;
    nanoem_mutable_buffer_t *m_mutableBuffer = nullptr;
    nanoem_buffer_t *m_buffer = nullptr;
    nanoem_u32_t m_language = 0;
};

SemiStandardBonePlugin::SemiStandardBonePlugin()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_factory = nanoemUnicodeStringFactoryCreate(&status);
}

SemiStandardBonePlugin::~SemiStandardBonePlugin()
{
    clearAllUIComponents();
    nanoemBufferDestroy(m_buffer);
    nanoemMutableBufferDestroy(m_mutableBuffer);
    nanoemMutableModelDestroy(m_mutableModel);
    nanoemUnicodeStringFactoryDestroy(m_factory);
}

void
SemiStandardBonePlugin::setLanguage(nanoem_u32_t value)
{
    m_language = value;
}

const char *
SemiStandardBonePlugin::name() const noexcept
{
    return m_language == 0 ? "準標準ボーンプラグイン" : "Semi-Standard Bone Plugin";
}

const char *
SemiStandardBonePlugin::description() const noexcept
{
    return name();
}

const char *
SemiStandardBonePlugin::version() const noexcept
{
    return "1.3.0";
}

int
SemiStandardBonePlugin::countAllFunctions() const noexcept
{
    return 1;
}

const char *
SemiStandardBonePlugin::functionName(int index) const noexcept
{
    const char *text = nullptr;
    if (index == 0) {
        text = m_language == 0 ? "設定と実行" : "Configure and Execute";
    }
    return text;
}

void
SemiStandardBonePlugin::setFunctionIndex(int /* value */)
{
}

void
SemiStandardBonePlugin::setInputData(const nanoem_u8_t *data, nanoem_u32_t size, nanoem_status_t *status)
{
    nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, status);
    nanoemMutableModelDestroy(m_mutableModel);
    m_mutableModel = nanoemMutableModelCreate(m_factory, status);
    nanoemModelLoadFromBuffer(nanoemMutableModelGetOriginObject(m_mutableModel), buffer, status);
    nanoemMutableModelResetAllocationSize(m_mutableModel);
    nanoemBufferDestroy(buffer);
    if (*status != NANOEM_STATUS_SUCCESS) {
        m_failureReason = "Invalid Model Data";
    }
}

void
SemiStandardBonePlugin::setComponentLayoutData(const char *name, const nanoem_u8_t *data, nanoem_u32_t size)
{
    for (auto item : m_checkboxes) {
        if (item->type_case == NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX &&
            strcmp(item->check_box->id, name) == 0) {
            if (auto component = nanoem__application__plugin__uicomponent__unpack(nullptr, size, data)) {
                item->check_box->value = component->check_box->value;
                nanoem__application__plugin__uicomponent__free_unpacked(component, nullptr);
                break;
            }
        }
    }
    for (auto item : m_materials) {
        if (item->type_case == NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX &&
            strcmp(item->check_box->id, name) == 0) {
            if (auto component = nanoem__application__plugin__uicomponent__unpack(nullptr, size, data)) {
                item->check_box->value = component->check_box->value;
                nanoem__application__plugin__uicomponent__free_unpacked(component, nullptr);
                break;
            }
        }
    }
}

bool
SemiStandardBonePlugin::loadWindowLayout()
{
    clearAllUIComponents();
    registerCallback("armTwistBone", "腕捩れボーン *", "Arm Twist Bone *", { "右腕", "右ひじ", "左腕", "左ひじ" }, true,
        [this](const VertexList &vertices, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_armTwistBoneNameInJapanese;
                const char *m_armTwistBoneNameInEnglish;
                const char *m_armBoneName;
                const char *m_elbowBoneName;
            } tuples[] = {
                { "右腕捩", "arm twist_R", "右腕", "右ひじ" },
                { "左腕捩", "arm twist_L", "左腕", "左ひじ" },
            };
            for (auto &tuple : tuples) {
                addTwistBone(tuple.m_armTwistBoneNameInJapanese, tuple.m_armTwistBoneNameInEnglish, tuple.m_armBoneName,
                    tuple.m_elbowBoneName, true, vertices, status);
            }
        });
    addCheckboxComponent("enableElbowOffset", "回転自動補正", "Enable Elbow Offset", {}, true);
    registerCallback("wristTwistBone", "手捩れボーン *", "Wrist Twist Bone *",
        { "右ひじ", "右手首", "左ひじ", "左手首" }, true, [this](const VertexList &vertices, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_wristTwistBoneNameInJapanese;
                const char *m_wristTwistBoneNameInEnglish;
                const char *m_elbowBoneName;
                const char *m_wristBoneName;
            } tuples[] = {
                { "右手捩", "wrist twist_R", "右ひじ", "右手首" },
                { "左手捩", "wrist twist_L", "左ひじ", "左手首" },
            };
            for (auto &tuple : tuples) {
                addTwistBone(tuple.m_wristTwistBoneNameInJapanese, tuple.m_wristTwistBoneNameInEnglish,
                    tuple.m_elbowBoneName, tuple.m_wristBoneName, false, vertices, status);
            }
        });
    registerCallback("chestBone", "上半身2ボーン *", "Upper Body 2 Bone *", { "上半身", "首" }, true,
        [this](const VertexList &vertices, nanoem_status_t &status) {
            if (!containsBone("上半身2")) {
                auto spineBone = findBone("上半身");
                auto neckBone = findBone("首");
                if (spineBone && neckBone) {
                    const glm::vec3 upperBodyOrigin(boneOrigin(spineBone)), neckBoneOrigin(boneOrigin(neckBone)),
                        chestBoneOrigin(glm::mix(upperBodyOrigin, neckBoneOrigin, 0.35f));
                    auto chestBone = createBone("上半身2", "upper body2", status);
                    nanoemMutableModelBoneSetVisible(chestBone, true);
                    nanoemMutableModelBoneSetRotateable(chestBone, true);
                    nanoemMutableModelBoneSetUserHandleable(chestBone, true);
                    nanoemMutableModelBoneSetOrigin(chestBone, glm::value_ptr(glm::vec4(chestBoneOrigin, 0)));
                    nanoemMutableModelBoneSetDestinationOrigin(
                        chestBone, glm::value_ptr(glm::vec4((neckBoneOrigin - chestBoneOrigin) * 0.8f, 0)));
                    nanoemMutableModelBoneSetParentBoneObject(chestBone, spineBone);
                    nanoemMutableModelBoneSetStageIndex(chestBone, nanoemModelBoneGetStageIndex(spineBone));
                    insertBoneAfter(chestBone, spineBone, status);
                    setTargetBone(spineBone, chestBone, status);
                    nanoem_rsize_t numBones, numBodies;
                    VertexList spineVertices;
                    for (auto item : vertices) {
                        const glm::vec3 origin(vertexOrigin(item));
                        if (isVertexDedicatedByBone(item, spineBone)) {
                            spineVertices.push_back(item);
                        }
                        else if (origin.y > chestBoneOrigin.y) {
                            replaceVertexBone(item, spineBone, chestBone, status);
                        }
                    }
                    for (auto item : spineVertices) {
                        glm::vec3 distance(vertexOrigin(item) - chestBoneOrigin);
                        if (distance.z > 0) {
                            distance.y += distance.z * 0.5f;
                        }
                        float v = distance.y / (neckBoneOrigin.y - chestBoneOrigin.y);
                        nanoem_mutable_model_vertex_t *vertex =
                            nanoemMutableModelVertexCreateAsReference(item, &status);
                        if (v < -0.35f) {
                            nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                            nanoemMutableModelVertexSetBoneObject(vertex, spineBone, 0);
                        }
                        else if (v > 0.35f) {
                            nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF1);
                            nanoemMutableModelVertexSetBoneObject(
                                vertex, nanoemMutableModelBoneGetOriginObject(chestBone), 0);
                        }
                        else {
                            float weight = int(((v + 0.35f) / 0.7f) * 100.0f) * 0.01f;
                            nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
                            nanoemMutableModelVertexSetBoneObject(
                                vertex, nanoemMutableModelBoneGetOriginObject(chestBone), 0);
                            nanoemMutableModelVertexSetBoneWeight(vertex, weight, 0);
                            nanoemMutableModelVertexSetBoneObject(vertex, spineBone, 1);
                            nanoemMutableModelVertexSetBoneWeight(vertex, 1.0f - weight, 1);
                        }
                        nanoemMutableModelVertexDestroy(vertex);
                    }
                    auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
                    for (nanoem_rsize_t i = 0; i < numBones; i++) {
                        nanoem_model_bone_t *innerBone = bones[i];
                        if (nanoemModelBoneGetParentBoneObject(innerBone) == spineBone) {
                            setParentBone(innerBone, chestBone, status);
                        }
                    }
                    auto bodies = nanoemModelGetAllRigidBodyObjects(opaque(), &numBodies);
                    for (nanoem_rsize_t i = 0; i < numBodies; i++) {
                        nanoem_model_rigid_body_t *innerBody = bodies[i];
                        if (nanoemModelRigidBodyGetBoneObject(innerBody) == spineBone) {
                            setRigidBodyBone(innerBody, chestBone, status);
                        }
                    }
                    if (isLabelGenerationEnabled()) {
                        insertLabelAfter(chestBone, spineBone, status);
                    }
                    nanoemMutableModelBoneDestroy(chestBone);
                }
                else {
                    if (!spineBone) {
                        addRequiredBoneNotFoundError("上半身2", "上半身");
                    }
                    if (!neckBone) {
                        addRequiredBoneNotFoundError("上半身2", "首");
                    }
                }
            }
        });
    registerCallback("waistBone", "腰ボーン", "Waist Bone", { "下半身", "右足", "左足" }, true,
        [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            if (!containsBone("腰")) {
                auto underBodyBone = findBone("下半身");
                auto underBodyParentBone = nanoemModelBoneGetParentBoneObject(underBodyBone);
                auto rightLegBone = findBone("右足");
                if (underBodyParentBone && underBodyBone && rightLegBone) {
                    const glm::vec3 lowerBodyBoneOrigin(boneOrigin(underBodyBone)),
                        rightLegBoneOrigin(boneOrigin(rightLegBone)),
                        waistBoneOrigin(0, glm::mix(lowerBodyBoneOrigin.y, rightLegBoneOrigin.y, 0.6f),
                            lowerBodyBoneOrigin.y * 0.02f);
                    auto waistBone = createBone("腰", "waist", status);
                    nanoemMutableModelBoneSetVisible(waistBone, true);
                    nanoemMutableModelBoneSetRotateable(waistBone, true);
                    nanoemMutableModelBoneSetUserHandleable(waistBone, true);
                    nanoemMutableModelBoneSetParentBoneObject(waistBone, underBodyParentBone);
                    nanoemMutableModelBoneSetOrigin(waistBone, glm::value_ptr(glm::vec4(waistBoneOrigin, 0)));
                    nanoemMutableModelBoneSetDestinationOrigin(
                        waistBone, glm::value_ptr(glm::vec4((lowerBodyBoneOrigin - waistBoneOrigin) * 0.8f, 0)));
                    nanoemMutableModelBoneSetStageIndex(waistBone, nanoemModelBoneGetStageIndex(underBodyParentBone));
                    insertBoneAfter(waistBone, underBodyParentBone, status);
                    nanoem_rsize_t numBones;
                    auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
                    if (auto s = createUnicodeString("センター先")) {
                        for (nanoem_rsize_t i = 0; i < numBones; i++) {
                            nanoem_model_bone_t *innerBone = bones[i];
                            auto name = nanoemModelBoneGetName(innerBone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                            if (nanoemModelBoneGetParentBoneObject(innerBone) == underBodyParentBone &&
                                nanoemUnicodeStringFactoryCompareString(m_factory, name, s) != 0) {
                                setParentBone(innerBone, waistBone, status);
                            }
                        }
                        nanoemUnicodeStringFactoryDestroyString(m_factory, s);
                    }
                    if (isLabelGenerationEnabled()) {
                        nanoem_model_label_t *foundLabel;
                        nanoem_model_label_item_t *foundLabelItem;
                        nanoem_rsize_t offset;
                        if (findBoneLabel(underBodyParentBone, foundLabel, foundLabelItem, offset)) {
                            insertLabelAfter(waistBone, underBodyParentBone, status);
                        }
                        else {
                            auto label = createCenterLabel(status);
                            auto mutableLabel = nanoemMutableModelLabelCreateAsReference(label, &status);
                            auto item = nanoemMutableModelLabelItemCreateFromBoneObject(
                                mutableLabel, nanoemMutableModelBoneGetOriginObject(waistBone), &status);
                            nanoemMutableModelLabelInsertItemObject(mutableLabel, item, -1, &status);
                            nanoemMutableModelLabelItemDestroy(item);
                            nanoemMutableModelLabelDestroy(mutableLabel);
                        }
                    }
                    const struct Tuple {
                        const char *m_nameInJapanese;
                        const char *m_target;
                    } tuples[] = {
                        { "腰キャンセル右", "右足" },
                        { "腰キャンセル左", "左足" },
                    };
                    for (auto &item : tuples) {
                        auto footBone = findBone(item.m_target);
                        if (!containsBone(item.m_nameInJapanese)) {
                            auto waistCancelBone = createBone(item.m_nameInJapanese, "", status);
                            nanoemMutableModelBoneSetRotateable(waistCancelBone, true);
                            nanoemMutableModelBoneSetUserHandleable(waistCancelBone, true);
                            nanoemMutableModelBoneSetOrigin(waistCancelBone, nanoemModelBoneGetOrigin(footBone));
                            nanoemMutableModelBoneSetParentBoneObject(
                                waistCancelBone, nanoemModelBoneGetParentBoneObject(footBone));
                            nanoemMutableModelBoneSetInherentOrientationEnabled(waistCancelBone, true);
                            nanoemMutableModelBoneSetInherentParentBoneObject(
                                waistCancelBone, nanoemMutableModelBoneGetOriginObject(waistBone));
                            nanoemMutableModelBoneSetInherentCoefficient(waistCancelBone, -1);
                            insertBoneBefore(waistCancelBone, footBone, status);
                            setParentBone(footBone, waistCancelBone, status);
                            nanoemMutableModelBoneDestroy(waistCancelBone);
                        }
                    }
                    nanoemMutableModelBoneDestroy(waistBone);
                }
                else {
                    if (!underBodyBone) {
                        addRequiredBoneNotFoundError("腰", "下半身");
                    }
                    if (!underBodyParentBone) {
                        m_failureReason.append(m_language == 0
                                ? "「腰」ボーンに必要な「下半身」の親ボーンがありません\n"
                                : "The required parent bone of \"下半身\" does not find to create the bone \"腰\"\n");
                    }
                    if (!rightLegBone) {
                        addRequiredBoneNotFoundError("腰", "右足");
                    }
                }
            }
        });
    registerCallback("legIKParent", "足IK親", "Parent of Leg IK", { "右足ＩＫ", "左足ＩＫ" }, true,
        [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_nameInJapanese;
                const char *m_nameInEnglish;
                const char *m_target;
            } tuples[] = {
                { "右足IK親", "leg IKP_R", "右足ＩＫ" },
                { "左足IK親", "leg IKP_L", "左足ＩＫ" },
            };
            for (auto &item : tuples) {
                if (!containsBone(item.m_nameInJapanese)) {
                    if (auto legIKBone = findBone(item.m_target)) {
                        auto legIKParentBone = createBone(item.m_nameInJapanese, item.m_nameInEnglish, status);
                        nanoemMutableModelBoneSetVisible(legIKParentBone, true);
                        nanoemMutableModelBoneSetMovable(legIKParentBone, true);
                        nanoemMutableModelBoneSetRotateable(legIKParentBone, true);
                        nanoemMutableModelBoneSetUserHandleable(legIKParentBone, true);
                        nanoemMutableModelBoneSetOrigin(
                            legIKParentBone, glm::value_ptr(glm::vec4(boneOrigin(legIKBone) * glm::vec3(1, 0, 1), 0)));
                        nanoemMutableModelBoneSetParentBoneObject(
                            legIKParentBone, nanoemModelBoneGetParentBoneObject(legIKBone));
                        nanoemMutableModelBoneSetTargetBoneObject(legIKParentBone, legIKBone);
                        insertBoneBefore(legIKParentBone, legIKBone, status);
                        setParentBone(legIKBone, legIKParentBone, status);
                        if (isLabelGenerationEnabled()) {
                            insertLabelBefore(legIKParentBone, legIKBone, status);
                        }
                        nanoemMutableModelBoneDestroy(legIKParentBone);
                    }
                    else {
                        addRequiredBoneNotFoundError(item.m_nameInJapanese, item.m_target);
                    }
                }
            }
        });
    registerCallback("test10", "足先EX *", "Toes EX *",
        { "右足", "右ひざ", "右足首", "右つま先ＩＫ", "左足", "左ひざ", "左足首", "左つま先ＩＫ" }, true,
        [this](const VertexList &vertices, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_toesExBoneNameInJapanese;
                const char *m_toesExBoneNameInEnglish;
                const char *m_footBoneName;
                const char *m_kneeBoneName;
                const char *m_ankleBoneName;
                const char *m_toesIKBoneName;
            } tuples[] = { { "右足先EX", "toe2_R", "右足", "右ひざ", "右足首", "右つま先ＩＫ" },
                { "左足先EX", "toe2_L", "左足", "左ひざ", "左足首", "左つま先ＩＫ" } };
            bool controllable = false;
            auto createInherentBone = [this, controllable, &status](const nanoem_model_bone_t *bone) {
                auto newBone = cloneBone(bone, status);
                nanoemMutableModelBoneSetVisible(newBone, controllable);
                nanoemMutableModelBoneSetUserHandleable(newBone, controllable);
                nanoemMutableModelBoneSetInherentOrientationEnabled(newBone, true);
                nanoemMutableModelBoneSetInherentCoefficient(newBone, 1);
                nanoemMutableModelBoneSetInherentParentBoneObject(newBone, bone);
                nanoemMutableModelBoneSetTargetBoneObject(newBone, nullptr);
                nanoemMutableModelBoneSetStageIndex(newBone, nanoemModelBoneGetStageIndex(bone) + 1);
                nanoem_rsize_t length;
                for (nanoem_rsize_t i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
                    auto language = static_cast<nanoem_language_type_t>(i);
                    if (auto ptr = nanoemUnicodeStringFactoryGetByteArray(
                            m_factory, nanoemModelBoneGetName(bone, language), &length, &status)) {
                        std::string s(reinterpret_cast<const char *>(ptr), length);
                        nanoemUnicodeStringFactoryDestroyByteArray(m_factory, ptr);
                        s.append("D");
                        if (auto u = createUnicodeString(s.c_str())) {
                            nanoemMutableModelBoneSetName(newBone, u, language, &status);
                            nanoemUnicodeStringFactoryDestroyString(m_factory, u);
                        }
                    }
                }
                return newBone;
            };
            for (auto &item : tuples) {
                auto footBone = findBone(item.m_footBoneName);
                auto kneeBone = findBone(item.m_kneeBoneName);
                auto ankleBone = findBone(item.m_ankleBoneName);
                auto toesIKBone = findBone(item.m_toesIKBoneName);
                if (containsBone(item.m_toesExBoneNameInJapanese)) {
                    continue;
                }
                else if (footBone && kneeBone && ankleBone && toesIKBone) {
                    const glm::vec3 ankleBoneOrigin(boneOrigin(ankleBone)), toesIKBoneOrigin(boneOrigin(toesIKBone)),
                        toesExBoneOrigin(glm::mix(ankleBoneOrigin, toesIKBoneOrigin, 2.0f / 3.0f));
                    auto newLegBone = createInherentBone(footBone);
                    auto newKneeBone = createInherentBone(kneeBone);
                    auto newAnkleBone = createInherentBone(ankleBone);
                    auto toesExBone =
                        createBone(item.m_toesExBoneNameInJapanese, item.m_toesExBoneNameInEnglish, status);
                    nanoemMutableModelBoneSetVisible(toesExBone, true);
                    nanoemMutableModelBoneSetRotateable(toesExBone, true);
                    nanoemMutableModelBoneSetUserHandleable(toesExBone, true);
                    nanoemMutableModelBoneSetOrigin(toesExBone, glm::value_ptr(glm::vec4(toesExBoneOrigin, 0)));
                    nanoemMutableModelBoneSetDestinationOrigin(toesExBone, glm::value_ptr(glm::vec4(0, 0, -1, 0)));
                    nanoemMutableModelBoneSetStageIndex(
                        toesExBone, nanoemModelBoneGetStageIndex(nanoemMutableModelBoneGetOriginObject(newAnkleBone)));
                    insertBoneLast(newLegBone, status);
                    insertBoneLast(newKneeBone, status);
                    insertBoneLast(newAnkleBone, status);
                    insertBoneLast(toesExBone, status);
                    nanoemMutableModelBoneSetParentBoneObject(
                        newKneeBone, nanoemMutableModelBoneGetOriginObject(newLegBone));
                    nanoemMutableModelBoneSetParentBoneObject(
                        newAnkleBone, nanoemMutableModelBoneGetOriginObject(newKneeBone));
                    nanoemMutableModelBoneSetParentBoneObject(
                        toesExBone, nanoemMutableModelBoneGetOriginObject(newAnkleBone));
                    for (auto item : vertices) {
                        if (isVertexDedicatedByBone(item, ankleBone)) {
                            nanoem_mutable_model_vertex_t *vertex =
                                nanoemMutableModelVertexCreateAsReference(item, &status);
                            float ankleBoneZ = nanoemModelBoneGetOrigin(ankleBone)[2],
                                  center = (nanoemModelVertexGetOrigin(item)[2] - ankleBoneZ) /
                                (toesExBoneOrigin.z - ankleBoneZ),
                                  weight = glm::clamp((center - 0.75f) * 2.0f, 0.0f, 1.0f);
                            nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
                            nanoemMutableModelVertexSetBoneObject(
                                vertex, nanoemMutableModelBoneGetOriginObject(toesExBone), 0);
                            nanoemMutableModelVertexSetBoneWeight(vertex, weight, 0);
                            nanoemMutableModelVertexSetBoneObject(vertex, ankleBone, 1);
                            nanoemMutableModelVertexSetBoneWeight(vertex, 1.0f - weight, 1);
                            nanoemMutableModelVertexDestroy(vertex);
                        }
                        replaceVertexBone(item, footBone, newLegBone, status);
                        replaceVertexBone(item, kneeBone, newKneeBone, status);
                        replaceVertexBone(item, ankleBone, newAnkleBone, status);
                    }
                    replaceBoneInAllRigidBodies(footBone, newLegBone, status);
                    replaceBoneInAllRigidBodies(kneeBone, newKneeBone, status);
                    replaceBoneInAllRigidBodies(ankleBone, newAnkleBone, status);
                    if (isLabelGenerationEnabled()) {
                        insertLabelAfter(toesExBone, ankleBone, status);
                        if (isCheckboxChecked("enableLegDControllable")) {
                            insertLabelAfter(newAnkleBone, ankleBone, status);
                            insertLabelAfter(newKneeBone, ankleBone, status);
                            insertLabelAfter(newLegBone, ankleBone, status);
                        }
                    }
                    nanoemMutableModelBoneDestroy(newLegBone);
                    nanoemMutableModelBoneDestroy(newKneeBone);
                    nanoemMutableModelBoneDestroy(newAnkleBone);
                    nanoemMutableModelBoneDestroy(toesExBone);
                }
                else {
                    if (!footBone) {
                        addRequiredBoneNotFoundError(item.m_toesExBoneNameInJapanese, item.m_footBoneName);
                    }
                    if (!kneeBone) {
                        addRequiredBoneNotFoundError(item.m_toesExBoneNameInJapanese, item.m_kneeBoneName);
                    }
                    if (!ankleBone) {
                        addRequiredBoneNotFoundError(item.m_toesExBoneNameInJapanese, item.m_ankleBoneName);
                    }
                    if (!toesIKBone) {
                        addRequiredBoneNotFoundError(item.m_toesExBoneNameInJapanese, item.m_toesIKBoneName);
                    }
                }
            }
        });
    addCheckboxComponent("enableLegDControllable", "足Dボーンを操作可能に", "Make Leg-D Bone Controllable", {}, false);
    registerCallback("dummyBone", "手持ちアクセサリ用ダミー", "Dummy Bone for Handheld Accessory",
        { "右手首", "右中指１", "左手首", "左中指１" }, true,
        [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_dummyBoneNameInJapanese;
                const char *m_dummyBoneNameInEnglish;
                const char *m_wristBoneName;
                const char *m_middleFingerBoneName;
                std::function<glm::vec2(const glm::vec2 &)> m_callback;
            } tuples[] = { { "右ダミー", "dummy_R", "右手首", "右中指１",
                               [](const glm::vec2 &v) { return glm::vec2(-v.y, v.x); } },
                { "左ダミー", "dummy_L", "左手首", "左中指１",
                    [](const glm::vec2 &v) { return glm::vec2(v.y, -v.x); } } };
            for (auto &item : tuples) {
                auto wristBone = findBone(item.m_wristBoneName);
                auto middleFingerBone = findBone(item.m_middleFingerBoneName);
                if (containsBone(item.m_dummyBoneNameInJapanese)) {
                    continue;
                }
                else if (wristBone && middleFingerBone) {
                    const glm::vec3 wristBoneOrigin(boneOrigin(wristBone)),
                        middleFingerBoneOrigin(glm::vec2(boneOrigin(middleFingerBone)), wristBoneOrigin.z),
                        center((wristBoneOrigin + middleFingerBoneOrigin) * 0.5f);
                    const glm::vec2 normalized(glm::normalize(middleFingerBoneOrigin - wristBoneOrigin)),
                        result(item.m_callback(normalized));
                    auto dummyBone = createBone(item.m_dummyBoneNameInJapanese, item.m_dummyBoneNameInEnglish, status);
                    nanoemMutableModelBoneSetVisible(dummyBone, true);
                    nanoemMutableModelBoneSetMovable(dummyBone, true);
                    nanoemMutableModelBoneSetRotateable(dummyBone, true);
                    nanoemMutableModelBoneSetUserHandleable(dummyBone, true);
                    nanoemMutableModelBoneSetOrigin(
                        dummyBone, glm::value_ptr(glm::vec4(center + glm::vec3(result * 0.25f, 0), 0)));
                    nanoemMutableModelBoneSetDestinationOrigin(
                        dummyBone, glm::value_ptr(glm::vec4(result * 1.2f, 0, 0)));
                    nanoemMutableModelBoneSetParentBoneObject(dummyBone, wristBone);
                    insertBoneAfter(dummyBone, wristBone, status);
                    if (isLabelGenerationEnabled()) {
                        insertLabelAfter(dummyBone, wristBone, status);
                    }
                    nanoemMutableModelBoneDestroy(dummyBone);
                }
                else {
                    if (!wristBone) {
                        addRequiredBoneNotFoundError(item.m_dummyBoneNameInJapanese, item.m_wristBoneName);
                    }
                    if (!middleFingerBone) {
                        addRequiredBoneNotFoundError(item.m_dummyBoneNameInJapanese, item.m_middleFingerBoneName);
                    }
                }
            }
        });
    registerCallback("shoulderCancelBone", "肩キャンセルボーン", "Shoulder Cancel Bone",
        { "右肩", "右腕", "左肩", "左腕" }, true, [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_parentBoneNameInJapanese;
                const char *m_cancelBoneNameInJapanese;
                const char *m_parentBoneNameInEnglish;
                const char *m_shoulderBoneName;
                const char *m_armBoneName;
            } tuples[] = {
                { "右肩P", "右肩C", "shoulderP_R", "右肩", "右腕" },
                { "左肩P", "左肩C", "shoulderP_L", "左肩", "左腕" },
            };
            for (auto &item : tuples) {
                auto shoulderBone = findBone(item.m_shoulderBoneName);
                auto armBone = findBone(item.m_armBoneName);
                if (containsBone(item.m_parentBoneNameInJapanese)) {
                    continue;
                }
                else if (shoulderBone && armBone) {
                    auto parentShoulderBone =
                        createBone(item.m_parentBoneNameInJapanese, item.m_parentBoneNameInEnglish, status);
                    nanoemMutableModelBoneSetRotateable(parentShoulderBone, true);
                    nanoemMutableModelBoneSetVisible(parentShoulderBone, true);
                    nanoemMutableModelBoneSetUserHandleable(parentShoulderBone, true);
                    nanoemMutableModelBoneSetOrigin(parentShoulderBone, nanoemModelBoneGetOrigin(shoulderBone));
                    nanoemMutableModelBoneSetParentBoneObject(
                        parentShoulderBone, nanoemModelBoneGetParentBoneObject(shoulderBone));
                    insertBoneBefore(parentShoulderBone, shoulderBone, status);
                    auto cancelShoulderBone = createBone(item.m_cancelBoneNameInJapanese, "", status);
                    nanoemMutableModelBoneSetRotateable(cancelShoulderBone, true);
                    nanoemMutableModelBoneSetUserHandleable(cancelShoulderBone, true);
                    nanoemMutableModelBoneSetInherentParentBoneObject(
                        cancelShoulderBone, nanoemMutableModelBoneGetOriginObject(parentShoulderBone));
                    nanoemMutableModelBoneSetInherentOrientationEnabled(cancelShoulderBone, true);
                    nanoemMutableModelBoneSetInherentCoefficient(cancelShoulderBone, -1);
                    nanoemMutableModelBoneSetOrigin(cancelShoulderBone, nanoemModelBoneGetOrigin(armBone));
                    nanoemMutableModelBoneSetParentBoneObject(cancelShoulderBone, shoulderBone);
                    insertBoneAfter(cancelShoulderBone, shoulderBone, status);
                    setParentBone(shoulderBone, parentShoulderBone, status);
                    setParentBone(armBone, cancelShoulderBone, status);
                    if (isLabelGenerationEnabled()) {
                        insertLabelBefore(parentShoulderBone, shoulderBone, status);
                    }
                    nanoemMutableModelBoneDestroy(parentShoulderBone);
                    nanoemMutableModelBoneDestroy(cancelShoulderBone);
                }
                else {
                    if (!shoulderBone) {
                        addRequiredBoneNotFoundError(item.m_parentBoneNameInJapanese, item.m_shoulderBoneName);
                    }
                    if (!armBone) {
                        addRequiredBoneNotFoundError(item.m_parentBoneNameInJapanese, item.m_armBoneName);
                    }
                }
            }
        });
    registerCallback("thumbProximalBone", "親指０ボーン *", "Thumb Proximal Bone *",
        { "右手首", "右親指１", "左手首", "左親指１" }, true,
        [this](const VertexList &vertices, nanoem_status_t &status) {
            const struct Tuple {
                const char *m_thumbProximalBoneNameInJapanese;
                const char *m_thumbProximalBoneNameInEnglish;
                const char *m_wristBoneName;
                const char *m_thumbIntermediateBoneName;
                const char *m_thumbDistalBoneName;
                const char *m_indexIntermediateBoneName;
                const glm::vec3 m_direction;
                std::function<glm::vec3(const glm::vec3 &)> m_callback;
            } tuples[] = {
                { "右親指０", "thumb0_R", "右手首", "右親指１", "右親指２", "右人指１",
                    glm::normalize(glm::vec3(1, -1, 0)), [](const glm::vec3 &v) { return glm::vec3(-v.y, v.x, 0); } },
                { "左親指０", "thumb0_L", "左手首", "左親指１", "左親指２", "左人指１",
                    glm::normalize(glm::vec3(-1, -1, 0)), [](const glm::vec3 &v) { return glm::vec3(v.y, -v.x, 0); } },
            };
            for (auto &tuple : tuples) {
                auto wristBone = findBone(tuple.m_wristBoneName);
                auto thumbIntermediateBone = findBone(tuple.m_thumbIntermediateBoneName);
                auto thumbDistalBone = findBone(tuple.m_thumbDistalBoneName);
                auto indexIntermediateBone = findBone(tuple.m_indexIntermediateBoneName);
                if (containsBone(tuple.m_thumbProximalBoneNameInJapanese)) {
                    continue;
                }
                else if (wristBone && thumbIntermediateBone) {
                    const glm::vec3 wristBoneOrigin(boneOrigin(wristBone)),
                        thumbIntermediateBoneOrigin(boneOrigin(thumbIntermediateBone)),
                        thumbProximalBoneOrigin(glm::mix(wristBoneOrigin, thumbIntermediateBoneOrigin, 2.0f / 3.0f));
                    auto thumbProximalBone = createBone(
                        tuple.m_thumbProximalBoneNameInJapanese, tuple.m_thumbProximalBoneNameInEnglish, status);
                    nanoemMutableModelBoneSetRotateable(thumbProximalBone, true);
                    nanoemMutableModelBoneSetVisible(thumbProximalBone, true);
                    nanoemMutableModelBoneSetUserHandleable(thumbProximalBone, true);
                    nanoemMutableModelBoneSetOrigin(
                        thumbProximalBone, glm::value_ptr(glm::vec4(thumbProximalBoneOrigin, 0)));
                    nanoemMutableModelBoneSetParentBoneObject(thumbProximalBone, wristBone);
                    nanoemMutableModelBoneSetTargetBoneObject(thumbProximalBone, thumbIntermediateBone);
                    insertBoneBefore(thumbProximalBone, thumbIntermediateBone, status);
                    setParentBone(thumbIntermediateBone, thumbProximalBone, status);
                    if (isLabelGenerationEnabled()) {
                        insertLabelBefore(thumbProximalBone, thumbIntermediateBone, status);
                    }
                    const nanoem_f32_t length = glm::length(thumbIntermediateBoneOrigin - thumbProximalBoneOrigin);
                    for (auto item : vertices) {
                        if (vertexContainsBone(item, wristBone) || vertexContainsBone(item, thumbIntermediateBone)) {
                            const glm::vec3 distance(
                                vertexOrigin(item) - (thumbProximalBoneOrigin + thumbIntermediateBoneOrigin) * 0.5f);
                            float weight =
                                glm::length(distance - glm::dot(distance, tuple.m_direction) * tuple.m_direction);
                            weight /= length * 1.4f;
                            if (weight < 1.0f) {
                                weight = glm::clamp((1.0f - weight) * 1.3f, 0.0f, 1.0f);
                                auto vertex = nanoemMutableModelVertexCreateAsReference(item, &status);
                                if (isVertexDedicatedByBone(item, wristBone)) {
                                    nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
                                    nanoemMutableModelVertexSetBoneObject(
                                        vertex, nanoemMutableModelBoneGetOriginObject(thumbProximalBone), 0);
                                    nanoemMutableModelVertexSetBoneWeight(vertex, weight, 0);
                                    nanoemMutableModelVertexSetBoneObject(vertex, wristBone, 1);
                                    nanoemMutableModelVertexSetBoneWeight(vertex, 1.0f - weight, 1);
                                }
                                else if (isVertexDedicatedByBone(item, thumbIntermediateBone)) {
                                    nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
                                    nanoemMutableModelVertexSetBoneObject(
                                        vertex, nanoemMutableModelBoneGetOriginObject(thumbProximalBone), 0);
                                    nanoemMutableModelVertexSetBoneWeight(vertex, weight, 0);
                                    nanoemMutableModelVertexSetBoneObject(vertex, thumbIntermediateBone, 1);
                                    nanoemMutableModelVertexSetBoneWeight(vertex, 1.0f - weight, 1);
                                }
                                else {
                                    for (nanoem_rsize_t i = 0; i < 4; i++) {
                                        if (nanoemModelVertexGetBoneObject(item, i) == wristBone) {
                                            nanoemMutableModelVertexSetBoneObject(
                                                vertex, nanoemMutableModelBoneGetOriginObject(thumbProximalBone), i);
                                            if (nanoemModelVertexGetBoneWeight(item, i) < weight) {
                                                nanoemMutableModelVertexSetBoneWeight(vertex, weight, i);
                                                nanoem_f32_t sum = 0;
                                                for (nanoem_rsize_t j = 0; j < 4; j++) {
                                                    if (i != j) {
                                                        sum += nanoemModelVertexGetBoneWeight(item, j);
                                                    }
                                                }
                                                if (sum > 0) {
                                                    for (nanoem_rsize_t j = 0; j < 4; j++) {
                                                        if (i != j) {
                                                            float w = nanoemModelVertexGetBoneWeight(item, i);
                                                            nanoemMutableModelVertexSetBoneWeight(
                                                                vertex, (w * (1.0f - weight)) / sum, j);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                nanoemMutableModelVertexDestroy(vertex);
                            }
                        }
                    }
                    nanoemMutableModelBoneDestroy(thumbProximalBone);
                }
                else {
                    if (!wristBone) {
                        addRequiredBoneNotFoundError(tuple.m_thumbProximalBoneNameInJapanese, tuple.m_wristBoneName);
                    }
                    if (!thumbIntermediateBone) {
                        addRequiredBoneNotFoundError(
                            tuple.m_thumbProximalBoneNameInJapanese, tuple.m_thumbIntermediateBoneName);
                    }
                }
                if (isCheckboxChecked("enableThumbLocalAxes") &&
                    containsBone(tuple.m_thumbProximalBoneNameInJapanese) && thumbIntermediateBone && thumbDistalBone &&
                    indexIntermediateBone) {
                    auto setLocalAxes = [&status](nanoem_model_bone_t *bone, const glm::vec3 &x, const glm::vec3 &z) {
                        auto mutableBone = nanoemMutableModelBoneCreateAsReference(bone, &status);
                        nanoemMutableModelBoneSetLocalAxesEnabled(mutableBone, true);
                        nanoemMutableModelBoneSetLocalXAxis(mutableBone, glm::value_ptr(glm::vec4(x, 0)));
                        nanoemMutableModelBoneSetLocalZAxis(mutableBone, glm::value_ptr(glm::vec4(z, 0)));
                        nanoemMutableModelBoneDestroy(mutableBone);
                    };
                    auto thumbProximalBone = findBone(tuple.m_thumbProximalBoneNameInJapanese);
                    const glm::vec3 thumbProximalBoneOrigin(boneOrigin(thumbProximalBone)),
                        thumbIntermediateBoneOrigin(boneOrigin(thumbIntermediateBone)),
                        thumbDistalBoneOrigin(boneOrigin(thumbDistalBone)),
                        indexIntermediateBoneOrigin(boneOrigin(indexIntermediateBone));
                    glm::vec3 axisZ(tuple.m_callback(thumbDistalBoneOrigin - thumbIntermediateBoneOrigin));
                    axisZ.z = -glm::length(axisZ) * 0.2f;
                    setLocalAxes(thumbProximalBone,
                        glm::normalize(thumbIntermediateBoneOrigin - thumbProximalBoneOrigin),
                        glm::normalize(thumbProximalBoneOrigin - indexIntermediateBoneOrigin));
                    setLocalAxes(thumbIntermediateBone,
                        glm::normalize(thumbDistalBoneOrigin - thumbIntermediateBoneOrigin), axisZ);
                    glm::vec3 thumbDistalDestinationOrigin;
                    if (auto targetBone = nanoemModelBoneGetTargetBoneObject(thumbDistalBone)) {
                        thumbDistalDestinationOrigin = boneOrigin(targetBone);
                    }
                    else {
                        thumbDistalDestinationOrigin = thumbDistalBoneOrigin +
                            glm::make_vec3(nanoemModelBoneGetDestinationOrigin(thumbDistalBone));
                    }
                    setLocalAxes(
                        thumbDistalBone, glm::normalize(thumbDistalDestinationOrigin - thumbDistalBoneOrigin), axisZ);
                }
            }
        });
    addCheckboxComponent(
        "enableThumbLocalAxes", "親指ローカル軸設定", "Enable Local Axes of  Thumb Proximal", {}, true);
    registerCallback("grooveBone", "グルーブボーン", "Groove Bone", { "センター" }, true,
        [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            if (!containsBone("グルーブ")) {
                if (auto centerBone = findBone("センター")) {
                    auto grooveBone = createBone("グルーブ", "groove", status);
                    nanoemMutableModelBoneSetVisible(grooveBone, true);
                    nanoemMutableModelBoneSetMovable(grooveBone, true);
                    nanoemMutableModelBoneSetRotateable(grooveBone, true);
                    nanoemMutableModelBoneSetUserHandleable(grooveBone, true);
                    nanoemMutableModelBoneSetOrigin(
                        grooveBone, glm::value_ptr(glm::vec4(boneOrigin(centerBone) + glm::vec3(0, 0.2, 0), 0)));
                    nanoemMutableModelBoneSetParentBoneObject(grooveBone, centerBone);
                    nanoemMutableModelBoneSetDestinationOrigin(grooveBone, glm::value_ptr(glm::vec4(0, 1.4, 0, 0)));
                    insertBoneAfter(grooveBone, centerBone, status);
                    nanoem_rsize_t numBones;
                    auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
                    auto s = createUnicodeString("センター先");
                    for (nanoem_rsize_t i = 0; i < numBones; i++) {
                        nanoem_model_bone_t *innerBone = bones[i];
                        auto name = nanoemModelBoneGetName(innerBone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                        if (nanoemModelBoneGetParentBoneObject(innerBone) == centerBone &&
                            nanoemUnicodeStringFactoryCompareString(m_factory, name, s) != 0) {
                            setParentBone(innerBone, grooveBone, status);
                        }
                    }
                    nanoemUnicodeStringFactoryDestroyString(m_factory, s);
                    if (isLabelGenerationEnabled()) {
                        nanoem_model_label_t *foundLabel;
                        nanoem_model_label_item_t *foundLabelItem;
                        nanoem_rsize_t offset;
                        if (findBoneLabel(centerBone, foundLabel, foundLabelItem, offset)) {
                            insertLabelAfter(grooveBone, centerBone, status);
                        }
                        else {
                            auto label = createCenterLabel(status);
                            auto mutableLabel = nanoemMutableModelLabelCreateAsReference(label, &status);
                            auto item = nanoemMutableModelLabelItemCreateFromBoneObject(
                                mutableLabel, nanoemMutableModelBoneGetOriginObject(grooveBone), &status);
                            nanoemMutableModelLabelInsertItemObject(mutableLabel, item, -1, &status);
                            nanoemMutableModelLabelItemDestroy(item);
                            nanoemMutableModelLabelDestroy(mutableLabel);
                        }
                    }
                    nanoemMutableModelBoneDestroy(grooveBone);
                }
            }
            else {
                addRequiredBoneNotFoundError("グルーブ", "センター");
            }
        });
    registerCallback("rootBone", "全ての親", "Root Bone", {}, true,
        [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            if (!containsBone("全ての親")) {
                nanoem_rsize_t numBones;
                auto rootBone = createBone("全ての親", "master", status);
                nanoemMutableModelBoneSetVisible(rootBone, true);
                nanoemMutableModelBoneSetMovable(rootBone, true);
                nanoemMutableModelBoneSetRotateable(rootBone, true);
                nanoemMutableModelBoneSetUserHandleable(rootBone, true);
                {
                    auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
                    const auto firstBone = bones[0];
                    nanoemMutableModelBoneSetTargetBoneObject(rootBone, firstBone);
                }
                insertBoneFirst(rootBone, status);
                /* don't handle offset 0 (root bone) */
                auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
                auto firstBone = numBones > 0 ? bones[1] : nullptr;
                for (nanoem_rsize_t i = 1; i < numBones; i++) {
                    nanoem_model_bone_t *innerBone = bones[i];
                    auto parentBone = nanoemModelBoneGetParentBoneObject(innerBone);
                    auto targetBone = nanoemModelBoneGetTargetBoneObject(innerBone);
                    if (!parentBone) {
                        setParentBone(innerBone, rootBone, status);
                    }
                    else if (targetBone && targetBone == firstBone) {
                        setTargetBone(innerBone, rootBone, status);
                    }
                }
                if (isLabelGenerationEnabled()) {
                    setRootLabel(rootBone, firstBone, status);
                }
                nanoemMutableModelBoneDestroy(rootBone);
            }
        });
    registerCallback("viewCenterBone", "操作中心", "View Center", {}, true,
        [this](const VertexList & /* vertices */, nanoem_status_t &status) {
            if (!containsBone("操作中心")) {
                auto viewCenterBone = createBone("操作中心", "view cnt", status);
                nanoemMutableModelBoneSetVisible(viewCenterBone, true);
                nanoemMutableModelBoneSetMovable(viewCenterBone, true);
                nanoemMutableModelBoneSetRotateable(viewCenterBone, true);
                nanoemMutableModelBoneSetUserHandleable(viewCenterBone, true);
                insertBoneFirst(viewCenterBone, status);
                nanoem_rsize_t numBones;
                auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
                auto firstBone = numBones > 0 ? bones[1] : nullptr;
                for (nanoem_rsize_t i = 1; i < numBones; i++) {
                    nanoem_model_bone_t *innerBone = bones[i];
                    if (nanoemModelBoneGetTargetBoneObject(innerBone) == firstBone) {
                        setTargetBone(innerBone, viewCenterBone, status);
                    }
                }
                if (isLabelGenerationEnabled()) {
                    setRootLabel(viewCenterBone, firstBone, status);
                }
                nanoemMutableModelBoneDestroy(viewCenterBone);
            }
        });
    addCheckboxComponent("generateLabel", "ボーン表示枠に自動登録", "Generate Labels", {}, true);
    nanoem_rsize_t numMaterials;
    auto materials = nanoemModelGetAllMaterialObjects(opaque(), &numMaterials);
    auto language = static_cast<nanoem_language_type_t>(m_language);
    char buffer[32];
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        auto component = new Nanoem__Application__Plugin__UIComponent;
        nanoem__application__plugin__uicomponent__init(component);
        component->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX;
        component->check_box = new Nanoem__Application__Plugin__UIComponent__CheckBox;
        nanoem__application__plugin__uicomponent__check_box__init(component->check_box);
        nanoem_rsize_t length;
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        snprintf(buffer, sizeof(buffer), "material%zu", i);
        copyString(component->check_box->id, buffer);
        if (auto ptr = nanoemUnicodeStringFactoryGetByteArray(
                m_factory, nanoemModelMaterialGetName(materials[i], language), &length, &status)) {
            if (strlen(reinterpret_cast<const char *>(ptr)) > 0) {
                copyString(component->check_box->text, reinterpret_cast<const char *>(ptr));
            }
            else if (auto fallbackPtr = nanoemUnicodeStringFactoryGetByteArray(m_factory,
                         nanoemModelMaterialGetName(materials[i], NANOEM_LANGUAGE_TYPE_FIRST_ENUM), &length, &status)) {
                copyString(component->check_box->text, reinterpret_cast<const char *>(fallbackPtr));
                nanoemUnicodeStringFactoryDestroyByteArray(m_factory, fallbackPtr);
            }
            nanoemUnicodeStringFactoryDestroyByteArray(m_factory, ptr);
        }
        component->check_box->value = true;
        m_materials.push_back(component);
    }
    serializeUIComponentList();
    return true;
}

bool
SemiStandardBonePlugin::execute()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t numVertices, numIndices, numMaterials, offset = 0;
    auto vertices = nanoemModelGetAllVertexObjects(opaque(), &numVertices);
    auto indices = nanoemModelGetAllVertexIndices(opaque(), &numIndices);
    auto materials = nanoemModelGetAllMaterialObjects(opaque(), &numMaterials);
    std::unordered_set<nanoem_model_vertex_t *> vertexSet;
    VertexList vertexList;
    m_failureReason.clear();
    m_recoverySuggestion.clear();
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(materials[i]);
        auto item = m_materials[i];
        if (item->check_box->value) {
            for (nanoem_rsize_t j = offset, numItems = offset + numIndices; j < numItems; j++) {
                vertexSet.insert(vertices[indices[j]]);
            }
        }
        offset += numIndices;
    }
    /* override removing vertices */
    offset = 0;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(materials[i]);
        auto item = m_materials[i];
        if (!item->check_box->value) {
            for (nanoem_rsize_t j = offset, numItems = offset + numIndices; j < numItems; j++) {
                vertexSet.erase(vertices[indices[j]]);
            }
        }
        offset += numIndices;
    }
    for (auto item : vertexSet) {
        vertexList.push_back(item);
    }
    std::sort(vertexList.begin(), vertexList.end(),
        [](const nanoem_model_vertex_t *left, const nanoem_model_vertex_t *right) {
            return nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(left)) <
                nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(right));
        });
    for (auto item : m_checkboxes) {
        if (item->type_case == NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX && item->check_box->value) {
            auto it = m_callbacks.find(item->check_box->id);
            if (it != m_callbacks.end()) {
                it->second(vertexList, status);
            }
        }
    }
    nanoemMutableBufferDestroy(m_mutableBuffer);
    m_mutableBuffer = nanoemMutableBufferCreate(&status);
    nanoemMutableModelSaveToBuffer(m_mutableModel, m_mutableBuffer, &status);
    nanoemBufferDestroy(m_buffer);
    m_buffer = nanoemMutableBufferCreateBufferObject(m_mutableBuffer, &status);
    return m_failureReason.empty();
}

nanoem_u32_t
SemiStandardBonePlugin::outputModelDataSize() const
{
    return nanoemBufferGetLength(m_buffer);
}

const nanoem_u8_t *
SemiStandardBonePlugin::outputModelData() const
{
    return nanoemBufferGetDataPtr(m_buffer);
}

nanoem_u32_t
SemiStandardBonePlugin::windowLayoutDataSize() const
{
    return m_windowLayoutData.size();
}

const nanoem_u8_t *
SemiStandardBonePlugin::windowLayoutData() const
{
    return m_windowLayoutData.data();
}

const char *
SemiStandardBonePlugin::failureReason() const noexcept
{
    return m_failureReason.c_str();
}

const char *
SemiStandardBonePlugin::recoverySuggestion() const noexcept
{
    return m_recoverySuggestion.c_str();
}

Nanoem__Application__Plugin__UIComponent *
SemiStandardBonePlugin::createCheckbox(const char *id, const char *text, bool checked)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX;
    auto check = item->check_box = new Nanoem__Application__Plugin__UIComponent__CheckBox;
    nanoem__application__plugin__uicomponent__check_box__init(check);
    copyString(check->id, id);
    copyString(check->text, text);
    check->value = checked ? 1 : 0;
    return item;
}

void
SemiStandardBonePlugin::registerCallback(const char *id, const char *textJA, const char *textEN,
    const std::initializer_list<const char *> &required, bool checked, Callback callback)
{
    addCheckboxComponent(id, textJA, textEN, required, checked);
    m_callbacks.insert(std::make_pair(id, callback));
}

void
SemiStandardBonePlugin::addCheckboxComponent(const char *id, const char *textJA, const char *textEN,
    const std::initializer_list<const char *> &required, bool checked)
{
    std::string text;
    if (required.size() > 0) {
        static const uint8_t kFAExclamationTriangle[] = { 0xef, 0x81, 0xb1, 0x0 };
        size_t count = 0;
        for (auto item : required) {
            count += findBone(item) ? 1 : 0;
        }
        if (count != required.size()) {
            text.append(reinterpret_cast<const char *>(kFAExclamationTriangle));
            text.push_back(' ');
        }
    }
    text.append(m_language == 0 ? textJA : textEN);
    m_checkboxes.push_back(createCheckbox(id, text.c_str(), checked));
}

void
SemiStandardBonePlugin::copyString(char *&dest, const char *text)
{
    size_t length = strlen(text);
    dest = new char[length + 1];
    strncpy(dest, text, length);
    dest[length] = 0;
}

void
SemiStandardBonePlugin::serializeUIComponentList()
{
    UIComponentList globalComponents;
    {
        auto component = new Nanoem__Application__Plugin__UIComponent;
        nanoem__application__plugin__uicomponent__init(component);
        component->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHILD_WINDOW;
        auto window = component->child_window = new Nanoem__Application__Plugin__UIComponent__ChildWindow;
        nanoem__application__plugin__uicomponent__child_window__init(component->child_window);
        copyString(window->id, "checkboxes");
        window->has_width = window->has_height = 1;
        window->width = 270;
        window->height = 420;
        window->n_components = m_checkboxes.size();
        window->components = new Nanoem__Application__Plugin__UIComponent *[m_checkboxes.size()];
        nanoem_rsize_t offset = 0;
        for (auto item : m_checkboxes) {
            window->components[offset++] = item;
        }
        globalComponents.push_back(component);
    }
    {
        auto component = new Nanoem__Application__Plugin__UIComponent;
        nanoem__application__plugin__uicomponent__init(component);
        component->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SAME_LINE;
        component->same_line = new Nanoem__Application__Plugin__UIComponent__SameLine;
        nanoem__application__plugin__uicomponent__same_line__init(component->same_line);
        globalComponents.push_back(component);
    }
    {
        auto component = new Nanoem__Application__Plugin__UIComponent;
        nanoem__application__plugin__uicomponent__init(component);
        component->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHILD_WINDOW;
        auto window = component->child_window = new Nanoem__Application__Plugin__UIComponent__ChildWindow;
        nanoem__application__plugin__uicomponent__child_window__init(component->child_window);
        copyString(component->child_window->id, "materials");
        window->has_width = window->has_height = window->has_border = true;
        window->border = true;
        window->width = 200;
        window->height = 420;
        window->n_components = m_materials.size();
        window->components = new Nanoem__Application__Plugin__UIComponent *[m_materials.size()];
        nanoem_rsize_t offset = 0;
        for (auto item : m_materials) {
            window->components[offset++] = item;
        }
        globalComponents.push_back(component);
    }
    Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
    window.n_items = globalComponents.size();
    window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(globalComponents.data());
    m_windowLayoutData.resize(nanoem__application__plugin__uiwindow__get_packed_size(&window));
    nanoem__application__plugin__uiwindow__pack(&window, m_windowLayoutData.data());
    {
        delete[] globalComponents[0]->child_window->components;
        delete[] globalComponents[0]->child_window->id;
        delete globalComponents[0]->child_window;
        delete globalComponents[0];
        delete globalComponents[1]->same_line;
        delete globalComponents[1];
        delete[] globalComponents[2]->child_window->components;
        delete[] globalComponents[2]->child_window->id;
        delete globalComponents[2]->child_window;
        delete globalComponents[2];
    }
}

void
SemiStandardBonePlugin::clearAllUIComponents()
{
    for (auto item : m_checkboxes) {
        destroyUIComponent(item);
    }
    m_checkboxes.clear();
    for (auto item : m_materials) {
        destroyUIComponent(item);
    }
    m_materials.clear();
    m_callbacks.clear();
    m_windowLayoutData.clear();
}

void
SemiStandardBonePlugin::destroyUIComponent(Nanoem__Application__Plugin__UIComponent *value)
{
    switch (value->type_case) {
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX: {
        delete[] value->check_box->id;
        delete[] value->check_box->text;
        delete value->check_box;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHILD_WINDOW: {
        for (size_t i = 0, numItems = value->child_window->n_components; i < numItems; i++) {
            destroyUIComponent(value->child_window->components[i]);
        }
        delete[] value->child_window->id;
        delete value->child_window;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SAME_LINE: {
        delete value->same_line;
        break;
    }
    default:
        break;
    }
    delete value;
}

glm::vec3
SemiStandardBonePlugin::boneOrigin(const nanoem_model_bone_t *bone) noexcept
{
    return glm::make_vec3(nanoemModelBoneGetOrigin(bone));
}

glm::vec3
SemiStandardBonePlugin::vertexOrigin(const nanoem_model_vertex_t *vertex) noexcept
{
    return glm::make_vec3(nanoemModelVertexGetOrigin(vertex));
}

int
SemiStandardBonePlugin::boneIndex(const nanoem_model_bone_t *bone) noexcept
{
    return nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bone));
}

int
SemiStandardBonePlugin::boneIndex(nanoem_mutable_model_bone_t *bone) noexcept
{
    return boneIndex(nanoemMutableModelBoneGetOriginObject(bone));
}

bool
SemiStandardBonePlugin::isVertexDedicatedByBone(
    const nanoem_model_vertex_t *vertex, const nanoem_model_bone_t *bone) noexcept
{
    return isVertexDedicatedByBone(vertex, bone, 0.97f);
}

bool
SemiStandardBonePlugin::isVertexDedicatedByBone(
    const nanoem_model_vertex_t *vertex, const nanoem_model_bone_t *bone, float threshold) noexcept
{
    float sum = 0.0f, total = 0.0f;
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        const nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertex, i);
        if (nanoemModelVertexGetBoneObject(vertex, i) == bone) {
            sum += weight;
        }
        total += weight;
    }
    return (sum / total) > threshold;
}

bool
SemiStandardBonePlugin::vertexContainsBone(
    const nanoem_model_vertex_t *vertex, const nanoem_model_bone_t *bone) noexcept
{
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        if (nanoemModelVertexGetBoneObject(vertex, i) == bone) {
            return true;
        }
    }
    return false;
}

nanoem_model_bone_t *
SemiStandardBonePlugin::findBone(const char *name) const
{
    nanoem_model_bone_t *bone = nullptr;
    if (auto s = createUnicodeString(name)) {
        nanoem_rsize_t numBones;
        auto bones = nanoemModelGetAllBoneObjects(opaque(), &numBones);
        for (nanoem_rsize_t i = 0; i < numBones; i++) {
            auto name = nanoemModelBoneGetName(bones[i], NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            if (nanoemUnicodeStringFactoryCompareString(m_factory, name, s) == 0) {
                bone = bones[i];
                break;
            }
        }
        nanoemUnicodeStringFactoryDestroyString(m_factory, s);
    }
    return bone;
}

nanoem_model_label_t *
SemiStandardBonePlugin::findLabel(const char *name) const
{
    nanoem_model_label_t *label = nullptr;
    if (auto s = createUnicodeString(name)) {
        nanoem_rsize_t numLabels;
        auto labels = nanoemModelGetAllLabelObjects(opaque(), &numLabels);
        for (nanoem_rsize_t i = 0; i < numLabels; i++) {
            auto name = nanoemModelLabelGetName(labels[i], NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            if (nanoemUnicodeStringFactoryCompareString(m_factory, name, s) == 0) {
                label = labels[i];
                break;
            }
        }
        nanoemUnicodeStringFactoryDestroyString(m_factory, s);
    }
    return label;
}

nanoem_mutable_model_bone_t *
SemiStandardBonePlugin::createBone(const char *japanese, const char *english, nanoem_status_t &status)
{
    auto bone = nanoemMutableModelBoneCreate(opaque(), &status);
    setBoneName(bone, japanese, english, status);
    return bone;
}

nanoem_mutable_model_bone_t *
SemiStandardBonePlugin::cloneBone(const nanoem_model_bone_t *value, nanoem_status_t &status)
{
    auto bone = nanoemMutableModelBoneCreate(opaque(), &status);
    nanoemMutableModelBoneCopy(bone, value, &status);
    return bone;
}

nanoem_model_label_t *
SemiStandardBonePlugin::createCenterLabel(nanoem_status_t &status)
{
    auto label = findLabel("センター");
    if (!label) {
        auto newLabel = nanoemMutableModelLabelCreate(opaque(), &status);
        if (auto s = createUnicodeString("センター")) {
            nanoemMutableModelLabelSetName(newLabel, s, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemUnicodeStringFactoryDestroyString(m_factory, s);
        }
        if (auto s = createUnicodeString("center")) {
            nanoemMutableModelLabelSetName(newLabel, s, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemUnicodeStringFactoryDestroyString(m_factory, s);
        }
        /* 0 for root and 1 for expression/morph */
        nanoemMutableModelInsertLabelObject(m_mutableModel, newLabel, 2, &status);
        nanoemMutableModelLabelDestroy(newLabel);
        label = findLabel("センター");
    }
    return label;
}

bool
SemiStandardBonePlugin::containsBone(const char *name) const
{
    return findBone(name) != nullptr;
}

bool
SemiStandardBonePlugin::findBoneLabel(const nanoem_model_bone_t *base, nanoem_model_label_t *&foundLabel,
    nanoem_model_label_item_t *&foundLabelItem, nanoem_rsize_t &offset) const noexcept
{
    nanoem_rsize_t numLabels, numItems;
    auto labels = nanoemModelGetAllLabelObjects(opaque(), &numLabels);
    for (nanoem_rsize_t i = 1; i < numLabels; i++) {
        auto label = labels[i];
        auto items = nanoemModelLabelGetAllItemObjects(label, &numItems);
        for (nanoem_rsize_t j = 0; j < numItems; j++) {
            auto item = items[j];
            if (nanoemModelLabelItemGetType(item) == NANOEM_MODEL_LABEL_ITEM_TYPE_BONE &&
                nanoemModelLabelItemGetBoneObject(item) == base) {
                foundLabel = label;
                foundLabelItem = item;
                offset = j;
                return true;
            }
        }
    }
    return false;
}

bool
SemiStandardBonePlugin::isLabelGenerationEnabled() const noexcept
{
    return isCheckboxChecked("generateLabel");
}

bool
SemiStandardBonePlugin::isCheckboxChecked(const char *id) const noexcept
{
    for (auto &item : m_checkboxes) {
        if (item->type_case == NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX && item->check_box->id &&
            strcmp(item->check_box->id, id) == 0) {
            return item->check_box->value != 0;
        }
    }
    return false;
}

nanoem_unicode_string_t *
SemiStandardBonePlugin::createUnicodeString(const char *value) const
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    return nanoemUnicodeStringFactoryCreateString(
        m_factory, reinterpret_cast<const nanoem_u8_t *>(value), strlen(value), &status);
}

void
SemiStandardBonePlugin::setBoneName(
    nanoem_mutable_model_bone_t *assignee, const char *japanese, const char *english, nanoem_status_t &status)
{
    if (japanese) {
        if (auto s = createUnicodeString(japanese)) {
            nanoemMutableModelBoneSetName(assignee, s, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
            nanoemUnicodeStringFactoryDestroyString(m_factory, s);
        }
    }
    if (english) {
        if (auto s = createUnicodeString(english)) {
            nanoemMutableModelBoneSetName(assignee, s, NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
            nanoemUnicodeStringFactoryDestroyString(m_factory, s);
        }
    }
}

void
SemiStandardBonePlugin::insertBoneAfter(
    nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status)
{
    nanoemMutableModelInsertBoneObject(m_mutableModel, assignee, boneIndex(base) + 1, &status);
}

void
SemiStandardBonePlugin::insertBoneBefore(
    nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status)
{
    nanoemMutableModelInsertBoneObject(m_mutableModel, assignee, boneIndex(base), &status);
}

void
SemiStandardBonePlugin::insertBoneFirst(nanoem_mutable_model_bone_t *assignee, nanoem_status_t &status)
{
    nanoemMutableModelInsertBoneObject(m_mutableModel, assignee, 0, &status);
}

void
SemiStandardBonePlugin::insertBoneLast(nanoem_mutable_model_bone_t *assignee, nanoem_status_t &status)
{
    nanoemMutableModelInsertBoneObject(m_mutableModel, assignee, -1, &status);
}

void
SemiStandardBonePlugin::insertLabelAfter(
    nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status)
{
    insertLabelWithOffset(assignee, base, 1, status);
}

void
SemiStandardBonePlugin::insertLabelBefore(
    nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base, nanoem_status_t &status)
{
    insertLabelWithOffset(assignee, base, 0, status);
}

void
SemiStandardBonePlugin::insertLabelWithOffset(nanoem_mutable_model_bone_t *assignee, const nanoem_model_bone_t *base,
    nanoem_rsize_t offset, nanoem_status_t &status)
{
    nanoem_model_label_t *foundLabel;
    nanoem_model_label_item_t *foundLabelItem;
    nanoem_rsize_t index;
    if (findBoneLabel(base, foundLabel, foundLabelItem, index)) {
        auto mutableLabel = nanoemMutableModelLabelCreateAsReference(foundLabel, &status);
        auto mutableItem = nanoemMutableModelLabelItemCreateFromBoneObject(
            mutableLabel, nanoemMutableModelBoneGetOriginObject(assignee), &status);
        nanoemMutableModelLabelInsertItemObject(mutableLabel, mutableItem, index + offset, &status);
        nanoemMutableModelLabelItemDestroy(mutableItem);
        nanoemMutableModelLabelDestroy(mutableLabel);
    }
}

void
SemiStandardBonePlugin::replaceVertexBone(nanoem_model_vertex_t *assignee, const nanoem_model_bone_t *fromBone,
    nanoem_mutable_model_bone_t *toBone, nanoem_status_t &status)
{
    auto origin = nanoemMutableModelBoneGetOriginObject(toBone);
    auto mutableAssignee = nanoemMutableModelVertexCreateAsReference(assignee, &status);
    for (nanoem_rsize_t i = 0; i < 4; i++) {
        if (nanoemModelVertexGetBoneObject(assignee, i) == fromBone) {
            nanoemMutableModelVertexSetBoneObject(mutableAssignee, origin, i);
        }
    }
    nanoemMutableModelVertexDestroy(mutableAssignee);
}

void
SemiStandardBonePlugin::setParentBone(
    nanoem_model_bone_t *assignee, nanoem_mutable_model_bone_t *value, nanoem_status_t &status)
{
    assert(boneIndex(value) != -1);
    auto origin = nanoemMutableModelBoneGetOriginObject(value);
    if (origin != assignee) {
        auto mutableAssignee = nanoemMutableModelBoneCreateAsReference(assignee, &status);
        nanoemMutableModelBoneSetParentBoneObject(mutableAssignee, origin);
        nanoemMutableModelBoneDestroy(mutableAssignee);
    }
}

void
SemiStandardBonePlugin::setTargetBone(
    nanoem_model_bone_t *assignee, nanoem_mutable_model_bone_t *value, nanoem_status_t &status)
{
    assert(boneIndex(value) != -1);
    auto origin = nanoemMutableModelBoneGetOriginObject(value);
    if (origin != assignee) {
        auto mutableAssignee = nanoemMutableModelBoneCreateAsReference(assignee, &status);
        nanoemMutableModelBoneSetTargetBoneObject(mutableAssignee, origin);
        nanoemMutableModelBoneDestroy(mutableAssignee);
    }
}

void
SemiStandardBonePlugin::setRootLabel(
    nanoem_mutable_model_bone_t *assignee, nanoem_model_bone_t *firstBone, nanoem_status_t &status)
{
    nanoem_model_label_t *foundLabel;
    nanoem_model_label_item_t *foundLabelItem;
    nanoem_rsize_t index;
    if (firstBone && !findBoneLabel(firstBone, foundLabel, foundLabelItem, index)) {
        auto label = createCenterLabel(status);
        auto mutableLabel = nanoemMutableModelLabelCreateAsReference(label, &status);
        auto item = nanoemMutableModelLabelItemCreateFromBoneObject(mutableLabel, firstBone, &status);
        nanoemMutableModelLabelInsertItemObject(mutableLabel, item, 0, &status);
        nanoemMutableModelLabelItemDestroy(item);
        nanoemMutableModelLabelDestroy(mutableLabel);
    }
    nanoem_rsize_t numLabels;
    if (auto labels = nanoemModelGetAllLabelObjects(opaque(), &numLabels)) {
        nanoem_rsize_t numItems;
        auto firstLabel = labels[0];
        auto mutableLabel = nanoemMutableModelLabelCreateAsReference(firstLabel, &status);
        auto items = nanoemModelLabelGetAllItemObjects(firstLabel, &numItems);
        for (nanoem_rsize_t i = 0; i < numItems; i++) {
            auto item = nanoemMutableModelLabelItemCreateAsReference(items[numItems - i - 1], &status);
            nanoemMutableModelLabelRemoveItemObject(mutableLabel, item, &status);
            nanoemMutableModelLabelItemDestroy(item);
        }
        auto item = nanoemMutableModelLabelItemCreateFromBoneObject(
            mutableLabel, nanoemMutableModelBoneGetOriginObject(assignee), &status);
        nanoemMutableModelLabelInsertItemObject(mutableLabel, item, 0, &status);
        nanoemMutableModelLabelItemDestroy(item);
        nanoemMutableModelLabelDestroy(mutableLabel);
    }
}

void
SemiStandardBonePlugin::replaceBoneInAllRigidBodies(
    const nanoem_model_bone_t *from, nanoem_mutable_model_bone_t *to, nanoem_status_t &status)
{
    nanoem_rsize_t numBodies;
    auto bodies = nanoemModelGetAllRigidBodyObjects(opaque(), &numBodies);
    for (nanoem_rsize_t i = 0; i < numBodies; i++) {
        auto body = bodies[i];
        if (nanoemModelRigidBodyGetBoneObject(body) == from) {
            setRigidBodyBone(body, to, status);
        }
    }
}

void
SemiStandardBonePlugin::setRigidBodyBone(
    nanoem_model_rigid_body_t *assignee, nanoem_mutable_model_bone_t *value, nanoem_status_t &status)
{
    auto origin = nanoemMutableModelBoneGetOriginObject(value);
    if (origin != nanoemModelRigidBodyGetBoneObject(assignee)) {
        auto mutableAssignee = nanoemMutableModelRigidBodyCreateAsReference(assignee, &status);
        nanoemMutableModelRigidBodySetBoneObject(mutableAssignee, origin);
        nanoemMutableModelRigidBodyDestroy(mutableAssignee);
    }
}

void
SemiStandardBonePlugin::addTwistBone(const char *nameInJapanese, const char *nameInEnglish,
    const char *parentTwistBoneName, const char *childTwistBoneName, bool enableElbowOffset, const VertexList &vertices,
    nanoem_status_t &status)
{
    if (!containsBone(nameInJapanese)) {
        auto parentTwistBone = findBone(parentTwistBoneName);
        auto childTwistBone = findBone(childTwistBoneName);
        if (parentTwistBone && childTwistBone) {
            glm::vec3 childTwistBoneOrigin(boneOrigin(childTwistBone));
            if (enableElbowOffset && isCheckboxChecked("enableElbowOffset")) {
                nanoem_f32_t numVertices = 0;
                float sum = 0.0f;
                for (auto &item : vertices) {
                    if (isVertexDedicatedByBone(item, parentTwistBone, 0.6f)) {
                        sum += nanoemModelVertexGetOrigin(item)[2];
                        numVertices += 1.0f;
                    }
                }
                if (numVertices > 0) {
                    float zoffset = (sum / numVertices - childTwistBoneOrigin.z) * 0.75f;
                    childTwistBoneOrigin.z += zoffset;
                }
            }
            const glm::vec3 parentTwistBoneOrigin(boneOrigin(parentTwistBone)),
                twistBoneOrigin(glm::mix(parentTwistBoneOrigin, childTwistBoneOrigin, 0.6f)),
                twistBoneAxis(glm::normalize(childTwistBoneOrigin - parentTwistBoneOrigin));
            float distance = glm::length(childTwistBoneOrigin - parentTwistBoneOrigin),
                  parentTwistBoneDot = glm::dot(parentTwistBoneOrigin - twistBoneOrigin, twistBoneAxis) * 0.8f,
                  childTwistBoneDot = glm::dot(childTwistBoneOrigin - twistBoneOrigin, twistBoneAxis) * 0.8f;
            auto twistBone = createBone(nameInJapanese, nameInEnglish, status);
            nanoemMutableModelBoneSetRotateable(twistBone, true);
            nanoemMutableModelBoneSetVisible(twistBone, true);
            nanoemMutableModelBoneSetUserHandleable(twistBone, true);
            nanoemMutableModelBoneSetFixedAxisEnabled(twistBone, true);
            nanoemMutableModelBoneSetParentBoneObject(twistBone, parentTwistBone);
            nanoemMutableModelBoneSetOrigin(twistBone, glm::value_ptr(glm::vec4(twistBoneOrigin, 0)));
            nanoemMutableModelBoneSetDestinationOrigin(twistBone, glm::value_ptr(glm::vec4(0)));
            nanoemMutableModelBoneSetFixedAxis(twistBone, glm::value_ptr(glm::vec4(twistBoneAxis, 0)));
            insertBoneAfter(twistBone, parentTwistBone, status);
            VertexList parentTwistBoneDedicatedVertices;
            for (auto &item : vertices) {
                float vertexTwistBoneDot = glm::dot(vertexOrigin(item) - twistBoneOrigin, twistBoneAxis);
                if (isVertexDedicatedByBone(item, parentTwistBone)) {
                    parentTwistBoneDot = glm::min(parentTwistBoneDot, vertexTwistBoneDot);
                    childTwistBoneDot = glm::max(childTwistBoneDot, vertexTwistBoneDot);
                    parentTwistBoneDedicatedVertices.push_back(item);
                }
                else if (vertexTwistBoneDot > 0.0f) {
                    if (vertexContainsBone(item, childTwistBone)) {
                        replaceVertexBone(item, parentTwistBone, twistBone, status);
                    }
                }
            }
            nanoem_mutable_model_bone_t *innerTwistBones[3] = { nullptr, nullptr, nullptr };
            for (int i = 0; i < 3; i++) {
                float coefficient = (i + 1) / 4.0f;
                const glm::vec3 innerTwistBoneOrigin(twistBoneOrigin +
                    twistBoneAxis * glm::vec3(glm::mix(parentTwistBoneDot, childTwistBoneDot, coefficient)));
                auto innerTwistBone = createBone(nullptr, nullptr, status);
                std::string s(nameInJapanese);
                s.push_back(i + '1');
                if (auto bone = findBone(s.c_str())) {
                    auto mutableBone = nanoemMutableModelBoneCreateAsReference(bone, &status);
                    nanoemMutableModelRemoveBoneObject(m_mutableModel, mutableBone, &status);
                    nanoemMutableModelBoneDestroy(mutableBone);
                }
                if (auto u = createUnicodeString(s.c_str())) {
                    nanoemMutableModelBoneSetName(innerTwistBone, u, NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
                    nanoemUnicodeStringFactoryDestroyString(m_factory, u);
                }
                nanoemMutableModelBoneSetRotateable(innerTwistBone, true);
                nanoemMutableModelBoneSetUserHandleable(innerTwistBone, true);
                nanoemMutableModelBoneSetInherentOrientationEnabled(innerTwistBone, true);
                nanoemMutableModelBoneSetInherentCoefficient(innerTwistBone, coefficient);
                nanoemMutableModelBoneSetOrigin(innerTwistBone, glm::value_ptr(glm::vec4(innerTwistBoneOrigin, 0)));
                nanoemMutableModelBoneSetDestinationOrigin(innerTwistBone, glm::value_ptr(glm::vec4(0)));
                nanoemMutableModelBoneSetParentBoneObject(innerTwistBone, parentTwistBone);
                nanoemMutableModelBoneSetInherentParentBoneObject(
                    innerTwistBone, nanoemMutableModelBoneGetOriginObject(twistBone));
                if (i == 0) {
                    insertBoneAfter(innerTwistBone, nanoemMutableModelBoneGetOriginObject(twistBone), status);
                }
                else {
                    insertBoneAfter(
                        innerTwistBone, nanoemMutableModelBoneGetOriginObject(innerTwistBones[i - 1]), status);
                }
                innerTwistBones[i] = innerTwistBone;
            }
            for (auto item : parentTwistBoneDedicatedVertices) {
                float vertexTwistBoneDot = glm::dot(vertexOrigin(item) - twistBoneOrigin, twistBoneAxis);
                float delta =
                    ((vertexTwistBoneDot - parentTwistBoneDot) / (childTwistBoneDot - parentTwistBoneDot)) * 4.0f;
                float weight = (int(100.0f * delta) % 100) / 100.0f;
                nanoem_mutable_model_vertex_t *vertex = nanoemMutableModelVertexCreateAsReference(item, &status);
                nanoemMutableModelVertexSetType(vertex, NANOEM_MODEL_VERTEX_TYPE_BDEF2);
                switch (int(delta)) {
                case 0: {
                    nanoemMutableModelVertexSetBoneObject(
                        vertex, nanoemMutableModelBoneGetOriginObject(innerTwistBones[0]), 0);
                    nanoemMutableModelVertexSetBoneObject(vertex, parentTwistBone, 1);
                    break;
                }
                case 1: {
                    nanoemMutableModelVertexSetBoneObject(
                        vertex, nanoemMutableModelBoneGetOriginObject(innerTwistBones[1]), 0);
                    nanoemMutableModelVertexSetBoneObject(
                        vertex, nanoemMutableModelBoneGetOriginObject(innerTwistBones[0]), 1);
                    break;
                }
                case 2: {
                    nanoemMutableModelVertexSetBoneObject(
                        vertex, nanoemMutableModelBoneGetOriginObject(innerTwistBones[2]), 0);
                    nanoemMutableModelVertexSetBoneObject(
                        vertex, nanoemMutableModelBoneGetOriginObject(innerTwistBones[1]), 1);
                    break;
                }
                case 3: {
                    nanoemMutableModelVertexSetBoneObject(vertex, nanoemMutableModelBoneGetOriginObject(twistBone), 0);
                    nanoemMutableModelVertexSetBoneObject(
                        vertex, nanoemMutableModelBoneGetOriginObject(innerTwistBones[2]), 1);
                    break;
                }
                case 4: {
                    nanoemMutableModelVertexSetBoneObject(vertex, childTwistBone, 0);
                    nanoemMutableModelVertexSetBoneObject(vertex, nanoemMutableModelBoneGetOriginObject(twistBone), 1);
                    break;
                }
                default:
                    break;
                }
                nanoemMutableModelVertexSetBoneWeight(vertex, weight, 0);
                nanoemMutableModelVertexSetBoneWeight(vertex, 1.0f - weight, 1);
                nanoemMutableModelVertexDestroy(vertex);
            }
            for (auto item : innerTwistBones) {
                nanoemMutableModelBoneDestroy(item);
            }
            setParentBone(childTwistBone, twistBone, status);
            if (isLabelGenerationEnabled()) {
                insertLabelAfter(twistBone, parentTwistBone, status);
            }
            nanoemMutableModelBoneDestroy(twistBone);
        }
        else {
            if (!parentTwistBone) {
                addRequiredBoneNotFoundError(nameInJapanese, parentTwistBoneName);
            }
            if (!childTwistBone) {
                addRequiredBoneNotFoundError(nameInJapanese, childTwistBoneName);
            }
        }
    }
}

void
SemiStandardBonePlugin::addRequiredBoneNotFoundError(const char *name, const char *required)
{
    std::ostringstream s;
    if (m_language == 0) {
        s << "「" << name << "」ボーンに必要な「" << required << "」ボーンがありません\n";
        m_failureReason.append(s.str());
        if (m_recoverySuggestion.empty()) {
            m_recoverySuggestion = "必要なボーンを作成してから再度実行してください";
        }
    }
    else {
        s << "The required bone \"" << required << "\" does not find to create the bone \"" << name << "\"\n";
        m_failureReason.append(s.str());
        if (m_recoverySuggestion.empty()) {
            m_recoverySuggestion = "Execute the plugin again after creating all prerequisite bones";
        }
    }
}

const nanoem_model_t *
SemiStandardBonePlugin::opaque() const
{
    return nanoemMutableModelGetOriginObject(m_mutableModel);
}

nanoem_model_t *
SemiStandardBonePlugin::opaque()
{
    return nanoemMutableModelGetOriginObject(m_mutableModel);
}

struct nanoem_wasm_plugin_model_io_t : SemiStandardBonePlugin { };

struct nanoem_application_plugin_model_io_t : SemiStandardBonePlugin { };

} /* namespace anonymous */

NANOEM_DECL_API nanoem_u32_t APIENTRY
nanoemApplicationPluginModelIOGetABIVersion(void)
{
    return 2u << 16;
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOInitialize(void)
{
}

NANOEM_DECL_API nanoem_application_plugin_model_io_t *APIENTRY
nanoemApplicationPluginModelIOCreate(void)
{
    return new nanoem_application_plugin_model_io_t;
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOSetLanguage(nanoem_application_plugin_model_io_t *plugin, int value)
{
    if (plugin) {
        plugin->setLanguage(static_cast<nanoem_u32_t>(value));
    }
}

NANOEM_DECL_API const char *APIENTRY
nanoemApplicationPluginModelIOGetName(const nanoem_application_plugin_model_io_t *plugin)
{
    return plugin ? plugin->name() : nullptr;
}

NANOEM_DECL_API const char *APIENTRY
nanoemApplicationPluginModelIOGetDescription(const nanoem_application_plugin_model_io_t *plugin)
{
    return plugin ? plugin->description() : nullptr;
}

NANOEM_DECL_API const char *APIENTRY
nanoemApplicationPluginModelIOGetVersion(const nanoem_application_plugin_model_io_t *plugin)
{
    return plugin ? plugin->version() : nullptr;
}

NANOEM_DECL_API int APIENTRY
nanoemApplicationPluginModelIOCountAllFunctions(const nanoem_application_plugin_model_io_t *plugin)
{
    return plugin ? plugin->countAllFunctions() : 0;
}

NANOEM_DECL_API const char *APIENTRY
nanoemApplicationPluginModelIOGetFunctionName(const nanoem_application_plugin_model_io_t *plugin, int index)
{
    return plugin ? plugin->functionName(index) : nullptr;
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOSetFunction(nanoem_application_plugin_model_io_t *plugin, int index, nanoem_i32_t *status)
{
    if (plugin) {
        plugin->setFunctionIndex(index);
        if (status) {
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        }
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOSetInputModelData(
    nanoem_application_plugin_model_io_t *plugin, const nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (plugin) {
        nanoem_status_t innerStatus = NANOEM_STATUS_SUCCESS;
        plugin->setInputData(data, length, &innerStatus);
        if (status) {
            *status = innerStatus == NANOEM_STATUS_SUCCESS ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS
                                                           : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOExecute(nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status)
{
    if (plugin) {
        bool result = plugin->execute();
        if (status) {
            *status =
                result ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOGetOutputModelDataSize(nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length)
{
    if (plugin && length) {
        *length = plugin->outputModelDataSize();
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOGetOutputModelData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (plugin && data) {
        memcpy(data, plugin->outputModelData(), length);
        if (status) {
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        }
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOLoadUIWindowLayout(nanoem_application_plugin_model_io_t *plugin, nanoem_i32_t *status)
{
    if (plugin) {
        plugin->loadWindowLayout();
        if (status) {
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        }
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOGetUIWindowLayoutDataSize(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u32_t *length)
{
    if (plugin && length) {
        *length = plugin->windowLayoutDataSize();
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOGetUIWindowLayoutData(
    nanoem_application_plugin_model_io_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    if (plugin && data) {
        memcpy(data, plugin->windowLayoutData(), length);
        if (status) {
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        }
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOSetUIComponentLayoutData(nanoem_application_plugin_model_io_t *plugin, const char *id,
    const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout, nanoem_i32_t *status)
{
    if (plugin && data && reloadLayout) {
        plugin->setComponentLayoutData(id, data, length);
        *reloadLayout = 0;
        if (status) {
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        }
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

NANOEM_DECL_API const char *APIENTRY
nanoemApplicationPluginModelIOGetFailureReason(const nanoem_application_plugin_model_io_t *plugin)
{
    return plugin ? plugin->failureReason() : nullptr;
}

NANOEM_DECL_API const char *APIENTRY
nanoemApplicationPluginModelIOGetRecoverySuggestion(const nanoem_application_plugin_model_io_t *plugin)
{
    return plugin ? plugin->recoverySuggestion() : nullptr;
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIODestroy(nanoem_application_plugin_model_io_t *plugin)
{
    delete plugin;
}

NANOEM_DECL_API void APIENTRY
nanoemApplicationPluginModelIOTerminate(void)
{
}

#ifdef WASM_WASI_SDK
NANOEM_DECL_API void *APIENTRY
nanoemApplicationPluginAllocateMemoryWASM(nanoem_rsize_t size)
{
    return malloc(size);
}

NANOEM_DECL_API void *APIENTRY
nanoemApplicationPluginReleaseMemoryWASM(void *ptr)
{
    free(ptr);
}

/* dummy function for WASI SDK */
int
main(int /* argc */, char ** /* argv */)
{
    return 0;
}
#endif
