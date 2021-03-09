/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <AppKit/AppKit.h>

namespace nanoem {
namespace macos {
class MainWindow;
} /* namespace macos */
} /* namespace nanoem */

@interface ApplicationDelegate : NSObject <NSApplicationDelegate> {
    nanoem::macos::MainWindow *m_window;
}

- (instancetype)init UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithWindow:(nanoem::macos::MainWindow *)window;

@end
