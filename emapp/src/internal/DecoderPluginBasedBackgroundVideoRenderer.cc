/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/DecoderPluginBasedBackgroundVideoRenderer.h"

#include "emapp/DefaultFileManager.h"
#include "emapp/PluginFactory.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/plugin/DecoderPlugin.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {

const char *const DecoderPluginBasedBackgroundVideoRenderer::kLabelPrefix =
    "@nanoem/DecoderPluginBasedBackgroundVideoRenderer";
const nanoem_u32_t DecoderPluginBasedBackgroundVideoRenderer::kBaseFPS = 60;

DecoderPluginBasedBackgroundVideoRenderer::DecoderPluginBasedBackgroundVideoRenderer(DefaultFileManager *fileManager)
    : m_fileManager(fileManager)
    , m_plugin(nullptr)
    , m_blitter(nullptr)
    , m_offset(false, 0)
    , m_lastRect(0)
{
    Inline::clearZeroMemory(m_desc);
    m_image = { SG_INVALID_ID };
}

DecoderPluginBasedBackgroundVideoRenderer::~DecoderPluginBasedBackgroundVideoRenderer() NANOEM_DECL_NOEXCEPT
{
}

bool
DecoderPluginBasedBackgroundVideoRenderer::load(const URI &fileURI, Error &error)
{
    bool loaded = false;
    IFileManager::DecoderPluginList plugins(
        m_fileManager->resolveVideoDecoderPluginList(fileURI.pathExtension().c_str()));
    for (IFileManager::DecoderPluginList::const_iterator it = plugins.begin(), end = plugins.end(); it != end; ++it) {
        plugin::DecoderPlugin *plugin = *it;
        PluginFactory::DecoderPluginProxy proxy(plugin);
        loaded = proxy.loadVideo(fileURI, kBaseFPS, error);
        int width = proxy.width();
        int height = proxy.height();
        if (loaded && width > 0 && height > 0) {
            m_desc.width = width;
            m_desc.height = height;
            m_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            m_desc.usage = SG_USAGE_DYNAMIC;
            m_desc.mag_filter = m_desc.min_filter = SG_FILTER_LINEAR;
            char label[Inline::kMarkerStringLength];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "%s/ColorImage", kLabelPrefix);
                m_desc.label = label;
            }
            m_image = sg::make_image(&m_desc);
            m_plugin = plugin;
            m_offset.first = true;
            m_fileURI = fileURI;
            SG_LABEL_IMAGE(m_image, label);
            break;
        }
    }
    if (!loaded && !error.hasReason()) {
        error = Error("No decodable video plugin found", 0, Error::kDomainTypeApplication);
    }
    return loaded;
}

void
DecoderPluginBasedBackgroundVideoRenderer::draw(
    sg_pass pass, const Vector4 &rect, nanoem_f32_t /* scaleFactor */, Project *project)
{
    if (sg::is_valid(pass) && sg::is_valid(m_image)) {
        const sg::NamedPass namedPass(tinystl::make_pair(pass, Project::kViewportPrimaryName));
        const sg::NamedImage namedImage(tinystl::make_pair(m_image, kLabelPrefix));
        if (!m_blitter) {
            m_blitter = nanoem_new(BlitPass(project, false));
            project->setBaseDuration(m_plugin->videoDuration());
            m_lastRect = rect;
        }
        if (m_lastRect != rect) {
            m_blitter->markAsDirty();
            m_lastRect = rect;
        }
        const PixelFormat format(project->findRenderPassPixelFormat(pass, project->sampleCount()));
        m_blitter->blit(project->sharedBatchDrawQueue(), namedPass, namedImage, rect, format);
    }
}

void
DecoderPluginBasedBackgroundVideoRenderer::seek(nanoem_f64_t value)
{
    if (m_offset.second != value) {
        m_offset.first = true;
        m_offset.second = value;
    }
}

void
DecoderPluginBasedBackgroundVideoRenderer::flush()
{
    if (m_offset.first) {
        PluginFactory::DecoderPluginProxy proxy(m_plugin);
        ByteArray bytes;
        Error error;
        if (!proxy.decodeVideoFrame(m_offset.second * kBaseFPS, bytes, error) || bytes.empty()) {
            bytes.resize(nanoem_rsize_t(4) * proxy.width() * proxy.height());
            memset(bytes.data(), 0xff, bytes.size());
        }
        sg_image_data content;
        Inline::clearZeroMemory(content);
        sg_range &sub = content.subimage[0][0];
        sub.ptr = bytes.data();
        sub.size = bytes.size();
        sg::update_image(m_image, &content);
        m_offset.first = false;
    }
}

void
DecoderPluginBasedBackgroundVideoRenderer::destroy()
{
    nanoem_delete_safe(m_blitter);
    sg::destroy_image(m_image);
    m_image = { SG_INVALID_ID };
}

URI
DecoderPluginBasedBackgroundVideoRenderer::fileURI() const NANOEM_DECL_NOEXCEPT
{
    return m_fileURI;
}

} /* namespace internal */
} /* namespace nanoem */
