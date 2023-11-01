/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_WEBGPUCONTEXT_H_
#define NANOEM_EMAPP_INTERNAL_WEBGPUCONTEXT_H_

#include "emapp/Forward.h"
#include "emapp/Project.h"

struct WGPUSurfaceDescriptor;

typedef struct WGPUInstanceImpl *WGPUInstance;
typedef struct WGPUAdapterImpl *WGPUAdapter;
typedef struct WGPUDeviceImpl *WGPUDevice;
typedef struct WGPUTextureViewImpl *WGPUTextureView;

namespace nanoem {
namespace internal {

class WebGPUContext : private NonCopyable {
public:
    WebGPUContext(const WGPUSurfaceDescriptor &surfaceDescriptor, const String &pluginPath);
    ~WebGPUContext() NANOEM_DECL_NOEXCEPT;

    Project::IRendererCapability *createRendererCapability();
    void beginDefaultPass(const Vector2UI16 &devicePixelWindowSize, nanoem_u32_t sampleCount);
    void resizeDefaultPass(const Vector2UI16 &devicePixelWindowSize, nanoem_u32_t sampleCount);
    void presentDefaultPass();

    WGPUInstance instance() NANOEM_DECL_NOEXCEPT;
    WGPUAdapter adapter() NANOEM_DECL_NOEXCEPT;
    WGPUDevice device() NANOEM_DECL_NOEXCEPT;

    WGPUTextureView renderTextureView() NANOEM_DECL_NOEXCEPT;
    WGPUTextureView resolveTextureView() NANOEM_DECL_NOEXCEPT;
    WGPUTextureView depthStencilTextureView() NANOEM_DECL_NOEXCEPT;

private:
    class PrivateContext;

    PrivateContext *m_context;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_WEBGPUCONTEXT_H_ */
