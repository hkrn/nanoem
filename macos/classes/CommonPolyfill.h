/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <AppKit/AppKit.h>

#ifndef NDEBUG
#define nanoem_objc_log(...) NSLog(__VA_ARGS__)
#else
#define nanoem_objc_log(...)
#endif

#define nanoem_objc_domain(name) @"com.github.nanoem." #name

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1100

@interface
NSFileWrapper (MacOSX_1090_polyfill)

@property (readonly, nonatomic) BOOL regularFile;
@property (readonly, nonatomic) BOOL symbolicLink;

@end

@implementation
NSFileWrapper (MacOSX_1090_polyfill)

- (BOOL)regularFile
{
    return [[self.fileAttributes objectForKey:NSFileType] isEqualToString:NSFileTypeRegular];
}

- (BOOL)directory
{
    return [[self.fileAttributes objectForKey:NSFileType] isEqualToString:NSFileTypeDirectory];
}

- (BOOL)symbolicLink
{
    return [[self.fileAttributes objectForKey:NSFileType] isEqualToString:NSFileTypeSymbolicLink];
}

@end

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070

static NSString *NSWindowDidChangeBackingPropertiesNotification = @"NSWindowDidChangeBackingPropertiesNotification";
static NSUInteger NSWindowCollectionBehaviorFullScreenPrimary = 0;
static const NSUInteger NSAutosaveInPlaceOperation = 4;

@interface
NSWindow (MacOSX_1060_polyfill)

@property (readonly, nonatomic) CGFloat backingScaleFactor;
@property (assign, nonatomic) BOOL restorable;

@end

@interface
NSView (MacOSX_1060_polyfill)

@property (assign, nonatomic) BOOL wantsBestResolutionOpenGLSurface;

@end

@interface
NSEvent (MacOSX_1060_polyfill)

@property (readonly, nonatomic) BOOL hasPreciseScrollingDeltas;
@property (readonly, nonatomic) CGFloat scrollingDeltaX;
@property (readonly, nonatomic) CGFloat scrollingDeltaY;

@end

@implementation
NSWindow (MacOSX_1060_polyfill)

- (CGFloat)backingScaleFactor
{
    return 1.0f;
}

- (BOOL)restorable
{
    return NO;
}

- (void)setRestorable:(BOOL)value
{
}

@end

@implementation
NSView (MacOSX_1060_polyfill)

- (BOOL)wantsBestResolutionOpenGLSurface
{
    return NO;
}

- (void)setWantsBestResolutionOpenGLSurface:(BOOL)value
{
}

@end

@implementation
NSEvent (MacOSX_1060_polyfill)

- (BOOL)hasPreciseScrollingDeltas
{
    return NO;
}

- (CGFloat)scrollingDeltaX
{
    return 0.0f;
}

- (CGFloat)scrollingDeltaY
{
    return 0.0f;
}

@end

#endif /* MAC_OS_X_VERSION_MAX_ALLOWED < 1070 */

#endif /* MAC_OS_X_VERSION_MAX_ALLOWED < 1100 */
