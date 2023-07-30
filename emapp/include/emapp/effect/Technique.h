/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_EFFECT_TECHNIQUE_H_
#define NANOEM_EMAPP_EFFECT_TECHNIQUE_H_

#include "emapp/ITechnique.h"

#include "emapp/Accessory.h"
#include "emapp/effect/Pass.h"

namespace nanoem {

class IPass;

namespace effect {

class Technique NANOEM_DECL_SEALED : public ITechnique {
public:
    Technique(Effect *effect, const String &name, const AnnotationMap &annotations, const PassList &passes);
    ~Technique() NANOEM_DECL_NOEXCEPT;

    void destroy();
    void ensureScriptCommandClear();

    IPass *execute(const IDrawable *drawable, bool scriptExternalColor) NANOEM_DECL_OVERRIDE;
    void resetScriptCommandState() NANOEM_DECL_OVERRIDE;
    void resetScriptExternalColor() NANOEM_DECL_OVERRIDE;
    bool hasNextScriptCommand() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    bool match(const Accessory::Material *material) const NANOEM_DECL_NOEXCEPT;
    bool match(const nanoem_model_material_t *materialPtr) const NANOEM_DECL_NOEXCEPT;
    void overrideObjectPipelineDescription(
        const IDrawable *drawable, PipelineDescriptor &pd) const NANOEM_DECL_NOEXCEPT;
    void overrideScenePipelineDescription(const IDrawable *drawable, PipelineDescriptor &pd) const NANOEM_DECL_NOEXCEPT;

    const Effect *effect() const NANOEM_DECL_NOEXCEPT;
    Effect *effect() NANOEM_DECL_NOEXCEPT;
    RenderPassScope *currentRenderPassScope() NANOEM_DECL_NOEXCEPT;
    RenderPassScope *renderPassScope() NANOEM_DECL_NOEXCEPT;
    void getAllPasses(PassList &value) const;
    String name() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    String subset() const;
    String passType() const;
    bool hasScriptExternal() const NANOEM_DECL_NOEXCEPT;
    sg_pipeline_desc &mutablePipelineDescription() NANOEM_DECL_NOEXCEPT;

private:
    struct ScriptExternal {
        ScriptExternal(Technique *techniquePtr);
        ~ScriptExternal() NANOEM_DECL_NOEXCEPT;
        void save(const IDrawable *drawable, const char *name);
        void blit();
        void reset();
        Technique *m_techniquePtr;
        sg_pass m_destinationPass;
        bool m_blitted;
        bool m_exists;
    };

    static void overrideColorState(const IDrawable *drawable, const PipelineDescriptor &pd,
        const sg_color_target_state &src, sg_color_target_state &dst) NANOEM_DECL_NOEXCEPT;
    static void overrideDepthState(
        const PipelineDescriptor &pd, const sg_depth_state &src, sg_depth_state &dst) NANOEM_DECL_NOEXCEPT;
    static void overrideStencilState(
        const PipelineDescriptor &pd, const sg_stencil_state &src, sg_stencil_state &dst) NANOEM_DECL_NOEXCEPT;
    static void overrideStencilFaceState(const PipelineDescriptor::Stencil &sd, const sg_stencil_face_state &src,
        sg_stencil_face_state &dst) NANOEM_DECL_NOEXCEPT;

    bool interpretScriptCommand(ScriptCommandType type, const String &value, const IDrawable *drawable,
        effect::Pass *&pass, size_t &scriptIndex, size_t &savedOffset);

    const String m_name;
    const String m_subset;
    const String m_passType;
    const int m_useDiffuseTexture;
    const int m_useSphereMapTexture;
    const int m_useToonTexture;
    Effect *m_effect;
    PassList m_passes;
    PassMap m_passRefs;
    RenderPassScope m_renderPassScope;
    LoopCounter::Stack m_counterStack;
    ScriptCommandMap m_scriptCommands;
    ScriptExternal m_scriptExternalColor;
    sg_pipeline_desc m_pipelineDescription;
    size_t m_renderTargetIndexOffset;
    tinystl::pair<size_t, size_t> m_offsets;
    int m_clearColorScriptIndex;
    int m_clearDepthScriptIndex;
};
typedef tinystl::vector<Technique *, TinySTLAllocator> TechniqueList;

} /* namespace effect */
} /* namespace nanoem */

#endif /* NANOEM_EFFECT_TECHNIQUE_H_ */
