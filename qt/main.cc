/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include <QApplication>
#include <QSemaphore>
#include <QTemporaryDir>

#include "emapp/Allocator.h"
#include "emapp/emapp.h"
#include "nanoem/nanoem.h"

#include "MainWindow.h"
#include "QtApplicationClient.h"
#include "QtApplicationService.h"

#include "bx/commandline.h"
#include "bx/os.h"

using namespace nanoem;

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Allocator::initialize();
    qt::QtApplicationService::setup();
    int ret;
    {
        QTemporaryDir tempDir;
        const QDir appdir(QApplication::applicationDirPath());
        const QByteArray tempDirPath(tempDir.path().toUtf8()), localeName(QLocale::system().name().toUtf8()),
            pluginPath(appdir.relativeFilePath("plugins/plugin_effect." BX_DL_EXT).toUtf8());
        JSON_Value *config = json_value_init_object();
        JSON_Object *root = json_object(config);
        json_object_dotset_string(root, "project.locale", localeName.constData());
        json_object_dotset_string(root, "project.tmp.path", tempDirPath.constData());
        json_object_dotset_string(root, "plugin.effect.path", pluginPath.constData());
        app.setApplicationVersion(nanoemGetVersionString());
        app.setOrganizationDomain(BaseApplicationService::kOrganizationDomain);
        bx::CommandLine commands(argc, argv);
        qt::QtApplicationClient::Bridge bridge;
        qt::QtApplicationClient client(&bridge);
        qt::QtApplicationService service(config, &bridge);
        {
            qt::MainWindow window(&commands, &service, &client);
            window.show();
            ret = app.exec();
        }
        json_value_free(config);
    }
    Allocator::destroy();
    return ret;
}
