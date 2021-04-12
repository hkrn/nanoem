#include "emapp/Allocator.h"
#include "emapp/emapp.h"
#include "emapp/internal/StubEventPublisher.h"

#include "emapp/plugin/ModelIOPlugin.h"
#include "emapp/sdk/Model.h"

#include "bx/commandline.h"

#include <stdio.h>

#if !BX_PLATFORM_WINDOWS
#define MAX_PATH PATH_MAX
#endif

using namespace nanoem;
using namespace nanoem::internal;

struct ServiceHolder {
    ServiceHolder(internal::StubEventPublisher *publisher)
        : m_root(json_value_init_object())
        , m_service(nullptr)
        , m_dllHandle(nullptr)
    {
        m_dllHandle = sg::openSharedLibrary(
#ifdef CMAKE_INTDIR
            "../plugins/sokol_noop." BX_DL_EXT
#else
            "../emapp/bundle/sokol_noop." BX_DL_EXT
#endif
        );
        sg_desc desc = {};
        desc.buffer_pool_size = 1024u;
        desc.image_pool_size = 4096u;
        desc.shader_pool_size = 1024u;
        desc.pipeline_pool_size = 1024u;
        desc.pass_pool_size = 512u;
        sg::setup(&desc);
        ThreadedApplicationService::setup();
        m_service = new ThreadedApplicationService(m_root);
        m_service->setEventPublisher(publisher);
        m_service->initialize(1.0f, 1.0f);
        m_project = m_service->createProject(glm::u16vec2(1), SG_PIXELFORMAT_RGBA8, 1.0f, 1.0f, nullptr);
    }
    ~ServiceHolder()
    {
        m_service->destroyProject(m_project);
        delete m_service;
        m_service = nullptr;
        json_value_free(m_root);
        m_root = nullptr;
        sg::closeSharedLibrary(m_dllHandle);
        m_dllHandle = nullptr;
        ThreadedApplicationService::terminate();
    }

    JSON_Value *m_root;
    ThreadedApplicationService *m_service;
    Project *m_project;
    void *m_dllHandle;
};

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
run(const bx::CommandLine &command, const char *pluginPath)
{
    internal::StubEventPublisher publisher;
    plugin::ModelIOPlugin plugin(&publisher);
    if (plugin.load(URI::createFromFilePath(pluginPath)) && plugin.create()) {
        const int numFunctions = plugin.countAllFunctions();
        fprintf(stderr, "name: %s\n", plugin.name());
        fprintf(stderr, "description: %s\n", plugin.description());
        fprintf(stderr, "version: %s\n", plugin.version());
        tinystl::vector<int, TinySTLAllocator> indices;
        indices.push_back(0);
        ServiceHolder holder(&publisher);
        Project *project = holder.m_project;
        Error error;
        if (command.hasArg("batch")) {
            const char *batchFilePath = command.findOption("batch");
            ByteArray layout, inputBytes, outputBytes;
            if (FILE *fp = fopen(batchFilePath, "r")) {
                char buffer[1024];
                while (fgets(buffer, sizeof(buffer), fp)) {
                    buffer[strlen(buffer) - 1] = 0;
                    layout.clear();
                    inputBytes.clear();
                    outputBytes.clear();
                    if (FILE *fp2 = fopen(buffer, "rb")) {
                        fseek(fp2, 0, SEEK_END);
                        long size = ftell(fp2);
                        fseek(fp2, 0, SEEK_SET);
                        inputBytes.resize(size);
                        fread(inputBytes.data(), size, 1, fp2);
                        fclose(fp2);
                        plugin.setInputData(inputBytes, error);
                        plugin.getUIWindowLayout(layout, error);
                        plugin.execute(error);
                        plugin.getOutputData(outputBytes, error);
                        fprintf(stderr, "%s: %d -> %d\n", buffer, inputBytes.size(), outputBytes.size());
                    }
                }
                fclose(fp);
            }
        }
        else {
            fprintf(stderr, "functions: %d\n", numFunctions);
            for (int i = 0; i < numFunctions; i++) {
                fprintf(stderr, "function[%d]: %s\n", i, plugin.functionName(i));
                ByteArray layout, inputBytes, outputBytes;
                plugin.setFunction(i, error);
                plugin.getUIWindowLayout(layout, error);
                plugin.setInputData(inputBytes, error);
                plugin.setAllSelectedVertexObjectIndices(indices.data(), indices.size(), error);
                plugin.setAllSelectedMaterialObjectIndices(indices.data(), indices.size(), error);
                plugin.setAllSelectedBoneObjectIndices(indices.data(), indices.size(), error);
                plugin.setAllSelectedMorphObjectIndices(indices.data(), indices.size(), error);
                plugin.setAllSelectedLabelObjectIndices(indices.data(), indices.size(), error);
                plugin.setAllSelectedRigidBodyObjectIndices(indices.data(), indices.size(), error);
                plugin.setAllSelectedJointObjectIndices(indices.data(), indices.size(), error);
                plugin.setInputAudio(project->audioPlayer(), error);
                plugin.setInputCamera(project->globalCamera(), error);
                plugin.setInputLight(project->globalLight(), error);
                plugin.execute(error);
                plugin.getOutputData(outputBytes, error);
                if (error.hasReason()) {
                    fprintf(stderr, "\n -> %s:%s\n", error.reasonConstString(), error.recoverySuggestionConstString());
                }
            }
        }
        plugin.destroy();
        plugin.unload();
    }
    else {
        fprintf(stderr, "%s cannot be loaded\n", pluginPath);
    }
#if BX_PLATFORM_WINDOWS
    if (const char *ptr = strrchr(pluginPath, '\\')) {
        updateDllDirectory(ptr, pluginPath);
    }
    else if (const char *ptr = strrchr(pluginPath, '/')) {
        updateDllDirectory(ptr, pluginPath);
    }
#endif
}

int
main(int argc, char *argv[])
{
    const bx::CommandLine command(argc, argv);
    Allocator::initialize();
    const char *pluginPath = command.findOption("plugin");
    if (!pluginPath) {
        fprintf(stderr, "plugin (--plugin) is missing\n");
        exit(-1);
    }
    run(command, pluginPath);
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
