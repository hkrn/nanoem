/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_DECODERPLUGINBASEDBACKGROUNDVIDEORENDERER_H_
#define NANOEM_EMAPP_INTERNAL_DECODERPLUGINBASEDBACKGROUNDVIDEORENDERER_H_

#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/URI.h"

namespace nanoem {

class DefaultFileManager;

namespace plugin {
class DecoderPlugin;
} /* namespace plugin */

namespace internal {

class BlitPass;

class DecoderPluginBasedBackgroundVideoRenderer : public IBackgroundVideoRenderer {
public:
    static const char *const kLabelPrefix;
    static const nanoem_u32_t kBaseFPS;

    DecoderPluginBasedBackgroundVideoRenderer(DefaultFileManager *fileManager);
    ~DecoderPluginBasedBackgroundVideoRenderer() NANOEM_DECL_NOEXCEPT;

    bool load(const URI &fileURI, Error &error) NANOEM_DECL_OVERRIDE;
    void draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project) NANOEM_DECL_OVERRIDE;
    void seek(nanoem_f64_t value) NANOEM_DECL_OVERRIDE;
    void flush() NANOEM_DECL_OVERRIDE;
    void destroy() NANOEM_DECL_OVERRIDE;
    URI fileURI() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    DefaultFileManager *m_fileManager;
    plugin::DecoderPlugin *m_plugin;
    BlitPass *m_blitter;
    URI m_fileURI;
    sg_image_desc m_desc;
    sg_image m_image;
    tinystl::pair<bool, nanoem_f64_t> m_offset;
    Vector4 m_lastRect;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_DECODERPLUGINBASEDBACKGROUNDVIDEORENDERER_H_ */
