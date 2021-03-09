/*
  Copyright (c) 2015-2021 hkrn All rights reserved

  This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/effect/RenderTargetMipmapGenerator.h"

#include "emapp/Effect.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace effect {
namespace {

static void
destroyAllImages(Effect *effect, RenderTargetMipmapGenerator::SGImageList &images) NANOEM_DECL_NOEXCEPT
{
    for (RenderTargetMipmapGenerator::SGImageList::const_iterator it = images.begin(), end = images.end(); it != end;
         ++it) {
        effect->removeImageLabel(*it);
        sg::destroy_image(*it);
    }
    images.clear();
}

static void
destroyAllPasses(RenderTargetMipmapGenerator::SGPassList &passes) NANOEM_DECL_NOEXCEPT
{
    for (RenderTargetMipmapGenerator::SGPassList::const_iterator it = passes.begin(), end = passes.end(); it != end;
         ++it) {
        sg::destroy_pass(*it);
    }
    passes.clear();
}

}

RenderTargetMipmapGenerator::RenderTargetMipmapGenerator(Effect *effect, const char *name, const sg_image_desc &desc)
    : m_project(effect->project())
    , m_blitter(nullptr)
{
    SG_PUSH_GROUP("effect::RenderTargetMipmapGenerator::ctor");
    char label[Inline::kMarkerStringLength];
    m_blitter = nanoem_new(internal::BlitPass(effect->project(), false));
    for (int i = 0, numMipmaps = desc.num_mipmaps; i < numMipmaps; i++) {
        sg_image_desc mipmapImageDesc(desc);
        mipmapImageDesc.num_mipmaps = 1;
        mipmapImageDesc.width >>= i;
        mipmapImageDesc.height >>= i;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "Effects/%s/%s/Mipmaps/%d/Color", effect->nameConstString(), name, i);
            mipmapImageDesc.label = label;
        }
        sg_image colorImage = sg::make_image(&mipmapImageDesc);
        nanoem_assert(sg::query_image_state(colorImage) == SG_RESOURCESTATE_VALID, "source color image must be valid");
        effect->setImageLabel(colorImage, label);
        m_sourceColorImages.push_back(colorImage);
        mipmapImageDesc.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "Effects/%s/%s/Mipmaps/%d/Depth", effect->nameConstString(), name, i);
            mipmapImageDesc.label = label;
        }
        sg_image depthImage = sg::make_image(&mipmapImageDesc);
        nanoem_assert(sg::query_image_state(depthImage) == SG_RESOURCESTATE_VALID, "source depth image must be valid");
        effect->setImageLabel(depthImage, label);
        m_sourceDepthImages.push_back(depthImage);
        sg_pass_desc passDesc;
        Inline::clearZeroMemory(passDesc);
        passDesc.color_attachments[0].image = colorImage;
        passDesc.depth_stencil_attachment.image = depthImage;
        if (Inline::isDebugLabelEnabled()) {
            StringUtils::format(
                label, sizeof(label), "Effects/%s/%s/Mipmaps/%d/Pass", effect->nameConstString(), name, i);
            passDesc.label = label;
        }
        sg_pass pass = sg::make_pass(&passDesc);
        nanoem_assert(sg::query_pass_state(pass) == SG_RESOURCESTATE_VALID, "source pass must be valid");
        m_sourcePasses.push_back(pass);
    }
    SG_POP_GROUP();
}

RenderTargetMipmapGenerator::~RenderTargetMipmapGenerator()
{
    nanoem_delete_safe(m_blitter);
}

void
RenderTargetMipmapGenerator::blitSourceImage(
    const Effect *effect, sg_image color, const PixelFormat &format, const char *name)
{
    SG_PUSH_GROUPF("effect::RenderTargetMipmapGenerator::blitSourceImage(name=%s)", name);
    static const Vector4 kRectCoordinate(0, 0, 1, 1);
    char label[Inline::kMarkerStringLength];
    const sg::NamedImage source(tinystl::make_pair(color, name));
    sg::PassBlock::IDrawQueue *drawQueue = m_project->sharedSerialDrawQueue();
    for (int i = 0, numMipmaps = Inline::saturateInt32(m_sourcePasses.size()); i < numMipmaps; i++) {
        StringUtils::format(
            label, sizeof(label), "Effects/%s/%s/Mipmaps/Source/%d", effect->nameConstString(), name, i);
        const char *destNamePtr = label;
        const sg::NamedPass dest(tinystl::make_pair(m_sourcePasses[i], destNamePtr));
        m_project->setRenderPassName(dest.first, destNamePtr);
        m_blitter->blit(drawQueue, dest, source, kRectCoordinate, format);
    }
    SG_POP_GROUP();
}

void
RenderTargetMipmapGenerator::generateAllMipmapImages(const Effect *effect, const PixelFormat &format,
    RenderTargetColorImageContainer *colorImageContainer,
    RenderTargetDepthStencilImageContainer *depthStencilImageContainer, const char *name)
{
    const sg_image_desc &colorImageDesc = colorImageContainer->colorImageDescription();
    if (m_destPasses.empty()) {
        const SGImageList *depthImages = depthStencilImageContainer->findMipmapImages(effect, colorImageDesc);
        char label[Inline::kMarkerStringLength];
        for (int i = 1; i < colorImageDesc.num_mipmaps; i++) {
            sg_pass_desc passDesc;
            Inline::clearZeroMemory(passDesc);
            passDesc.color_attachments[0].image = colorImageContainer->colorImageHandle();
            passDesc.color_attachments[0].mip_level = i;
            passDesc.depth_stencil_attachment.image = depthImages->data()[i - 1];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Effects/%s/%s/Mipmaps/%d/Pass", effect->nameConstString(),
                    colorImageContainer->nameConstString(), i);
                passDesc.label = label;
            }
            sg_pass destPass = sg::make_pass(&passDesc);
            nanoem_assert(sg::query_pass_state(destPass) == SG_RESOURCESTATE_VALID, "pass must be valid");
            m_destPasses.push_back(destPass);
        }
    }
    generateAllMipmapImages(effect, format, colorImageDesc, name);
}

void
RenderTargetMipmapGenerator::generateAllMipmapImages(const Effect *effect, const PixelFormat &format,
    OffscreenRenderTargetImageContainer *offscreenImageContainer, const char *name)
{
    const sg_image_desc &colorImageDesc = offscreenImageContainer->colorImageDescription();
    if (m_destPasses.empty()) {
        char label[Inline::kMarkerStringLength];
        offscreenImageContainer->generateDepthStencilMipmapImages(effect);
        for (int i = 1; i < colorImageDesc.num_mipmaps; i++) {
            sg_pass_desc passDesc;
            Inline::clearZeroMemory(passDesc);
            passDesc.color_attachments[0].image = offscreenImageContainer->colorImageHandle();
            passDesc.color_attachments[0].mip_level = i;
            passDesc.depth_stencil_attachment.image = offscreenImageContainer->mipmapDepthStencilImageHandleAt(i - 1);
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "Effects/%s/%s/Mipmaps/%d/Pass", effect->nameConstString(),
                    offscreenImageContainer->nameConstString(), i);
                passDesc.label = label;
            }
            sg_pass destPass = sg::make_pass(&passDesc);
            nanoem_assert(sg::query_pass_state(destPass) == SG_RESOURCESTATE_VALID, "pass must be valid");
            m_destPasses.push_back(destPass);
        }
    }
    generateAllMipmapImages(effect, format, colorImageDesc, name);
}

void
RenderTargetMipmapGenerator::generateAllMipmapImages(
    const Effect *effect, const PixelFormat &format, const sg_image_desc &colorImageDesc, const char *name)
{
    SG_PUSH_GROUPF("effect::RenderTargetMipmapGenerator::generateAllMipmapImages(name=%s)", name);
    static const Vector4 kRectCoordinate(0, 0, 1, 1);
    char label[Inline::kMarkerStringLength];
    sg::PassBlock::IDrawQueue *drawQueue = m_project->sharedSerialDrawQueue();
    for (int i = 1; i < colorImageDesc.num_mipmaps; i++) {
        const int offset = i - 1;
        StringUtils::format(
            label, sizeof(label), "Effects/%s/%s/Mipmaps/Source/%d", effect->nameConstString(), name, i);
        const char *sourceImageName = label;
        const sg::NamedImage sourceImage = tinystl::make_pair(m_sourceColorImages[offset], sourceImageName);
        StringUtils::format(label, sizeof(label), "Effects/%s/%s/Mipmaps/Dest/%d", effect->nameConstString(), name, i);
        const char *destNamePtr = label;
        const sg::NamedPass dest(tinystl::make_pair(m_destPasses[offset], destNamePtr));
        const Vector4 viewport(0, 0, colorImageDesc.width >> i, colorImageDesc.height >> i);
        m_project->setRenderPassName(dest.first, destNamePtr);
        m_blitter->blit(drawQueue, dest, sourceImage, kRectCoordinate, format, viewport);
    }
    SG_POP_GROUP();
}

void
RenderTargetMipmapGenerator::destroy(Effect *effect) NANOEM_DECL_NOEXCEPT
{
    SG_PUSH_GROUPF("effect::RenderTargetMipmapGenerator::destroy(name=%s)", effect->nameConstString());
    destroyAllPasses(m_sourcePasses);
    destroyAllPasses(m_destPasses);
    destroyAllImages(effect, m_sourceColorImages);
    destroyAllImages(effect, m_sourceDepthImages);
    m_blitter->destroy();
    SG_POP_GROUP();
}

} /* namespace effect */
} /* namespace nanoem */
