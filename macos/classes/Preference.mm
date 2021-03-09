/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>

#import "Preference.h"

#include "emapp/BaseApplicationService.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationService.h"

namespace {
static NSString *kCompatClientUUIDKey = @"clientUUID";
static NSString *kCompatEnableAnalyticsKey = @"enable-analytics";
static NSString *kCompatRendererNameKey = @"NMRendererName";
static NSString *kClientUUIDKey = @"NMClientUUID";
static NSString *kEnableAnalyticsKey = @"NMEnableAnalytics";
static NSString *kEnableModelEditingKey = @"NMEnableModelEditing";
static NSString *kPreferredEditingFPSKey = @"NMPreferredEditingFPS";
static NSString *kRendererNameKey = @"NMRendererNameGen2";
static NSString *kDefaultColorPixelFormatNameKey = @"NMDefaultColorPixelFormat";
static NSString *kSkinDeformAcceleratorKey = @"NMEnableSkinDeformAcceleratorGen2";
static NSString *kUndoSoftLimit = @"NMUndoSoftLimit";
static NSString *kGFXBufferPoolSizeKey = @"NMGFXBufferPoolSize";
static NSString *kGFXImagePoolSizeKey = @"NMGFXImagePoolSize";
static NSString *kGFXShaderPoolSizeKey = @"NMGFXShaderPoolSize";
static NSString *kGFXPassPoolSizeKey = @"NMGFXPassPoolSize";
static NSString *kGFXPipelinePoolSizeKey = @"NMGFXPipelinePoolSize";
static NSString *kGFXUniformBufferSizeKey = @"NMGFXUniformBufferSize";
static NSString *kHighDPIViewportModeKey = @"NMHighDPIViewportMode";
} /* namespace anonymous */

namespace nanoem {
namespace macos {

bool
Preference::isMetalAvailable()
{
    bool result = false;
    if (@available(macOS 10.11, *)) {
        NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
        result = devices.count > 0 && !(NSAppKitVersionNumber10_13 > NSAppKitVersionNumber);
    }
    return result;
}

Preference::Preference(NSUserDefaults *defaults, ThreadedApplicationService *service, JSON_Value *config)
    : m_preference(service)
    , m_defaults(defaults)
    , m_config(config)
{
    load();
}

Preference::~Preference()
{
    NSString *renderer = [NSString stringWithUTF8String:m_preference.rendererBackend()];
    [m_defaults setObject:m_clientUUID forKey:kClientUUIDKey];
    [m_defaults setBool:m_preference.isAnalyticsEnabled() forKey:kEnableAnalyticsKey];
    [m_defaults setObject:renderer forKey:kRendererNameKey];
    [m_defaults setInteger:m_preference.preferredEditingFPS() forKey:kPreferredEditingFPSKey];
    [m_defaults setInteger:m_preference.defaultColorPixelFormat() forKey:kDefaultColorPixelFormatNameKey];
    [m_defaults setInteger:m_preference.highDPIViewportMode() forKey:kHighDPIViewportModeKey];
    [m_defaults setBool:m_preference.isModelEditingEnabled() forKey:kEnableModelEditingKey];
    [m_defaults setBool:m_preference.isSkinDeformAcceleratorEnabled() forKey:kSkinDeformAcceleratorKey];
    [m_defaults setInteger:m_preference.undoSoftLimit() forKey:kUndoSoftLimit];
    [m_defaults setInteger:m_preference.gfxBufferPoolSize() forKey:kGFXBufferPoolSizeKey];
    [m_defaults setInteger:m_preference.gfxImagePoolSize() forKey:kGFXImagePoolSizeKey];
    [m_defaults setInteger:m_preference.gfxShaderPoolSize() forKey:kGFXShaderPoolSizeKey];
    [m_defaults setInteger:m_preference.gfxPassPoolSize() forKey:kGFXPassPoolSizeKey];
    [m_defaults setInteger:m_preference.gfxPipelinePoolSize() forKey:kGFXPipelinePoolSizeKey];
    [m_defaults setInteger:m_preference.gfxUniformBufferSize() forKey:kGFXUniformBufferSizeKey];
    [m_defaults removeObjectForKey:kCompatClientUUIDKey];
    [m_defaults removeObjectForKey:kCompatEnableAnalyticsKey];
    [m_defaults removeObjectForKey:kCompatRendererNameKey];
    [m_defaults synchronize];
}

const ApplicationPreference *
Preference::applicationPreference() const
{
    return &m_preference;
}

ApplicationPreference *
Preference::mutableApplicationPreference()
{
    return &m_preference;
}

NSString *
Preference::clientUUID() const
{
    return m_clientUUID;
}

uint32_t
Preference::vendorId() const
{
    return m_vendorId;
}

uint32_t
Preference::deviceId() const
{
    return m_deviceId;
}

void
Preference::load()
{
    m_clientUUID = [m_defaults stringForKey:kClientUUIDKey];
    if (!m_clientUUID) {
        if (NSString *clientUUID = [m_defaults stringForKey:kCompatClientUUIDKey]) {
            m_clientUUID = clientUUID;
        }
        else {
            CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
            CFStringRef uuidString = CFUUIDCreateString(kCFAllocatorDefault, uuid);
            [m_defaults setObject:(__bridge NSString *) uuidString forKey:kClientUUIDKey];
            CFRelease(uuidString);
            [m_defaults synchronize];
            m_clientUUID = [m_defaults stringForKey:kClientUUIDKey];
        }
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kEnableAnalyticsKey]) {
        m_preference.setAnalyticsEnabled(value.boolValue != NO);
    }
    else if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kCompatEnableAnalyticsKey]) {
        m_preference.setAnalyticsEnabled(value.boolValue != NO);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kEnableModelEditingKey]) {
        m_preference.setModelEditingEnabled(value.boolValue != NO);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kSkinDeformAcceleratorKey]) {
        m_preference.setSkinDeformAcceleratorEnabled(value.boolValue != NO);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kHighDPIViewportModeKey]) {
        m_preference.setHighDPIViewportMode(
            static_cast<ApplicationPreference::HighDPIViewportModeType>(value.intValue));
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kUndoSoftLimit]) {
        m_preference.setUndoSoftLimit(value.integerValue);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kGFXBufferPoolSizeKey]) {
        m_preference.setGFXBufferPoolSize(value.integerValue);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kGFXImagePoolSizeKey]) {
        m_preference.setGFXImagePoolSize(value.integerValue);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kGFXShaderPoolSizeKey]) {
        m_preference.setGFXShaderPoolSize(value.integerValue);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kGFXPassPoolSizeKey]) {
        m_preference.setGFXPassPoolSize(value.integerValue);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kGFXPipelinePoolSizeKey]) {
        m_preference.setGFXPipelinePoolSize(value.integerValue);
    }
    if (NSNumber *value = (NSNumber *) [m_defaults objectForKey:kGFXUniformBufferSizeKey]) {
        m_preference.setGFXUniformBufferSize(value.integerValue);
    }
    if (NSString *renderer = [m_defaults stringForKey:kRendererNameKey]) {
        m_preference.setRendererBackend(renderer.UTF8String);
    }
    else if (NSString *renderer = [m_defaults stringForKey:kCompatRendererNameKey]) {
        m_preference.setRendererBackend(renderer.UTF8String);
    }
    else {
        m_preference.setRendererBackend(
            isMetalAvailable() ? BaseApplicationService::kRendererMetal : BaseApplicationService::kRendererOpenGL);
    }
    m_preference.setDefaultColorPixelFormat(
        static_cast<sg_pixel_format>([m_defaults integerForKey:kDefaultColorPixelFormatNameKey]));
    m_preference.setPreferredEditingFPS([m_defaults integerForKey:kPreferredEditingFPSKey]);
    NSURL *bundlePluginsURL = [NSBundle mainBundle].builtInPlugInsURL;
    NSURL *pluginURL = [bundlePluginsURL URLByAppendingPathComponent:@"plugin_effect.dylib"];
    JSON_Object *root = json_object(m_config);
    json_object_dotset_string(root, "plugin.effect.path", pluginURL.path.UTF8String);
    NSError *error = nil;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSURL *cacheURL = [fileManager URLForDirectory:NSCachesDirectory
                                          inDomain:NSUserDomainMask
                                 appropriateForURL:nil
                                            create:YES
                                             error:&error];
    if (!error) {
        NSURL *effectCacheURL = [cacheURL URLByAppendingPathComponent:@"com.github.nanoem/effects"];
        [fileManager createDirectoryAtURL:effectCacheURL withIntermediateDirectories:YES attributes:nil error:&error];
        if (!error) {
            json_object_dotset_string(root, "plugin.effect.cache.path", effectCacheURL.path.UTF8String);
        }
    }
}

} /* namespace macos */
} /* namespace nanoem */
