/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <Foundation/Foundation.h>

typedef void (^MenuItemActionItemCallback)(void);

@class NSMenu;
@class NSMenuItem;

@interface MenuItemCollection : NSObject {
    NSMenu *m_menu;
    NSMutableDictionary *m_actions;
}

@property (nonatomic, readonly) NSMenu *menu;

- (instancetype)init UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithTitle:(NSString *)title;

- (void)setParentMenu:(NSMenuItem *)parentMenu;

- (NSMenuItem *)addItemWithTitle:(NSString *)title;
- (NSMenuItem *)addItemWithTitle:(NSString *)title action:(MenuItemActionItemCallback)actionApply;
- (NSMenuItem *)addItemWithTitle:(NSString *)title
                          action:(MenuItemActionItemCallback)actionApply
                   keyEquivalent:(NSString *)keyEquiv;
- (void)removeItemWithTitle:(NSString *)title;

- (void)removeAllItems;
- (void)addSeparator;

@end
