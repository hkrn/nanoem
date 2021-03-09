#include "emapp/Allocator.h"
#include "emapp/emapp.h"
#include "emapp/internal/StubEventPublisher.h"

#include "emapp/plugin/DecoderPlugin.h"
#include "emapp/plugin/EncoderPlugin.h"
#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Encoder.h"

#include "bx/commandline.h"

#include <stdio.h>

#if !BX_PLATFORM_WINDOWS
#define MAX_PATH PATH_MAX
#endif

using namespace nanoem;
using namespace nanoem::internal;

#if BX_PLATFORM_WINDOWS
static void
updateDllDirectory(const char *ptr, const char *base)
{
    char buf[MAX_PATH];
    strncpy_s(buf, sizeof(buf), base, ptr - base);
    SetDllDirectoryA(buf);
}
#endif

static void
run(const char *decoderPluginPath, const char *encoderPluginPath, const char *audioPath)
{
    internal::StubEventPublisher publisher;
    plugin::DecoderPlugin decoder(&publisher);
    plugin::EncoderPlugin encoder(&publisher);
#if BX_PLATFORM_WINDOWS
    if (const char *ptr = strrchr(audioPath, '\\')) {
        updateDllDirectory(ptr, audioPath);
    }
    else if (const char *ptr = strrchr(audioPath, '/')) {
        updateDllDirectory(ptr, audioPath);
    }
#endif
    uint32_t fps = 30, width = 128, height = 128;
    if (decoder.load(URI::createFromFilePath(decoderPluginPath)) && decoder.create()) {
        Error error;
        decoder.setOption(NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS, fps, error);
        if (decoder.open(URI::createFromFilePath(audioPath), error)) {
            Error error;
            nanoem_frame_index_t duration =
                decoder.audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION);
            if (encoder.load(URI::createFromFilePath(encoderPluginPath)) && encoder.create()) {
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS,
                    decoder.audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS), error);
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS,
                    decoder.audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS), error);
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY,
                    decoder.audioFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY), error);
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION, duration, error);
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS, fps, error);
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, width, error);
                encoder.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, height, error);
                if (encoder.open(URI::createFromFilePath("output.mp4"), error)) {
                    ByteArray audioBuffer, videoBuffer;
                    videoBuffer.resize(width * height * 4);
                    for (nanoem_frame_index_t i = 0; i < duration; i++) {
                        decoder.decodeAudioFrame(i, audioBuffer, error);
                        encoder.encodeAudioFrame(i, audioBuffer.data(), audioBuffer.size(), error);
                        encoder.encodeVideoFrame(i, videoBuffer.data(), videoBuffer.size(), error);
                    }
                    encoder.close(error);
                }
                encoder.destroy();
            }
            decoder.close(error);
        }
        else {
            fprintf(stderr, "%s:%s\n", decoder.failureReason(), decoder.recoverySuggestion());
        }
        decoder.destroy();
    }
    decoder.unload();
    encoder.unload();
}

int
main(int argc, char *argv[])
{
    const bx::CommandLine command(argc, argv);
    Allocator::initialize();
    const char *commonPluginPath = command.findOption("plugin");
    const char *decoderPluginPath = command.findOption("decoder", commonPluginPath);
    if (!decoderPluginPath) {
        fprintf(stderr, "decoder plugin (--encoder or --plugin) is missing\n");
        exit(-1);
    }
    const char *encoderPluginPath = command.findOption("encoder", commonPluginPath);
    if (!encoderPluginPath) {
        fprintf(stderr, "decoder plugin (--encoder or --plugin) is missing\n");
        exit(-1);
    }
    const char *audioPath = command.findOption("input");
    if (!audioPath) {
        fprintf(stderr, "audio (--input) is missing\n");
        exit(-1);
    }
    run(decoderPluginPath, encoderPluginPath, audioPath);
    Allocator::destroy();
    return 0;
}

#if BX_PLATFORM_WINDOWS && !BX_PLATFORM_WINRT
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    BX_UNUSED_3(hInstance, hPrevInstance, nCmdShow);
    SetDllDirectoryW(L"");
    char *argv[64], buffer[1024];
    uint32_t size = BX_COUNTOF(buffer);
    int argc = 0;
    bx::tokenizeCommandLine(lpCmdLine, buffer, size, argc, argv, BX_COUNTOF(argv));
    return main(argc, argv);
}
#endif
