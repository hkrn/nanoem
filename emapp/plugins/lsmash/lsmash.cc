/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/sdk/Encoder.h"
#include "emapp/sdk/UI.h"

#include <stdio.h>
#include <string.h>
#include <vector>

#include "lsmash.h"
extern "C" {
#include "thread.h"
}

#include "emapp/src/protoc/common.pb-c.c"
#include "emapp/src/protoc/plugin.pb-c.c"

namespace {

using namespace nanoem::application::plugin;

class LSmashEncodeWorker {
public:
    LSmashEncodeWorker(lsmash_root_t *root, nanoem_u32_t audioTrackID, nanoem_u32_t videoTrackID,
        nanoem_u32_t audioSummaryIndex, nanoem_u32_t videoSummaryIndex)
        : m_thread(nullptr)
        , m_root(root)
        , m_audioTrackID(audioTrackID)
        , m_videoTrackID(videoTrackID)
        , m_audioSummaryIndex(audioSummaryIndex)
        , m_videoSummaryIndex(videoSummaryIndex)
        , m_running(true)
    {
        m_thread = thread_create(threadEntryPoint, this, "LSmashEncodeWorker", THREAD_STACK_SIZE_DEFAULT);
        thread_mutex_init(&m_mutex);
        thread_signal_init(&m_signal);
    }
    ~LSmashEncodeWorker()
    {
        thread_signal_term(&m_signal);
        thread_mutex_term(&m_mutex);
        thread_destroy(m_thread);
    }

    void
    sendAudioPacket(const nanoem_frame_index_t frameIndex, const nanoem_u8_t *data, nanoem_u32_t length,
        nanoem_u32_t samplesPerFrame)
    {
        Packet packet;
        packet.m_type = kPacketTypeAudio;
        packet.m_currentFrameIndex = frameIndex;
        packet.m_data = new uint8_t[length];
        packet.u.audio.m_length = length;
        packet.u.audio.m_samplesInFrame = samplesPerFrame;
        memcpy(packet.m_data, data, length);
        thread_mutex_lock(&m_mutex);
        m_packets.push_back(packet);
        thread_mutex_unlock(&m_mutex);
    }
    void
    sendVideoPacket(const nanoem_frame_index_t frameIndex, const nanoem_u8_t *data, nanoem_rsize_t length,
        nanoem_u32_t width, nanoem_u32_t height)
    {
        Packet packet;
        packet.m_type = kPacketTypeVideo;
        packet.m_currentFrameIndex = frameIndex;
        packet.m_data = new uint8_t[length];
        packet.u.video.m_width = width;
        packet.u.video.m_height = height;
        memcpy(packet.m_data, data, length);
        thread_mutex_lock(&m_mutex);
        m_packets.push_back(packet);
        thread_mutex_unlock(&m_mutex);
    }
    void
    waitForCompletion()
    {
        if (m_running) {
            Packet packet;
            packet.m_type = kPacketTypeFinish;
            packet.m_currentFrameIndex = 0;
            packet.m_data = nullptr;
            thread_mutex_lock(&m_mutex);
            m_packets.push_back(packet);
            thread_mutex_unlock(&m_mutex);
            thread_signal_wait(&m_signal, THREAD_SIGNAL_WAIT_INFINITE);
        }
    }

private:
    enum PacketType {
        kPacketTypeFirstEnum,
        kPacketTypeAudio = kPacketTypeFirstEnum,
        kPacketTypeVideo,
        kPacketTypeFinish,
        kPacketTypeMaxEnum,
    };
    struct Packet {
        PacketType m_type;
        nanoem_frame_index_t m_currentFrameIndex;
        nanoem_u8_t *m_data;
        union {
            struct {
                nanoem_u32_t m_length;
                nanoem_u32_t m_samplesInFrame;
            } audio;
            struct {
                nanoem_u32_t m_width;
                nanoem_u32_t m_height;
            } video;
        } u;
    };

    static int
    threadEntryPoint(void *userData)
    {
        LSmashEncodeWorker *worker = static_cast<LSmashEncodeWorker *>(userData);
        worker->execute();
        return 0;
    }

    void
    execute()
    {
        while (m_running) {
            thread_mutex_lock(&m_mutex);
            if (!m_packets.empty()) {
                Packet packet = m_packets.front();
                m_packets.erase(m_packets.begin());
                thread_mutex_unlock(&m_mutex);
                switch (packet.m_type) {
                case kPacketTypeAudio: {
                    internalEncodeAudio(&packet);
                    delete[] packet.m_data;
                    break;
                }
                case kPacketTypeVideo: {
                    internalEncodeVideo(&packet);
                    delete[] packet.m_data;
                    break;
                }
                case kPacketTypeFinish: {
                    internalFinishMovie();
                    m_running = false;
                    break;
                }
                default:
                    break;
                }
            }
            else {
                thread_mutex_unlock(&m_mutex);
            }
        }
    }
    void
    internalEncodeAudio(const Packet *packet)
    {
        const nanoem_u8_t *data = packet->m_data;
        nanoem_rsize_t size = packet->u.audio.m_length;
        lsmash_sample_t *sample = lsmash_create_sample(size);
        lsmash_sample_alloc(sample, size);
        memcpy(sample->data, data, size);
        sample->cts = sample->dts =
            static_cast<nanoem_u64_t>(packet->m_currentFrameIndex) * packet->u.audio.m_samplesInFrame;
        sample->index = m_audioSummaryIndex;
        lsmash_append_sample(m_root, m_audioTrackID, sample);
    }
    void
    internalEncodeVideo(const Packet *packet)
    {
        const nanoem_u8_t *data = packet->m_data;
        nanoem_rsize_t length = packet->u.video.m_width * packet->u.video.m_height;
        const nanoem_rsize_t actualSize = length * 3;
        lsmash_sample_t *sample = lsmash_create_sample(actualSize);
        lsmash_sample_alloc(sample, actualSize);
        for (nanoem_rsize_t i = 0; i < length; i++) {
            const nanoem_u8_t *src = data + i * 4;
            nanoem_u8_t *dst = sample->data + i * 3;
            memcpy(dst, src, sizeof(*dst) * 3);
        }
        sample->cts = sample->dts = packet->m_currentFrameIndex;
        sample->index = m_videoSummaryIndex;
        lsmash_append_sample(m_root, m_videoTrackID, sample);
    }
    void
    internalFinishMovie()
    {
        lsmash_flush_pooled_samples(m_root, m_videoTrackID, 0);
        if (m_audioTrackID > 0) {
            lsmash_flush_pooled_samples(m_root, m_audioTrackID, 0);
        }
        lsmash_finish_movie(m_root, NULL);
        thread_signal_raise(&m_signal);
    }

    std::vector<Packet> m_packets;
    thread_ptr_t m_thread;
    thread_mutex_t m_mutex;
    thread_signal_t m_signal;
    lsmash_root_t *m_root;
    nanoem_u32_t m_audioTrackID;
    nanoem_u32_t m_videoTrackID;
    nanoem_u32_t m_audioSummaryIndex;
    nanoem_u32_t m_videoSummaryIndex;
    volatile bool m_running;
};

struct LSmashEncoder {
    LSmashEncoder()
        : m_file(nullptr)
        , m_root(nullptr)
        , m_audioSummary(nullptr)
        , m_videoSummary(nullptr)
        , m_duration(0)
        , m_format(0)
        , m_yflip(0)
    {
        memset(m_error, 0, sizeof(m_error));
        m_brands[0] = ISOM_BRAND_TYPE_QT;
        m_brands[1] = ISOM_BRAND_TYPE_ISOM;
        initializeFileParameters();
        initializeAudioSummary();
        initializeVideoSummary();
        lsmash_initialize_movie_parameters(&m_movieParameters);
        lsmash_initialize_track_parameters(&m_audioTrackParameters);
        lsmash_initialize_track_parameters(&m_videoTrackParameters);
        lsmash_initialize_media_parameters(&m_audioMediaParameters);
        lsmash_initialize_media_parameters(&m_videoMediaParameters);
        m_movieParameters.timescale = 30;
        const nanoem_u32_t audioTrackFlags = ISOM_TRACK_ENABLED;
        m_audioTrackParameters.mode = static_cast<lsmash_track_mode>(audioTrackFlags);
        const nanoem_u32_t videoTrackFlags = ISOM_TRACK_ENABLED | ISOM_TRACK_IN_MOVIE;
        m_videoTrackParameters.mode = static_cast<lsmash_track_mode>(videoTrackFlags);
        m_audioMediaParameters.compact_sample_size_table = m_videoMediaParameters.compact_sample_size_table = 1;
        m_audioMediaParameters.no_sample_dependency_table = m_videoMediaParameters.no_sample_dependency_table = 1;
        m_audioMediaParameters.ISO_language = m_videoMediaParameters.ISO_language = ISOM_LANGUAGE_CODE_UNDEFINED;
    }
    ~LSmashEncoder()
    {
        if (m_audioSummary) {
            lsmash_cleanup_summary(reinterpret_cast<lsmash_summary_t *>(m_audioSummary));
            m_audioSummary = nullptr;
        }
        if (m_videoSummary) {
            lsmash_cleanup_summary(reinterpret_cast<lsmash_summary_t *>(m_videoSummary));
            m_videoSummary = nullptr;
        }
        if (m_root) {
            lsmash_destroy_root(m_root);
            m_root = nullptr;
        }
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        int result = 0;
        if (!m_root) {
            result = lsmash_open_file(filePath, 0, &m_fileParameters);
            if (result == 0) {
                const lsmash_crop_t crop = { { 0, 1 }, { 0, 1 }, { 0, 1 }, { 0, 1 } };
                lsmash_convert_crop_into_clap(
                    crop, m_videoSummary->width, m_videoSummary->height, &m_videoSummary->clap);
                m_root = lsmash_create_root();
                m_file = lsmash_set_file(m_root, &m_fileParameters);
                nanoem_u32_t audioTrackID = 0, audioSummaryIndex = 0;
                if (m_audioSummary->frequency > 0 && m_audioSummary->channels > 0 && m_audioSummary->sample_size > 0) {
                    m_audioSummary->samples_in_frame = m_audioSummary->frequency / m_movieParameters.timescale;
                    m_audioMediaParameters.timescale = m_audioSummary->frequency;
                    audioTrackID = lsmash_create_track(m_root, ISOM_MEDIA_HANDLER_TYPE_AUDIO_TRACK);
                    audioSummaryIndex = lsmash_add_sample_entry(m_root, audioTrackID, m_audioSummary);
                    lsmash_set_track_parameters(m_root, audioTrackID, &m_audioTrackParameters);
                    lsmash_set_media_parameters(m_root, audioTrackID, &m_audioMediaParameters);
                }
                m_videoMediaParameters.timescale = m_movieParameters.timescale;
                const nanoem_u32_t videoTrackID = lsmash_create_track(m_root, ISOM_MEDIA_HANDLER_TYPE_VIDEO_TRACK),
                                   videoSummaryIndex = lsmash_add_sample_entry(m_root, videoTrackID, m_videoSummary);
                lsmash_set_movie_parameters(m_root, &m_movieParameters);
                lsmash_set_track_parameters(m_root, videoTrackID, &m_videoTrackParameters);
                lsmash_set_media_parameters(m_root, videoTrackID, &m_videoMediaParameters);
                m_worker =
                    new LSmashEncodeWorker(m_root, audioTrackID, videoTrackID, audioSummaryIndex, videoSummaryIndex);
            }
        }
        handleStatusCode(result, status);
        return result == 0;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_movieParameters.timescale = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION: {
            if (Inline::validateArgument<nanoem_frame_index_t>(value, size, status)) {
                m_duration = *static_cast<const nanoem_frame_index_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_audioSummary->frequency = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_audioSummary->channels = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_audioSummary->sample_size = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_videoSummary->width = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_videoSummary->height = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_yflip = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HDR_NUM_BITS:
            /* do nothing */
            break;
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    void
    encodeAudioFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t *status)
    {
        m_worker->sendAudioPacket(currentFrameIndex, data, size, m_audioSummary->samples_in_frame);
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    }
    void
    encodeVideoFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_u32_t size,
        nanoem_application_plugin_status_t *status)
    {
        const nanoem_u32_t width = m_videoSummary->width, height = m_videoSummary->height, length = width * height;
        if (size / 4 == length) {
            m_worker->sendVideoPacket(currentFrameIndex, data, size, m_videoSummary->width, m_videoSummary->height);
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        }
        else {
            *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
        }
    }
    int
    interrupt(nanoem_application_plugin_status_t *status)
    {
        return close(status);
    }
    const char *
    failureReason() const
    {
        return m_error[0] ? m_error : nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }
    int
    close(nanoem_application_plugin_status_t *status)
    {
        int result = 0;
        if (m_root) {
            if (m_worker) {
                m_worker->waitForCompletion();
                delete m_worker;
                m_worker = nullptr;
            }
            result = lsmash_close_file(&m_fileParameters);
            handleStatusCode(result, status);
            lsmash_cleanup_summary(reinterpret_cast<lsmash_summary_t *>(m_audioSummary));
            lsmash_cleanup_summary(reinterpret_cast<lsmash_summary_t *>(m_videoSummary));
            initializeAudioSummary();
            initializeVideoSummary();
            lsmash_destroy_root(m_root);
            m_root = nullptr;
        }
        return result == 0;
    }
    void
    loadUIWindowLayout(nanoem_application_plugin_status_t *status)
    {
        static const char kByteUnits[] = { 'B', 'K', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y' };
        char buffer[128];
        clearUIWindowLayout();
        const nanoem_u64_t duration = m_duration;
        nanoem_u64_t estimatedLength = (m_audioSummary->samples_in_frame * duration) +
            (static_cast<nanoem_u64_t>(m_videoSummary->width) * m_videoSummary->height * 3 * duration *
                m_movieParameters.timescale / 30);
        nanoem_u64_t scaledLength = estimatedLength;
        nanoem_u8_t index = 0;
        nanoem_f64_t actualValue = estimatedLength * 1.05;
        while (scaledLength != (scaledLength & 0x3ff) && index < sizeof(kByteUnits)) {
            scaledLength /= 1000;
            actualValue *= 0.001;
            ++index;
        }
        snprintf(buffer, sizeof(buffer), "Estimated export video size: %0.2f %cB", actualValue, kByteUnits[index]);
        m_components.push_back(createLabel(buffer));
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
    nanoem_rsize_t
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
    setUIComponentData(const char *id, const nanoem_u8_t *data, nanoem_u32_t length)
    {
        nanoem_mark_unused(id);
        nanoem_mark_unused(data);
        nanoem_mark_unused(length);
    }

    void
    initializeFileParameters()
    {
        nanoem_u32_t mode = LSMASH_FILE_MODE_WRITE;
        memset(&m_fileParameters, 0, sizeof(m_fileParameters));
        m_fileParameters.mode = static_cast<lsmash_file_mode>(mode);
        m_fileParameters.major_brand = ISOM_BRAND_TYPE_QT;
        m_fileParameters.brands = m_brands;
        m_fileParameters.brand_count = 2;
        m_fileParameters.opaque = this;
    }
    void
    initializeAudioSummary()
    {
        m_audioSummary = reinterpret_cast<lsmash_audio_summary_t *>(lsmash_create_summary(LSMASH_SUMMARY_TYPE_AUDIO));
        m_audioSummary->sample_type = QT_CODEC_TYPE_LPCM_AUDIO;
        {
            lsmash_codec_specific_t *spec =
                lsmash_create_codec_specific_data(LSMASH_CODEC_SPECIFIC_DATA_TYPE_QT_AUDIO_FORMAT_SPECIFIC_FLAGS,
                    LSMASH_CODEC_SPECIFIC_FORMAT_STRUCTURED);
            lsmash_qt_audio_format_specific_flags_t *format =
                static_cast<lsmash_qt_audio_format_specific_flags_t *>(spec->data.structured);
            nanoem_u32_t flags = QT_AUDIO_FORMAT_FLAG_SIGNED_INTEGER | QT_AUDIO_FORMAT_FLAG_PACKED;
            format->format_flags = static_cast<lsmash_qt_audio_format_specific_flag>(flags);
            lsmash_add_codec_specific_data(reinterpret_cast<lsmash_summary_t *>(m_audioSummary), spec);
            lsmash_destroy_codec_specific_data(spec);
        }
        {
            lsmash_codec_specific_t *spec = lsmash_create_codec_specific_data(
                LSMASH_CODEC_SPECIFIC_DATA_TYPE_QT_AUDIO_CHANNEL_LAYOUT, LSMASH_CODEC_SPECIFIC_FORMAT_STRUCTURED);
            lsmash_qt_audio_channel_layout_t *layout =
                static_cast<lsmash_qt_audio_channel_layout_t *>(spec->data.structured);
            layout->channelLayoutTag = QT_CHANNEL_LAYOUT_STEREO;
            lsmash_add_codec_specific_data(reinterpret_cast<lsmash_summary_t *>(m_audioSummary), spec);
            lsmash_destroy_codec_specific_data(spec);
        }
    }
    void
    initializeVideoSummary()
    {
        m_videoSummary = reinterpret_cast<lsmash_video_summary_t *>(lsmash_create_summary(LSMASH_SUMMARY_TYPE_VIDEO));
        m_videoSummary->sample_type = QT_CODEC_TYPE_RAW_VIDEO;
        m_videoSummary->depth = QT_VIDEO_DEPTH_COLOR_24;
        const char name[sizeof(m_videoSummary->compressorname)] = "nanoem L-Smash plugin";
        m_videoSummary->compressorname[0] = static_cast<nanoem_u8_t>(sizeof(name) - 1);
        memcpy(m_videoSummary->compressorname + 1, name, sizeof(m_videoSummary->compressorname) - 1);
    }
    void
    handleStatusCode(int code, nanoem_application_plugin_status_t *status)
    {
        if (code != 0) {
            switch (code) {
            case LSMASH_ERR_NAMELESS: {
                strncpy(m_error, "LSMASH_ERR_NAMELESS", sizeof(m_error));
                break;
            }
            case LSMASH_ERR_MEMORY_ALLOC: {
                strncpy(m_error, "LSMASH_ERR_MEMORY_ALLOC", sizeof(m_error));
                break;
            }
            case LSMASH_ERR_INVALID_DATA: {
                strncpy(m_error, "LSMASH_ERR_INVALID_DATA", sizeof(m_error));
                break;
            }
            case LSMASH_ERR_FUNCTION_PARAM: {
                strncpy(m_error, "LSMASH_ERR_FUNCTION_PARAM", sizeof(m_error));
                break;
            }
            case LSMASH_ERR_PATCH_WELCOME:
            case LSMASH_ERR_UNKNOWN: {
                strncpy(m_error, "LSMASH_ERR_UNKNOWN", sizeof(m_error));
                break;
            }
            case LSMASH_ERR_IO: {
                strncpy(m_error, "LSMASH_ERR_IO", sizeof(m_error));
                break;
            }
            default:
                break;
            }
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON);
        }
    }

    typedef std::vector<Nanoem__Application__Plugin__UIComponent *> ComponentList;
    ComponentList m_components;
    LSmashEncodeWorker *m_worker;
    lsmash_file_t *m_file;
    lsmash_root_t *m_root;
    lsmash_audio_summary_t *m_audioSummary;
    lsmash_video_summary_t *m_videoSummary;
    lsmash_file_parameters_t m_fileParameters;
    lsmash_movie_parameters_t m_movieParameters;
    lsmash_track_parameters_t m_audioTrackParameters;
    lsmash_track_parameters_t m_videoTrackParameters;
    lsmash_media_parameters_t m_audioMediaParameters;
    lsmash_media_parameters_t m_videoMediaParameters;
    char m_error[32];
    lsmash_brand_type m_brands[2];
    nanoem_u32_t m_duration;
    nanoem_u32_t m_format;
    nanoem_u32_t m_yflip;
};

} /* namespace anonymous */

struct nanoem_application_plugin_encoder_t : LSmashEncoder { };

nanoem_u32_t APIENTRY
nanoemApplicationPluginEncoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginEncoderInitialize()
{
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    nanoem_application_plugin_encoder_t *opaque = new nanoem_application_plugin_encoder_t();
    return opaque;
}

int APIENTRY
nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_i32_t *status)
{
    int result = 0;
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
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
    static const char *kFormatExtensions[] = { "mov" };
    *length = Inline::saturateInt32U(sizeof(kFormatExtensions) / sizeof(kFormatExtensions[0]));
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
        *length = Inline::saturateInt32U(plugin->getUIWindowLayoutDataSize());
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
    if (nanoem_likely(plugin && id && data && reloadLayout)) {
        plugin->setUIComponentData(id, data, length);
        *reloadLayout = 1;
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
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
