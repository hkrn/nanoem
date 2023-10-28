/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_WEBGPUCONTEXT_H_
#define NANOEM_EMAPP_INTERNAL_WEBGPUCONTEXT_H_

#define WGPU_SKIP_DECLARATIONS
#include "webgpu-headers/webgpu.h"

#include "emapp/Forward.h"

namespace nanoem {
namespace internal {

class WebGPUContext : private NonCopyable {
public:
    WebGPUContext(const WGPUSurfaceDescriptor &surfaceDescriptor, const String &pluginPath);
    ~WebGPUContext() NANOEM_DECL_NOEXCEPT;

    void beginDefaultPass();
    void resizeDefaultPass(const Vector2UI16 &devicePixelWindowSize);
    void presentDefaultPass();

    WGPUInstance instance() NANOEM_DECL_NOEXCEPT;
    WGPUSurface surface() NANOEM_DECL_NOEXCEPT;
    WGPUAdapter adapter() NANOEM_DECL_NOEXCEPT;
    WGPUDevice device() NANOEM_DECL_NOEXCEPT;

    WGPUTextureView renderTextureView() NANOEM_DECL_NOEXCEPT;
    WGPUTextureView resolveTextureView() NANOEM_DECL_NOEXCEPT;
    WGPUTextureView depthStencilTextureView() NANOEM_DECL_NOEXCEPT;

private:
    static void handleAdapterRequest(
        WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata);
    static void handleDeviceRequest(
        WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata);

    WGPUProcCreateInstance wgpuCreateInstance;
    WGPUProcInstanceCreateSurface wgpuInstanceCreateSurface;
    WGPUProcInstanceRequestAdapter wgpuInstanceRequestAdapter;
    WGPUProcAdapterRequestDevice wgpuAdapterRequestDevice;
    WGPUProcSurfaceGetCapabilities wgpuSurfaceGetCapabilities;
    WGPUProcSurfaceGetCurrentTexture wgpuSurfaceGetCurrentTexture;
    WGPUProcTextureCreateView wgpuTextureCreateView;
    WGPUProcSurfaceConfigure wgpuSurfaceConfigure;
    WGPUProcTextureViewRelease wgpuTextureViewRelease;
    WGPUProcTextureRelease wgpuTextureRelease;
    WGPUProcDeviceRelease wgpuDeviceRelease;
    WGPUProcAdapterRelease wgpuAdapterRelease;
    WGPUProcSurfaceRelease wgpuSurfaceRelease;
    WGPUProcInstanceRelease wgpuInstanceRelease;

    String m_pluginPath;
    WGPUInstance m_instance;
    WGPUSurface m_surface;
    WGPUSurfaceCapabilities m_surfaceCapabilities;
    WGPUSurfaceTexture m_surfaceTexture;
    WGPUTextureView m_surfaceTextureView;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_WEBGPUCONTEXT_H_ */
