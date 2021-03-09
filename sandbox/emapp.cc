#include "common.h"
#include "emapp/Allocator.h"

using namespace nanoem;
using namespace nanoem::internal;
using namespace test;

int
main(int argc, char *argv[])
{
    BX_UNUSED_2(argc, argv);
#if 0
    TestScope scope;
    Application *application = scope.application();
    {
        ProjectPtr project = scope.createProject();
        {
            project->setEffectPluginEnabled(true);
            for (int i = 0; i < 3; i++) {
                Accessory *accessory = createAccessory(project.get());
                project->addAccessory(accessory);
            }
            for (int i = 0; i < 3; i++) {
                Model *model = createModel(project.get());
                project->addModel(model);
            }
            Accessory *accessory = createAccessory(project.get(), "effects/offscreen.x");
            createSourceEffect(accessory, project.get(), "effects/offscreen.fx");
            project->addAccessory(accessory);
            Model *model = createModel(project.get(), "effects/offscreen.pmx");
            model->setName("offscreen");
            createSourceEffect(model, project.get(), "effects/offscreen.fx");
            project->addModel(model);
        }
        const URI &fileURI = URI::createFromFilePath(NANOEM_TEST_OUTPUT_PATH "/project_save_after_deleting_origin.nma");
        Error error;
        cr_expect_eq(
            application->fileManager()->saveAsFile(fileURI, IFileManager::kDialogTypeSaveProject, project.get(), error),
            true);
        cr_expect_eq(error.hasReason(), false);
        ProjectPtr transientProject = scope.createProject();
        cr_expect_eq(application->fileManager()->loadFromFile(
                         fileURI, IFileManager::kDialogTypeOpenProject, transientProject.get(), error),
            true);
        cr_expect_eq(error.hasReason(), false);
        // delete origin project path (and cannot load project from origin project path at saving)
        cr_expect_eq(FileUtils::remove(fileURI.absolutePath()), true);
        cr_expect_eq(application->fileManager()->saveAsFile(
                         fileURI, IFileManager::kDialogTypeSaveProject, transientProject.get(), error),
            true);
        cr_expect_eq(error.hasReason(), false);
        ProjectPtr newProject = scope.createProject();
        cr_expect_eq(
            application->fileManager()->loadFromFile(fileURI, IFileManager::kDialogTypeOpenProject, newProject.get(), error),
            true);
        cr_expect_eq(error.hasReason(), false);
        cr_expect_eq(newProject->allAccessories().size(), 4);
        cr_expect_eq(newProject->allModels().size(), 4);
        cr_expect_eq(newProject->drawableOrderList().size(), 8);
        cr_expect_eq(newProject->transformOrderList().size(), 4);
        const Project::AccessoryList &allAccessories = projectAllAccessories(newProject.get());
        const Project::ModelList &allModels = projectAllModels(newProject.get());
        cr_assert_eq(allAccessories.size(), 4);
        cr_assert_eq(allModels.size(), 4);
        cr_expect_eq(allAccessories[0]->fileURI().fragment(), String("Accessory/test/test.x"));
        cr_expect_eq(allAccessories[1]->fileURI().fragment(), String("Accessory/test-1/test.x"));
        cr_expect_eq(allAccessories[2]->fileURI().fragment(), String("Accessory/test-2/test.x"));
        cr_expect_eq(allAccessories[3]->fileURI().fragment(), String("Acceesory/offscreen/offscreen.x"));
        cr_expect_not_null(allAccessories[3]->activeEffect());
        cr_expect_eq(allModels[0]->fileURI().fragment(), modelPath(0, "test.pmx"));
        cr_expect_eq(allModels[1]->fileURI().fragment(), modelPath(1, "test.pmx"));
        cr_expect_eq(allModels[2]->fileURI().fragment(), modelPath(2, "test.pmx"));
        cr_expect_eq(allModels[3]->fileURI().fragment(), String("Model/offscreen/offscreen.pmx"));
        cr_expect_not_null(allModels[3]->activeEffect());
        // cr_expect_eq(FileUtils::remove(fileURI.absolutePath()), true);
    }
#endif
    return 0;
}

#if BX_PLATFORM_WINDOWS && !BX_PLATFORM_WINRT
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    BX_UNUSED_4(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    return main(1, &lpCmdLine);
}
#endif
