/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "ApplicationDelegate.h"

#import "MainWindow.h"

#include "CocoaThreadedApplicationService.h"
#include "emapp/emapp.h"
#include "emapp/private/CommonInclude.h"

@implementation ApplicationDelegate

- (instancetype)initWithWindow:(nanoem::macos::MainWindow *)window
{
    if (self = [super init]) {
        m_window = window;
    }
    return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)application
{
    BX_UNUSED_1(application);
    m_window->confirmBeforeClose();
    return NSTerminateCancel;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    (void) notification;
    NSApplication *app = [NSApplication sharedApplication];
    [app stop:app];
    nanoem::macos::CocoaThreadedApplicationService::sendDummyEvent(app, false);
}

/* Handle files opened from Finder (double click / "Open With") ourselves instead of letting
   NSDocumentController instantiate Document without a project holder, which always failed with
   "nanoem cannot open files in the ... format".
   - .nanoem packages go through the same path as File > Open Project
   - everything else (nma/nmm/pmm projects, PMX/PMD models, VMD/VPD, accessories...) goes through
     the same path as drag and drop */
- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls
{
    BX_UNUSED_1(application);
    nanoem::ThreadedApplicationClient *client = m_window->client();
    for (NSURL *url in urls) {
        if (!url.isFileURL) {
            continue;
        }
        NSString *filePath = url.path;
        const nanoem::URI &fileURI = nanoem::URI::createFromFilePath(filePath.UTF8String);
        if ([url.pathExtension caseInsensitiveCompare:@"nanoem"] == NSOrderedSame) {
            client->sendLoadFileMessage(fileURI, nanoem::IFileManager::kDialogTypeOpenProject);
            m_window->setTitle(filePath.lastPathComponent);
        }
        else {
            client->sendDropFileMessage(fileURI);
            if (nanoem::Project::isLoadableExtension(fileURI)) {
                m_window->setTitle(filePath.lastPathComponent);
            }
        }
    }
}

@end
