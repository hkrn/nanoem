/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/WebGPUContext.h"

#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {

void
WebGPUContext::handleAdapterRequest(
    WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata)
{
    if (status == WGPURequestAdapterStatus_Success) {
        WebGPUContext *self = static_cast<WebGPUContext *>(userdata);
        self->m_adapter = adapter;
    }
    else {
        EMLOG_ERROR("Failed to request WebGPU adapter: {}", message);
    }
}

void
WebGPUContext::handleDeviceRequest(
    WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata)
{
    if (status == WGPURequestDeviceStatus_Success) {
        WebGPUContext *self = static_cast<WebGPUContext *>(userdata);
        self->m_device = device;
    }
    else {
        EMLOG_ERROR("Failed to request WebGPU device: {}", message);
    }
}

WebGPUContext::WebGPUContext(const WGPUSurfaceDescriptor &surfaceDescriptor, const String &pluginPath)
    : m_dllHandle(nullptr)
    , m_instance(nullptr)
    , m_surface(nullptr)
    , m_surfaceTextureView(nullptr)
    , m_adapter(nullptr)
    , m_device(nullptr)
{
    m_dllHandle = bx::dlopen(pluginPath.c_str());
    typedef void (*PFN_sgx_dawn_setup)();
    if (PFN_sgx_dawn_setup sgx_dawn_setup = reinterpret_cast<PFN_sgx_dawn_setup>(bx::dlsym(m_dllHandle, "sgx_dawn_setup"))) {
        sgx_dawn_setup();
    }
    wgpuCreateInstance = reinterpret_cast<WGPUProcCreateInstance>(bx::dlsym(m_dllHandle, "wgpuCreateInstance"));
    wgpuInstanceCreateSurface =
        reinterpret_cast<WGPUProcInstanceCreateSurface>(bx::dlsym(m_dllHandle, "wgpuInstanceCreateSurface"));
    wgpuInstanceRequestAdapter =
        reinterpret_cast<WGPUProcInstanceRequestAdapter>(bx::dlsym(m_dllHandle, "wgpuInstanceRequestAdapter"));
    wgpuAdapterRequestDevice =
        reinterpret_cast<WGPUProcAdapterRequestDevice>(bx::dlsym(m_dllHandle, "wgpuAdapterRequestDevice"));
    wgpuSurfaceGetCapabilities =
        reinterpret_cast<WGPUProcSurfaceGetCapabilities>(bx::dlsym(m_dllHandle, "wgpuSurfaceGetCapabilities"));
    wgpuSurfaceGetCurrentTexture =
        reinterpret_cast<WGPUProcSurfaceGetCurrentTexture>(bx::dlsym(m_dllHandle, "wgpuSurfaceGetCurrentTexture"));
    wgpuTextureCreateView =
        reinterpret_cast<WGPUProcTextureCreateView>(bx::dlsym(m_dllHandle, "wgpuTextureCreateView"));
    wgpuSurfaceConfigure = reinterpret_cast<WGPUProcSurfaceConfigure>(bx::dlsym(m_dllHandle, "wgpuSurfaceConfigure"));
    wgpuTextureViewRelease =
        reinterpret_cast<WGPUProcTextureViewRelease>(bx::dlsym(m_dllHandle, "wgpuTextureViewRelease"));
    wgpuTextureRelease = reinterpret_cast<WGPUProcTextureRelease>(bx::dlsym(m_dllHandle, "wgpuTextureRelease"));
    wgpuDeviceRelease = reinterpret_cast<WGPUProcDeviceRelease>(bx::dlsym(m_dllHandle, "wgpuDeviceRelease"));
    wgpuAdapterRelease = reinterpret_cast<WGPUProcAdapterRelease>(bx::dlsym(m_dllHandle, "wgpuAdapterRelease"));
    wgpuSurfaceRelease = reinterpret_cast<WGPUProcSurfaceRelease>(bx::dlsym(m_dllHandle, "wgpuSurfaceRelease"));
    wgpuInstanceRelease = reinterpret_cast<WGPUProcInstanceRelease>(bx::dlsym(m_dllHandle, "wgpuInstanceRelease"));
    Inline::clearZeroMemory(m_surfaceTexture);
    m_instance = wgpuCreateInstance(nullptr);
    m_surface = wgpuInstanceCreateSurface(m_instance, &surfaceDescriptor);
    WGPURequestAdapterOptions adapterOptions;
    Inline::clearZeroMemory(adapterOptions);
    adapterOptions.compatibleSurface = m_surface;
    wgpuInstanceRequestAdapter(m_instance, &adapterOptions, &handleAdapterRequest, this);
    WGPUFeatureName requiredFeatures[] = { WGPUFeatureName_Depth32FloatStencil8 };
    WGPUDeviceDescriptor deviceDescriptor;
    Inline::clearZeroMemory(deviceDescriptor);
    deviceDescriptor.requiredFeatureCount = BX_COUNTOF(requiredFeatures);
    deviceDescriptor.requiredFeatures = requiredFeatures;
    wgpuAdapterRequestDevice(m_adapter, &deviceDescriptor, &handleDeviceRequest, this);
    if (wgpuSurfaceGetCapabilities) {
        wgpuSurfaceGetCapabilities(m_surface, m_adapter, &m_surfaceCapabilities);
    }
}

WebGPUContext::~WebGPUContext() NANOEM_DECL_NOEXCEPT
{
    wgpuDeviceRelease(m_device);
    m_device = nullptr;
    wgpuAdapterRelease(m_adapter);
    m_adapter = nullptr;
    wgpuSurfaceRelease(m_surface);
    m_surface = nullptr;
    wgpuInstanceRelease(m_instance);
    m_instance = nullptr;
    bx::dlclose(m_dllHandle);
    m_dllHandle = nullptr;
}

void
WebGPUContext::beginDefaultPass()
{
    wgpuSurfaceGetCurrentTexture(m_surface, &m_surfaceTexture);
    m_surfaceTextureView = wgpuTextureCreateView(m_surfaceTexture.texture, nullptr);
}

void
WebGPUContext::resizeDefaultPass(const Vector2UI16 &devicePixelWindowSize)
{
    WGPUSurfaceConfiguration configuration;
    Inline::clearZeroMemory(configuration);
    configuration.device = m_device;
    configuration.format = m_surfaceCapabilities.formats[0];
    configuration.alphaMode = m_surfaceCapabilities.alphaModes[0];
    configuration.usage = WGPUTextureUsage_RenderAttachment;
    configuration.presentMode = WGPUPresentMode_Fifo;
    configuration.width = devicePixelWindowSize.x;
    configuration.height = devicePixelWindowSize.y;
    wgpuSurfaceConfigure(m_surface, &configuration);
}

void
WebGPUContext::presentDefaultPass()
{
    wgpuTextureViewRelease(m_surfaceTextureView);
    wgpuTextureRelease(m_surfaceTexture.texture);
}

WGPUInstance
WebGPUContext::instance() NANOEM_DECL_NOEXCEPT
{
    return m_instance;
}

WGPUSurface
WebGPUContext::surface() NANOEM_DECL_NOEXCEPT
{
    return m_surface;
}

WGPUAdapter
WebGPUContext::adapter() NANOEM_DECL_NOEXCEPT
{
    return m_adapter;
}

WGPUDevice
WebGPUContext::device() NANOEM_DECL_NOEXCEPT
{
    return m_device;
}

WGPUTextureView
WebGPUContext::renderTextureView() NANOEM_DECL_NOEXCEPT
{
    return m_surfaceTextureView;
}

WGPUTextureView
WebGPUContext::resolveTextureView() NANOEM_DECL_NOEXCEPT
{
    return nullptr;
}

WGPUTextureView
WebGPUContext::depthStencilTextureView() NANOEM_DECL_NOEXCEPT
{
    return m_surfaceTextureView;
}

} /* namespace internal */
} /* namespace nanoem */
