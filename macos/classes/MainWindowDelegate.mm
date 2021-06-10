/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "MainWindowDelegate.h"

#import "MainWindow.h"

#include "emapp/EnumUtils.h"
#include "emapp/ThreadedApplicationClient.h"

@implementation MainWindowDelegate

- (instancetype)initWithWindow:(nanoem::macos::MainWindow *)window
{
    if (self = [super init]) {
        m_window = window;
    }
    return self;
}

- (void)windowDidResize:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    m_window->handleWindowResize();
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    m_window->client()->sendDeactivateMessage();
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    m_window->client()->sendActivateMessage();
}

- (void)windowDidChangeBackingProperties:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    m_window->handleWindowChangeDevicePixelRatio();
}

- (void)windowDidChangeOcclusionState:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    NSWindowOcclusionState state = m_window->nativeWindow().occlusionState;
    if (nanoem::EnumUtils::isEnabledT<NSWindowOcclusionState>(state, NSWindowOcclusionStateVisible)) {
        m_window->client()->sendActivateMessage();
    }
    else {
        m_window->client()->sendDeactivateMessage();
    }
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    m_window->handleAssignFocus();
}

- (void)windowDidBecomeKey
{
    m_window->handleAssignFocus();
}

- (void)windowDidResignMain:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
    m_window->handleResignFocus();
}

- (void)windowDidResignKey
{
    m_window->handleResignFocus();
}

- (BOOL)windowShouldClose:(id)sender
{
    BX_UNUSED_1(sender);
    m_window->confirmBeforeClose();
    return NO;
}

- (void)windowDidChangeScreen:(NSNotification *)notification
{
    BX_UNUSED_1(notification);
}

@end
