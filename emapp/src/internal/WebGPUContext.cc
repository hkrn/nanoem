/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/WebGPUContext.h"

#include "emapp/private/CommonInclude.h"
#if defined(NANOEM_WEBGPU_DAWN)
#include "dawn/webgpu.h"
#else
#include "webgpu-headers/webgpu.h"
typedef int WGPULoggingType;
#endif /* NANOEM_WEBGPU_DAWN */

#include "bx/hash.h"

namespace nanoem {
namespace internal {
namespace {

class RenderCapability : public Project::IRendererCapability {
public:
    static const nanoem_u32_t kMaxSampleLevel;

    ~RenderCapability() NANOEM_DECL_NOEXCEPT;

    nanoem_u32_t suggestedSampleLevel(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool supportsSampleLevel(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
};

const nanoem_u32_t RenderCapability::kMaxSampleLevel = 2;

RenderCapability::~RenderCapability() NANOEM_DECL_NOEXCEPT
{
}

nanoem_u32_t
RenderCapability::suggestedSampleLevel(nanoem_u32_t /* value */) const NANOEM_DECL_NOEXCEPT
{
    return kMaxSampleLevel;
}

bool
RenderCapability::supportsSampleLevel(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    return value <= kMaxSampleLevel;
}

} /* namespace anonymous */

class WebGPUContext::PrivateContext : private NonCopyable {
public:
    static void handleAdapterRequest(
        WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT;
    static void handleDeviceRequest(
        WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT;
    static void handleUncapturedErrorCallback(
        WGPUErrorType type, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT;
    static void handleDeviceLostCallback(
        WGPUDeviceLostReason reason, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT;
    static void handleLoggingCallback(WGPULoggingType type, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT;

    PrivateContext(const WGPUSurfaceDescriptor &surfaceDescriptor, const String &pluginPath);
    ~PrivateContext();

    void initializeAllProcedures();
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
    bool recordLog(const char *message);

    void *m_dllHandle;
    WGPUInstance m_instance;
    WGPUSurface m_surface;
#if defined(NANOEM_WEBGPU_DAWN)
    WGPUSwapChain m_swapChain;
#else
    WGPUSurfaceCapabilities m_surfaceCapabilities;
    WGPUSurfaceTexture m_surfaceTexture;
#endif /* NANOEM_WEBGPU_DAWN */
    WGPUTextureView m_surfaceTextureView;
    WGPUTexture m_depthStencilTexture;
    WGPUTextureView m_depthStencilTextureView;
    WGPUTexture m_msaaTexture;
    WGPUTextureView m_msaaTextureView;
    WGPUAdapter m_adapter;
    WGPUDevice m_device;
    tinystl::unordered_set<nanoem_u32_t, TinySTLAllocator> m_loggings;

    WGPUProcCreateInstance _wgpuCreateInstance;
    WGPUProcInstanceCreateSurface _wgpuInstanceCreateSurface;
    WGPUProcInstanceRequestAdapter _wgpuInstanceRequestAdapter;
    WGPUProcDeviceSetUncapturedErrorCallback _wgpuDeviceSetUncapturedErrorCallback;
    WGPUProcDevicePushErrorScope _wgpuDevicePushErrorScope;
    WGPUProcDevicePopErrorScope _wgpuDevicePopErrorScope;
    WGPUProcDeviceCreateTexture _wgpuDeviceCreateTexture;
    WGPUProcTextureCreateView _wgpuTextureCreateView;
    WGPUProcTextureViewRelease _wgpuTextureViewRelease;
    WGPUProcTextureRelease _wgpuTextureRelease;
    WGPUProcDeviceRelease _wgpuDeviceRelease;
    WGPUProcAdapterRelease _wgpuAdapterRelease;
    WGPUProcSurfaceRelease _wgpuSurfaceRelease;
    WGPUProcInstanceRelease _wgpuInstanceRelease;
    WGPUProcAdapterRequestDevice _wgpuAdapterRequestDevice;
    WGPUProcAdapterGetProperties _wgpuAdapterGetProperties;
#if defined(NANOEM_WEBGPU_DAWN)
    WGPUProcInstanceProcessEvents _wgpuInstanceProcessEvents;
    WGPUProcDeviceCreateSwapChain _wgpuDeviceCreateSwapChain;
    WGPUProcDeviceSetDeviceLostCallback _wgpuDeviceSetDeviceLostCallback;
    WGPUProcDeviceSetLoggingCallback _wgpuDeviceSetLoggingCallback;
    WGPUProcSwapChainGetCurrentTextureView _wgpuSwapChainGetCurrentTextureView;
    WGPUProcSwapChainPresent _wgpuSwapChainPresent;
    WGPUProcSwapChainRelease _wgpuSwapChainRelease;
#else
    WGPUProcSurfaceGetCapabilities _wgpuSurfaceGetCapabilities;
    WGPUProcSurfaceGetCurrentTexture _wgpuSurfaceGetCurrentTexture;
    WGPUProcSurfaceConfigure _wgpuSurfaceConfigure;
#endif /* NANOEM_WEBGPU_DAWN */
};

WebGPUContext::PrivateContext::PrivateContext(const WGPUSurfaceDescriptor &surfaceDescriptor, const String &pluginPath)
    : m_dllHandle(nullptr)
    , m_instance(nullptr)
#if defined(NANOEM_WEBGPU_DAWN)
    , m_swapChain(nullptr)
#else
    , m_surface(nullptr)
#endif /* NANOEM_WEBGPU_DAWN */
    , m_surfaceTextureView(nullptr)
    , m_depthStencilTexture(nullptr)
    , m_depthStencilTextureView(nullptr)
    , m_msaaTexture(nullptr)
    , m_msaaTextureView(nullptr)
    , m_adapter(nullptr)
    , m_device(nullptr)
    , _wgpuCreateInstance(nullptr)
    , _wgpuInstanceCreateSurface(nullptr)
    , _wgpuInstanceRequestAdapter(nullptr)
    , _wgpuDeviceSetUncapturedErrorCallback(nullptr)
    , _wgpuDevicePushErrorScope(nullptr)
    , _wgpuDevicePopErrorScope(nullptr)
    , _wgpuDeviceCreateTexture(nullptr)
    , _wgpuTextureCreateView(nullptr)
    , _wgpuTextureViewRelease(nullptr)
    , _wgpuTextureRelease(nullptr)
    , _wgpuDeviceRelease(nullptr)
    , _wgpuAdapterRelease(nullptr)
    , _wgpuSurfaceRelease(nullptr)
    , _wgpuInstanceRelease(nullptr)
    , _wgpuAdapterRequestDevice(nullptr)
    , _wgpuAdapterGetProperties(nullptr)
#if defined(NANOEM_WEBGPU_DAWN)
    , _wgpuInstanceProcessEvents(nullptr)
    , _wgpuDeviceCreateSwapChain(nullptr)
    , _wgpuDeviceSetDeviceLostCallback(nullptr)
    , _wgpuDeviceSetLoggingCallback(nullptr)
    , _wgpuSwapChainGetCurrentTextureView(nullptr)
    , _wgpuSwapChainPresent(nullptr)
    , _wgpuSwapChainRelease(nullptr)
#else
    , _wgpuSurfaceGetCapabilities(nullptr)
    , _wgpuSurfaceGetCurrentTexture(nullptr)
    , _wgpuSurfaceConfigure(nullptr)
#endif /* NANOEM_WEBGPU_DAWN */
{
    m_dllHandle = bx::dlopen(pluginPath.c_str());
    initializeAllProcedures();
    m_instance = _wgpuCreateInstance(nullptr);
    WGPURequestAdapterOptions adapterOptions;
    Inline::clearZeroMemory(adapterOptions);
    m_surface = _wgpuInstanceCreateSurface(m_instance, &surfaceDescriptor);
    adapterOptions.compatibleSurface = m_surface;
#if defined(NANOEM_WEBGPU_DAWN)
    WGPUDawnTogglesDescriptor toggleDescriptor;
    const char *toggles[] = { "allow_unsafe_apis", "use_dxc" };
    Inline::clearZeroMemory(toggleDescriptor);
    toggleDescriptor.chain.sType = WGPUSType_DawnTogglesDescriptor;
    toggleDescriptor.enabledToggleCount = BX_COUNTOF(toggles);
    toggleDescriptor.enabledToggles = toggles;
    adapterOptions.nextInChain = &toggleDescriptor.chain;
#endif /* NANOEM_WEBGPU_DAWN */
    _wgpuInstanceRequestAdapter(m_instance, &adapterOptions, &handleAdapterRequest, this);
    WGPUFeatureName requiredFeatures[] = { WGPUFeatureName_Depth32FloatStencil8 };
    WGPUDeviceDescriptor deviceDescriptor;
    Inline::clearZeroMemory(deviceDescriptor);
    deviceDescriptor.requiredFeatureCount = BX_COUNTOF(requiredFeatures);
    deviceDescriptor.requiredFeatures = requiredFeatures;
    _wgpuAdapterRequestDevice(m_adapter, &deviceDescriptor, &handleDeviceRequest, this);
    _wgpuDeviceSetUncapturedErrorCallback(m_device, handleUncapturedErrorCallback, this);
#if defined(NANOEM_WEBGPU_DAWN)
    _wgpuDeviceSetLoggingCallback(m_device, handleLoggingCallback, this);
    _wgpuDeviceSetDeviceLostCallback(m_device, handleDeviceLostCallback, this);
#else
    Inline::clearZeroMemory(m_surfaceTexture);
    if (_wgpuSurfaceGetCapabilities) {
        _wgpuSurfaceGetCapabilities(m_surface, m_adapter, &m_surfaceCapabilities);
    }
#endif /* NANOEM_WEBGPU_DAWN */
}

WebGPUContext::PrivateContext::~PrivateContext()
{
    if (m_surfaceTextureView) {
        _wgpuTextureViewRelease(m_surfaceTextureView);
        m_surfaceTextureView = nullptr;
    }
    if (m_depthStencilTextureView) {
        _wgpuTextureViewRelease(m_depthStencilTextureView);
        m_depthStencilTextureView = nullptr;
    }
    if (m_depthStencilTexture) {
        _wgpuTextureRelease(m_depthStencilTexture);
        m_depthStencilTexture = nullptr;
    }
    if (m_msaaTextureView) {
        _wgpuTextureViewRelease(m_msaaTextureView);
        m_msaaTextureView = nullptr;
    }
    if (m_msaaTexture) {
        _wgpuTextureRelease(m_msaaTexture);
        m_msaaTexture = nullptr;
    }
    if (m_device) {
        _wgpuDeviceRelease(m_device);
        m_device = nullptr;
    }
    if (m_adapter) {
        _wgpuAdapterRelease(m_adapter);
        m_adapter = nullptr;
    }
#if defined(NANOEM_WEBGPU_DAWN)
    if (m_swapChain) {
        _wgpuSwapChainRelease(m_swapChain);
        m_swapChain = nullptr;
    }
#endif /* NANOEM_WEBGPU_DAWN */
    if (m_surface) {
        _wgpuSurfaceRelease(m_surface);
        m_surface = nullptr;
    }
    if (m_instance) {
        _wgpuInstanceRelease(m_instance);
        m_instance = nullptr;
    }
    if (m_dllHandle) {
        bx::dlclose(m_dllHandle);
        m_dllHandle = nullptr;
    }
}

void
WebGPUContext::PrivateContext::handleAdapterRequest(
    WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT
{
    if (status == WGPURequestAdapterStatus_Success) {
        PrivateContext *self = static_cast<PrivateContext *>(userdata);
        WGPUAdapterProperties properties;
        Inline::clearZeroMemory(properties);
        self->_wgpuAdapterGetProperties(adapter, &properties);
        EMLOG_INFO("WGPU adapter retrieved: vendor={}, architecture={}, name={}, description={}", properties.vendorName,
            properties.architecture, properties.name, properties.driverDescription);
        self->m_adapter = adapter;
    }
    else {
        EMLOG_ERROR("Failed to request WebGPU adapter: {}", message);
    }
}

void
WebGPUContext::PrivateContext::handleDeviceRequest(
    WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT
{
    if (status == WGPURequestDeviceStatus_Success) {
        PrivateContext *self = static_cast<PrivateContext *>(userdata);
        self->m_device = device;
    }
    else {
        EMLOG_ERROR("Failed to request WebGPU device: {}", message);
    }
}

void
WebGPUContext::PrivateContext::handleUncapturedErrorCallback(
    WGPUErrorType type, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT
{
    PrivateContext *self = static_cast<PrivateContext *>(userdata);
    if (type != WGPUErrorType_NoError && self->recordLog(message)) {
        EMLOG_ERROR("WGPU Uncaptured error {}: {}", int(type), message);
    }
}

void
WebGPUContext::PrivateContext::handleDeviceLostCallback(
    WGPUDeviceLostReason reason, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT
{
    PrivateContext *self = static_cast<PrivateContext *>(userdata);
    if (self->recordLog(message)) {
        EMLOG_ERROR("WGPU Device lost {}: {}", int(reason), message);
    }
}

void
WebGPUContext::PrivateContext::handleLoggingCallback(
    WGPULoggingType type, char const *message, void *userdata) NANOEM_DECL_NOEXCEPT
{
    PrivateContext *self = static_cast<PrivateContext *>(userdata);
    if (self->recordLog(message)) {
        EMLOG_INFO("WGPU Logging callback {}: {}", int(type), message);
    }
}

void
WebGPUContext::PrivateContext::initializeAllProcedures()
{
    typedef void (*PFN_sgx_dawn_setup)();
    if (PFN_sgx_dawn_setup sgx_dawn_setup =
            reinterpret_cast<PFN_sgx_dawn_setup>(bx::dlsym(m_dllHandle, "sgx_dawn_setup"))) {
        sgx_dawn_setup();
    }
    _wgpuCreateInstance = reinterpret_cast<WGPUProcCreateInstance>(bx::dlsym(m_dllHandle, "wgpuCreateInstance"));
    _wgpuInstanceCreateSurface =
        reinterpret_cast<WGPUProcInstanceCreateSurface>(bx::dlsym(m_dllHandle, "wgpuInstanceCreateSurface"));
    _wgpuInstanceRequestAdapter =
        reinterpret_cast<WGPUProcInstanceRequestAdapter>(bx::dlsym(m_dllHandle, "wgpuInstanceRequestAdapter"));
    _wgpuAdapterRequestDevice =
        reinterpret_cast<WGPUProcAdapterRequestDevice>(bx::dlsym(m_dllHandle, "wgpuAdapterRequestDevice"));
    _wgpuAdapterGetProperties =
        reinterpret_cast<WGPUProcAdapterGetProperties>(bx::dlsym(m_dllHandle, "wgpuAdapterGetProperties"));
    _wgpuDeviceSetUncapturedErrorCallback = reinterpret_cast<WGPUProcDeviceSetUncapturedErrorCallback>(
        bx::dlsym(m_dllHandle, "wgpuDeviceSetUncapturedErrorCallback"));
    _wgpuDevicePushErrorScope =
        reinterpret_cast<WGPUProcDevicePushErrorScope>(bx::dlsym(m_dllHandle, "wgpuDevicePushErrorScope"));
    _wgpuDevicePopErrorScope =
        reinterpret_cast<WGPUProcDevicePopErrorScope>(bx::dlsym(m_dllHandle, "wgpuDevicePopErrorScope"));
    _wgpuDeviceCreateTexture =
        reinterpret_cast<WGPUProcDeviceCreateTexture>(bx::dlsym(m_dllHandle, "wgpuDeviceCreateTexture"));
    _wgpuTextureCreateView =
        reinterpret_cast<WGPUProcTextureCreateView>(bx::dlsym(m_dllHandle, "wgpuTextureCreateView"));
    _wgpuTextureViewRelease =
        reinterpret_cast<WGPUProcTextureViewRelease>(bx::dlsym(m_dllHandle, "wgpuTextureViewRelease"));
    _wgpuTextureRelease = reinterpret_cast<WGPUProcTextureRelease>(bx::dlsym(m_dllHandle, "wgpuTextureRelease"));
    _wgpuDeviceRelease = reinterpret_cast<WGPUProcDeviceRelease>(bx::dlsym(m_dllHandle, "wgpuDeviceRelease"));
    _wgpuAdapterRelease = reinterpret_cast<WGPUProcAdapterRelease>(bx::dlsym(m_dllHandle, "wgpuAdapterRelease"));
    _wgpuSurfaceRelease = reinterpret_cast<WGPUProcSurfaceRelease>(bx::dlsym(m_dllHandle, "wgpuSurfaceRelease"));
    _wgpuInstanceRelease = reinterpret_cast<WGPUProcInstanceRelease>(bx::dlsym(m_dllHandle, "wgpuInstanceRelease"));
#if defined(NANOEM_WEBGPU_DAWN)
    _wgpuInstanceProcessEvents =
        reinterpret_cast<WGPUProcInstanceProcessEvents>(bx::dlsym(m_dllHandle, "wgpuInstanceProcessEvents"));
    _wgpuDeviceCreateSwapChain =
        reinterpret_cast<WGPUProcDeviceCreateSwapChain>(bx::dlsym(m_dllHandle, "wgpuDeviceCreateSwapChain"));
    _wgpuDeviceSetDeviceLostCallback = reinterpret_cast<WGPUProcDeviceSetDeviceLostCallback>(
        bx::dlsym(m_dllHandle, "wgpuDeviceSetDeviceLostCallback"));
    _wgpuDeviceSetLoggingCallback =
        reinterpret_cast<WGPUProcDeviceSetLoggingCallback>(bx::dlsym(m_dllHandle, "wgpuDeviceSetLoggingCallback"));
    _wgpuSwapChainGetCurrentTextureView = reinterpret_cast<WGPUProcSwapChainGetCurrentTextureView>(
        bx::dlsym(m_dllHandle, "wgpuSwapChainGetCurrentTextureView"));
    _wgpuSwapChainPresent = reinterpret_cast<WGPUProcSwapChainPresent>(bx::dlsym(m_dllHandle, "wgpuSwapChainPresent"));
    _wgpuSwapChainRelease = reinterpret_cast<WGPUProcSwapChainRelease>(bx::dlsym(m_dllHandle, "wgpuSwapChainRelease"));
#else
    _wgpuSurfaceGetCapabilities =
        reinterpret_cast<WGPUProcSurfaceGetCapabilities>(bx::dlsym(m_dllHandle, "wgpuSurfaceGetCapabilities"));
    _wgpuSurfaceGetCurrentTexture =
        reinterpret_cast<WGPUProcSurfaceGetCurrentTexture>(bx::dlsym(m_dllHandle, "wgpuSurfaceGetCurrentTexture"));
    _wgpuSurfaceConfigure = reinterpret_cast<WGPUProcSurfaceConfigure>(bx::dlsym(m_dllHandle, "wgpuSurfaceConfigure"));
#endif /* NANOEM_WEBGPU_DAWN */
}

void
WebGPUContext::PrivateContext::beginDefaultPass(const Vector2UI16 &devicePixelWindowSize, nanoem_u32_t sampleCount)
{
    _wgpuDevicePushErrorScope(m_device, WGPUErrorFilter_Validation);
    if (!m_surfaceTextureView) {
        resizeDefaultPass(devicePixelWindowSize, sampleCount);
    }
#if defined(NANOEM_WEBGPU_DAWN)
    m_surfaceTextureView = _wgpuSwapChainGetCurrentTextureView(m_swapChain);
#else
    _wgpuSurfaceGetCurrentTexture(m_surface, &m_surfaceTexture);
    m_surfaceTextureView = _wgpuTextureCreateView(m_surfaceTexture.texture, nullptr);
#endif /* NANOEM_WEBGPU_DAWN */
}

void
WebGPUContext::PrivateContext::resizeDefaultPass(const Vector2UI16 &devicePixelWindowSize, nanoem_u32_t sampleCount)
{
    WGPUTextureFormat colorFormat;
#if defined(NANOEM_WEBGPU_DAWN)
    WGPUSwapChainDescriptor swapChainDescriptor;
    Inline::clearZeroMemory(swapChainDescriptor);
    colorFormat = WGPUTextureFormat_BGRA8Unorm;
    swapChainDescriptor.format = colorFormat;
    swapChainDescriptor.usage = WGPUTextureUsage_RenderAttachment;
    swapChainDescriptor.presentMode = WGPUPresentMode_Fifo;
    swapChainDescriptor.width = devicePixelWindowSize.x;
    swapChainDescriptor.height = devicePixelWindowSize.y;
    if (m_swapChain) {
        _wgpuSwapChainRelease(m_swapChain);
    }
    m_swapChain = _wgpuDeviceCreateSwapChain(m_device, m_surface, &swapChainDescriptor);
#else
    WGPUSurfaceConfiguration configuration;
    Inline::clearZeroMemory(configuration);
    colorFormat = m_surfaceCapabilities.formats[0];
    configuration.device = m_device;
    configuration.format = colorFormat;
    configuration.alphaMode = m_surfaceCapabilities.alphaModes[0];
    configuration.usage = WGPUTextureUsage_RenderAttachment;
    configuration.presentMode = WGPUPresentMode_Fifo;
    configuration.width = devicePixelWindowSize.x;
    configuration.height = devicePixelWindowSize.y;
    _wgpuSurfaceConfigure(m_surface, &configuration);
#endif /* NANOEM_WEBGPU_DAWN */
    WGPUTextureDescriptor textureDescriptor;
    Inline::clearZeroMemory(textureDescriptor);
    textureDescriptor.dimension = WGPUTextureDimension_2D;
    textureDescriptor.format = WGPUTextureFormat_Depth32FloatStencil8;
    textureDescriptor.size.width = devicePixelWindowSize.x;
    textureDescriptor.size.height = devicePixelWindowSize.y;
    textureDescriptor.size.depthOrArrayLayers = 1;
    textureDescriptor.usage = WGPUTextureUsage_RenderAttachment;
    textureDescriptor.sampleCount = sampleCount;
    textureDescriptor.mipLevelCount = 1;
    if (m_depthStencilTexture) {
        _wgpuTextureRelease(m_depthStencilTexture);
    }
    m_depthStencilTexture = _wgpuDeviceCreateTexture(m_device, &textureDescriptor);
    if (m_depthStencilTextureView) {
        _wgpuTextureViewRelease(m_depthStencilTextureView);
    }
    m_depthStencilTextureView = _wgpuTextureCreateView(m_depthStencilTexture, nullptr);
    if (sampleCount > 1) {
        textureDescriptor.format = colorFormat;
        if (m_msaaTexture) {
            _wgpuTextureRelease(m_msaaTexture);
        }
        m_msaaTexture = _wgpuDeviceCreateTexture(m_device, &textureDescriptor);
        if (m_msaaTextureView) {
            _wgpuTextureViewRelease(m_msaaTextureView);
        }
        m_msaaTextureView = _wgpuTextureCreateView(m_msaaTexture, nullptr);
    }
}

void
WebGPUContext::PrivateContext::presentDefaultPass()
{
#if defined(NANOEM_WEBGPU_DAWN)
    _wgpuSwapChainPresent(m_swapChain);
    _wgpuTextureViewRelease(m_surfaceTextureView);
    m_surfaceTextureView = nullptr;
    _wgpuDevicePopErrorScope(m_device, handleUncapturedErrorCallback, this);
    _wgpuInstanceProcessEvents(m_instance);
#else
    _wgpuTextureRelease(m_surfaceTexture.texture);
    _wgpuTextureViewRelease(m_surfaceTextureView);
    m_surfaceTextureView = nullptr;
    _wgpuDevicePopErrorScope(m_device, handleUncapturedErrorCallback, this);
#endif /* NANOEM_WEBGPU_DAWN */
}

WGPUInstance
WebGPUContext::PrivateContext::instance() NANOEM_DECL_NOEXCEPT
{
    return m_instance;
}

WGPUAdapter
WebGPUContext::PrivateContext::adapter() NANOEM_DECL_NOEXCEPT
{
    return m_adapter;
}

WGPUDevice
WebGPUContext::PrivateContext::device() NANOEM_DECL_NOEXCEPT
{
    return m_device;
}

WGPUTextureView
WebGPUContext::PrivateContext::renderTextureView() NANOEM_DECL_NOEXCEPT
{
    WGPUTextureView textureView;
    if (m_msaaTextureView) {
        textureView = m_msaaTextureView;
    }
    else {
        textureView = m_surfaceTextureView;
    }
    return textureView;
}

WGPUTextureView
WebGPUContext::PrivateContext::resolveTextureView() NANOEM_DECL_NOEXCEPT
{
    WGPUTextureView textureView = nullptr;
    if (m_msaaTextureView) {
        textureView = m_surfaceTextureView;
    }
    return textureView;
}

WGPUTextureView
WebGPUContext::PrivateContext::depthStencilTextureView() NANOEM_DECL_NOEXCEPT
{
    return m_depthStencilTextureView;
}

bool
WebGPUContext::PrivateContext::recordLog(const char *message)
{
    const uint32_t key = bx::hash<bx::HashMurmur2A>(message);
    tinystl::unordered_set<nanoem_u32_t>::const_iterator it = m_loggings.find(key);
    bool recorded = false;
    if (it == m_loggings.end()) {
        recorded = true;
        m_loggings.insert(key);
    }
    return recorded;
}

WebGPUContext::WebGPUContext(const WGPUSurfaceDescriptor &surfaceDescriptor, const String &pluginPath)
    : m_context(nanoem_new(PrivateContext(surfaceDescriptor, pluginPath)))
{
}

WebGPUContext::~WebGPUContext() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete(m_context);
    m_context = nullptr;
}

Project::IRendererCapability *
WebGPUContext::createRendererCapability()
{
    return nanoem_new(RenderCapability);
}

void
WebGPUContext::beginDefaultPass(const Vector2UI16 &devicePixelWindowSize, nanoem_u32_t sampleCount)
{
    m_context->beginDefaultPass(devicePixelWindowSize, sampleCount);
}

void
WebGPUContext::resizeDefaultPass(const Vector2UI16 &devicePixelWindowSize, nanoem_u32_t sampleCount)
{
    m_context->resizeDefaultPass(devicePixelWindowSize, sampleCount);
}

void
WebGPUContext::presentDefaultPass()
{
    m_context->presentDefaultPass();
}

WGPUInstance
WebGPUContext::instance() NANOEM_DECL_NOEXCEPT
{
    return m_context->instance();
}

WGPUAdapter
WebGPUContext::adapter() NANOEM_DECL_NOEXCEPT
{
    return m_context->adapter();
}

WGPUDevice
WebGPUContext::device() NANOEM_DECL_NOEXCEPT
{
    return m_context->device();
}

WGPUTextureView
WebGPUContext::renderTextureView() NANOEM_DECL_NOEXCEPT
{
    return m_context->renderTextureView();
}

WGPUTextureView
WebGPUContext::resolveTextureView() NANOEM_DECL_NOEXCEPT
{
    return m_context->resolveTextureView();
}

WGPUTextureView
WebGPUContext::depthStencilTextureView() NANOEM_DECL_NOEXCEPT
{
    return m_context->depthStencilTextureView();
}

} /* namespace internal */
} /* namespace nanoem */
