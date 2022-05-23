/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#define NOMINMAX
#include <algorithm>
#include <vector>

#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Encoder.h"
#include "emapp/sdk/UI.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/timestamp.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

#include "emapp/src/protoc/common.pb-c.c"
#include "emapp/src/protoc/plugin.pb-c.c"

namespace {

using namespace nanoem::application::plugin;

static void
handleFFmpegLog(void *ptr, int level, const char *fmt, va_list vl)
{
#if !defined(NDEBUG)
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, vl);
#if defined(_WIN32)
    OutputDebugStringA(buffer);
#else
    (void) ptr;
    (void) level;
    fputs(buffer, stderr);
#endif
#else
    av_log_default_callback(ptr, level, fmt, vl);
#endif
}

struct FFmpegEncoder {
    static const char kAudioCodecComponentID[];
    static const char kVideoCodecComponentID[];
    static const char kVideoPixelFormatComponentID[];
    static const int kMinimumSampleRate;
    static const int kMinimumNumChannels;

    FFmpegEncoder()
        : m_formatContext(nullptr)
        , m_audioStream(nullptr)
        , m_videoStream(nullptr)
        , m_audioCodecContext(nullptr)
        , m_videoCodecContext(nullptr)
        , m_resampleContext(nullptr)
        , m_scaleContext(nullptr)
        , m_tempAudioBuffer(nullptr)
        , m_audioCodecID(AV_CODEC_ID_PCM_S16LE)
        , m_videoCodecID(AV_CODEC_ID_RAWVIDEO)
        , m_videoPixelFormat(AV_PIX_FMT_BGR24)
        , m_fps(0)
        , m_duration(0)
        , m_numFrequency(0)
        , m_numChannels(0)
        , m_numBits(0)
        , m_width(0)
        , m_height(0)
        , m_yflip(0)
        , m_nextAudioPTS(0)
    {
        *m_reason = 0;
    }
    ~FFmpegEncoder()
    {
        clearUIWindowLayout();
    }

    int
    openAudioCodec()
    {
        const AVCodec *codec = avcodec_find_encoder(m_audioCodecID);
        if ((m_audioStream = avformat_new_stream(m_formatContext, codec)) != nullptr) {
            m_audioCodecContext = avcodec_alloc_context3(codec);
            m_audioCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
            m_audioCodecContext->sample_rate = std::max(m_numFrequency, kMinimumSampleRate);
            m_audioCodecContext->channels = std::max(m_numChannels, kMinimumNumChannels);
            m_audioCodecContext->channel_layout = av_get_default_channel_layout(m_audioCodecContext->channels);
            m_audioCodecContext->time_base.num = 1;
            m_audioCodecContext->time_base.den = m_audioCodecContext->sample_rate;
            AVSampleFormat inputSampleFormat;
            switch (m_numBits) {
            case 32: {
                inputSampleFormat = AV_SAMPLE_FMT_S32;
                break;
            }
            case 24: {
                /* needs conversion S24 to FLT */
                inputSampleFormat = AV_SAMPLE_FMT_FLT;
                break;
            }
            case 16: {
                inputSampleFormat = AV_SAMPLE_FMT_S16;
                break;
            }
            case 8: {
                inputSampleFormat = AV_SAMPLE_FMT_U8;
                break;
            }
            default:
                inputSampleFormat = AV_SAMPLE_FMT_S16;
                break;
            }
            if ((m_formatContext->oformat->flags & AVFMT_GLOBALHEADER) != 0) {
                m_audioCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            m_resampleContext = swr_alloc_set_opts(nullptr, m_audioCodecContext->channel_layout,
                m_audioCodecContext->sample_fmt, m_audioCodecContext->sample_rate,
                av_get_default_channel_layout(m_numChannels), inputSampleFormat, m_numFrequency, 0, nullptr);
            swr_init(m_resampleContext);
        }
        int rc = avcodec_open2(m_audioCodecContext, codec, nullptr);
        if (rc == 0 && m_audioStream) {
            rc = avcodec_parameters_from_context(m_audioStream->codecpar, m_audioCodecContext);
        }
        return rc;
    }
    int
    openVideoCodec()
    {
        const AVCodec *codec = avcodec_find_encoder(m_videoCodecID);
        if ((m_videoStream = avformat_new_stream(m_formatContext, codec)) != nullptr) {
            m_videoCodecContext = avcodec_alloc_context3(codec);
            m_videoCodecContext->width = static_cast<int>(m_width);
            m_videoCodecContext->height = static_cast<int>(m_height);
            m_videoCodecContext->pix_fmt = m_videoPixelFormat;
            m_videoCodecContext->time_base.num = 1;
            m_videoCodecContext->time_base.den = static_cast<int>(m_fps);
            if ((m_formatContext->oformat->flags & AVFMT_GLOBALHEADER) != 0) {
                m_videoCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            }
            const AVPixelFormat sourcePixelFormat = AV_PIX_FMT_RGBA;
            m_scaleContext = sws_getContext(m_width, m_height, sourcePixelFormat, m_width, m_height,
                m_videoCodecContext->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
        }
        int rc = avcodec_open2(m_videoCodecContext, codec, nullptr);
        if (rc == 0 && m_videoStream) {
            rc = avcodec_parameters_from_context(m_videoStream->codecpar, m_videoCodecContext);
        }
        return rc;
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        bool succeeded = true;
        avformat_alloc_output_context2(&m_formatContext, 0, 0, filePath);
        if (m_numChannels > 0 && m_numFrequency > 0) {
            succeeded = wrapCall(openAudioCodec(), status);
        }
        succeeded = succeeded && wrapCall(openVideoCodec(), status) &&
            wrapCall(avio_open(&m_formatContext->pb, filePath, AVIO_FLAG_WRITE), status);
        if (succeeded && !wrapCall(avformat_write_header(m_formatContext, nullptr), status)) {
            avio_closep(&m_formatContext->pb);
            succeeded = false;
        }
        if (!succeeded) {
            avformat_free_context(m_formatContext);
            m_formatContext = nullptr;
        }
        return succeeded ? 1 : 0;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_fps = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION: {
            if (Inline::validateArgument<nanoem_frame_index_t>(value, size, status)) {
                m_duration = *static_cast<const nanoem_frame_index_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_numFrequency = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_numChannels = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_numBits = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_width = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_height = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_yflip = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    void
    encodeAudioFrame(nanoem_frame_index_t /* currentFrameIndex */, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t *status)
    {
        int inputSampleCount = size / (m_numChannels * (m_numBits / 8)),
            outputSampleCount =
                av_rescale_rnd(inputSampleCount, m_audioCodecContext->sample_rate, m_numFrequency, AV_ROUND_UP);
        ScopedAudioFrame output(m_audioStream->codecpar, outputSampleCount, m_nextAudioPTS);
        if (!wrapCall(av_frame_get_buffer(output, 0), status) || !wrapCall(av_frame_make_writable(output), status)) {
            return;
        }
        const nanoem_u8_t *dataPtr = data;
        if (m_numBits == 24) {
            const nanoem_rsize_t numComponents = size / 3;
            if (!m_tempAudioBuffer) {
                m_tempAudioBuffer = new float[numComponents];
            }
            for (nanoem_rsize_t i = 0; i < numComponents; i++) {
                const nanoem_u8_t *ptr = &dataPtr[i * 3];
                int v = 0;
                if ((ptr[2] & 0x80) != 0) {
                    v = (0xff << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
                }
                else {
                    v = (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
                }
                m_tempAudioBuffer[i] = v / 8388607.0f;
            }
            dataPtr = reinterpret_cast<const nanoem_u8_t *>(m_tempAudioBuffer);
        }
        if (wrapCall(swr_convert(
                         m_resampleContext, &output.m_opaque->data[0], outputSampleCount, &dataPtr, inputSampleCount),
                status) &&
            wrapCall(avcodec_send_frame(m_audioCodecContext, output), status)) {
            AVPacket packet = {};
            av_init_packet(&packet);
            if (wrapCall(avcodec_receive_packet(m_audioCodecContext, &packet), status)) {
                av_packet_rescale_ts(&packet, m_audioCodecContext->time_base, m_audioStream->time_base);
                packet.stream_index = m_audioStream->index;
                makeFailureReason(av_interleaved_write_frame(m_formatContext, &packet), status);
                if (status && *status == NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS) {
                    m_nextAudioPTS += outputSampleCount;
                }
            }
        }
    }
    void
    encodeVideoFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t *status)
    {
        const AVCodecParameters *parameters = m_videoStream->codecpar;
        ScopedVideoFrame frame(parameters, currentFrameIndex);
        if (!wrapCall(av_frame_get_buffer(frame, 0), status) || !wrapCall(av_frame_make_writable(frame), status)) {
            return;
        }
        const nanoem_u8_t *dataPtr[] = { 0, 0, 0, 0 };
        int lineSizePtr[] = { 0, 0, 0, 0 }, stride = size / m_height;
        if (m_yflip) {
            const nanoem_u8_t *ptr = data + stride * (m_height - 1);
            lineSizePtr[0] = -stride;
            dataPtr[0] = ptr;
        }
        else {
            lineSizePtr[0] = stride;
            dataPtr[0] = data;
        }
        sws_scale(m_scaleContext, dataPtr, lineSizePtr, 0, m_height, frame.m_opaque->data, frame.m_opaque->linesize);
        if (wrapCall(avcodec_send_frame(m_videoCodecContext, frame), status)) {
            AVPacket packet = {};
            av_init_packet(&packet);
            if (wrapCall(avcodec_receive_packet(m_videoCodecContext, &packet), status)) {
                av_packet_rescale_ts(&packet, m_videoCodecContext->time_base, m_videoStream->time_base);
                packet.stream_index = m_videoStream->index;
                makeFailureReason(av_interleaved_write_frame(m_formatContext, &packet), status);
            }
        }
    }
    int
    interrupt(nanoem_application_plugin_status_t *status)
    {
        return close(status);
    }
    int
    close(nanoem_application_plugin_status_t *status)
    {
        if (m_formatContext) {
            makeFailureReason(av_write_trailer(m_formatContext), status);
        }
        if (m_audioCodecContext) {
            makeFailureReason(avcodec_close(m_audioCodecContext), status);
            avcodec_free_context(&m_audioCodecContext);
        }
        if (m_videoCodecContext) {
            makeFailureReason(avcodec_close(m_videoCodecContext), status);
            avcodec_free_context(&m_videoCodecContext);
        }
        if (m_resampleContext) {
            swr_close(m_resampleContext);
            swr_free(&m_resampleContext);
        }
        if (m_scaleContext) {
            sws_freeContext(m_scaleContext);
            m_scaleContext = nullptr;
        }
        if (m_tempAudioBuffer) {
            delete[] m_tempAudioBuffer;
            m_tempAudioBuffer = nullptr;
        }
        if (m_formatContext) {
            makeFailureReason(avio_closep(&m_formatContext->pb), status);
            avformat_free_context(m_formatContext);
            m_formatContext = nullptr;
        }
        return 1;
    }
    void
    loadUIWindowLayout(nanoem_application_plugin_status_t *status)
    {
        const char *audioCodecs[] = { "PCM" };
        const char *videoCodecs[] = { "Raw Video", "UT Codec Video" };
        const char *videoPixelFormats[] = {
            "RGB",
            "RGBA",
            "YUV420P",
            "YUV422P",
            "YUV444P",
        };
        clearUIWindowLayout();
        const nanoem_u32_t numVideoCodecs = sizeof(videoCodecs) / sizeof(videoCodecs[0]);
        m_components.push_back(createLabel("Video Codec"));
        m_components.push_back(createCombobox(kVideoCodecComponentID, videoCodecs, numVideoCodecs, 0));
        const nanoem_u32_t numVideoPixelFormats = sizeof(videoPixelFormats) / sizeof(videoPixelFormats[0]);
        m_components.push_back(createLabel("Video Pixel Format"));
        m_components.push_back(
            createCombobox(kVideoPixelFormatComponentID, videoPixelFormats, numVideoPixelFormats, 0));
        const nanoem_u32_t numAudioCodecs = sizeof(audioCodecs) / sizeof(audioCodecs[0]);
        m_components.push_back(createLabel("Audio Codec"));
        m_components.push_back(createCombobox(kAudioCodecComponentID, audioCodecs, numAudioCodecs, 0));
        nanoem_application_plugin_status_assign_success(status);
    }
    void
    clearUIWindowLayout()
    {
        for (auto component : m_components) {
            destroyComponent(component);
        }
        m_components.clear();
    }
    nanoem_u32_t
    getUIWindowLayoutDataSize() const
    {
        Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
        window.n_items = m_components.size();
        window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(m_components.data());
        return nanoem__application__plugin__uiwindow__get_packed_size(&window);
    }
    void
    getUIWindowLayoutData(nanoem_u8_t *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
    {
        Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
        window.n_items = m_components.size();
        window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(m_components.data());
        if (nanoem__application__plugin__uiwindow__get_packed_size(&window) <= length) {
            nanoem__application__plugin__uiwindow__pack(&window, data);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    void
    setUIComponentData(const char *id, const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout)
    {
        nanoem_mark_unused(id);
        nanoem_mark_unused(reloadLayout);
        if (Nanoem__Application__Plugin__UIComponent *component =
                nanoem__application__plugin__uicomponent__unpack(nullptr, length, data)) {
            if (StringUtils::equals(id, kAudioCodecComponentID)) {
                switch (component->combo_box->selected_index) {
                case 0:
                    m_audioCodecID = AV_CODEC_ID_PCM_S16LE;
                    break;
                }
            }
            else if (StringUtils::equals(id, kVideoCodecComponentID)) {
                switch (component->combo_box->selected_index) {
                case 0:
                    m_videoPixelFormat = AV_PIX_FMT_BGR24;
                    m_videoCodecID = AV_CODEC_ID_RAWVIDEO;
                    break;
                case 1:
                    m_videoPixelFormat = AV_PIX_FMT_GBRP;
                    m_videoCodecID = AV_CODEC_ID_UTVIDEO;
                    break;
                }
            }
            else if (StringUtils::equals(id, kVideoPixelFormatComponentID)) {
                switch (component->combo_box->selected_index) {
                case 0:
                    m_videoPixelFormat = m_videoCodecID == AV_CODEC_ID_RAWVIDEO ? AV_PIX_FMT_BGR24 : AV_PIX_FMT_GBRP;
                    break;
                case 1:
                    m_videoPixelFormat = m_videoCodecID == AV_CODEC_ID_RAWVIDEO ? AV_PIX_FMT_BGRA : AV_PIX_FMT_GBRAP;
                    break;
                case 2:
                    m_videoPixelFormat = AV_PIX_FMT_YUV420P;
                    break;
                case 3:
                    m_videoPixelFormat = AV_PIX_FMT_YUV422P;
                    break;
                case 4:
                    m_videoPixelFormat = AV_PIX_FMT_YUV444P;
                    break;
                }
            }
            nanoem__application__plugin__uicomponent__free_unpacked(component, nullptr);
        }
    }
    const char *
    failureReason() const
    {
        return strlen(m_reason) > 0 ? m_reason : nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }

    int
    makeFailureReason(int rc, nanoem_application_plugin_status_t *status)
    {
        bool result = true;
        if (rc == AVERROR(EAGAIN) || rc == AVERROR_EOF) {
            rc = 0;
        }
        else if (rc < 0) {
            av_make_error_string(m_reason, sizeof(m_reason), rc);
            result = false;
        }
        if (status) {
            *status =
                result ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
        return rc;
    }
    bool
    wrapCall(int rc, nanoem_application_plugin_status_t *status)
    {
        return makeFailureReason(rc, status) >= 0;
    }

    struct ScopedAudioFrame {
        ScopedAudioFrame(const AVCodecParameters *parameters, int numSamplesPerFrame, nanoem_frame_index_t pts)
            : m_opaque(av_frame_alloc())
        {
            m_opaque->format = parameters->format;
            m_opaque->channel_layout = parameters->channel_layout;
            m_opaque->channels = parameters->channels;
            m_opaque->sample_rate = parameters->sample_rate;
            m_opaque->nb_samples = numSamplesPerFrame;
            m_opaque->pts = pts;
        }
        ~ScopedAudioFrame()
        {
            av_frame_free(&m_opaque);
        }

        operator AVFrame *()
        {
            return m_opaque;
        }

        AVFrame *m_opaque;
    };
    struct ScopedVideoFrame {
        ScopedVideoFrame(const AVCodecParameters *parameters, nanoem_frame_index_t pts)
            : m_opaque(av_frame_alloc())
        {
            m_opaque->format = parameters->format;
            m_opaque->width = parameters->width;
            m_opaque->height = parameters->height;
            m_opaque->pts = pts;
        }
        ~ScopedVideoFrame()
        {
            av_frame_free(&m_opaque);
        }

        operator AVFrame *()
        {
            return m_opaque;
        }

        AVFrame *m_opaque;
    };

    typedef std::vector<Nanoem__Application__Plugin__UIComponent *> ComponentList;

    char m_reason[1024];
    ComponentList m_components;
    AVFormatContext *m_formatContext;
    AVStream *m_audioStream;
    AVStream *m_videoStream;
    AVCodecContext *m_audioCodecContext;
    AVCodecContext *m_videoCodecContext;
    SwrContext *m_resampleContext;
    SwsContext *m_scaleContext;
    nanoem_f32_t *m_tempAudioBuffer;
    AVCodecID m_audioCodecID;
    AVCodecID m_videoCodecID;
    AVPixelFormat m_videoPixelFormat;
    nanoem_u32_t m_fps;
    nanoem_u32_t m_duration;
    int m_numFrequency;
    int m_numChannels;
    int m_numBits;
    nanoem_u32_t m_width;
    nanoem_u32_t m_height;
    nanoem_u32_t m_yflip;
    nanoem_i64_t m_nextAudioPTS;
};
const char FFmpegEncoder::kAudioCodecComponentID[] = "ffmpeg.audio-codec";
const char FFmpegEncoder::kVideoCodecComponentID[] = "ffmpeg.video-codec";
const char FFmpegEncoder::kVideoPixelFormatComponentID[] = "ffmpeg.video-pixel-format";
const int FFmpegEncoder::kMinimumSampleRate = 44100;
const int FFmpegEncoder::kMinimumNumChannels = 2;

struct FFmpegDecoder {
    FFmpegDecoder()
        : m_audioFormatContext(nullptr)
        , m_videoFormatContext(nullptr)
        , m_audioCodecContext(nullptr)
        , m_videoCodecContext(nullptr)
        , m_decodedPixelFormat(AV_PIX_FMT_RGBA)
        , m_scaleContext(nullptr)
        , m_fps(0)
        , m_audioStreamIndex(-1)
        , m_videoStreamIndex(-1)
    {
        *m_reason = 0;
    }
    ~FFmpegDecoder()
    {
        nanoem_application_plugin_status_t status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        closeAudio(&status);
        closeVideo(&status);
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        int audioStreamIndex = -1, videoStreamIndex = -1;
        if (m_audioStreamIndex < 0) {
            if (!wrapCall(avformat_open_input(&m_audioFormatContext, filePath, nullptr, nullptr), status)) {
                return 0;
            }
            else if (!wrapCall(avformat_find_stream_info(m_audioFormatContext, nullptr), status)) {
                return 0;
            }
            int rc = av_find_best_stream(m_audioFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
            if (rc >= 0) {
                audioStreamIndex = rc;
                const AVStream *stream = m_audioFormatContext->streams[audioStreamIndex];
                const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
                m_audioCodecContext = avcodec_alloc_context3(codec);
                if (!wrapCall(avcodec_parameters_to_context(m_audioCodecContext, stream->codecpar), status)) {
                    return 0;
                }
                else if (!wrapCall(avcodec_open2(m_audioCodecContext, codec, nullptr), status)) {
                    return 0;
                }
                m_audioStreamIndex = audioStreamIndex;
            }
            else if (rc != AVERROR_STREAM_NOT_FOUND) {
                makeFailureReason(rc, status);
                return 0;
            }
        }
        if (m_videoStreamIndex < 0) {
            if (!wrapCall(avformat_open_input(&m_videoFormatContext, filePath, nullptr, nullptr), status)) {
                return 0;
            }
            else if (!wrapCall(avformat_find_stream_info(m_videoFormatContext, nullptr), status)) {
                return 0;
            }
            int rc = av_find_best_stream(m_videoFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
            if (rc >= 0) {
                videoStreamIndex = rc;
                const AVStream *stream = m_videoFormatContext->streams[videoStreamIndex];
                const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
                m_videoCodecContext = avcodec_alloc_context3(codec);
                if (!wrapCall(avcodec_parameters_to_context(m_videoCodecContext, stream->codecpar), status)) {
                    return 0;
                }
                else if (!wrapCall(avcodec_open2(m_videoCodecContext, codec, nullptr), status)) {
                    return 0;
                }
                int width = m_videoCodecContext->width, height = m_videoCodecContext->height;
                m_scaleContext = sws_getContext(width, height, m_videoCodecContext->pix_fmt, width, height,
                    m_decodedPixelFormat, SWS_BILINEAR, nullptr, nullptr, nullptr);
                m_videoStreamIndex = videoStreamIndex;
            }
            else if (rc != AVERROR_STREAM_NOT_FOUND) {
                makeFailureReason(rc, status);
                return 0;
            }
        }
        return m_audioStreamIndex != -1 || m_videoStreamIndex != -1;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_fps = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_AUDIO_LOCATION: {
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_VIDEO_LOCATION: {
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    void
    audioFormatValue(nanoem_u32_t key, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS: {
            nanoem_u32_t bits = m_audioCodecContext ? nanoem_u32_t(m_audioCodecContext->bits_per_raw_sample) : 0;
            Inline::assignOption(bits, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS: {
            nanoem_u32_t channels = m_audioCodecContext ? nanoem_u32_t(m_audioCodecContext->channels) : 0;
            Inline::assignOption(channels, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY: {
            nanoem_u32_t frequency = m_audioCodecContext ? nanoem_u32_t(m_audioCodecContext->sample_rate) : 0;
            Inline::assignOption(frequency, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION: {
            nanoem_frame_index_t duration =
                static_cast<nanoem_frame_index_t>(av_rescale(m_videoFormatContext->duration, 30, AV_TIME_BASE));
            Inline::assignOption(duration, value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    videoFormatValue(nanoem_u32_t key, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH: {
            nanoem_u32_t width = m_videoCodecContext ? nanoem_u32_t(m_videoCodecContext->width) : 0;
            Inline::assignOption(width, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT: {
            nanoem_u32_t height = m_videoCodecContext ? nanoem_u32_t(m_videoCodecContext->height) : 0;
            Inline::assignOption(height, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_STRIDE: {
            nanoem_u32_t stride = 0;
            Inline::assignOption(stride, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION: {
            nanoem_frame_index_t duration =
                static_cast<nanoem_frame_index_t>(av_rescale(m_videoFormatContext->duration, 30, AV_TIME_BASE));
            Inline::assignOption(duration, value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    decodeAudioFrame(nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size,
        nanoem_application_plugin_status_t *status)
    {
        const AVStream *stream = m_audioFormatContext->streams[m_audioStreamIndex];
        AVFrame *frame = av_frame_alloc();
        int result =
            av_seek_frame(m_audioFormatContext, m_audioStreamIndex, convertTimestamp(currentFrameIndex, stream), 0);
        if (result >= 0 && decodeFrame(m_audioFormatContext, m_audioCodecContext, m_audioStreamIndex, frame, result)) {
            nanoem_u32_t length = nanoem_u32_t((nanoem_u32_t(m_audioCodecContext->sample_rate) / m_fps) *
                nanoem_u32_t(av_get_bytes_per_sample(static_cast<AVSampleFormat>(frame->format))));
            nanoem_u8_t *dataPtr = *data = new nanoem_u8_t[length];
            memcpy(dataPtr, frame->data[0], length);
            *size = length;
        }
        else {
            makeFailureReason(result, status);
            *data = nullptr;
            *size = 0;
        }
        av_frame_free(&frame);
    }
    void
    decodeVideoFrame(nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size,
        nanoem_application_plugin_status_t *status)
    {
        const AVStream *stream = m_videoFormatContext->streams[m_videoStreamIndex];
        AVFrame *frame = av_frame_alloc();
        int result =
            av_seek_frame(m_videoFormatContext, m_videoStreamIndex, convertTimestamp(currentFrameIndex, stream), 0);
        if (result >= 0 && decodeFrame(m_videoFormatContext, m_videoCodecContext, m_videoStreamIndex, frame, result)) {
            nanoem_u8_t *frameData[4];
            int frameLineSize[4], height = frame->height;
            makeFailureReason(
                av_image_alloc(frameData, frameLineSize, frame->width, height, m_decodedPixelFormat, 1), status);
            makeFailureReason(
                sws_scale(m_scaleContext, frame->data, frame->linesize, 0, height, frameData, frameLineSize), status);
            nanoem_u32_t length = frameLineSize[0] * height;
            nanoem_u8_t *dataPtr = *data = new nanoem_u8_t[length];
            memcpy(dataPtr, frameData[0], length);
            av_freep(&frameData[0]);
            *size = length;
        }
        else {
            makeFailureReason(result, status);
            *data = nullptr;
            *size = 0;
        }
        av_frame_free(&frame);
    }
    void
    destroyAudioFrame(nanoem_frame_index_t /* currentFrameIndex */, nanoem_u8_t *data, nanoem_u32_t /* size */)
    {
        delete[] data;
    }
    void
    destroyVideoFrame(nanoem_frame_index_t /* currentFrameIndex */, nanoem_u8_t *data, nanoem_u32_t /* size */)
    {
        delete[] data;
    }
    const char *
    failureReason() const
    {
        return strlen(m_reason) > 0 ? m_reason : nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }
    int
    close(nanoem_application_plugin_status_t *status)
    {
        closeAudio(status);
        closeVideo(status);
        m_scaleContext = nullptr;
        return 1;
    }

    bool
    decodeFrame(AVFormatContext *format, AVCodecContext *codec, int streamIndex, AVFrame *&frame, int &result)
    {
        AVPacket packet = {};
        av_init_packet(&packet);
        while (true) {
            result = av_read_frame(format, &packet);
            if (result != 0) {
                break;
            }
            else if (packet.stream_index != streamIndex) {
                continue;
            }
            result = avcodec_send_packet(codec, &packet);
            if (result != 0 && result != AVERROR(EAGAIN)) {
                break;
            }
            av_packet_unref(&packet);
            result = avcodec_receive_frame(codec, frame);
            if (result == 0 || result != AVERROR(EAGAIN)) {
                break;
            }
        }
        return result == 0;
    }
    nanoem_i64_t
    convertTimestamp(nanoem_frame_index_t currentFrameIndex, const AVStream *stream) const
    {
        const AVRational r = { 1, static_cast<int>(m_fps) };
        return av_rescale_q(currentFrameIndex, r, stream->time_base);
    }
    void
    closeAudio(nanoem_application_plugin_status_t *status)
    {
        if (m_audioCodecContext && m_audioStreamIndex >= 0) {
            makeFailureReason(avcodec_close(m_audioCodecContext), status);
            m_audioStreamIndex = -1;
        }
        avformat_close_input(&m_audioFormatContext);
    }
    void
    closeVideo(nanoem_application_plugin_status_t *status)
    {
        if (m_videoCodecContext && m_videoStreamIndex >= 0) {
            makeFailureReason(avcodec_close(m_videoCodecContext), status);
            m_videoStreamIndex = -1;
        }
        avformat_close_input(&m_videoFormatContext);
        if (m_scaleContext) {
            sws_freeContext(m_scaleContext);
            m_scaleContext = nullptr;
        }
    }
    int
    makeFailureReason(int rc, nanoem_application_plugin_status_t *status)
    {
        bool result = true;
        if (rc == AVERROR(EAGAIN) || rc == AVERROR_EOF) {
            rc = 0;
        }
        else if (rc < 0) {
            av_make_error_string(m_reason, sizeof(m_reason), rc);
            result = false;
        }
        if (status) {
            *status =
                result ? NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS : NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
        }
        return rc;
    }
    bool
    wrapCall(int rc, nanoem_application_plugin_status_t *status)
    {
        return makeFailureReason(rc, status) >= 0;
    }

    char m_reason[1024];
    AVFormatContext *m_audioFormatContext;
    AVFormatContext *m_videoFormatContext;
    AVCodecContext *m_audioCodecContext;
    AVCodecContext *m_videoCodecContext;
    AVPixelFormat m_decodedPixelFormat;
    SwsContext *m_scaleContext;
    nanoem_u32_t m_fps;
    int m_audioStreamIndex;
    int m_videoStreamIndex;
};

} /* namespace anonymous */

struct nanoem_application_plugin_encoder_t : FFmpegEncoder { };

struct nanoem_application_plugin_decoder_t : FFmpegDecoder { };

nanoem_u32_t
nanoemApplicationPluginEncoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginEncoderInitialize()
{
    av_log_set_callback(handleFFmpegLog);
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    return new nanoem_application_plugin_encoder_t();
}

int APIENTRY
nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->open(filePath, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderSetOption(nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t key,
    const void *value, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->setOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeAudioFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeAudioFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeVideoFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeVideoFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderInterrupt(nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(encoder)) {
        encoder->interrupt(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t * /* encoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "avi" /* , "mkv" */ };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

void APIENTRY
nanoemApplicationPluginEncoderLoadUIWindowLayout(nanoem_application_plugin_encoder_t *plugin, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_likely(plugin)) {
        plugin->loadUIWindowLayout(statusPtr);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize(
    nanoem_application_plugin_encoder_t *plugin, nanoem_u32_t *length)
{
    if (nanoem_likely(plugin && length)) {
        *length = plugin->getUIWindowLayoutDataSize();
    }
}

void APIENTRY
nanoemApplicationPluginEncoderGetUIWindowLayoutData(
    nanoem_application_plugin_encoder_t *plugin, nanoem_u8_t *data, nanoem_u32_t length, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_likely(plugin && data)) {
        plugin->getUIWindowLayoutData(data, length, statusPtr);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginEncoderSetUIComponentLayoutData(nanoem_application_plugin_encoder_t *plugin, const char *id,
    const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_likely(plugin && id && data && reloadLayout)) {
        plugin->setUIComponentData(id, data, length, reloadLayout);
    }
    else if (status) {
        *statusPtr = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetFailureReason(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetRecoverySuggestion(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginEncoderClose(nanoem_application_plugin_encoder_t *encoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->close(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder)
{
    delete encoder;
}

void APIENTRY
nanoemApplicationPluginEncoderTerminate()
{
}

nanoem_u32_t
nanoemApplicationPluginDecoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginDecoderInitialize()
{
    av_log_set_callback(handleFFmpegLog);
}

nanoem_application_plugin_decoder_t *APIENTRY
nanoemApplicationPluginDecoderCreate()
{
    return new nanoem_application_plugin_decoder_t();
}

int APIENTRY
nanoemApplicationPluginDecoderOpen(
    nanoem_application_plugin_decoder_t *decoder, const char *filePath, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(decoder)) {
        result = decoder->open(filePath, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginDecoderSetOption(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    const void *value, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->setOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderGetAudioFormatValue(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    void *value, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->audioFormatValue(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderGetVideoFormatValue(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    void *value, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->videoFormatValue(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDecodeAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->decodeAudioFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDecodeVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(decoder)) {
        decoder->decodeVideoFrame(currentFrameIndex, data, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data, nanoem_u32_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyAudioFrame(currentFrameIndex, data, size);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data, nanoem_u32_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyVideoFrame(currentFrameIndex, data, size);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions(
    nanoem_application_plugin_decoder_t * /* decoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "alac", "flac" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions(
    nanoem_application_plugin_decoder_t * /* decoder */, nanoem_u32_t *length)
{
    static const char *kFormatExtensions[] = { "avi", "mkv" };
    *length = sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]);
    return kFormatExtensions;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetFailureReason(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetRecoverySuggestion(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginDecoderClose(nanoem_application_plugin_decoder_t *decoder, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    int result = 0;
    if (nanoem_is_not_null(decoder)) {
        result = decoder->close(statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginDecoderDestroy(nanoem_application_plugin_decoder_t *decoder)
{
    delete decoder;
}

void APIENTRY
nanoemApplicationPluginDecoderTerminate()
{
}
