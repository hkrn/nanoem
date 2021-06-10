/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_D3D11BACKGROUNDDRAWER_H_
#define NANOEM_EMAPP_WIN32_D3D11BACKGROUNDDRAWER_H_

#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/URI.h"

struct ID3D11Device;
struct IMFSourceReader;
struct IMFTransform;
struct IMFMediaBuffer;

namespace nanoem {

class ThreadedApplicationService;

namespace internal {
class DecoderPluginBasedBackgroundVideoRenderer;
} /* namespace internal */

namespace win32 {

#if WINVER >= _WIN32_WINNT_WIN7

class D3D11BackgroundVideoDrawer : public IBackgroundVideoRenderer, private NonCopyable {
public:
    D3D11BackgroundVideoDrawer(ThreadedApplicationService *service);
    ~D3D11BackgroundVideoDrawer();

    bool load(const URI &fileURI, Error &error) override;
    void draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project) override;
    void seek(nanoem_f64_t value) override;
    void flush() override;
    void destroy() override;
    URI fileURI() const noexcept override;

private:
    static const char *const kLabelPrefix;
    void decodeSampleBuffer(LONGLONG pos, Error &error);
    void updateTexture(IMFMediaBuffer *outputBuffer, Error &error);

    IMFSourceReader *m_reader = nullptr;
    IMFTransform *m_transform = nullptr;
    ThreadedApplicationService *m_service = nullptr;
    internal::DecoderPluginBasedBackgroundVideoRenderer *m_decoderPluginBasedBackgroundVideoRenderer = nullptr;
    URI m_fileURI;
    sg_image m_image = { SG_INVALID_ID };
    sg_image_desc m_description;
    bool m_durationUpdated = false;
};

#else /* WINVER >= _WIN32_WINNT_WIN7 */

class D3D11BackgroundVideoDrawer : public IBackgroundVideoRenderer {
public:
    bool
    load(const URI &fileURI, Error &error)
    {
        BX_UNUSED_2(fileURI, error);
        return false;
    }
    void
    draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project)
    {
        BX_UNUSED_4(pass, rect, scaleFactor, project);
    }
    void
    seek(nanoem_f64_t seconds)
    {
        BX_UNUSED_1(seconds);
    }
    void
    flush()
    {
    }
    void
    destroy()
    {
    }
    URI
    fileURI() const NANOEM_DECL_NOEXCEPT
    {
        return URI();
    }
};

#endif /* WINVER >= _WIN32_WINNT_WIN7 */

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_D3D11BACKGROUNDDRAWER_H_ */
