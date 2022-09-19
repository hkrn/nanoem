/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <AppKit/AppKit.h>

#import "ApplicationDelegate.h"
#import "MainWindow.h"
#import "MainWindowDelegate.h"
#import "Preference.h"

#include "bx/commandline.h"
#include "emapp/Allocator.h"
#include "emapp/emapp.h"

#include "CocoaThreadedApplicationService.h"
#if defined(NANOEM_ENABLE_LOGGING)
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#endif

using namespace nanoem;

int
main(int argc, char *argv[])
{
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *executablePath = mainBundle.executableURL.path;
    if ([executablePath hasPrefix:@"/private/var/folders"] && [executablePath containsString:@"AppTranslocation"]) {
        NSAlert *alert = [[NSAlert alloc] init];
        alert.alertStyle = NSAlertStyleCritical;
        alert.messageText = NSLocalizedString(@"Cannot execute from here", @"");
        alert.informativeText = NSLocalizedString(@"Move nanoem.app to Application folder and retry again.", @"");
        [alert runModal];
        return 1;
    }
    Allocator::initialize();
    ThreadedApplicationService::setup();
#if defined(NANOEM_ENABLE_LOGGING)
    spdlog::cfg::load_env_levels();
    spdlog::stdout_color_mt("emapp");
#endif /* NANOEM_ENABLE_LOGGING */
    @autoreleasepool {
        JSON_Value *config = json_value_init_object();
        NSLocale *locale = [NSLocale currentLocale];
        JSON_Object *root = json_object(config);
        NSURL *url = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory
                                                            inDomain:NSUserDomainMask
                                                   appropriateForURL:nil
                                                              create:YES
                                                               error:nil];
        NSString *domainString = [[NSString alloc] initWithUTF8String:BaseApplicationService::kOrganizationDomain];
        NSURL *applicationDirectoryURL = [url URLByAppendingPathComponent:domainString isDirectory:YES];
        NSURL *redoDirectoryURL = [applicationDirectoryURL URLByAppendingPathComponent:@"redo" isDirectory:YES];
        NSURL *tempDirectoryURL = [applicationDirectoryURL URLByAppendingPathComponent:@"tmp" isDirectory:YES];
        NSURL *sentryDatabaseURL = [applicationDirectoryURL URLByAppendingPathComponent:@"sentry-db" isDirectory:NO];
        NSURL *sentryHandlerURL = [mainBundle.builtInPlugInsURL URLByAppendingPathComponent:@"crashpad_handler"];
        NSError *error = nil;
        NSDictionary *attributes =
            [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithShort:0700], NSFilePosixPermissions, nil];
        [[NSFileManager defaultManager] createDirectoryAtURL:tempDirectoryURL
                                 withIntermediateDirectories:YES
                                                  attributes:attributes
                                                       error:&error];
        [[NSFileManager defaultManager] createDirectoryAtURL:redoDirectoryURL
                                 withIntermediateDirectories:YES
                                                  attributes:attributes
                                                       error:&error];
        json_object_dotset_string(root, "project.home", NSHomeDirectory().UTF8String);
        json_object_dotset_string(root, "project.locale", locale.localeIdentifier.UTF8String);
        json_object_dotset_string(root, "project.tmp.path", tempDirectoryURL.path.UTF8String);
        json_object_dotset_string(root, "macos.redo.path", redoDirectoryURL.path.UTF8String);
        json_object_dotset_string(root, "macos.sentry.handler.path", sentryHandlerURL.path.UTF8String);
        json_object_dotset_string(root, "macos.sentry.database.path", sentryDatabaseURL.path.UTF8String);
        {
            const bx::CommandLine command(argc, argv);
            NSApplication *app = macos::CocoaThreadedApplicationService::createApplication();
            if (@available(macOS 10.12.1, *)) {
                if (NSClassFromString(@"NSTouchBar") != nil) {
                    app.automaticCustomizeTouchBarMenuItemEnabled = YES;
                }
            }
            if (command.hasArg("locale")) {
                json_object_dotset_string(root, "project.locale", command.findOption("locale"));
            }
            macos::CocoaThreadedApplicationService service(config);
            ThreadedApplicationClient client;
            service.start();
            client.connect();
            {
                macos::Preference preference([NSUserDefaults standardUserDefaults], &service, config);
                NSWindow *nativeWindow = macos::CocoaThreadedApplicationService::createMainWindow();
                macos::MainWindow *window =
                    new macos::MainWindow(&command, &service, &client, nativeWindow, &preference);
                window->initialize();
                ApplicationDelegate *applicationDelegate = [[ApplicationDelegate alloc] initWithWindow:window];
                app.delegate = applicationDelegate;
                MainWindowDelegate *windowDelegate = [[MainWindowDelegate alloc] initWithWindow:window];
                nativeWindow.delegate = windowDelegate;
                [app run];
                while (window->isRunning()) {
                    window->processMessage(app);
                }
                window->terminate();
                delete window;
            }
            client.close();
            service.stop();
        }
        json_value_free(config);
    }
    ThreadedApplicationService::terminate();
    Allocator::destroy();
    return 0;
}
