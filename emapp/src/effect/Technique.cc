/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/effect/Technique.h"

#include "emapp/Effect.h"
#include "emapp/IImageView.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Material.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {
namespace {

class EnumStringifyUtils : private NonCopyable {
public:
    static const char *toString(sg_blend_op value) NANOEM_DECL_NOEXCEPT;
    static const char *toString(sg_blend_factor value) NANOEM_DECL_NOEXCEPT;
    static const char *toString(sg_compare_func value) NANOEM_DECL_NOEXCEPT;
    static const char *toString(sg_stencil_op value) NANOEM_DECL_NOEXCEPT;
    static const char *toString(bool value) NANOEM_DECL_NOEXCEPT;
};

const char *
EnumStringifyUtils::toString(sg_blend_op value) NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case _SG_BLENDOP_DEFAULT:
        return "_SG_BLENDOP_DEFAULT";
    case SG_BLENDOP_ADD:
        return "ADD";
    case SG_BLENDOP_SUBTRACT:
        return "SUBTRACT";
    case SG_BLENDOP_REVERSE_SUBTRACT:
        return "REVERSE_SUBTRACT";
#if defined(NANOEM_ENABLE_BLENDOP_MINMAX)
    case SG_BLENDOP_MIN:
        return "MIN";
    case SG_BLENDOP_MAX:
        return "MAX";
#endif
    case _SG_BLENDOP_NUM:
        return "_SG_BLENDOP_NUM";
    case _SG_BLENDOP_FORCE_U32:
        return "_SG_BLENDOP_FORCE_U32";
    }
}

const char *
EnumStringifyUtils::toString(sg_blend_factor value) NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case _SG_BLENDFACTOR_DEFAULT:
        return "_SG_BLENDFACTOR_DEFAULT";
    case SG_BLENDFACTOR_ZERO:
        return "ZERO";
    case SG_BLENDFACTOR_ONE:
        return "ONE";
    case SG_BLENDFACTOR_SRC_COLOR:
        return "SRC_COLOR";
    case SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR:
        return "ONE_MINUS_SRC_COLOR";
    case SG_BLENDFACTOR_SRC_ALPHA:
        return "SRC_ALPHA";
    case SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA:
        return "ONE_MINUS_SRC_ALPHA";
    case SG_BLENDFACTOR_DST_COLOR:
        return "DST_COLOR";
    case SG_BLENDFACTOR_ONE_MINUS_DST_COLOR:
        return "ONE_MINUS_DST_COLOR";
    case SG_BLENDFACTOR_DST_ALPHA:
        return "DST_ALPHA";
    case SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA:
        return "ONE_MINUS_DST_ALPHA";
    case SG_BLENDFACTOR_SRC_ALPHA_SATURATED:
        return "SRC_ALPHA_SATURATED";
    case SG_BLENDFACTOR_BLEND_COLOR:
        return "BLEND_COLOR";
    case SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR:
        return "ONE_MINUS_BLEND_COLOR";
    case SG_BLENDFACTOR_BLEND_ALPHA:
        return "BLEND_ALPHA";
    case SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA:
        return "ONE_MINUS_BLEND_ALPHA";
    case _SG_BLENDFACTOR_NUM:
        return "_SG_BLENDFACTOR_NUM";
    case _SG_BLENDFACTOR_FORCE_U32:
        return "_SG_BLENDFACTOR_FORCE_U32";
    }
}

const char *
EnumStringifyUtils::toString(sg_compare_func value) NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case _SG_COMPAREFUNC_DEFAULT:
        return "_SG_COMPAREFUNC_DEFAULT";
    case SG_COMPAREFUNC_NEVER:
        return "NEVER";
    case SG_COMPAREFUNC_LESS:
        return "LESS";
    case SG_COMPAREFUNC_EQUAL:
        return "EQUAL";
    case SG_COMPAREFUNC_LESS_EQUAL:
        return "LESS_EQUAL";
    case SG_COMPAREFUNC_GREATER:
        return "GREATER";
    case SG_COMPAREFUNC_NOT_EQUAL:
        return "NOT_EQUAL";
    case SG_COMPAREFUNC_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case SG_COMPAREFUNC_ALWAYS:
        return "ALWAYS";
    case _SG_COMPAREFUNC_NUM:
        return "_SG_COMPAREFUNC_NUM";
    case _SG_COMPAREFUNC_FORCE_U32:
        return "_SG_COMPAREFUNC_FORCE_U32";
    }
}

const char *
EnumStringifyUtils::toString(sg_stencil_op value) NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case _SG_STENCILOP_DEFAULT:
        return "_SG_STENCILOP_DEFAULT";
    case SG_STENCILOP_KEEP:
        return "KEEP";
    case SG_STENCILOP_ZERO:
        return "ZERO";
    case SG_STENCILOP_REPLACE:
        return "REPLACE";
    case SG_STENCILOP_INCR_CLAMP:
        return "INCR_CLAMP";
    case SG_STENCILOP_DECR_CLAMP:
        return "DECR_CLAMP";
    case SG_STENCILOP_INVERT:
        return "INVERT";
    case SG_STENCILOP_INCR_WRAP:
        return "INCR_WRAP";
    case SG_STENCILOP_DECR_WRAP:
        return "DECR_WRAP";
    case _SG_STENCILOP_NUM:
        return "_SG_STENCILOP_NUM";
    case _SG_STENCILOP_FORCE_U32:
        return "_SG_STENCILOP_FORCE_U32";
    }
}

const char *
EnumStringifyUtils::toString(bool value) NANOEM_DECL_NOEXCEPT
{
    return value ? "true" : "false";
}

} /* namespace anonymous */

static const char kSubsetKeyLiteral[] = "Subset";
static const char kMMDPassKeyLiteral[] = "MMDPass";
static const char kUseTextureKeyLiteral[] = "UseTexture";
static const char kUseSphereMapKeyLiteral[] = "UseSphereMap";
static const char kUseToonKeyLiteral[] = "UseToon";

Technique::ScriptExternal::ScriptExternal(Technique *techniquePtr)
    : m_techniquePtr(techniquePtr)
    , m_blitted(false)
    , m_exists(false)
{
    m_destinationPass = { SG_INVALID_ID };
}

Technique::ScriptExternal::~ScriptExternal() NANOEM_DECL_NOEXCEPT
{
}

void
Technique::ScriptExternal::save(const IDrawable *drawable, const char *name)
{
    Effect *effect = m_techniquePtr->effect();
    Project *project = effect->project();
    sg_pass pass = effect->resetRenderPass(drawable);
    m_destinationPass = pass;
    SG_PUSH_GROUPF("Technique::ScriptExternal::save(pass=%s)", project->findRenderPassName(pass));
    project->setRenderPassName(pass, name);
    project->setScriptExternalRenderPass(pass, effect->clearColor(), effect->clearDepth());
    SG_POP_GROUP();
}

void
Technique::ScriptExternal::blit()
{
    if (m_exists && !m_blitted) {
        Effect *effect = m_techniquePtr->effect();
        Project *project = effect->project();
        const sg_pass lastDrawnRenderPass = project->lastDrawnRenderPass();
        SG_PUSH_GROUPF("Technique::ScriptExternal::blit(dest=%s, source=%s)",
            project->findRenderPassName(m_destinationPass), project->findRenderPassName(lastDrawnRenderPass));
        project->blitRenderPass(project->sharedSerialDrawQueue(), m_destinationPass, lastDrawnRenderPass);
        effect->generateRenderTargetMipmapImagesChain();
        project->resetScriptExternalRenderPass();
        m_blitted = true;
        SG_POP_GROUP();
    }
}

void
Technique::ScriptExternal::reset()
{
    if (m_exists) {
        m_techniquePtr->effect()->project()->resetScriptExternalRenderPass();
        m_blitted = false;
    }
}

Technique::Technique(Effect *effect, const String &name, const AnnotationMap &annotations, const PassList &passes)
    : m_name(name)
    , m_subset(annotations.stringAnnotation(kSubsetKeyLiteral, String()))
    , m_passType(annotations.stringAnnotation(kMMDPassKeyLiteral, String()))
    , m_useDiffuseTexture(annotations.triBooleanAnnotation(kUseTextureKeyLiteral, -1))
    , m_useSphereMapTexture(annotations.triBooleanAnnotation(kUseSphereMapKeyLiteral, -1))
    , m_useToonTexture(annotations.triBooleanAnnotation(kUseToonKeyLiteral, -1))
    , m_effect(effect)
    , m_passes(passes)
    , m_scriptExternalColor(this)
    , m_renderTargetIndexOffset(SIZE_MAX)
    , m_offsets(0, 0)
    , m_clearColorScriptIndex(-1)
    , m_clearDepthScriptIndex(-1)
{
    const String &v = annotations.stringAnnotation(kScriptKeyLiteral, String());
    if (!v.empty()) {
        Effect::parseScript(
            v, m_scriptCommands, m_clearColorScriptIndex, m_clearDepthScriptIndex, m_scriptExternalColor.m_exists);
    }
    else {
        String s;
        for (PassList::const_iterator it = passes.begin(), end = passes.end(); it != end; ++it) {
            const Pass *pass = *it;
            s.append("Pass=");
            s.append(pass->nameConstString());
            s.append(";");
        }
        Effect::parseScript(s, m_scriptCommands);
    }
    for (PassList::const_iterator it = passes.begin(), end = passes.end(); it != end; ++it) {
        Pass *pass = *it;
        pass->setParentTechnique(this);
        m_passRefs.insert(tinystl::make_pair(pass->name(), pass));
    }
    Inline::clearZeroMemory(m_pipelineDescription);
}

Technique::~Technique() NANOEM_DECL_NOEXCEPT
{
    for (PassList::iterator it = m_passes.begin(), end = m_passes.end(); it != end; ++it) {
        Pass *pass = *it;
        nanoem_delete(pass);
    }
    m_passes.clear();
}

void
Technique::destroy()
{
    SG_PUSH_GROUPF("effect::Technique::destroy(name=%s)", nameConstString());
    for (PassList::iterator it = m_passes.begin(), end = m_passes.end(); it != end; ++it) {
        Pass *pass = *it;
        pass->destroy();
    }
    SG_POP_GROUP();
}

void
Technique::ensureScriptCommandClear()
{
    if (m_clearColorScriptIndex > -1 && m_clearDepthScriptIndex > -1) {
        PassMap::const_iterator it = m_passRefs.find(m_scriptCommands[m_clearColorScriptIndex].second);
        if (it != m_passRefs.end()) {
            Pass *pass = it->second;
            pass->ensureScriptCommandClearDepth();
            pass->ensureScriptCommandClearColor();
        }
        // output.push_back(tinystl::make_pair(kScriptCommandTypeClear, kScriptClearColorValue));
        // output.push_back(tinystl::make_pair(kScriptCommandTypeClear, kScriptClearDepthValue));
    }
    else if (m_clearColorScriptIndex > -1) {
        PassMap::const_iterator it = m_passRefs.find(m_scriptCommands[m_clearColorScriptIndex].second);
        if (it != m_passRefs.end()) {
            Pass *pass = it->second;
            pass->ensureScriptCommandClearColor();
        }
        // output.push_back(tinystl::make_pair(kScriptCommandTypeClear, kScriptClearColorValue));
    }
    else if (m_clearDepthScriptIndex > -1) {
        PassMap::const_iterator it = m_passRefs.find(m_scriptCommands[m_clearDepthScriptIndex].second);
        if (it != m_passRefs.end()) {
            Pass *pass = it->second;
            pass->ensureScriptCommandClearDepth();
        }
        // output.push_back(tinystl::make_pair(kScriptCommandTypeClear, kScriptClearDepthValue));
    }
}

IPass *
Technique::execute(const IDrawable *drawable, bool scriptExternalColor)
{
    const size_t numScriptIndices = m_scriptCommands.size();
    Pass *passPtr = nullptr;
    if (scriptExternalColor) {
        for (size_t scriptIndex = m_offsets.first; scriptIndex < numScriptIndices; scriptIndex++) {
            const tinystl::pair<ScriptCommandType, String> &script = m_scriptCommands[scriptIndex];
            if (!LoopCounter::isScriptCommandIgnorable(script.first, m_counterStack)) {
                if (script.first == kScriptCommandTypeSetScriptExternal) {
                    SG_INSERT_MARKERF("%d: %s/SetScriptExternal", scriptIndex, nameConstString());
                    char nameBuffer[Inline::kMarkerStringLength];
                    StringUtils::format(nameBuffer, sizeof(nameBuffer), "%s/%s/ScriptExternalColor",
                        m_effect->nameConstString(), nameConstString());
                    m_effect->updateCurrentRenderTargetPixelFormatSampleCount();
                    m_scriptExternalColor.save(drawable, nameBuffer);
                    m_offsets.second = scriptIndex + 1;
                    break;
                }
                else if (!interpretScriptCommand(
                             script.first, script.second, drawable, passPtr, scriptIndex, m_offsets.first)) {
                    break;
                }
            }
        }
    }
    else {
        m_scriptExternalColor.blit();
        size_t scriptIndex = m_offsets.second;
        for (; scriptIndex < numScriptIndices; scriptIndex++) {
            const tinystl::pair<ScriptCommandType, String> &script = m_scriptCommands[scriptIndex];
            if (!LoopCounter::isScriptCommandIgnorable(script.first, m_counterStack) &&
                !interpretScriptCommand(
                    script.first, script.second, drawable, passPtr, scriptIndex, m_offsets.second)) {
                break;
            }
        }
        if (scriptIndex == numScriptIndices) {
            m_offsets.second = numScriptIndices;
        }
    }
    return passPtr;
}

void
Technique::resetScriptCommandState()
{
    m_counterStack.clear();
    m_offsets.first = m_offsets.second = 0;
    m_renderPassScope.reset(nullptr);
}

void
Technique::resetScriptExternalColor()
{
    m_scriptExternalColor.reset();
}

bool
Technique::hasNextScriptCommand() const NANOEM_DECL_NOEXCEPT
{
    return m_offsets.second < m_scriptCommands.size();
}

bool
Technique::match(const Accessory::Material *material) const NANOEM_DECL_NOEXCEPT
{
    bool found = true;
    if (material) {
        bool isSphereMapTexture =
            material->sphereTextureMapType() == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_MULTIPLY ||
            material->sphereTextureMapType() == NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_ADD;
        const IImageView *imagePtr = material->diffuseImage();
        const int hasTexture = imagePtr && sg::is_valid(imagePtr->handle()) ? 1 : 0;
        const int hasToonTexture = 0;
        if (m_useDiffuseTexture >= 0 && !isSphereMapTexture) {
            found = found && (hasTexture == m_useDiffuseTexture);
        }
        if (m_useSphereMapTexture >= 0 && isSphereMapTexture) {
            found = found && (hasTexture == m_useSphereMapTexture);
        }
        if (m_useToonTexture >= 0) {
            found = found && (hasToonTexture == m_useToonTexture);
        }
    }
    return found;
}

bool
Technique::match(const nanoem_model_material_t *materialPtr) const NANOEM_DECL_NOEXCEPT
{
    const model::Material *material = model::Material::cast(materialPtr);
    bool found = true;
    if (m_useDiffuseTexture >= 0) {
        const IImageView *imagePtr = material->diffuseImage();
        found = found && ((imagePtr && sg::is_valid(imagePtr->handle()) ? 1 : 0) == m_useDiffuseTexture);
    }
    if (m_useSphereMapTexture >= 0) {
        const IImageView *imagePtr = material->sphereMapImage();
        const int hasSphereMapTexture = imagePtr && sg::is_valid(imagePtr->handle()) &&
                nanoemModelMaterialGetSphereMapTextureType(materialPtr) !=
                    NANOEM_MODEL_MATERIAL_SPHERE_MAP_TEXTURE_TYPE_NONE
            ? 1
            : 0;
        found = found && (hasSphereMapTexture == m_useSphereMapTexture);
    }
    if (m_useToonTexture >= 0) {
        const int hasToonTexture = 1;
        found = found && (hasToonTexture == m_useToonTexture);
    }
    return found;
}

void
Technique::overrideObjectPipelineDescription(
    const IDrawable *drawable, PipelineDescriptor &pd) const NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUPF("effect::Technique::overrideObjectPipelineDescription(name=%s)", drawable->nameConstString());
    sg_pipeline_desc &body = pd.m_body;
    memcpy(&body.layout, &m_pipelineDescription.layout, sizeof(body.layout));
    body.index_type = m_pipelineDescription.index_type;
    if (body.cull_mode == _SG_CULLMODE_DEFAULT) {
        body.cull_mode = m_pipelineDescription.cull_mode;
        SG_INSERT_MARKERF("effect::Technique::overrideObjectPipelineDescription(cullMode=%d)", body.cull_mode);
    }
    if (body.primitive_type == _SG_PRIMITIVETYPE_DEFAULT) {
        body.primitive_type = m_pipelineDescription.primitive_type;
        SG_INSERT_MARKERF(
            "effect::Technique::overrideObjectPipelineDescription(primitiveType=%d)", body.primitive_type);
    }
    overrideColorState(drawable, pd, m_pipelineDescription.colors[0], body.colors[0]);
    overrideDepthState(pd, m_pipelineDescription.depth, body.depth);
    overrideStencilState(pd, m_pipelineDescription.stencil, body.stencil);
    m_effect->overridePipelineDescription(body, Effect::kScriptClassTypeObject);
    SG_POP_GROUP();
}

void
Technique::overrideScenePipelineDescription(
    const IDrawable *drawable, PipelineDescriptor &pd) const NANOEM_DECL_NOEXCEPT
{
    sg_pipeline_desc &body = pd.m_body;
    body.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    body.face_winding = SG_FACEWINDING_CW;
    body.cull_mode = SG_CULLMODE_BACK;
    sg_layout_desc &ld = body.layout;
    Inline::clearZeroMemory(ld.attrs);
    ld.buffers[0].stride = sizeof(sg::QuadVertexUnit);
    /* dummy location to be same as Accessory/Model vertex description */
    ld.attrs[0] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[1] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[2] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[3] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[4] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[5] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[6] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT2 };
    ld.attrs[7] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT2 };
    overrideColorState(drawable, pd, m_pipelineDescription.colors[0], body.colors[0]);
    overrideDepthState(pd, m_pipelineDescription.depth, body.depth);
    overrideStencilState(pd, m_pipelineDescription.stencil, body.stencil);
    m_effect->overridePipelineDescription(body, Effect::kScriptClassTypeScene);
}

const Effect *
Technique::effect() const NANOEM_DECL_NOEXCEPT
{
    return m_effect;
}

Effect *
Technique::effect() NANOEM_DECL_NOEXCEPT
{
    return m_effect;
}

RenderPassScope *
Technique::currentRenderPassScope() NANOEM_DECL_NOEXCEPT
{
    RenderPassScope *offscreenRenderPassScope = m_effect->project()->offscreenRenderPassScope();
    return offscreenRenderPassScope ? offscreenRenderPassScope : renderPassScope();
}

RenderPassScope *
Technique::renderPassScope() NANOEM_DECL_NOEXCEPT
{
    return &m_renderPassScope;
}

void
Technique::getAllPasses(PassList &value) const
{
    value = m_passes;
}

String
Technique::name() const
{
    return m_name;
}

const char *
Technique::nameConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_name.c_str();
}

String
Technique::subset() const
{
    return m_subset;
}

String
Technique::passType() const
{
    return m_passType;
}

bool
Technique::hasScriptExternal() const NANOEM_DECL_NOEXCEPT
{
    return m_scriptExternalColor.m_exists;
}

sg_pipeline_desc &
Technique::mutablePipelineDescription() NANOEM_DECL_NOEXCEPT
{
    return m_pipelineDescription;
}

void
Technique::overrideColorState(const IDrawable *drawable, const PipelineDescriptor &pd, const sg_color_state &csrc,
    sg_color_state &cdst) NANOEM_DECL_NOEXCEPT
{
    const sg_blend_state &src = csrc.blend;
    sg_blend_state &dst = cdst.blend;
    const bool hasBlendEnabled = pd.m_hasBlendEnabled;
    if (!hasBlendEnabled) {
        dst.enabled = true;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(enabled=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.enabled), EnumStringifyUtils::toString(hasBlendEnabled));
    const bool hasBlendSourceFactorRGB = pd.m_hasBlendSourceFactorRGB;
    if (!hasBlendSourceFactorRGB) {
        dst.src_factor_rgb =
            src.src_factor_rgb != _SG_BLENDFACTOR_DEFAULT ? src.src_factor_rgb : SG_BLENDFACTOR_SRC_ALPHA;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(srcFactorRGB=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.src_factor_rgb), EnumStringifyUtils::toString(hasBlendSourceFactorRGB));
    const bool hasBlendDestFactorRGB = pd.m_hasBlendDestFactorRGB;
    if (!hasBlendDestFactorRGB) {
        dst.dst_factor_rgb = src.dst_factor_rgb != _SG_BLENDFACTOR_DEFAULT ? src.dst_factor_rgb
            : (drawable && drawable->isAddBlendEnabled())                  ? SG_BLENDFACTOR_ONE
                                                                           : SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(dstFactorRGB=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.dst_factor_rgb), EnumStringifyUtils::toString(hasBlendDestFactorRGB));
    const bool hasBlendSourceFactorAlpha = pd.m_hasBlendSourceFactorAlpha;
    if (!hasBlendSourceFactorAlpha) {
        dst.src_factor_alpha =
            src.src_factor_alpha != _SG_BLENDFACTOR_DEFAULT ? src.src_factor_alpha : SG_BLENDFACTOR_ONE;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(srcFactorAlpha=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.src_factor_alpha), EnumStringifyUtils::toString(hasBlendSourceFactorAlpha));
    const bool hasBlendDestFactorAlpha = pd.m_hasBlendDestFactorAlpha;
    if (!hasBlendDestFactorAlpha) {
        dst.dst_factor_alpha =
            src.dst_factor_alpha != _SG_BLENDFACTOR_DEFAULT ? src.dst_factor_alpha : SG_BLENDFACTOR_ZERO;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(dstFactorAlpha=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.dst_factor_alpha), EnumStringifyUtils::toString(hasBlendDestFactorAlpha));
    const bool hasColorWriteMask = pd.m_hasColorWriteMask;
    if (!hasColorWriteMask) {
        cdst.write_mask = csrc.write_mask;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(colorWriteMask=0x%x, wasSet=%s)", cdst.write_mask,
        EnumStringifyUtils::toString(hasColorWriteMask));
    const bool hasBlendOpRGB = pd.m_hasBlendOpRGB;
    if (!hasBlendOpRGB) {
        dst.op_rgb = src.op_rgb != _SG_BLENDOP_DEFAULT ? src.op_rgb : SG_BLENDOP_ADD;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(opRGB=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.op_rgb), EnumStringifyUtils::toString(hasBlendOpRGB));
#if defined(NANOEM_ENABLE_BLENDOP_MINMAX)
    const bool hasBlendOpAlpha = pd.m_hasBlendOpAlpha;
    if (!hasBlendOpAlpha) {
        dst.op_alpha = src.op_alpha != _SG_BLENDOP_DEFAULT ? src.op_alpha : SG_BLENDOP_MAX;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideColorState(opAlpha=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.op_alpha), EnumStringifyUtils::toString(hasBlendOpAlpha));
#endif /* NANOEM_ENABLE_BLENDOP_MINMAX */
}

void
Technique::overrideDepthState(
    const PipelineDescriptor &pd, const sg_depth_state &src, sg_depth_state &dst) NANOEM_DECL_NOEXCEPT
{
    const bool hasDepthCompareFunc = pd.m_hasDepthCompareFunc;
    if (!hasDepthCompareFunc) {
        const bool hasDepthCompareFuncSet = src.compare != _SG_COMPAREFUNC_DEFAULT;
        dst.compare = hasDepthCompareFuncSet ? src.compare : SG_COMPAREFUNC_LESS_EQUAL;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideDepthState(depthCompareFunc=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.compare), EnumStringifyUtils::toString(hasDepthCompareFunc));
    const bool hasDepthWriteEnabled = pd.m_hasDepthWriteEnabled;
    if (!hasDepthWriteEnabled) {
        dst.write_enabled = src.write_enabled;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideDepthState(depthWriteEnabled=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.write_enabled), EnumStringifyUtils::toString(hasDepthWriteEnabled));
}

void
Technique::overrideStencilState(
    const PipelineDescriptor &pd, const sg_stencil_state &src, sg_stencil_state &dst) NANOEM_DECL_NOEXCEPT
{
    const bool hasStencilEnabled = pd.m_hasStencilEnabled;
    if (!hasStencilEnabled) {
        dst.enabled = src.enabled;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilState(stencilEnabled=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.enabled), EnumStringifyUtils::toString(hasStencilEnabled));
    const bool hasStencilRef = pd.m_hasStencilRef;
    if (!hasStencilRef) {
        dst.ref = src.ref;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilState(stencilRef=0x%x, wasSet=%s)", dst.ref,
        EnumStringifyUtils::toString(hasStencilRef));
    const bool hasStencilReadMask = pd.m_hasStencilReadMask;
    if (!hasStencilReadMask) {
        dst.read_mask = src.read_mask;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilState(stencilReadMask=0x%x, wasSet=%s)", dst.read_mask,
        EnumStringifyUtils::toString(hasStencilReadMask));
    const bool hasStencilWriteMask = pd.m_hasStencilWriteMask;
    if (!hasStencilWriteMask) {
        dst.write_mask = src.write_mask;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilState(stencilWriteMask=0x%x, wasSet=%s)", dst.write_mask,
        EnumStringifyUtils::toString(hasStencilWriteMask));
    overrideStencilFaceState(pd.m_stencilFront, src.front, dst.front);
    overrideStencilFaceState(pd.m_stencilBack, src.back, dst.back);
}

void
Technique::overrideStencilFaceState(const PipelineDescriptor::Stencil &sd, const sg_stencil_face_state &src,
    sg_stencil_face_state &dst) NANOEM_DECL_NOEXCEPT
{
    const bool hasCompareFunc = sd.m_hasCompareFunc;
    if (!hasCompareFunc) {
        dst.compare = src.compare;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilFaceState(compareFunc=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.compare), EnumStringifyUtils::toString(hasCompareFunc));
    const bool hasPassOp = sd.m_hasPassOp;
    if (!hasPassOp) {
        dst.pass_op = src.pass_op;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilFaceState(passOp=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.pass_op), EnumStringifyUtils::toString(hasPassOp));
    const bool hasFailOp = sd.m_hasFailOp;
    if (!hasFailOp) {
        dst.fail_op = src.fail_op;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilFaceState(failOp=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.fail_op), EnumStringifyUtils::toString(hasFailOp));
    const bool hasDepthFailOp = sd.m_hasDepthFailOp;
    if (!hasDepthFailOp) {
        dst.depth_fail_op = src.depth_fail_op;
    }
    SG_INSERT_MARKERF("effect::Technique::overrideStencilFaceState(depthFailOp=%s, wasSet=%s)",
        EnumStringifyUtils::toString(dst.depth_fail_op), EnumStringifyUtils::toString(hasDepthFailOp));
}

bool
Technique::interpretScriptCommand(ScriptCommandType type, const String &value, const IDrawable *drawable,
    effect::Pass *&pass, size_t &scriptIndex, size_t &savedOffset)
{
    bool continuable = true;
    switch (type) {
    case kScriptCommandTypePushLoopCounter: {
        SG_INSERT_MARKERF(
            "%d: %s/%s/LoopByCount=%s", scriptIndex, m_effect->nameConstString(), nameConstString(), value.c_str());
        m_effect->pushLoopCounter(value, scriptIndex, m_counterStack);
        break;
    }
    case kScriptCommandTypeClear: {
        SG_INSERT_MARKERF(
            "%d: %s/%s/Clear=%s", scriptIndex, m_effect->nameConstString(), nameConstString(), value.c_str());
        char nameBuffer[Inline::kMarkerStringLength];
        StringUtils::format(
            nameBuffer, sizeof(nameBuffer), "Effects/%s/Techniques/%s", m_effect->nameConstString(), nameConstString());
        m_effect->clearRenderPass(drawable, nameBuffer, value, currentRenderPassScope());
        break;
    }
    case kScriptCommandTypeSetRenderDepthStencilTarget: {
        SG_INSERT_MARKERF("%d: %s/%s/SetRenderDepthStencilTarget=%s", scriptIndex, m_effect->nameConstString(),
            nameConstString(), value.empty() ? "(null)" : value.c_str());
        m_effect->setRenderTargetDepthStencilImageDescription(value);
        break;
    }
    case kScriptCommandTypeGetLoopIndex: {
        SG_INSERT_MARKERF("%d: %s/%s/GetLoopIndex=%s", scriptIndex, m_effect->nameConstString(), nameConstString(),
            value.empty() ? "(null)" : value.c_str());
        m_effect->handleLoopGetIndex(value, m_counterStack);
        break;
    }
    case kScriptCommandTypeClearSetColor: {
        SG_INSERT_MARKERF(
            "%d: %s/%s/ClearSetColor=%s", scriptIndex, m_effect->nameConstString(), nameConstString(), value.c_str());
        m_effect->setClearColor(value);
        break;
    }
    case kScriptCommandTypeClearSetDepth: {
        SG_INSERT_MARKERF(
            "%d: %s/%s/ClearSetDepth=%s", scriptIndex, m_effect->nameConstString(), nameConstString(), value.c_str());
        m_effect->setClearDepth(value);
        break;
    }
    case kScriptCommandTypePopLoopCounter: {
        SG_INSERT_MARKERF("%d: %s/%s/LoopEnd", scriptIndex, m_effect->nameConstString(), nameConstString());
        m_effect->popLoopCounter(m_counterStack, scriptIndex);
        savedOffset = scriptIndex + 1;
        break;
    }
    case kScriptCommandTypeSetRenderColorTarget0:
    case kScriptCommandTypeSetRenderColorTarget1:
    case kScriptCommandTypeSetRenderColorTarget2:
    case kScriptCommandTypeSetRenderColorTarget3: {
        const size_t renderTargetIndexOffset = type - kScriptCommandTypeSetRenderColorTarget0;
        SG_INSERT_MARKERF("%d: %s/%s/RenderColorTarget%d=%s", scriptIndex, m_effect->nameConstString(),
            nameConstString(), renderTargetIndexOffset, value.empty() ? "(null)" : value.c_str());
        m_effect->setRenderTargetColorImageDescription(drawable, renderTargetIndexOffset, value);
        m_renderTargetIndexOffset = renderTargetIndexOffset;
        break;
    }
    case kScriptCommandTypeExecutePass: {
        SG_INSERT_MARKERF(
            "%d: %s/%s/Pass=%s", scriptIndex, m_effect->nameConstString(), nameConstString(), value.c_str());
        PassMap::iterator it = m_passRefs.find(value);
        if (it != m_passRefs.end()) {
            pass = it->second;
            pass->setRenderTargetIndexOffset(m_renderTargetIndexOffset);
            pass->setTechniqueScriptIndex(scriptIndex);
            savedOffset = scriptIndex + 1;
            continuable = false;
        }
        else {
            m_effect->logger()->log("Pass \"%s/%s/%s\" cannot be executed due to not found",
                m_effect->nameConstString(), nameConstString(), value.c_str());
        }
        break;
    }
    case kScriptCommandTypeDraw:
    case kScriptCommandTypeMaxEnum:
    case kScriptCommandTypeSetScriptExternal:
    default:
        m_effect->logger()->log("Command %d in \"%s/%s\" at %d is not accepted therefore ignored", type,
            m_effect->nameConstString(), nameConstString(), scriptIndex);
        break;
    }
    return continuable;
}

} /* namespace effect */
} /* namespace nanoem */
