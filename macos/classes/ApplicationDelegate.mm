/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "ApplicationDelegate.h"

#import "MainWindow.h"

#include "CocoaThreadedApplicationService.h"

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

@end
