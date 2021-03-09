#include "emapp/Allocator.h"
#include "emapp/emapp.h"
#include "emapp/internal/EditingModelTrait.h"
#include "emapp/internal/StubEventPublisher.h"
#include "emapp/model/Importer.h"

#include "bx/commandline.h"

#include <stdio.h>

using namespace nanoem;

static void
transform(Model *model, Motion *motion)
{
    // factory.createVertexMorphFromCurrentPose();
    // factory.createBoneMorphFromCurrentPose();
    // factory.normalizeAllVertices();
    internal::EditingModelTrait::Bone bone(model);
    bone.createRootParentBone();
    bone.createStagingProxyBone(model->activeBone());
    BX_UNUSED(motion);
}

static void
output(const bx::CommandLine &command, Model *model, Error &error)
{
    if (command.hasArg("output")) {
        const char *outputPath = command.findOption("output");
        IFileWriter *modelWriter = FileUtils::createFileWriter();
        if (modelWriter->open(URI::createFromFilePath(outputPath), false, error)) {
            fprintf(stderr, "Opened Model %s\n", outputPath);
            if (model->save(modelWriter, error)) {
                fprintf(stderr, "Saved Model %s\n", outputPath);
            }
            else {
                fprintf(stderr, "[ERROR] Failed saving model(%llu): %s\n", error.code(), error.reasonConstString());
            }
            modelWriter->commit(error);
            FileUtils::destroyFileWriter(modelWriter);
        }
        else {
            fprintf(stderr, "[ERROR] Failed saving model(%llu): %s\n", error.code(), error.reasonConstString());
        }
        FileUtils::destroyFileWriter(modelWriter);
    }
}

static void
run(const bx::CommandLine &command)
{
    ThreadedApplicationService service(0);
    internal::StubEventPublisher publisher;
    Error error;
    service.setEventPublisher(&publisher);
    service.initialize(1.0f);
    Project *project = service.createProject(glm::vec2(1), SG_PIXELFORMAT_RGBA8, 1.0f, 1.0f, 0);
    project->setLanguage(ITranslator::kLanguageTypeJapanese);
    if (command.hasArg("model")) {
        const URI &fileURI = URI::createFromFilePath(command.findOption("model"));
        if (Model::isLoadableExtension(fileURI)) {
            const char *inputPath = fileURI.absolutePathConstString();
            FileReaderScope modelScope(service.translator());
            Error error;
            if (modelScope.open(URI::createFromFilePath(inputPath), error)) {
                fprintf(stderr, "Opened Model %s\n", inputPath);
                ByteArray bytes;
                FileUtils::read(modelScope, bytes, error);
                Model *model = project->createModel();
                if (model->load(bytes, error)) {
                    fprintf(stderr, "Loaded Mdoel %s\n", inputPath);
                    model->upload();
                    project->addModel(model);
                    Motion *motion = 0;
                    if (command.hasArg("motion")) {
                        const char *motionPath = command.findOption("motion");
                        FileReaderScope motionScope(service.translator());
                        if (motionScope.open(URI::createFromFilePath(motionPath), error)) {
                            fprintf(stderr, "Opened Motion %s\n", motionPath);
                            FileUtils::read(motionScope, bytes, error);
                            motion = project->createMotion();
                            motion->setFormat(URI::createFromFilePath(motionPath));
                            motion->load(bytes, 0, error);
                            assert(!error.hasReason());
                            project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeDisable);
                            project->destroyMotion(project->addModelMotion(motion, model));
                            project->restart();
                        }
                    }
                    transform(model, motion);
                    output(command, model, error);
                }
            }
        }
        else {
            const char *inputPath = fileURI.absolutePathConstString();
            FileReaderScope modelScope(service.translator());
            Error error;
            if (modelScope.open(URI::createFromFilePath(inputPath), error)) {
                fprintf(stderr, "Opened Model %s\n", inputPath);
                ByteArray bytes;
                FileUtils::read(modelScope, bytes, error);
                Model *model = project->createModel();
                project->addModel(model);
                model::Importer importer(model);
                Model::ImportSetting setting(fileURI);
                importer.execute(bytes.data(), bytes.size(), setting, error);
                if (error.hasReason()) {
                    fprintf(stderr, "[ERROR] Failed executing conversion: %s\n", error.reasonConstString());
                }
                else {
                    output(command, model, error);
                }
            }
        }
    }
    service.destroyProject(project);
    service.destroy();
}

int
main(int argc, char *argv[])
{
    Allocator::initialize();
    ThreadedApplicationService::setup();
    void *dllHandle = sg::openSharedLibrary("../emapp/sokol_noop." BX_DL_EXT);
    bx::CommandLine command(argc, argv);
    run(command);
    sg::closeSharedLibrary(dllHandle);
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
