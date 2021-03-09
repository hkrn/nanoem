/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "MenuItemCollection.h"

#import <AppKit/AppKit.h>

@implementation MenuItemCollection

@synthesize menu = m_menu;

- (instancetype)initWithTitle:(NSString *)title
{
    if (self = [super init]) {
        m_menu = [[NSMenu alloc] initWithTitle:title];
        m_menu.autoenablesItems = NO;
        m_actions = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (void)clearAllBlocks
{
    [m_actions removeAllObjects];
}

- (void)dealloc
{
    [self clearAllBlocks];
}

- (void)setParentMenu:(NSMenuItem *)parentMenu
{
    parentMenu.submenu = m_menu;
}

- (NSMenuItem *)addItemWithTitle:(NSString *)title
{
    return [self addItemWithTitle:title action:nil keyEquivalent:@""];
}

- (NSMenuItem *)addItemWithTitle:(NSString *)title action:(MenuItemActionItemCallback)actionApply
{
    return [self addItemWithTitle:title action:actionApply keyEquivalent:@""];
}

- (NSMenuItem *)addItemWithTitle:(NSString *)title
                          action:(MenuItemActionItemCallback)actionApply
                   keyEquivalent:(NSString *)keyEquiv
{
    NSMenuItem *item = [m_menu addItemWithTitle:title action:@selector(handleBlockCallback:) keyEquivalent:keyEquiv];
    item.target = self;
    if (actionApply) {
        [m_actions setObject:actionApply forKey:title];
    }
    return item;
}

- (void)addSeparator
{
    [m_menu addItem:[NSMenuItem separatorItem]];
}

- (void)removeItemWithTitle:(NSString *)title
{
    NSInteger index = [m_menu indexOfItemWithTitle:title];
    if (index != -1) {
        MenuItemActionItemCallback action = [m_actions objectForKey:title];
        if (action) {
            [m_actions removeObjectForKey:title];
        }
        [m_menu removeItemAtIndex:index];
    }
}

- (void)removeAllItems
{
    [self clearAllBlocks];
    [m_menu removeAllItems];
}

- (void)handleBlockCallback:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *) sender;
    NSString *title = [menuItem title];
    MenuItemActionItemCallback actionApply = [m_actions objectForKey:title];
    if (actionApply) {
        actionApply();
    }
}

@end
