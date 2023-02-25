#include "emapp/Allocator.h"
#include "emapp/emapp.h"
#include "emapp/internal/StubEventPublisher.h"

#include "emapp/plugin/DecoderPlugin.h"
#include "emapp/plugin/EncoderPlugin.h"
#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Encoder.h"

#include "bx/commandline.h"
#include "bx/rng.h"

#include "glm/gtc/noise.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include <stdio.h>

#if !BX_PLATFORM_WINDOWS
#define MAX_PATH PATH_MAX
#endif

using namespace nanoem;
using namespace nanoem::internal;

static void
fillBuffer(int index, int width, int height, ByteArray &bytes)
{
    uint32_t *ptr = reinterpret_cast<uint32_t *>(bytes.data());
    bx::RngShr3 r(index + 1), rng(r.gen());
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            size_t offset = j * width + i;
            uint8_t v = bx::frnd(&rng) * 0xff;
            ptr[offset] = uint32_t(0xff << 24 | v << 16 | v << 8 | v << 0);
        }
    }
}

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
createFallbackVideo(const char *outputPath, const bx::CommandLine &command, plugin::EncoderPlugin &encoderPlugin)
{
    nanoem_u32_t width = strtol(command.findOption("width", "640"), 0, 10);
    nanoem_u32_t height = strtol(command.findOption("height", "480"), 0, 10);
    Error error;
    encoderPlugin.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, width, error);
    encoderPlugin.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, height, error);
    if (encoderPlugin.open(URI::createFromFilePath(outputPath), error)) {
        bx::debugPrintf("video encoder plugin was opened");
        ByteArray buffer(width * height * 4);
        nanoem_frame_index_t duration = strtol(command.findOption("duration", "300"), 0, 10);
        for (nanoem_frame_index_t i = 0; i < duration; i++) {
            fillBuffer(i, width, height, buffer);
            encoderPlugin.encodeVideoFrame(i, buffer.data(), buffer.size(), error);
#if 0
            char filename[MAX_PATH];
            snprintf(filename, sizeof(filename), "frame_%d.png", i);
            FILE *fp = fopen(filename, "wb");
            stbi_write_png_to_func(
                [](void *opaque, void *data, int size) {
                    FILE *fp = static_cast<FILE *>(opaque);
                    fwrite(data, size, 1, fp);
                },
                fp, width, height, 4, buffer.data(), width * 4);
            fclose(fp);
#endif
            bx::debugPrintf("frame %d is written", i);
        }
        encoderPlugin.close(error);
        bx::debugPrintf("video encoder plugin was closed");
    }
}

int
main(int argc, char *argv[])
{
    const bx::CommandLine command(argc, argv);
    Allocator::initialize();
    const char *encoderPluginPath = command.findOption("plugin");
    const char *outputPath = command.findOption("output", "output.avi");
    if (encoderPluginPath && outputPath) {
        StubEventPublisher publisher;
        plugin::DecoderPlugin decoderPlugin(&publisher);
        plugin::EncoderPlugin encoderPlugin(&publisher);
#if BX_PLATFORM_WINDOWS
        if (const char *ptr = strrchr(encoderPluginPath, '\\')) {
            updateDllDirectory(ptr, encoderPluginPath);
        }
        else if (const char *ptr = strrchr(encoderPluginPath, '/')) {
            updateDllDirectory(ptr, encoderPluginPath);
        }
#endif
        if (encoderPlugin.load(URI::createFromFilePath(encoderPluginPath))) {
            bx::debugPrintf("video encoder plugin was loaded");
            if (encoderPlugin.create()) {
                bx::debugPrintf("video encoder plugin was created");
                if (decoderPlugin.load(URI::createFromFilePath(encoderPluginPath))) {
                    Error error;
                    bx::debugPrintf("video decoder plugin was loaded");
                    int fps = strtol(command.findOption("fps", "30"), 0, 10);
                    encoderPlugin.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS, fps, error);
                    if (decoderPlugin.create()) {
                        const char *inputPath = command.findOption("input");
                        decoderPlugin.setOption(NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS, 30, error);
                        if (decoderPlugin.open(URI::createFromFilePath(inputPath), error)) {
                            nanoem_u32_t width =
                                decoderPlugin.videoFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH);
                            nanoem_u32_t height =
                                decoderPlugin.videoFormatValue(NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT);
                            nanoem_frame_index_t duration = decoderPlugin.videoDuration();
                            encoderPlugin.setOption(NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH, width, error);
                            encoderPlugin.setOption(
                                NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT, height, error);
                            if (encoderPlugin.open(URI::createFromFilePath(outputPath), error)) {
                                bx::debugPrintf("video encoder plugin was opened");
                                ByteArray buffer(width * height * 4);
                                for (nanoem_frame_index_t i = 0; i < duration; i++) {
                                    decoderPlugin.decodeVideoFrame(i, buffer, error);
                                    bx::debugPrintf("frame %d is read", i);
                                    encoderPlugin.encodeVideoFrame(i, buffer.data(), buffer.size(), error);
                                    bx::debugPrintf("frame %d is written", i);
                                }
                                encoderPlugin.close(error);
                                bx::debugPrintf("video encoder plugin was closed");
                            }
                            else {
                                createFallbackVideo(outputPath, command, encoderPlugin);
                            }
                            decoderPlugin.close(error);
                            bx::debugPrintf("video decoder plugin was closed");
                        }
                        else {
                            createFallbackVideo(outputPath, command, encoderPlugin);
                        }
                        decoderPlugin.destroy();
                        bx::debugPrintf("video decoder plugin was destroyed");
                    }
                    else {
                        createFallbackVideo(outputPath, command, encoderPlugin);
                    }
                    decoderPlugin.unload();
                    bx::debugPrintf("video encoder plugin was unloaded");
                }
                else {
                    createFallbackVideo(outputPath, command, encoderPlugin);
                }
                encoderPlugin.destroy();
                bx::debugPrintf("video encoder plugin was destroyed");
            }
            encoderPlugin.unload();
            bx::debugPrintf("video encoder plugin was unloaded");
        }
        else {
            bx::debugPrintf("plugin cannot be loaded");
        }
    }
    else {
        bx::debugPrintf("plugin is missing");
    }
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
