#include "emapp/emapp.h"

#include "emapp/Allocator.h"
#include "emapp/internal/StubEventPublisher.h"
#include "emapp/private/CommonInclude.h"

#include "emapp/internal/project/PMM.h"

#include "bx/commandline.h"
#include "bx/file.h"

#include "bx/string.h"

using namespace nanoem;

static void
loadCommon(const JSON_Object *item, const char *key, IFileManager::DialogType type, Project *project,
    IFileManager *fileManager)
{
    Error error;
    if (const char *path = json_object_dotget_string(item, key)) {
        const URI &fileURI = URI::createFromFilePath(path);
        fileManager->loadFromFile(fileURI, type, project, error);
    }
}

static void
setModelBinding(const JSON_Array *bindings, Project *project)
{
    Model *model = project->activeModel();
    for (size_t i = 0, numBindings = json_array_get_count(bindings); i < numBindings; i++) {
        const JSON_Object *item = json_array_get_object(bindings, i);
        const char *base = json_object_dotget_string(item, "base");
        const char *parentModel = json_object_dotget_string(item, "parent.model");
        const char *parentBone = json_object_dotget_string(item, "parent.bone");
        if (base && parentModel && parentBone) {
            const nanoem_model_bone_t *bone = model->findBone(base);
            model->setOutsideParent(bone, StringPair(parentModel, parentBone));
            CommandRegistrator registrator(project);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(model);
        }
    }
}

static void
loadModel(const JSON_Object *item, Project *project, IFileManager *fileManager)
{
    Error error;
    loadCommon(item, "path", IFileManager::kDialogTypeLoadModelFile, project, fileManager);
    loadCommon(item, "motion.path", IFileManager::kDialogTypeOpenModelMotionFile, project, fileManager);
    if (const JSON_Array *bindings = json_object_dotget_array(item, "bindings")) {
        setModelBinding(bindings, project);
    }
}

static void
saveProject(const bx::CommandLine &command, Project *project)
{
    const char *path = command.findOption('o', "output", "output.nma");
    IFileWriter *writer = FileUtils::createFileWriter();
    Error error;
    if (writer->open(URI::createFromFilePath(path), false, error)) {
        Error error;
        project->saveAsArchive(writer, error);
        writer->commit(error);
        FileUtils::destroyFileWriter(writer);
    }
}

static void
run(const bx::CommandLine &command)
{
    JSON_Value *root = json_value_init_object();
    void *dll = sg::openSharedLibrary("../emapp/sokol_noop." BX_DL_EXT);
    json_object_dotset_string(json_object(root), "plugin.effect.path", "../emapp/plugins/plugin_effect." BX_DL_EXT);
    JSON_Value *config = json_parse_file_with_comments(command.findOption('c', "config"));
#if 0
    if (dll && config) {
        ThreadedApplicationService service(&command, root);
        internal::StubEventPublisher publisher;
        service.setEventPublisher(&publisher);
        service.initialize(1.0f);
        Project *project = service.createProject(glm::u16vec2(1), SG_PIXELFORMAT_RGBA8, 1.0f, nullptr);
        /* if (command.hasArg("redo")) {
            const char *path = command.findOption("redo");
            IFileReader *reader = FileUtils::createFileReader();
            if (bx::open(reader, path)) {
                Error error;
                uint32_t sequence;
                project->loadFromRedo(reader, sequence, error);
                fprintf(stderr, "%d:%llu\n", sequence, bx::seek(reader, 0, bx::Whence::Current));
                bx::close(reader);
            }
            FileUtils::destroyFileReader(reader);
        }
        else */
        {
            project->setEffectPluginEnabled(true);
            IFileManager *fileManager = service.fileManager();
            Error error;
            const JSON_Array *models = json_object_dotget_array(json_object(config), "test.models");
            for (size_t i = 0, numModels = json_array_get_count(models); i < numModels; i++) {
                const JSON_Object *modelItem = json_array_get_object(models, i);
                loadModel(modelItem, project, fileManager);
            }
            loadCommon(
                json_object(config), "test.audio.path", IFileManager::kDialogTypeOpenAudio, project, fileManager);
            loadCommon(
                json_object(config), "test.video.path", IFileManager::kDialogTypeOpenVideo, project, fileManager);
            loadCommon(json_object(config), "test.camera.path", IFileManager::kDialogTypeOpenCameraMotion, project,
                fileManager);
            loadCommon(
                json_object(config), "test.light.path", IFileManager::kDialogTypeOpenLightMotion, project, fileManager);
            saveProject(command, project);
            service.destroyProject(project);
        }
        service.destroy();
    }
#elif 1
    BX_UNUSED_1(config);
    json_object_dotset_string(json_object(root), "plugin.effect.path", "");
    FileReaderScope scope(nullptr);
    Error error;
    const URI &fileURI = URI::createFromFilePath(command.findOption('i', "input", "test.pmm"));
    if (scope.open(fileURI, error)) {
        ThreadedApplicationService service(root);
        internal::StubEventPublisher publisher;
        service.setEventPublisher(&publisher);
        service.initialize(1.0f, 1.0f);
        Project *project = service.createProject(glm::vec2(1), SG_PIXELFORMAT_RGBA8, 1.0f, 1.0f, "");
        project->setFileURI(fileURI);
        internal::project::PMM pmm(project);
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        pmm.load(bytes.data(), bytes.size(), error, nullptr);
        {
            FileReaderScope scope(service.translator());
            const URI &fileURI2 = URI::createFromFilePath(command.findOption('p', "pose", "pose.vpd"));
            if (scope.open(fileURI2, error)) {
                model::BindPose pose;
                ByteArray bytes;
                FileUtils::read(scope, bytes, error);
                Model *model = project->allModels()->data()[0];
                model->loadPose(bytes, error);
            }
        }
        service.destroyProject(project);
        service.destroy();
    }
#endif
    sg::closeSharedLibrary(dll);
    json_value_free(root);
}

int
main(int argc, char *argv[])
{
    Allocator::initialize();
    ThreadedApplicationService::setup();
    bx::CommandLine command(argc, argv);
    run(command);
    ThreadedApplicationService::terminate();
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
