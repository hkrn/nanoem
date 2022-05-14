/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/CapturingPassState.h"

#include <stdio.h> /* for sprintf in stb_image_write.h */

#include "emapp/ApplicationMenuBuilder.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/Constants.h"
#include "emapp/DefaultFileManager.h"
#include "emapp/EnumUtils.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IModalDialog.h"
#include "emapp/IVideoRecorder.h"
#include "emapp/Project.h"
#include "emapp/StateController.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/BasePass.h"
#include "emapp/internal/PluginUI.h"
#include "emapp/plugin/DecoderPlugin.h"
#include "emapp/plugin/EncoderPlugin.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/sdk/Encoder.h"

#include "imgui/imgui.h"

#include "emapp/src/protoc/plugin.pb-c.h"

namespace nanoem {
namespace internal {
namespace {

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_ASSERT(x) /* prevent _wassert confliction on win32 */
#include "stb/stb_image_write.h"

#include "emapp/private/shaders/color_transform_fs_glsl_core33.h"
#include "emapp/private/shaders/color_transform_fs_glsl_es3.h"
#include "emapp/private/shaders/color_transform_fs_msl_macos.h"
#include "emapp/private/shaders/color_transform_fs_spirv.h"
#include "emapp/private/shaders/color_transform_ps_dxbc.h"
#include "emapp/private/shaders/color_transform_vs_dxbc.h"
#include "emapp/private/shaders/color_transform_vs_glsl_core33.h"
#include "emapp/private/shaders/color_transform_vs_glsl_es3.h"
#include "emapp/private/shaders/color_transform_vs_msl_macos.h"
#include "emapp/private/shaders/color_transform_vs_spirv.h"

class ImageWriter : private NonCopyable {
public:
    static const int kJPEGQualityPercentage = 95;

    ImageWriter(const URI &fileURI, Error &error);
    ~ImageWriter() NANOEM_DECL_NOEXCEPT;

    void write(const nanoem_u8_t *data, int width, int height, sg_pixel_format pixelFormat);

private:
    static void callback(void *context, void *data, int size);
    bool writeImageAs(const char *extension);

    const URI m_fileURI;
    FileWriterScope m_scope;
    Error &m_error;
};

ImageWriter::ImageWriter(const URI &fileURI, Error &error)
    : m_fileURI(fileURI)
    , m_error(error)
{
}

ImageWriter::~ImageWriter() NANOEM_DECL_NOEXCEPT
{
    if (!m_error.hasReason()) {
        m_scope.commit(m_error);
    }
    else {
        Error error;
        m_scope.rollback(error);
        if (error.hasReason()) {
            m_error = error;
        }
    }
}

void
ImageWriter::write(const nanoem_u8_t *data, int width, int height, sg_pixel_format pixelFormat)
{
    if (pixelFormat == SG_PIXELFORMAT_RGBA8 && writeImageAs("png")) {
        int stride = width * 4;
        stbi_write_png_to_func(callback, this, width, height, 4, data, stride);
    }
    else if (pixelFormat == SG_PIXELFORMAT_RGBA8 && writeImageAs("bmp")) {
        stbi_write_bmp_to_func(callback, this, width, height, 4, data);
    }
    else if (pixelFormat == SG_PIXELFORMAT_RGBA8 && writeImageAs("tga")) {
        stbi_write_tga_to_func(callback, this, width, height, 4, data);
    }
    else if (pixelFormat == SG_PIXELFORMAT_RGBA8 && writeImageAs("jpg")) {
        stbi_write_jpg_to_func(callback, this, width, height, 4, data, kJPEGQualityPercentage);
    }
    else if (pixelFormat == SG_PIXELFORMAT_RGBA32F && writeImageAs("hdr")) {
        const nanoem_f32_t *dataPtr = reinterpret_cast<const nanoem_f32_t *>(data);
        stbi_write_hdr_to_func(callback, this, width, height, 8, dataPtr);
    }
}

void
ImageWriter::callback(void *context, void *data, int size)
{
    ImageWriter *self = static_cast<ImageWriter *>(context);
    FileUtils::write(self->m_scope.writer(), data, size, self->m_error);
}

bool
ImageWriter::writeImageAs(const char *extension)
{
    return StringUtils::equalsIgnoreCase(m_fileURI.pathExtension().c_str(), extension) &&
        m_scope.open(m_fileURI, m_error);
}

} /* namespace anonymous */

class CapturingPassState::ImageBlitter NANOEM_DECL_SEALED : public BasePass {
public:
    ImageBlitter(Project *project, bool flipY);
    ~ImageBlitter() NANOEM_DECL_NOEXCEPT;

    void blit(sg_pass pass);
    void setRect(const Vector4 &rect);

private:
    void draw(sg_pipeline pipeline, sg_pass dest, sg_image source);
    void setupShaderDescription(sg_shader_desc &desc) NANOEM_DECL_OVERRIDE;
    void setupPipelineDescription(sg_pipeline_desc &desc) NANOEM_DECL_OVERRIDE;
    const char *name() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    sg::QuadVertexUnit m_vertices[4];
};

CapturingPassState::ImageBlitter::ImageBlitter(Project *project, bool flipY)
    : BasePass(project)
{
    if (flipY) {
        m_vertices[0].m_texcoord = Vector4(0, 1, 0, 0);
        m_vertices[1].m_texcoord = Vector4(1, 1, 0, 0);
        m_vertices[2].m_texcoord = Vector4(0, 0, 0, 0);
        m_vertices[3].m_texcoord = Vector4(1, 0, 0, 0);
    }
    else {
        m_vertices[0].m_texcoord = Vector4(0, 0, 0, 0);
        m_vertices[1].m_texcoord = Vector4(1, 0, 0, 0);
        m_vertices[2].m_texcoord = Vector4(0, 1, 0, 0);
        m_vertices[3].m_texcoord = Vector4(1, 1, 0, 0);
    }
}

CapturingPassState::ImageBlitter::~ImageBlitter() NANOEM_DECL_NOEXCEPT
{
}

void
CapturingPassState::ImageBlitter::blit(sg_pass pass)
{
    sg_pipeline pipeline = { SG_INVALID_ID };
    PixelFormat format;
    format.setNumSamples(1);
    format.setColorPixelFormat(SG_PIXELFORMAT_RGBA8, 0);
    format.setDepthPixelFormat(SG_PIXELFORMAT_DEPTH_STENCIL);
    format.setNumColorAttachemnts(1);
    setupPipeline(format, pipeline);
    draw(pipeline, pass, m_project->viewportPrimaryImage());
}

void
CapturingPassState::ImageBlitter::setRect(const Vector4 &rect)
{
    sg_buffer &vb = m_bindings.vertex_buffers[0];
    nanoem_f32_t px = rect.x - 1, py = rect.y * (sg::query_features().origin_top_left ? 1 : -1) + 1, pw = rect.z * 2.0f,
                 ph = rect.w * -2.0f;
    m_vertices[0].m_position = Vector4(px, py, 0, 0);
    m_vertices[1].m_position = Vector4(pw + px, py, 0, 0);
    m_vertices[2].m_position = Vector4(px, ph + py, 0, 0);
    m_vertices[3].m_position = Vector4(pw + px, ph + py, 0, 0);
    if (!sg::is_valid(vb)) {
        sg_buffer_desc vbd;
        Inline::clearZeroMemory(vbd);
        vbd.usage = SG_USAGE_STREAM;
        vbd.size = sizeof(m_vertices);
        if (Inline::isDebugLabelEnabled()) {
            vbd.label = "@nanoem/CapturingPassState/ImageBlitter/Vertices";
        }
        vb = sg::make_buffer(&vbd);
        nanoem_assert(sg::query_buffer_state(vb) == SG_RESOURCESTATE_VALID, "vertex buffer must be valid");
        SG_LABEL_BUFFER(vb, vbd.label);
    }
    sg::update_buffer(vb, m_vertices, sizeof(m_vertices));
}

void
CapturingPassState::ImageBlitter::draw(sg_pipeline pipeline, sg_pass dest, sg_image source)
{
    sg_pass_action pa;
    Inline::clearZeroMemory(pa);
    pa.colors[0].action = pa.depth.action = pa.stencil.action = SG_ACTION_CLEAR;
    m_bindings.fs_images[0] = source;
    sg::PassBlock pb(m_project->sharedBatchDrawQueue(), dest, pa);
    pb.applyPipelineBindings(pipeline, m_bindings);
    pb.draw(0, 4);
}

void
CapturingPassState::ImageBlitter::setupShaderDescription(sg_shader_desc &desc)
{
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        desc.fs.bytecode.ptr = g_nanoem_color_transform_ps_dxbc_data;
        desc.fs.bytecode.size = g_nanoem_color_transform_ps_dxbc_size;
        desc.vs.bytecode.ptr = g_nanoem_color_transform_vs_dxbc_data;
        desc.vs.bytecode.size = g_nanoem_color_transform_vs_dxbc_size;
        desc.fs.images[0] = sg_shader_image_desc { nullptr, SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    }
    else if (sg::is_backend_metal(backend)) {
        desc.fs.bytecode.ptr = g_nanoem_color_transform_fs_msl_macos_data;
        desc.fs.bytecode.size = g_nanoem_color_transform_fs_msl_macos_size;
        desc.vs.bytecode.ptr = g_nanoem_color_transform_vs_msl_macos_data;
        desc.vs.bytecode.size = g_nanoem_color_transform_vs_msl_macos_size;
        desc.fs.images[0] = sg_shader_image_desc { nullptr, SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
    }
    else if (backend == SG_BACKEND_GLCORE33) {
        desc.fs.source = reinterpret_cast<const char *>(g_nanoem_color_transform_fs_glsl_core33_data);
        desc.vs.source = reinterpret_cast<const char *>(g_nanoem_color_transform_vs_glsl_core33_data);
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
        desc.fs.images[0] = sg_shader_image_desc { "SPIRV_Cross_Combined", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
#else
        desc.fs.images[0] = sg_shader_image_desc { "SPIRV_Cross_Combinedu_textureu_textureSampler", SG_IMAGETYPE_2D,
            SG_SAMPLERTYPE_FLOAT };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
    }
    else if (backend == SG_BACKEND_GLES3) {
        desc.fs.source = reinterpret_cast<const char *>(g_nanoem_color_transform_fs_glsl_es3_data);
        desc.vs.source = reinterpret_cast<const char *>(g_nanoem_color_transform_vs_glsl_es3_data);
#if defined(NANOEM_ENABLE_SHADER_OPTIMIZED)
        desc.fs.images[0] = sg_shader_image_desc { "SPIRV_Cross_Combined", SG_IMAGETYPE_2D, SG_SAMPLERTYPE_FLOAT };
#else
        desc.fs.images[0] = sg_shader_image_desc { "SPIRV_Cross_Combinedu_textureu_textureSampler", SG_IMAGETYPE_2D,
            SG_SAMPLERTYPE_FLOAT };
#endif /* NANOEM_ENABLE_SHADER_OPTIMIZED */
    }
    desc.vs.entry = "nanoemVSMain";
    desc.fs.entry = "nanoemPSMain";
    desc.attrs[0] = sg_shader_attr_desc { "a_position", "SV_POSITION", 0 };
    desc.attrs[1] = sg_shader_attr_desc { "a_normal", "NORMAL", 0 };
    desc.attrs[2] = sg_shader_attr_desc { "a_texcoord0", "TEXCOORD", 0 };
    if (backend == SG_BACKEND_D3D11) {
        desc.attrs[3] = sg_shader_attr_desc { "a_texcoord1", "TEXCOORD", 1 };
        desc.attrs[4] = sg_shader_attr_desc { "a_texcoord2", "TEXCOORD", 2 };
        desc.attrs[5] = sg_shader_attr_desc { "a_texcoord3", "TEXCOORD", 3 };
        desc.attrs[6] = sg_shader_attr_desc { "a_texcoord4", "TEXCOORD", 4 };
        desc.attrs[7] = sg_shader_attr_desc { "a_color0", "COLOR", 0 };
    }
}

void
CapturingPassState::ImageBlitter::setupPipelineDescription(sg_pipeline_desc &desc)
{
    desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLE_STRIP;
    desc.face_winding = SG_FACEWINDING_CW;
    desc.cull_mode = SG_CULLMODE_BACK;
    sg_layout_desc &ld = desc.layout;
    ld.buffers[0].stride = sizeof(sg::QuadVertexUnit);
    ld.attrs[0] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[1] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT3 };
    ld.attrs[2] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT2 };
    const sg_backend backend = sg::query_backend();
    if (backend == SG_BACKEND_D3D11) {
        ld.attrs[3] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT4 };
        ld.attrs[4] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT4 };
        ld.attrs[5] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT4 };
        ld.attrs[6] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_texcoord), SG_VERTEXFORMAT_FLOAT4 };
        ld.attrs[7] = sg_vertex_attr_desc { 0, offsetof(sg::QuadVertexUnit, m_position), SG_VERTEXFORMAT_FLOAT4 };
    }
}

const char *
CapturingPassState::ImageBlitter::name() const NANOEM_DECL_NOEXCEPT
{
    return "CapturingPassState/ImageBlitterPass";
}

struct ExportSize {
    const char *title;
    int width;
    int height;
};

static const ExportSize kExportableSizeList[] = { { "Application Default (960x640)", 960, 640 },
    { "XGA (1024x768)", 1024, 768 }, { "720p (1280x720)", 1280, 720 }, { "WXGA (1600x900)", 1600, 900 },
    { "UXGA (1600x1200)", 1600, 1200 }, { "1080p (1920x1080)", 1920, 1080 }, { "WUXGA (1920x1200)", 1920, 1200 },
    { "WQHD (2560x1440)", 2560, 1440 }, { "WQXGA (2560x1600)", 2560, 1600 }, { "4K (3840x2160)", 3840, 2160 },
    { "5K (5120x2880)", 5120, 2880 }, { "8K (7680x4320)", 7680, 4320 } };

class CapturingPassState::BaseModalDialog : public IModalDialog {
public:
    BaseModalDialog();
    ~BaseModalDialog();

    Vector4 desiredWindowSize(const Vector2UI16 &deviceScaleWindowSize,
        nanoem_f32_t deviceScalePxielRatio) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IModalDialog *onSaved(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onDiscarded(Project *project) NANOEM_DECL_OVERRIDE;
    bool isCancelled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setProgress(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void setRowHeight(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void setTitle(const char *value) NANOEM_DECL_OVERRIDE;
    void setText(const char *value) NANOEM_DECL_OVERRIDE;

protected:
    static void drawSeparator();
    Vector2UI16 resolutionSize(const Project *project) const;
    int sampleLevel() const;
    bool drawResolutionCombo(const Project *project);
    bool drawMSAACombo(const Project *project);

    bool m_viewportAspectRatioEnabled;
    bool m_preventFrameMisalighmentEnabled;

private:
    ApplicationMenuBuilder::MenuItemType m_sampleType;
    int m_resolutionSizeComboIndex;
};

CapturingPassState::BaseModalDialog::BaseModalDialog()
    : m_viewportAspectRatioEnabled(true)
    , m_preventFrameMisalighmentEnabled(true)
    , m_sampleType(ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx16)
    , m_resolutionSizeComboIndex(-1)
{
}

CapturingPassState::BaseModalDialog::~BaseModalDialog()
{
}

Vector4
CapturingPassState::BaseModalDialog::desiredWindowSize(
    const Vector2UI16 &deviceScaleWindowSize, nanoem_f32_t /* deviceScalePxielRatio */) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_f32_t widthRatio = 0.65f, heightRatio = 0.65f;
    const nanoem_f32_t widthOffsetRatio = (1.0f - widthRatio) * 0.5f, heightOffsetRatio = (1.0f - heightRatio) * 0.5f;
    return Vector4(deviceScaleWindowSize.x * widthOffsetRatio, deviceScaleWindowSize.y * heightOffsetRatio,
        deviceScaleWindowSize.x * widthRatio, deviceScaleWindowSize.y * heightRatio);
}

nanoem_u32_t
CapturingPassState::BaseModalDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk | kButtonTypeCancel;
}

IModalDialog *
CapturingPassState::BaseModalDialog::onSaved(Project *project)
{
    BX_UNUSED_1(project);
    return nullptr;
}

IModalDialog *
CapturingPassState::BaseModalDialog::onDiscarded(Project *project)
{
    BX_UNUSED_1(project);
    return nullptr;
}

bool
CapturingPassState::BaseModalDialog::isCancelled() const NANOEM_DECL_NOEXCEPT
{
    return false;
}

void
CapturingPassState::BaseModalDialog::setProgress(nanoem_f32_t value)
{
    BX_UNUSED_1(value);
}

void
CapturingPassState::BaseModalDialog::setRowHeight(nanoem_f32_t value)
{
    BX_UNUSED_1(value);
}

void
CapturingPassState::BaseModalDialog::setTitle(const char *value)
{
    BX_UNUSED_1(value);
}

void
CapturingPassState::BaseModalDialog::setText(const char *value)
{
    BX_UNUSED_1(value);
}

Vector2UI16
CapturingPassState::BaseModalDialog::resolutionSize(const Project *project) const
{
    Vector2UI16 value;
    if (m_resolutionSizeComboIndex >= 0) {
        const ExportSize &size = kExportableSizeList[m_resolutionSizeComboIndex];
        value = Vector2UI16(size.width, size.height);
    }
    else {
        value = project->viewportImageSize();
    }
    return value;
}

void
CapturingPassState::BaseModalDialog::drawSeparator()
{
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
}

int
CapturingPassState::BaseModalDialog::sampleLevel() const
{
    switch (m_sampleType) {
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx16:
        return 4;
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx8:
        return 3;
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx4:
        return 2;
    case ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx2:
        return 1;
    default:
        return 0;
    }
}

bool
CapturingPassState::BaseModalDialog::drawResolutionCombo(const Project *project)
{
    const ITranslator *translator = project->translator();
    ImGui::TextUnformatted(translator->translate("nanoem.window.dialog.export.resolution-size"));
    bool changed = false;
    const char *sameAsViewportText = translator->translate("nanoem.window.dialog.export.same-as-viewport");
    if (ImGui::BeginCombo("##size",
            m_resolutionSizeComboIndex >= 0 ? kExportableSizeList[m_resolutionSizeComboIndex].title
                                            : sameAsViewportText)) {
        if (ImGui::Selectable(sameAsViewportText)) {
            m_resolutionSizeComboIndex = -1;
            changed = true;
        }
        for (size_t i = 0; i < BX_COUNTOF(kExportableSizeList); i++) {
            if (ImGui::Selectable(kExportableSizeList[i].title)) {
                m_resolutionSizeComboIndex = Inline::saturateInt32(i);
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool
CapturingPassState::BaseModalDialog::drawMSAACombo(const Project *project)
{
    const ITranslator *translator = project->translator();
    ImGui::TextUnformatted(translator->translate("nanoem.window.dialog.export.sample-level"));
    const ApplicationMenuBuilder::MenuItemType types[] = {
        ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx16,
        ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx8,
        ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx4,
        ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx2,
        ApplicationMenuBuilder::kMenuItemTypeProjectDisableMSAA,
    };
    bool changed = false;
    if (ImGui::BeginCombo("##msaa", translator->translate(ApplicationMenuBuilder::menuItemString(m_sampleType)))) {
        for (size_t i = 0; i < BX_COUNTOF(types); i++) {
            ApplicationMenuBuilder::MenuItemType type = types[i];
            if (ImGui::Selectable(translator->translate(ApplicationMenuBuilder::menuItemString(type)))) {
                m_sampleType = type;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

class CapturingPassAsImageState::ModalDialog : public BaseModalDialog {
public:
    ModalDialog(CapturingPassAsImageState *parent);
    ~ModalDialog();

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onCancelled(Project *project) NANOEM_DECL_OVERRIDE;
    bool isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    CapturingPassAsImageState *m_parent;
    nanoem_frame_index_t m_captureFrameIndexAt;
    bool m_captureAtCurretFrameIndex;
};

CapturingPassAsImageState::ModalDialog::ModalDialog(CapturingPassAsImageState *parent)
    : m_parent(parent)
    , m_captureFrameIndexAt(0)
    , m_captureAtCurretFrameIndex(false)
{
}

CapturingPassAsImageState::ModalDialog::~ModalDialog()
{
}

const char *
CapturingPassAsImageState::ModalDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return "Export Image Configuration";
}

void
CapturingPassAsImageState::ModalDialog::draw(const Project *project)
{
    StateController *stateController = m_parent->stateController();
    BaseApplicationService *applicationPtr = stateController->application();
    const ITranslator *translator = applicationPtr->translator();
    ImGui::PushItemWidth(-1);
    ImGui::Columns(2);
    ImGui::PushItemWidth(-1);
    drawResolutionCombo(project);
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    drawMSAACombo(project);
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    ImGui::PopItemWidth();
    drawSeparator();
    ImGui::Checkbox(
        translator->translate("nanoem.window.dialog.export.viewport-aspect-ratio"), &m_viewportAspectRatioEnabled);
    ImGui::Checkbox(translator->translate("nanoem.window.dialog.export.image.seek"), &m_captureAtCurretFrameIndex);
    if (!m_captureAtCurretFrameIndex) {
        drawSeparator();
        const nanoem_frame_index_t min = 0, max = project->duration();
        ImGui::TextUnformatted(translator->translate("nanoem.window.dialog.export.image.seek.at"));
        ImGui::SameLine();
        ImGui::PushItemWidth(100 * project->windowDevicePixelRatio());
        ImGui::DragScalar("##frame", ImGuiDataType_U32, &m_captureFrameIndexAt, 1.0f, &min, &max);
        ImGui::PopItemWidth();
    }
}

IModalDialog *
CapturingPassAsImageState::ModalDialog::onAccepted(Project *project)
{
    StringList availableExtensions;
    availableExtensions.push_back("png");
    availableExtensions.push_back("bmp");
    availableExtensions.push_back("jpg");
    availableExtensions.push_back("tga");
    m_parent->setSampleLevel(sampleLevel());
    m_parent->setOutputImageSize(resolutionSize(project));
    m_parent->setStartFrameIndex(
        m_captureAtCurretFrameIndex ? project->currentLocalFrameIndex() : m_captureFrameIndexAt);
    project->eventPublisher()->publishCompleteExportingImageConfigurationEvent(availableExtensions);
    return nullptr;
}

IModalDialog *
CapturingPassAsImageState::ModalDialog::onCancelled(Project *project)
{
    project->eventPublisher()->publishCompleteExportingImageConfigurationEvent(StringList());
    m_parent->stateController()->application()->clearAllModalDialog();
    return nullptr;
}

bool
CapturingPassAsImageState::ModalDialog::isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    BX_UNUSED_1(value);
    return true;
}

class CapturingPassAsVideoState::ModalDialog : public BaseModalDialog, PluginUI::IReactor {
public:
    ModalDialog(CapturingPassAsVideoState *parent);
    ~ModalDialog();

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onCancelled(Project *project) NANOEM_DECL_OVERRIDE;
    bool isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    void handleUIComponent(
        const char *id, const Nanoem__Application__Plugin__UIComponent *component) NANOEM_DECL_OVERRIDE;
    bool reloadUILayout(ByteArray &bytes, Error &error) NANOEM_DECL_OVERRIDE;

private:
    static void drawStringPairList(const char *text, const StringPairList &items, String &value);

    CapturingPassAsVideoState *m_parent;
    PluginUI m_pluginUI;
    tinystl::pair<int, int> m_exportFrameIndexRange;
    plugin::EncoderPlugin *m_plugin;
    String m_audioCodec;
    String m_videoCodec;
    String m_videoFormat;
    bool m_reloadLayout;
    bool m_enableVideoRecorder;
};

CapturingPassAsVideoState::ModalDialog::ModalDialog(CapturingPassAsVideoState *parent)
    : BaseModalDialog()
    , m_parent(parent)
    , m_pluginUI(this, nullptr, nullptr, parent->stateController()->currentProject()->windowDevicePixelRatio())
    , m_exportFrameIndexRange(0, 0)
    , m_plugin(nullptr)
    , m_reloadLayout(false)
    , m_enableVideoRecorder(false)
{
    m_exportFrameIndexRange.second = parent->stateController()->currentProject()->duration();
}

CapturingPassAsVideoState::ModalDialog::~ModalDialog()
{
}

const char *
CapturingPassAsVideoState::ModalDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return "Encoding Video Configuration";
}

void
CapturingPassAsVideoState::ModalDialog::draw(const Project *project)
{
    StateController *stateController = m_parent->stateController();
    BaseApplicationService *applicationPtr = stateController->application();
    const ITranslator *translator = applicationPtr->translator();
    ImGui::PushItemWidth(-1);
    ImGui::Columns(3);
    ImGui::PushItemWidth(-1);
    if (drawResolutionCombo(project) && m_plugin) {
        const Vector2UI16 size(resolutionSize(project));
        Error error;
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, size.x, error);
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, size.y, error);
        m_reloadLayout = true;
    }
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    m_reloadLayout |= drawMSAACombo(project);
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    ImGui::PushItemWidth(-1);
    ImGui::TextUnformatted(translator->translate("nanoem.window.dialog.export.video.range"));
    if (ImGui::DragIntRange2("##range", &m_exportFrameIndexRange.first, &m_exportFrameIndexRange.second, 1.0f, 0,
            project->duration(), "%dF", nullptr, ImGuiSliderFlags_AlwaysClamp) &&
        m_plugin) {
        const nanoem_frame_index_t duration = m_exportFrameIndexRange.second - m_exportFrameIndexRange.first;
        Error error;
        m_plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION, duration, error);
        m_reloadLayout = true;
    }
    ImGui::PopItemWidth();
    ImGui::Columns(1);
    ImGui::Checkbox(
        translator->translate("nanoem.window.dialog.export.viewport-aspect-ratio"), &m_viewportAspectRatioEnabled);
    ImGui::Checkbox(translator->translate("nanoem.window.dialog.export.prevent-frame-misalighment"),
        &m_preventFrameMisalighmentEnabled);
    if (applicationPtr->hasVideoRecorder()) {
        drawSeparator();
        ImGui::Checkbox(
            translator->translate("nanoem.window.dialog.export.video.capture-video.native"), &m_enableVideoRecorder);
        if (m_enableVideoRecorder) {
            IVideoRecorder *videoRecorder = m_parent->m_videoRecorder = applicationPtr->createVideoRecorder();
            videoRecorder->setFileURI(m_parent->fileURI());
            StringPairList items;
            if (project->audioPlayer()->isLoaded()) {
                videoRecorder->getAllAvailableAudioCodecs(items);
                if (m_audioCodec.empty() && !items.empty()) {
                    m_audioCodec = items[0].first;
                }
                drawStringPairList(
                    translator->translate("nanoem.window.dialog.export.audio.codec"), items, m_audioCodec);
            }
            videoRecorder->getAllAvailableVideoCodecs(items);
            if (m_videoCodec.empty() && !items.empty()) {
                m_videoCodec = items[0].first;
            }
            drawStringPairList(translator->translate("nanoem.window.dialog.export.video.codec"), items, m_videoCodec);
            videoRecorder->getAllAvailableVideoProfiles(items);
            if (m_videoFormat.empty() && !items.empty()) {
                m_videoFormat = items[0].first;
            }
            drawStringPairList(
                translator->translate("nanoem.window.dialog.export.video.profile"), items, m_videoFormat);
        }
    }
    if (!m_enableVideoRecorder) {
        const IFileManager::EncoderPluginList plugins(stateController->fileManager()->allVideoEncoderPluginList());
        ImGui::TextUnformatted("Plugins");
        if (ImGui::BeginCombo("##plugin", m_plugin ? m_plugin->name().c_str() : "(none)")) {
            for (IFileManager::EncoderPluginList::const_iterator it = plugins.begin(), end = plugins.end(); it != end;
                 ++it) {
                plugin::EncoderPlugin *plugin = *it;
                const String name(plugin->name());
                if (ImGui::Selectable(name.c_str())) {
                    const Vector2UI16 size(resolutionSize(project));
                    const nanoem_frame_index_t duration =
                        m_exportFrameIndexRange.second - m_exportFrameIndexRange.first;
                    const nanoem_u32_t fps = 60;
                    Error error;
                    plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS, fps, error);
                    plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, size.x, error);
                    plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, size.y, error);
                    plugin->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION, duration, error);
                    const IAudioPlayer *audio = project->audioPlayer();
                    if (audio->isLoaded()) {
                        plugin->setOption(
                            NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS, audio->bitsPerSample(), error);
                        plugin->setOption(
                            NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS, audio->numChannels(), error);
                        plugin->setOption(
                            NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY, audio->sampleRate(), error);
                    }
                    m_plugin = plugin;
                    m_reloadLayout = true;
                }
            }
            ImGui::EndCombo();
        }
        if (m_plugin) {
            Error error;
            if (m_reloadLayout) {
                if (!m_pluginUI.reload(error)) {
                    m_pluginUI.clear();
                }
                error.notify(applicationPtr->eventPublisher());
                m_reloadLayout = false;
            }
            m_pluginUI.draw();
        }
    }
    drawSeparator();
    ImGui::TextUnformatted(translator->translate("nanoem.window.dialog.capture-video.caution.exporting"));
    if (project->windowDevicePixelRatio() > 1.0f) {
        ImGui::TextUnformatted(translator->translate("nanoem.window.dialog.capture-video.caution.retina"));
    }
    ImGui::PopItemWidth();
}

IModalDialog *
CapturingPassAsVideoState::ModalDialog::onAccepted(Project *project)
{
    StringList availableExtensions;
    nanoem_frame_index_t rangeFrom = m_exportFrameIndexRange.first, rangeTo = m_exportFrameIndexRange.second;
    if (rangeFrom > rangeTo) {
        const nanoem_frame_index_t temp = rangeFrom;
        rangeFrom = rangeTo;
        rangeTo = glm::min(temp, project->duration());
    }
    else {
        rangeTo = glm::min(rangeTo, project->duration());
    }
    if (m_enableVideoRecorder) {
        StringPairList items;
        IVideoRecorder *videoRecorder = m_parent->m_videoRecorder;
        videoRecorder->setAudioCodec(m_audioCodec);
        videoRecorder->setVideoCodec(m_videoCodec);
        videoRecorder->setVideoProfile(m_videoFormat);
        videoRecorder->setViewportAspectRatioEnabled(m_viewportAspectRatioEnabled);
        IVideoRecorder::FormatPairList formats;
        videoRecorder->getAllAvailableVideoPixelFormats(formats);
        videoRecorder->setVideoPixelFormat(SG_PIXELFORMAT_RGBA8);
        videoRecorder->setSize(resolutionSize(project));
        videoRecorder->makeConfigured();
        videoRecorder->getAllAvailableExtensions(availableExtensions);
    }
    else {
        const IFileManager::EncoderPluginList plugins(
            m_parent->stateController()->fileManager()->allVideoEncoderPluginList());
        m_parent->stateController()->application()->destroyVideoRecorder(m_parent->m_videoRecorder);
        m_parent->m_videoRecorder = nullptr;
        m_parent->setViewportAspectRatioEnabled(m_viewportAspectRatioEnabled);
        m_parent->setPreventFrameMisalighmentEnabled(m_preventFrameMisalighmentEnabled);
        m_parent->setStartFrameIndex(rangeFrom);
        m_parent->setEndFrameIndex(rangeTo);
        m_parent->setSampleLevel(sampleLevel());
        if (m_plugin) {
            m_parent->setEncoderPlugin(m_plugin);
            PluginFactory::EncoderPluginProxy proxy(m_plugin);
            availableExtensions = proxy.availableEncodingVideoFormatExtensions();
        }
    }
    m_parent->setOutputImageSize(resolutionSize(project));
    project->eventPublisher()->publishCompleteExportingVideoConfigurationEvent(availableExtensions);
    return nullptr;
}

IModalDialog *
CapturingPassAsVideoState::ModalDialog::onCancelled(Project *project)
{
    project->eventPublisher()->publishCompleteExportingVideoConfigurationEvent(StringList());
    m_parent->stateController()->application()->clearAllModalDialog();
    return nullptr;
}

bool
CapturingPassAsVideoState::ModalDialog::isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    bool result = true;
    if (EnumUtils::isEnabled(value, IModalDialog::kButtonTypeOk) && !m_enableVideoRecorder) {
        result &= m_plugin != nullptr;
    }
    return result;
}

void
CapturingPassAsVideoState::ModalDialog::handleUIComponent(
    const char *id, const Nanoem__Application__Plugin__UIComponent *component)
{
    PluginFactory::EncoderPluginProxy proxy(m_plugin);
    const nanoem_rsize_t size = nanoem__application__plugin__uicomponent__get_packed_size(component);
    if (size > 0) {
        ByteArray bytes(size);
        if (nanoem__application__plugin__uicomponent__pack(component, bytes.data())) {
            Error error;
            proxy.setUIComponentLayout(id, bytes, m_reloadLayout, error);
            error.notify(m_parent->stateController()->application()->eventPublisher());
        }
    }
}

bool
CapturingPassAsVideoState::ModalDialog::reloadUILayout(ByteArray &bytes, Error &error)
{
    PluginFactory::EncoderPluginProxy proxy(m_plugin);
    return proxy.getUIWindowLayout(bytes, error);
}

void
CapturingPassAsVideoState::ModalDialog::drawStringPairList(const char *text, const StringPairList &items, String &value)
{
    char buffer[Inline::kMarkerStringLength];
    ImGui::TextUnformatted(text);
    StringUtils::format(buffer, sizeof(buffer), "##%s", text);
    const char *defaultValueText = nullptr;
    for (StringPairList::const_iterator it = items.begin(), end = items.end(); it != end; ++it) {
        if (it->first == value) {
            defaultValueText = it->second.c_str();
            break;
        }
    }
    if (ImGui::BeginCombo(buffer, defaultValueText)) {
        for (StringPairList::const_iterator it = items.begin(), end = items.end(); it != end; ++it) {
            if (ImGui::Selectable(it->second.c_str())) {
                value = it->first;
            }
        }
        ImGui::EndCombo();
    }
}

CapturingPassState::CapturingPassState(StateController *stateControllerPtr, Project *project)
    : m_stateControllerPtr(stateControllerPtr)
    , m_blitter(nullptr)
    , m_project(project)
    , m_saveState(nullptr)
    , m_state(kNone)
    , m_lastViewportDevicePixelRatio(0)
    , m_lastPreferredMotionFPS(0)
    , m_lastSampleLevel(0)
    , m_sampleLevel(0)
    , m_lastVideoPTS(Motion::kMaxFrameIndex)
    , m_startFrameIndex(0)
    , m_blittedCount(0)
    , m_viewportAspectRatioEnabled(false)
    , m_displaySyncDisabled(false)
    , m_preventFrameMisalighmentEnabled(true)
{
    Inline::clearZeroMemory(m_outputImageDescription);
    Inline::clearZeroMemory(m_outputPassDescription);
    m_outputImageDescription.pixel_format = SG_PIXELFORMAT_RGBA8;
    m_outputImageDescription.render_target = true;
    m_outputPass = { SG_INVALID_ID };
    m_asyncCount = 0;
    m_blitter = nanoem_new(ImageBlitter(m_project, false));
}

CapturingPassState::~CapturingPassState() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_blitter);
}

void
CapturingPassState::save(Project *project)
{
    m_outputPassDescription.color_attachments[0].image = sg::make_image(&m_outputImageDescription);
    sg_image_desc depthStencilDescription(m_outputImageDescription);
    depthStencilDescription.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
    m_outputPassDescription.depth_stencil_attachment.image = sg::make_image(&depthStencilDescription);
    m_outputPass = sg::make_pass(&m_outputPassDescription);
    m_lastLogicalScaleUniformedViewportImageSize = project->logicalScaleUniformedViewportImageSize();
    m_lastViewportDevicePixelRatio = project->viewportDevicePixelRatio();
    m_lastPreferredMotionFPS = project->preferredMotionFPS();
    m_lastSampleLevel = project->sampleLevel();
    m_displaySyncDisabled = project->isDisplaySyncDisabled();
    project->saveState(m_saveState);
    project->setActiveAccessory(nullptr);
    project->setActiveModel(nullptr);
    project->setSampleLevel(m_sampleLevel);
    project->setPhysicsSimulationMode(
        project->physicsEngine()->simulationMode() != PhysicsEngine::kSimulationModeDisable
            ? PhysicsEngine::kSimulationModeEnableTracing
            : PhysicsEngine::kSimulationModeDisable);
    project->setPreferredMotionFPS(60, m_preventFrameMisalighmentEnabled ? false : true);
    project->seek(m_startFrameIndex, true);
    project->restart(m_startFrameIndex);
}

void
CapturingPassState::restore(Project *project)
{
    if (project) {
        project->setViewportCaptured(false);
        if (m_saveState) {
            project->restoreState(m_saveState, true);
            project->destroyState(m_saveState);
        }
        project->setViewportDevicePixelRatio(m_lastViewportDevicePixelRatio);
        project->setPreferredMotionFPS(m_lastPreferredMotionFPS, m_displaySyncDisabled);
        project->setSampleLevel(m_lastSampleLevel);
        project->resizeUniformedViewportImage(m_lastLogicalScaleUniformedViewportImageSize);
        project->restart(project->currentLocalFrameIndex());
    }
}

bool
CapturingPassState::transitDestruction(Project *project)
{
    bool deletable = false;
    switch (m_state) {
    case internal::CapturingPassState::kInitialized:
    case internal::CapturingPassState::kConfigured:
    case internal::CapturingPassState::kReady:
    case internal::CapturingPassState::kBlitted: {
        cancel();
        break;
    }
    case internal::CapturingPassState::kNone:
    case internal::CapturingPassState::kFinished:
    case internal::CapturingPassState::kCancelled: {
        setStateTransition(kDestroyReady);
        break;
    }
    case internal::CapturingPassState::kDestroyReady: {
        restore(project);
        setStateTransition(kRestored);
        break;
    }
    case internal::CapturingPassState::kRestored: {
        const int asyncCount = m_asyncCount;
        if (asyncCount <= 0) {
            destroy();
            setStateTransition(kDestroyed);
        }
        break;
    }
    case internal::CapturingPassState::kDestroyed: {
        m_stateControllerPtr->application()->clearAllModalDialog();
        deletable = true;
        break;
    }
    default:
        nanoem_assert(0, "should not be reach here");
        break;
    }
    return deletable;
}

void
CapturingPassState::setOutputImageSize(const Vector2UI16 &value)
{
    m_outputImageDescription.width = value.x;
    m_outputImageDescription.height = value.y;
    m_frameImageData.resize(value.x * value.y * 4 * sizeof(nanoem_u8_t));
    sg_buffer_desc desc;
    Inline::clearZeroMemory(desc);
    desc.size = m_frameImageData.size();
    desc.usage = SG_USAGE_STREAM;
    if (Inline::isDebugLabelEnabled()) {
        desc.label = "@nanoem/CapturingPassState/FrameStagingBuffer";
    }
    m_frameStagingBuffer = sg::make_buffer(&desc);
    nanoem_assert(
        sg::query_buffer_state(m_frameStagingBuffer) == SG_RESOURCESTATE_VALID, "frame staging buffer must be valid");
    SG_LABEL_BUFFER(m_frameStagingBuffer, desc.label);
}

void
CapturingPassState::setPixelFormat(sg_pixel_format value)
{
    m_outputImageDescription.pixel_format = value;
}

void
CapturingPassState::setStartFrameIndex(nanoem_frame_index_t value)
{
    m_startFrameIndex = value;
}

void
CapturingPassState::setSampleLevel(nanoem_u32_t value)
{
    m_sampleLevel = value;
}

void
CapturingPassState::setViewportAspectRatioEnabled(bool value)
{
    m_viewportAspectRatioEnabled = value;
}

void
CapturingPassState::setPreventFrameMisalighmentEnabled(bool value)
{
    m_preventFrameMisalighmentEnabled = value;
}

URI
CapturingPassState::fileURI() const
{
    return m_fileURI;
}

void
CapturingPassState::setFileURI(const URI &value)
{
    m_fileURI = value;
}

bool
CapturingPassState::prepareCapturingViewport(Project *project)
{
    const nanoem_u16_t width = m_outputImageDescription.width, height = m_outputImageDescription.height;
    StateTransition state = stateTransition();
    if (state == kInitialized && width > 0 && height > 0) {
        project->setViewportCaptured(true);
        project->setViewportDevicePixelRatio(1.0f);
        project->resizeUniformedViewportImage(Vector2UI16(width, height));
        if (m_viewportAspectRatioEnabled) {
            const Vector4 layout(project->logicalScaleUniformedViewportLayoutRect());
            const nanoem_f32_t ratio = glm::max(width / layout.z, height / layout.w);
            const Vector2 viewportImageSizeF(layout.z * ratio, layout.w * ratio);
            project->resizeUniformedViewportLayout(Vector4UI16(0, 0, viewportImageSizeF.x, viewportImageSizeF.y));
            const Vector4 rect((width - viewportImageSizeF.x) / width, (height - viewportImageSizeF.y) / height,
                (viewportImageSizeF.x / width), (viewportImageSizeF.y / height));
            m_blitter->setRect(rect);
        }
        else {
            m_blitter->setRect(Vector4(0, 0, 1, 1));
        }
        state = kConfigured;
        setStateTransition(state);
    }
    else if (state >= kConfigured && state < kReady && project->isViewportCaptured()) {
        blitOutputPass();
        state = kReady;
        setStateTransition(state);
    }
    return state >= kReady && state < kFinished;
}

void
CapturingPassState::readPassImage()
{
    sg::read_pass(m_outputPass, m_frameStagingBuffer, m_frameImageData.data(), m_frameImageData.size());
    if (m_outputImageDescription.pixel_format == SG_PIXELFORMAT_RGBA8) {
        nanoem_u32_t *dataPtr = reinterpret_cast<nanoem_u32_t *>(m_frameImageData.data());
        for (size_t i = 0, size = m_frameImageData.size() / sizeof(*dataPtr); i < size; i++) {
            nanoem_u32_t *ptr = dataPtr + i, v = *ptr;
            *ptr = 0 | ((v & 0x000000ff) << 16) | (v & 0x0000ff00) | ((v & 0x00ff0000) >> 16) | (v & 0xff000000);
        }
    }
}

StateController *
CapturingPassState::stateController()
{
    return m_stateControllerPtr;
}

const ByteArray &
CapturingPassState::frameImageData() const
{
    return m_frameImageData;
}

nanoem_u8_t *
CapturingPassState::mutableFrameImageDataPtr()
{
    return m_frameImageData.data();
}

bx::Mutex &
CapturingPassState::mutex()
{
    return m_mutex;
}

sg_image_desc
CapturingPassState::outputImageDescription() const
{
    return m_outputImageDescription;
}

CapturingPassState::StateTransition
CapturingPassState::stateTransition()
{
    bx::MutexScope scope(m_mutex);
    BX_UNUSED_1(scope);
    return m_state;
}

void
CapturingPassState::setStateTransition(StateTransition value)
{
    bx::MutexScope scope(m_mutex);
    BX_UNUSED_1(scope);
    m_state = value;
}

sg_pass
CapturingPassState::outputPass() const
{
    return m_outputPass;
}

sg_buffer
CapturingPassState::frameStagingBuffer() const
{
    return m_frameStagingBuffer;
}

nanoem_frame_index_t
CapturingPassState::startFrameIndex() const
{
    return m_startFrameIndex;
}

nanoem_frame_index_t
CapturingPassState::lastVideoPTS() const
{
    return m_lastVideoPTS;
}

void
CapturingPassState::setLastVideoPTS(nanoem_frame_index_t value)
{
    m_lastVideoPTS = value;
}

bool
CapturingPassState::hasSaveState() const
{
    return m_saveState != nullptr;
}

void
CapturingPassState::blitOutputPass()
{
    m_blitter->blit(m_outputPass);
}

void
CapturingPassState::incrementAsyncCount()
{
    bx::atomicAddAndFetch(&m_asyncCount, 1);
}

void
CapturingPassState::decrementAsyncCount()
{
    bx::atomicSubAndFetch(&m_asyncCount, 1);
}

bool
CapturingPassState::canBlitTransition()
{
    return m_blittedCount++ >= 2;
}

void
CapturingPassState::destroy()
{
    sg::destroy_image(m_outputPassDescription.color_attachments[0].image);
    sg::destroy_image(m_outputPassDescription.depth_stencil_attachment.image);
    sg::destroy_pass(m_outputPass);
    m_outputPass = { SG_INVALID_ID };
}

CapturingPassAsImageState::CapturingPassAsImageState(StateController *stateControllerPtr, Project *project)
    : CapturingPassState(stateControllerPtr, project)
{
}

CapturingPassAsImageState::~CapturingPassAsImageState() NANOEM_DECL_NOEXCEPT
{
}

bool
CapturingPassAsImageState::start(Error & /* error */)
{
    nanoem_assert(!fileURI().isEmpty(), "must NOT be empty");
    setStateTransition(kInitialized);
    return true;
}

IModalDialog *
CapturingPassAsImageState::createDialog()
{
    StateController *controller = stateController();
    const ITranslator *translator = controller->currentProject()->translator();
    String message;
    StringUtils::format(message, translator->translate("nanoem.window.progress.capture-image"),
        fileURI().lastPathComponentConstString());
    return ModalDialogFactory::createProgressDialog(
        controller->application(), "Capturing Image", message, handleCancelExportingImage, this);
}

bool
CapturingPassAsImageState::capture(Project *project, Error &error)
{
    StateTransition state(stateTransition());
    if (prepareCapturingViewport(project)) {
        const sg_image_desc desc(outputImageDescription());
        if (state == kReady) {
            blitOutputPass();
            if (canBlitTransition()) {
                state = kBlitted;
                setStateTransition(state);
            }
        }
        else if (state == kBlitted) {
            SG_PUSH_GROUPF(
                "internal::CapturingPassAsImageState::capture(width=%d, height=%d)", desc.width, desc.height);
            readPassImage();
            ImageWriter writer(fileURI(), error);
            const nanoem_u32_t width = desc.width, height = desc.height;
            writer.write(mutableFrameImageDataPtr(), width, height, desc.pixel_format);
            state = kFinished;
            setStateTransition(state);
            SG_POP_GROUP();
        }
    }
    return state >= kFinished;
}

void
CapturingPassAsImageState::cancel()
{
    setStateTransition(kCancelled);
}

void
CapturingPassAsImageState::addExportImageDialog(Project * /* project */)
{
    BaseApplicationService *applicationPtr = stateController()->application();
    IModalDialog *dialog = nanoem_new(ModalDialog(this));
    applicationPtr->addModalDialog(dialog);
}

IModalDialog *
CapturingPassAsImageState::handleCancelExportingImage(void *userData, Project * /* project */)
{
    CapturingPassState *self = static_cast<CapturingPassState *>(userData);
    self->cancel();
    return nullptr;
}

CapturingPassAsVideoState::CapturingPassAsVideoState(StateController *stateControllerPtr, Project *project)
    : CapturingPassState(stateControllerPtr, project)
    , m_encoderPluginPtr(nullptr)
    , m_videoRecorder(nullptr)
    , m_destroyingVideoRecorder(nullptr)
    , m_endFrameIndex(project->duration())
    , m_amount(0)
{
}

CapturingPassAsVideoState::~CapturingPassAsVideoState() NANOEM_DECL_NOEXCEPT
{
    setEncoderPlugin(nullptr);
}

bool
CapturingPassAsVideoState::start(Error &error)
{
    nanoem_assert(!fileURI().isEmpty(), "must NOT be empty");
    StateTransition state = kNone;
    if (m_videoRecorder) {
        m_videoRecorder->setFileURI(fileURI());
        state = m_videoRecorder->start(error) ? kInitialized : kCancelled;
        setStateTransition(state);
    }
    else if (m_encoderPluginPtr) {
        const sg_image_desc desc(outputImageDescription());
        const nanoem_u32_t fps = 60;
        const bool topLeft = sg::query_features().origin_top_left;
        Project *project = stateController()->currentProject();
        m_encoderPluginPtr->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS, fps, error);
        m_encoderPluginPtr->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION, duration(), error);
        m_encoderPluginPtr->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, desc.width, error);
        m_encoderPluginPtr->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, desc.height, error);
        m_encoderPluginPtr->setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP, !topLeft, error);
        const IAudioPlayer *audio = project->audioPlayer();
        if (audio->isLoaded()) {
            m_encoderPluginPtr->setOption(
                NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS, audio->bitsPerSample(), error);
            m_encoderPluginPtr->setOption(
                NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS, audio->numChannels(), error);
            m_encoderPluginPtr->setOption(
                NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY, audio->sampleRate(), error);
        }
        if (m_encoderPluginPtr->open(fileURI(), error)) {
            state = kInitialized;
        }
        else {
            state = kCancelled;
        }
        setStateTransition(state);
    }
    return state == kInitialized;
}

IModalDialog *
CapturingPassAsVideoState::createDialog()
{
    const Project *project = stateController()->currentProject();
    const ITranslator *translator = project->translator();
    String message;
    StringUtils::format(message, translator->translate("nanoem.window.progress.capture-video"),
        fileURI().lastPathComponentConstString(), project->actualFPS());
    return ModalDialogFactory::createProgressDialog(
        stateController()->application(), "Capturing Video", message, handleCancelExportingVideo, this);
}

bool
CapturingPassAsVideoState::capture(Project *project, Error &error)
{
    if (prepareCapturingViewport(project)) {
        const sg_image_desc desc(outputImageDescription());
        BX_UNUSED_1(desc);
        SG_PUSH_GROUPF("CapturingPassAsVideoState::capture(frameIndex=%d, amount=%.3f, width=%d, height=%d)",
            project->currentLocalFrameIndex(), m_amount, desc.width, desc.height);
        nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
        const nanoem_f32_t deltaScaleFactor = project->motionFPSScaleFactor();
        const nanoem_frame_index_t audioPTS = static_cast<nanoem_frame_index_t>(
                                       frameIndex * deltaScaleFactor + m_amount * deltaScaleFactor),
                                   videoPTS = audioPTS - nanoem_frame_index_t(startFrameIndex() * deltaScaleFactor),
                                   durationFrameIndices = static_cast<nanoem_frame_index_t>(duration());
        if (m_videoRecorder) {
            handleCaptureViaVideoRecorder(
                project, frameIndex, audioPTS, videoPTS, durationFrameIndices, deltaScaleFactor);
        }
        else {
            handleCaptureViaEncoderPlugin(
                project, frameIndex, audioPTS, videoPTS, durationFrameIndices, deltaScaleFactor, error);
        }
        SG_POP_GROUP();
    }
    return stateTransition() >= kFinished;
}

void
CapturingPassAsVideoState::cancel()
{
    Error error;
    stopEncoding(error);
    setStateTransition(kCancelled);
}

void
CapturingPassAsVideoState::restore(Project *project)
{
    if (hasSaveState()) {
        setEncoderPlugin(nullptr);
        CapturingPassState::restore(project);
    }
}

void
CapturingPassAsVideoState::setEncoderPlugin(plugin::EncoderPlugin *value)
{
    if (value != m_encoderPluginPtr) {
        Error error;
        stopEncoding(error);
        m_encoderPluginPtr = value;
    }
}

void
CapturingPassAsVideoState::setEndFrameIndex(nanoem_frame_index_t value)
{
    m_endFrameIndex = value;
}

void
CapturingPassAsVideoState::addExportVideoDialog(Project * /* project */)
{
    BaseApplicationService *applicationPtr = stateController()->application();
    IModalDialog *dialog = nanoem_new(ModalDialog(this));
    applicationPtr->addModalDialog(dialog);
}

IModalDialog *
CapturingPassAsVideoState::handleCancelExportingVideo(void *userData, Project * /* project */)
{
    CapturingPassState *self = static_cast<CapturingPassState *>(userData);
    self->cancel();
    return nullptr;
}

void
CapturingPassAsVideoState::calculateFrameIndex(
    nanoem_f32_t deltaScaleFactor, nanoem_f32_t &amount, nanoem_frame_index_t &frameIndex)
{
    amount += (1.0f / deltaScaleFactor);
    if (amount > 1.0f - Constants::kEpsilon) {
        frameIndex += 1;
        amount = 0;
    }
}

nanoem_frame_index_t
CapturingPassAsVideoState::duration() const NANOEM_DECL_NOEXCEPT
{
    nanoem_assert(
        m_endFrameIndex >= startFrameIndex(), "must be m_endFrameIndex greater than or equal to m_startFrameIndex");
    return m_endFrameIndex - startFrameIndex();
}

void
CapturingPassAsVideoState::handleCaptureViaVideoRecorder(Project *project, nanoem_frame_index_t frameIndex,
    nanoem_frame_index_t audioPTS, nanoem_frame_index_t videoPTS, nanoem_frame_index_t durationFrameIndices,
    nanoem_f32_t deltaScaleFactor)
{
    BX_UNUSED_1(audioPTS);
    if (m_videoRecorder->isCancelled()) {
        finishEncoding();
    }
    else if (m_videoRecorder->isStarted()) {
        if (frameIndex > durationFrameIndices) {
            finishEncoding();
        }
        else if (m_videoRecorder->capture(videoPTS)) {
            if (IModalDialog *dialog = stateController()->application()->currentModalDialog()) {
                const ITranslator *translator = project->translator();
                char message[Inline::kLongNameStackBufferSize];
                nanoem_f32_t percentage =
                    nanoem_f32_t((frameIndex - startFrameIndex()) / nanoem_f64_t(durationFrameIndices));
                StringUtils::format(message, sizeof(message),
                    translator->translate("nanoem.window.progress.capture-video"),
                    fileURI().lastPathComponentConstString(), project->actualFPS());
                dialog->setProgress(percentage);
                dialog->setText(message);
            }
            calculateFrameIndex(deltaScaleFactor, m_amount, frameIndex);
            if (frameIndex > durationFrameIndices) {
                finishEncoding();
            }
            else {
                project->seek(frameIndex, m_amount, true);
            }
        }
    }
}

void
CapturingPassAsVideoState::handleCaptureViaEncoderPlugin(Project *project, nanoem_frame_index_t frameIndex,
    nanoem_frame_index_t audioPTS, nanoem_frame_index_t videoPTS, nanoem_frame_index_t durationFrameIndices,
    nanoem_f32_t deltaScaleFactor, Error &error)
{
    struct AsyncReadHandler {
        AsyncReadHandler(CapturingPassAsVideoState *state, nanoem_frame_index_t audioPTS, nanoem_frame_index_t videoPTS)
            : m_state(state)
            , m_audioPTS(audioPTS)
            , m_videoPTS(videoPTS)
        {
            m_state->incrementAsyncCount();
        }
        ~AsyncReadHandler()
        {
            m_state->decrementAsyncCount();
        }
        static void
        handleReadPassAsync(const void *data, size_t size, void *opaque)
        {
            AsyncReadHandler *self = static_cast<AsyncReadHandler *>(opaque);
            self->readPassAsync(data, size);
            nanoem_delete(self);
        }
        void
        readPassAsync(const void *data, size_t size)
        {
            bx::MutexScope scope(m_state->mutex());
            Error error;
            if (m_state->frameImageData().size() == size) {
                memcpy(m_state->mutableFrameImageDataPtr(), data, size);
                if (m_state->outputImageDescription().pixel_format == SG_PIXELFORMAT_RGBA8) {
                    nanoem_u32_t *dataPtr = reinterpret_cast<nanoem_u32_t *>(m_state->mutableFrameImageDataPtr());
                    for (size_t i = 0, numPixels = size / sizeof(*dataPtr); i < numPixels; i++) {
                        nanoem_u32_t *ptr = dataPtr + i, v = *ptr;
                        *ptr = 0 | ((v & 0x000000ff) << 16) | (v & 0x0000ff00) | ((v & 0x00ff0000) >> 16) |
                            (v & 0xff000000);
                    }
                }
                if (!m_state->encodeVideoFrame(m_state->frameImageData(), m_audioPTS, m_videoPTS, error)) {
                    m_state->stopEncoding(error);
                    m_state->setStateTransition(kCancelled);
                }
            }
            else {
                m_state->stopEncoding(error);
                m_state->setStateTransition(kCancelled);
            }
        }
        CapturingPassAsVideoState *m_state;
        nanoem_frame_index_t m_audioPTS;
        nanoem_frame_index_t m_videoPTS;
    };
    BX_UNUSED_1(error);
    blitOutputPass();
    StateTransition state = stateTransition();
    if (state == kReady) {
        /* same as blit except without calculation of frame index */
        seekAndProgress(project, frameIndex, durationFrameIndices);
        if (canBlitTransition()) {
            setStateTransition(kBlitted);
        }
    }
    else if (state == kBlitted) {
        if (sg::read_pass_async) {
            AsyncReadHandler *handler = nanoem_new(AsyncReadHandler(this, audioPTS, videoPTS));
            sg::read_pass_async(outputPass(), frameStagingBuffer(), &AsyncReadHandler::handleReadPassAsync, handler);
        }
        else {
            readPassImage();
            if (!encodeVideoFrame(frameImageData(), audioPTS, videoPTS, error)) {
                setStateTransition(kCancelled);
            }
        }
        calculateFrameIndex(deltaScaleFactor, m_amount, frameIndex);
        seekAndProgress(project, frameIndex, durationFrameIndices);
    }
}

bool
CapturingPassAsVideoState::encodeVideoFrame(
    const ByteArray &frameData, nanoem_frame_index_t audioPTS, nanoem_frame_index_t videoPTS, Error &error)
{
    bool continuable = true;
    if (m_encoderPluginPtr && (lastVideoPTS() == Motion::kMaxFrameIndex || videoPTS > lastVideoPTS())) {
        const Project *project = stateController()->currentProject();
        const IAudioPlayer *audioPlayer = project->audioPlayer();
        const ByteArray *samplesPtr = audioPlayer->linearPCMSamples();
        if (audioPlayer->isLoaded() && !samplesPtr->empty()) {
            const size_t bufferSize = size_t(audioPlayer->numChannels() * audioPlayer->sampleRate() *
                (audioPlayer->bitsPerSample() / 8) * project->invertedPreferredMotionFPS());
            const size_t offset = size_t(audioPTS * bufferSize),
                         rest = samplesPtr->size() >= offset ? samplesPtr->size() - offset : 0;
            ByteArray slice(bufferSize);
            memcpy(slice.data(), samplesPtr->data() + offset, glm::min(rest, bufferSize));
            continuable &=
                m_encoderPluginPtr->encodeAudioFrame(nanoem_frame_index_t(videoPTS), slice.data(), slice.size(), error);
        }
        continuable &= m_encoderPluginPtr->encodeVideoFrame(videoPTS, frameData.data(), frameData.size(), error);
        setLastVideoPTS(videoPTS);
    }
    return continuable;
}

void
CapturingPassAsVideoState::seekAndProgress(
    Project *project, nanoem_frame_index_t frameIndex, nanoem_frame_index_t durationFrameIndices)
{
    if (frameIndex > m_endFrameIndex) {
        setStateTransition(kFinished);
    }
    else {
        if (IModalDialog *dialog = stateController()->application()->currentModalDialog()) {
            const ITranslator *translator = project->translator();
            char message[Inline::kLongNameStackBufferSize];
            nanoem_f32_t percentage =
                nanoem_f32_t((frameIndex - startFrameIndex()) / nanoem_f64_t(durationFrameIndices));
            StringUtils::format(message, sizeof(message), translator->translate("nanoem.window.progress.capture-video"),
                fileURI().lastPathComponentConstString(), project->actualFPS());
            dialog->setProgress(percentage);
            dialog->setText(message);
        }
        project->seek(frameIndex, m_amount, true);
    }
}

void
CapturingPassAsVideoState::finishEncoding()
{
    if (m_videoRecorder) {
        BaseApplicationService *application = stateController()->application();
        Error error;
        if (m_videoRecorder->finish(error)) {
            application->clearAllModalDialog();
            setStateTransition(kFinished);
        }
        else {
            error.addModalDialog(application);
            setStateTransition(kCancelled);
        }
        m_destroyingVideoRecorder = m_videoRecorder;
        m_videoRecorder = nullptr;
    }
}

void
CapturingPassAsVideoState::stopEncoding(Error &error)
{
    if (plugin::EncoderPlugin *plugin = static_cast<plugin::EncoderPlugin *>(
            bx::atomicExchangePtr(reinterpret_cast<void **>(&m_encoderPluginPtr), nullptr))) {
        plugin->close(error);
        plugin->wait();
        plugin = nullptr;
        setStateTransition(kFinished);
    }
}

void
CapturingPassAsVideoState::destroy()
{
    CapturingPassState::destroy();
    if (m_destroyingVideoRecorder) {
        BaseApplicationService *application = stateController()->application();
        application->destroyVideoRecorder(m_destroyingVideoRecorder);
        m_destroyingVideoRecorder = nullptr;
    }
}

} /* namespace internal */
} /* namespace nanoem */
