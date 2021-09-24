/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "D3D11BackgroundDrawer.h"
#if WINVER >= _WIN32_WINNT_WIN7

#include "COMInline.h"

/* include system header first due to UUID type confliction problem */
#include <Mferror.h>
#include <d3d11.h>
#include <dxgi.h>
#include <evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <propvarutil.h>

#include "emapp/Constants.h"
#include "emapp/EnumUtils.h"
#include "emapp/Error.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationService.h"
#include "emapp/internal/BlitPass.h"
#include "emapp/internal/DecoderPluginBasedBackgroundVideoRenderer.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace win32 {

const char *const D3D11BackgroundVideoDrawer::kLabelPrefix = "@nanoem/D3D11BackgroundVideoRenderer";

D3D11BackgroundVideoDrawer::D3D11BackgroundVideoDrawer(ThreadedApplicationService *service)
    : m_service(service)
{
    Inline::clearZeroMemory(m_description);
}

D3D11BackgroundVideoDrawer::~D3D11BackgroundVideoDrawer()
{
}

bool
D3D11BackgroundVideoDrawer::load(const URI &fileURI, Error &error)
{
    MutableWideString ws;
    StringUtils::getWideCharString(fileURI.absolutePathConstString(), ws);
    IMFSourceReader *reader = nullptr;
    IMFAttributes *attributes = nullptr;
    MFCreateAttributes(&attributes, 2);
    attributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, 1);
    attributes->SetUINT32(MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS, 1);
    COMInline::wrapCall(MFCreateSourceReaderFromURL(ws.data(), nullptr, &reader), error);
    COMInline::safeRelease(attributes);
    if (!error.hasReason()) {
        IMFMediaType *nativeMediaType, *inputIYUV, *outputRGB;
        UINT32 width, height, numFPS, denFPS;
        COMInline::wrapCall(
            reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &nativeMediaType), error);
        COMInline::wrapCall(MFGetAttributeRatio(nativeMediaType, MF_MT_FRAME_RATE, &numFPS, &denFPS), error);
        COMInline::wrapCall(MFGetAttributeSize(nativeMediaType, MF_MT_FRAME_SIZE, &width, &height), error);
        COMInline::safeRelease(nativeMediaType);
        COMInline::wrapCall(MFCreateMediaType(&inputIYUV), error);
        COMInline::wrapCall(inputIYUV->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), error);
        COMInline::wrapCall(inputIYUV->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_IYUV), error);
        COMInline::wrapCall(MFSetAttributeSize(inputIYUV, MF_MT_FRAME_SIZE, width, height), error);
        COMInline::wrapCall(MFSetAttributeRatio(inputIYUV, MF_MT_FRAME_RATE, numFPS, denFPS), error);
        COMInline::wrapCall(
            reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, inputIYUV), error);
        COMInline::wrapCall(MFCreateMediaType(&outputRGB), error);
        COMInline::wrapCall(outputRGB->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video), error);
        COMInline::wrapCall(outputRGB->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_ARGB32), error);
        COMInline::wrapCall(outputRGB->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, 1), error);
        COMInline::wrapCall(MFSetAttributeSize(outputRGB, MF_MT_FRAME_SIZE, width, height), error);
        COMInline::wrapCall(MFSetAttributeRatio(outputRGB, MF_MT_FRAME_RATE, numFPS, denFPS), error);
        IMFActivate **activates = nullptr;
        UINT32 numActivates = 0;
        COMInline::wrapCall(
            MFTEnumEx(MFT_CATEGORY_VIDEO_PROCESSOR, MFT_ENUM_FLAG_ALL, nullptr, nullptr, &activates, &numActivates),
            error);
        for (uint32_t i = 0; i < numActivates; i++) {
            IMFActivate *activate = activates[i];
            COMInline::wrapCall(activate->ActivateObject(IID_PPV_ARGS(&m_transform)), error);
        }
        COMInline::wrapCall(m_transform->SetInputType(0, inputIYUV, 0), error);
        COMInline::wrapCall(m_transform->SetOutputType(0, outputRGB, 0), error);
        COMInline::safeRelease(inputIYUV);
        COMInline::safeRelease(outputRGB);
        CoTaskMemFree(activates);
        destroy();
        if (!error.hasReason()) {
            m_description.pixel_format = SG_PIXELFORMAT_RGBA8;
            m_description.usage = SG_USAGE_DYNAMIC;
            m_description.width = Inline::saturateInt32(width);
            m_description.height = Inline::saturateInt32(height);
            m_description.mag_filter = m_description.min_filter = SG_FILTER_NEAREST;
            m_description.wrap_u = m_description.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
            char label[Inline::kMarkerStringLength];
            if (Inline::isDebugLabelEnabled()) {
                StringUtils::format(label, sizeof(label), "%s/ColorImage", kLabelPrefix);
                m_description.label = label;
            }
            m_image = sg::make_image(&m_description);
            SG_LABEL_IMAGE(m_image, label);
            m_fileURI = fileURI;
            m_reader = reader;
        }
    }
    if (!m_reader) {
        Error error2;
        COMInline::safeRelease(reader);
        m_decoderPluginBasedBackgroundVideoRenderer =
            nanoem_new(internal::DecoderPluginBasedBackgroundVideoRenderer(m_service->defaultFileManager()));
        if (m_decoderPluginBasedBackgroundVideoRenderer->load(fileURI, error2)) {
            error = Error();
        }
        else {
            m_decoderPluginBasedBackgroundVideoRenderer->destroy();
            nanoem_delete_safe(m_decoderPluginBasedBackgroundVideoRenderer);
            if (!error.hasReason()) {
                error = error2;
            }
        }
    }
    return !error.hasReason();
}

void
D3D11BackgroundVideoDrawer::draw(sg_pass pass, const Vector4 &rect, nanoem_f32_t scaleFactor, Project *project)
{
    Error error;
    if (m_reader && sg::is_valid(m_image)) {
        const sg::NamedPass namedPass(tinystl::make_pair(pass, Project::kViewportPrimaryName));
        const sg::NamedImage namedImage(tinystl::make_pair(m_image, kLabelPrefix));
        const PixelFormat format(project->findRenderPassPixelFormat(pass, project->sampleCount()));
        project->sharedImageBlitter()->blit(project->sharedBatchDrawQueue(), namedPass, namedImage, rect, format);
        if (!m_durationUpdated) {
            PROPVARIANT pd;
            PropVariantInit(&pd);
            m_reader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &pd);
            project->setBaseDuration((pd.hVal.QuadPart / 10000000.0) * Constants::kHalfBaseFPS);
            PropVariantClear(&pd);
            m_durationUpdated = true;
        }
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->draw(pass, rect, scaleFactor, project);
    }
}

void
D3D11BackgroundVideoDrawer::seek(nanoem_f64_t value)
{
    if (m_reader) {
        Error error;
        PROPVARIANT variant;
        const LONGLONG pos = static_cast<LONGLONG>(value * 10000000);
        InitPropVariantFromInt64(pos, &variant);
        COMInline::wrapCall(m_reader->SetCurrentPosition(GUID_NULL, variant), error);
        PropVariantClear(&variant);
        decodeSampleBuffer(pos, error);
    }
    else if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->seek(value);
    }
}

void
D3D11BackgroundVideoDrawer::flush()
{
    if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->flush();
    }
}

void
D3D11BackgroundVideoDrawer::decodeSampleBuffer(LONGLONG seek, Error &error)
{
    IMFSample *inputSample = nullptr;
    DWORD videoStreamFlags;
    LONGLONG timestamp = 0;
    while (timestamp < seek && !error.hasReason()) {
        COMInline::safeRelease(inputSample);
        COMInline::wrapCall(m_reader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &videoStreamFlags,
                                &timestamp, &inputSample),
            error);
    }
#if 1
    if (inputSample) {
        IMFMediaBuffer *outputBuffer = nullptr;
        inputSample->ConvertToContiguousBuffer(&outputBuffer);
        COMInline::safeRelease(inputSample);
        updateTexture(outputBuffer, error);
        COMInline::safeRelease(outputBuffer);
    }
#else
    m_transform->ProcessInput(0, inputSample, 0);
    COMInline::safeRelease(inputSample);
    IMFMediaBuffer *outputBuffer = nullptr;
    COMInline::wrapCall(
        MFCreate2DMediaBuffer(m_description.width, m_description.height, 21, false, &outputBuffer), error);
    MFT_OUTPUT_DATA_BUFFER output = {};
    COMInline::wrapCall(MFCreateSample(&outputSample), error);
    COMInline::wrapCall(outputSample->AddBuffer(outputBuffer), error);
    output.pSample = outputSample;
    m_transform->ProcessOutput(0, 1, &output, &status);
    updateTexture(outputBuffer, error);
    COMInline::safeRelease(outputSample);
    COMInline::safeRelease(outputBuffer);
#endif
}

void
D3D11BackgroundVideoDrawer::updateTexture(IMFMediaBuffer *outputBuffer, Error &error)
{
    IMF2DBuffer *buffer2D = nullptr;
    COMInline::wrapCall(outputBuffer->QueryInterface(IID_PPV_ARGS(&buffer2D)), error);
    if (buffer2D && !error.hasReason()) {
        BYTE *bytes = nullptr;
        LONG stride;
        COMInline::wrapCall(buffer2D->Lock2D(&bytes, &stride), error);
        sg_image_data content;
        Inline::clearZeroMemory(content);
        content.subimage[0][0].ptr = bytes;
        content.subimage[0][0].size = stride * m_description.height;
        sg::update_image(m_image, &content);
        COMInline::wrapCall(buffer2D->Unlock2D(), error);
        COMInline::safeRelease(buffer2D);
    }
}

void
D3D11BackgroundVideoDrawer::destroy()
{
    if (m_decoderPluginBasedBackgroundVideoRenderer) {
        m_decoderPluginBasedBackgroundVideoRenderer->destroy();
        nanoem_delete_safe(m_decoderPluginBasedBackgroundVideoRenderer);
    }
    COMInline::safeRelease(m_reader);
    sg::destroy_image(m_image);
}

URI
D3D11BackgroundVideoDrawer::fileURI() const noexcept
{
    return m_decoderPluginBasedBackgroundVideoRenderer ? m_decoderPluginBasedBackgroundVideoRenderer->fileURI()
                                                       : m_fileURI;
}

} /* namespace win32 */
} /* namespace nanoem */

#endif /* WINVER >= _WIN32_WINNT_WIN7 */
