/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/RenderTargetNormalizer.h"

#include "emapp/Effect.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {

RenderTargetNormalizer::RenderTargetNormalizer(Effect *effect)
    : m_effect(effect)
{
    Inline::clearZeroMemory(m_originColorImageFormats);
    Inline::clearZeroMemory(m_normalizedColorImageFormat);
    Inline::clearZeroMemory(m_originColorImageRefs);
    Inline::clearZeroMemory(m_normalizedColorImages);
    Inline::clearZeroMemory(m_originPasses);
    Inline::clearZeroMemory(m_normalizedPasses);
}

RenderTargetNormalizer::~RenderTargetNormalizer() NANOEM_DECL_NOEXCEPT
{
}

void
RenderTargetNormalizer::destroy()
{
    SG_PUSH_GROUP("effect::RenderTargetNormalizer::destroy");
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        SG_INSERT_MARKERF(
            "effect::RenderTargetNormalizer::destroy(index=%d, origin=%d, normalizePass=%d, normalizeColorImage=%d)", i,
            m_originPasses[i].first.id, m_normalizedPasses[i].first.id, m_normalizedColorImages[i].first.id);
        sg::destroy_pass(m_originPasses[i].first);
        sg::destroy_pass(m_normalizedPasses[i].first);
        sg::destroy_image(m_normalizedColorImages[i].first);
    }
    SG_POP_GROUP();
}

void
RenderTargetNormalizer::normalize(const IDrawable *drawable, const Pass *passPtr,
    const PixelFormat &preferredPixelFormat, sg_pixel_format normalizedColorFormat,
    sg_pass_desc &currentPassDescriptionRef)
{
    Project *project = m_effect->project();
    sg_pass_desc originPassDescription;
    for (nanoem_rsize_t i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        const sg_image originColorImage = currentPassDescriptionRef.color_attachments[i].image;
        if (sg::is_valid(originColorImage)) {
            Inline::clearZeroMemory(originPassDescription);
            originPassDescription.color_attachments[0].image = originColorImage;
            originPassDescription.depth_stencil_attachment = currentPassDescriptionRef.depth_stencil_attachment;
            if (normalizedColorFormat != project->viewportPixelFormat() &&
                originColorImage.id == project->viewportPrimaryImage().id) {
                normalizePrimaryViewportImage(
                    preferredPixelFormat, originPassDescription, i, normalizedColorFormat, currentPassDescriptionRef);
            }
            else if (const RenderTargetColorImageContainer *containerPtr =
                         m_effect->searchRenderTargetColorImageContainer(drawable, originColorImage)) {
                const sg_pixel_format colorImageFormat = containerPtr->colorImageDescription().pixel_format;
                PixelFormat pixelFormat(preferredPixelFormat);
                if (i > 0) {
                    pixelFormat.setColorPixelFormat(colorImageFormat, 0);
                }
                if (normalizedColorFormat != colorImageFormat) {
                    normalizeRenderTargetColorImage(containerPtr, pixelFormat, originPassDescription, i,
                        normalizedColorFormat, currentPassDescriptionRef);
                }
                else {
                    resolveRenderTargetIOConfliction(passPtr, containerPtr, pixelFormat, originPassDescription, i,
                        normalizedColorFormat, currentPassDescriptionRef);
                }
            }
        }
    }
}

void
RenderTargetNormalizer::read()
{
    SG_PUSH_GROUP("effect::RenderTargetNormalizer::read");
    PixelFormat format(m_normalizedColorImageFormat);
    internal::BlitPass *blitter = m_effect->project()->sharedImageBlitter();
    sg::PassBlock::IDrawQueue *drawQueue = m_effect->project()->sharedBatchDrawQueue();
    format.setNumColorAttachemnts(1);
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        const sg::NamedImage &source = m_originColorImageRefs[i];
        if (sg::is_valid(source.first)) {
            blitter->blit(drawQueue, m_normalizedPasses[i], source, Vector4(0, 0, 1, 1), format);
        }
    }
    SG_POP_GROUP();
}

void
RenderTargetNormalizer::apply(
    const sg_bindings &bindings, const IDrawable *drawable, effect::Pass *pass, sg::PassBlock &pb)
{
    PipelineDescriptor dest(pass->pipelineDescriptor());
    pass->technique()->overrideObjectPipelineDescription(drawable, dest);
    dest.m_body.color_count = m_normalizedColorImageFormat.numColorAttachments();
    dest.m_body.colors[0].pixel_format = m_normalizedColorImageFormat.colorPixelFormat(0);
    sg_pipeline pipeline = pass->registerPipelineSet(dest.m_body);
    pb.applyPipelineBindings(pipeline, bindings);
    SG_INSERT_MARKERF("effect::RenderTargetNormalizer::apply(pipeline=%d, format=%d)", pipeline.id,
        m_normalizedColorImageFormat.colorPixelFormat(0));
}

void
RenderTargetNormalizer::write()
{
    SG_PUSH_GROUP("effect::RenderTargetNormalizer::write");
    internal::BlitPass *blitter = m_effect->project()->sharedImageBlitter();
    sg::PassBlock::IDrawQueue *drawQueue = m_effect->project()->sharedBatchDrawQueue();
    for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
        const sg::NamedImage &source = m_normalizedColorImages[i];
        if (sg::is_valid(source.first)) {
            const PixelFormat &format = m_originColorImageFormats[i];
            blitter->blit(drawQueue, m_originPasses[i], source, Vector4(0, 0, 1, 1), format);
        }
    }
    SG_POP_GROUP();
}

void
RenderTargetNormalizer::normalizePrimaryViewportImage(const PixelFormat &originFormat,
    const sg_pass_desc &originPassDescription, nanoem_rsize_t index, sg_pixel_format normalizedColorFormat,
    sg_pass_desc &currentPassDescriptionRef)
{
    sg_image &colorImage = m_normalizedColorImages[index].first;
    if (!sg::is_valid(colorImage)) {
        const Project *project = m_effect->project();
        sg_image_desc id;
        Inline::clearZeroMemory(id);
        const Vector2 imageSize(project->deviceScaleUniformedViewportImageSize());
        id.width = static_cast<int>(imageSize.x);
        id.height = static_cast<int>(imageSize.y);
        id.render_target = true;
        id.pixel_format = normalizedColorFormat;
        id.sample_count = project->sampleCount();
        createNormalizePass(Project::kViewportPrimaryName, originFormat, id, originPassDescription, index);
    }
    currentPassDescriptionRef.color_attachments[index].image = colorImage;
}

void
RenderTargetNormalizer::normalizeOffscreenRenderTargetColorImage(const sg_image_desc &originImageDescription,
    const PixelFormat &originFormat, const sg_pass_desc &originPassDescription, nanoem_rsize_t index,
    sg_pixel_format normalizedColorFormat, sg_pass_desc &currentPassDescriptionRef)
{
    sg_image &colorImage = m_normalizedColorImages[index].first;
    if (!sg::is_valid(colorImage)) {
        sg_image_desc id(originImageDescription);
        id.render_target = true;
        id.pixel_format = normalizedColorFormat;
        createNormalizePass(id.label, originFormat, id, originPassDescription, index);
    }
    currentPassDescriptionRef.color_attachments[index].image = colorImage;
}

void
RenderTargetNormalizer::normalizeRenderTargetColorImage(const RenderTargetColorImageContainer *containerPtr,
    const PixelFormat &originFormat, const sg_pass_desc &originPassDescription, nanoem_rsize_t index,
    sg_pixel_format normalizedColorFormat, sg_pass_desc &currentPassDescriptionRef)
{
    const sg_image_desc &originImageDescription = containerPtr->colorImageDescription();
    sg_image &colorImage = m_normalizedColorImages[index].first;
    if (!sg::is_valid(colorImage)) {
        sg_image_desc id(originImageDescription);
        id.pixel_format = normalizedColorFormat;
        createNormalizePass(containerPtr->nameConstString(), originFormat, id, originPassDescription, index);
    }
    currentPassDescriptionRef.color_attachments[index].image = colorImage;
}

void
RenderTargetNormalizer::resolveRenderTargetIOConfliction(const Pass *passPtr,
    const RenderTargetColorImageContainer *containerPtr, const PixelFormat &originFormat,
    const sg_pass_desc &originPassDescription, nanoem_rsize_t index, sg_pixel_format normalizedColorFormat,
    sg_pass_desc &currentPassDescriptionRef)
{
    if (const ImageSamplerList *images = m_effect->findImageSamplerList(passPtr)) {
        for (int i = 0; i < SG_MAX_COLOR_ATTACHMENTS; i++) {
            sg_image outputColorImage = currentPassDescriptionRef.color_attachments[i].image;
            if (sg::is_valid(outputColorImage)) {
                for (ImageSamplerList::const_iterator it = images->begin(), end = images->end(); it != end; ++it) {
                    const ImageSampler &inputSampler = *it;
                    if (inputSampler.m_image.id == outputColorImage.id) {
                        const sg_image_desc &originImageDescription = containerPtr->colorImageDescription();
                        sg_image &colorImage = m_normalizedColorImages[index].first;
                        if (!sg::is_valid(colorImage)) {
                            const char *name = containerPtr->nameConstString();
                            sg_image_desc normalizedImageDescription(originImageDescription);
                            normalizedImageDescription.pixel_format = normalizedColorFormat;
                            createNormalizePass(
                                name, originFormat, normalizedImageDescription, originPassDescription, index);
                        }
                        currentPassDescriptionRef.color_attachments[index].image = colorImage;
                    }
                }
            }
        }
    }
}

void
RenderTargetNormalizer::createNormalizePass(const char *name, const PixelFormat &originFormat,
    const sg_image_desc &normalizedImageDescription, const sg_pass_desc &originPassDescription, nanoem_rsize_t index)
{
    SG_PUSH_GROUP("effect::RenderTargetNormalizer::createNormalizerPass");
    Project *project = m_effect->project();
    sg_image &normalizedColorImage = m_normalizedColorImages[index].first;
    StringUtils::format(
        m_originColorImageNames[index], "Effects/%s/RTN/%s/OriginImage", m_effect->nameConstString(), name);
    StringUtils::format(
        m_normalizedColorImageNames[index], "Effects/%s/RTN/%s/NormalizedImage", m_effect->nameConstString(), name);
    {
        sg_pass originPass = { SG_INVALID_ID };
        if (Inline::isDebugLabelEnabled()) {
            sg_image_desc labeledNormalizedImageDescription(normalizedImageDescription);
            labeledNormalizedImageDescription.label = m_normalizedColorImageNames[index].c_str();
            normalizedColorImage = sg::make_image(&labeledNormalizedImageDescription);
            sg_pass_desc labeledOriginPassDescription(originPassDescription);
            labeledOriginPassDescription.label = m_originColorImageNames[index].c_str();
            originPass = sg::make_pass(&labeledOriginPassDescription);
        }
        else {
            normalizedColorImage = sg::make_image(&normalizedImageDescription);
            originPass = sg::make_pass(&originPassDescription);
        }
        nanoem_assert(sg::query_image_state(normalizedColorImage) == SG_RESOURCESTATE_VALID, "image must be valid");
        nanoem_assert(sg::query_pass_state(originPass) == SG_RESOURCESTATE_VALID, "pass must be valid");
        m_originPasses[index].first = originPass;
        const char *sourceName = m_normalizedColorImages[index].second = m_normalizedColorImageNames[index].c_str();
        m_effect->setImageLabel(normalizedColorImage, sourceName);
        const char *destName = m_originPasses[index].second = m_originColorImageNames[index].c_str();
        project->setRenderPassName(originPass, destName);
    }
    {
        sg_pass_desc newOriginPassDescription(originPassDescription);
        sg_pass normalizePass = { SG_INVALID_ID };
        newOriginPassDescription.color_attachments[0].image = normalizedColorImage;
        if (Inline::isDebugLabelEnabled()) {
            sg_pass_desc labeledOriginPassDescription(newOriginPassDescription);
            labeledOriginPassDescription.label = m_normalizedColorImageNames[index].c_str();
            normalizePass = sg::make_pass(&labeledOriginPassDescription);
        }
        else {
            normalizePass = sg::make_pass(&newOriginPassDescription);
        }
        nanoem_assert(sg::query_pass_state(normalizePass) == SG_RESOURCESTATE_VALID, "pass must be valid");
        m_normalizedPasses[index].first = normalizePass;
        const char *destName = m_normalizedPasses[index].second = m_normalizedColorImageNames[index].c_str();
        project->setRenderPassName(normalizePass, destName);
        m_originColorImageRefs[index].first = originPassDescription.color_attachments[0].image;
        m_originColorImageRefs[index].second = m_originColorImageNames[index].c_str();
    }
    PixelFormat &format = m_originColorImageFormats[index];
    format = originFormat;
    format.setNumColorAttachemnts(1);
    SG_POP_GROUP();
}

PixelFormat
RenderTargetNormalizer::normalizedColorImagePixelFormat() const NANOEM_DECL_NOEXCEPT
{
    return m_normalizedColorImageFormat;
}

void
RenderTargetNormalizer::setNormalizedColorImagePixelFormat(const PixelFormat &value)
{
    m_normalizedColorImageFormat = value;
}

} /* namespace effect */
} /* namespace nanoem */
