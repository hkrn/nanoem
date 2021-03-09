/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <Foundation/Foundation.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#define TDocumentType UIDocument
#elif TARGET_OS_MAC
#import <AppKit/AppKit.h>
#define TDocumentType NSDocument
#else
#error "Unknown Platform"
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1080
typedef NS_ENUM(NSUInteger, DocumentErrorType) { DocumentErrorTypeNone };
#else
enum { DocumentErrorTypeNone };
typedef NSUInteger DocumentErrorType;
#define instancetype id
#endif

#include "emapp/Error.h"
#include "emapp/IDrawable.h"
#include "emapp/IEffect.h"
#include "nanoem/ext/parson/parson.h"

namespace nanoem {

class Accessory;
class BaseApplicationService;
class Effect;
class IProjectHolder;
class Model;
class Motion;
class Project;

namespace plugin {
class DecoderPlugin;
} /* namesace internal */

namespace osx {

class FileWrapperTextureLoader {
public:
    static FileWrapperTextureLoader *create(Accessory *accessory, NSFileWrapper *directoryFileWrapper);
    static FileWrapperTextureLoader *create(Effect *effect, NSFileWrapper *directoryFileWrapper);
    static FileWrapperTextureLoader *create(Model *model, NSFileWrapper *directoryFileWrapper);

    FileWrapperTextureLoader(NSFileWrapper *directoryWrapper);
    ~FileWrapperTextureLoader();

    void upload(const IEffect::ImageResourceParameter &parameter, IEffect *effect);

private:
    static void destroy(void *userData, const Accessory *accessory);
    static void destroy(void *userData, const Effect *effect);
    static void destroy(void *userData, const Model *model);
    void uploadSynchronized(
        IEffect *effect, const IEffect::ImageResourceParameter &parameter, const void *bytes, size_t length);
    NSFileWrapper *retrieveFileWrapper(NSString *keyPath) const;

    NSFileWrapper *m_directoryWrapper;
};

} /* namespace osx */
} /* namespace nanoem */

@interface DocumentUUID : NSObject <NSCopying> {
    CFUUIDRef m_uuid;
    NSString *m_string;
}

@property (readonly, nonatomic) CFUUIDRef UUID;
@property (readonly, nonatomic) NSString *string;

+ (BOOL)isValid:(NSString *)value;

- (instancetype)initWithCFUUID:(CFUUIDRef)uuid;
- (instancetype)initWithString:(NSString *)string;
- (instancetype)initWithUTF8String:(const char *)string;
- (id)copyWithZone:(NSZone *)zone;

@end

@interface Document : TDocumentType {
    nanoem::BaseApplicationService *m_applicationPtr;
    nanoem::IProjectHolder *m_projectHolderPtr;
    nanoem::plugin::DecoderPlugin *m_decoderPlugin;
    JSON_Value *m_json;
    NSFileWrapper *m_rootFileWrapper;
    NSMutableDictionary *m_allAccessoryDirectoryWrapperDictionary;
    NSMutableDictionary *m_allAccessoryFileWrapperDictionary;
    NSMutableDictionary *m_allModelDirectoryWrapperDictionary;
    NSMutableDictionary *m_allModelFileWrapperDictionary;
    NSMutableDictionary *m_allMotionDictionary;
    NSMutableDictionary *m_accessoryUUIDDictionary;
    NSMutableDictionary *m_modelUUIDDictionary;
    NSMutableDictionary *m_motionUUIDDictionary;
    NSMutableDictionary *m_uuidAccessoryDictionary;
    NSMutableDictionary *m_uuidModelDictionary;
    NSMutableDictionary *m_uuidMotionDictionary;
    NSMutableSet *m_allAudioURLSet;
    NSMutableSet *m_allVideoURLSet;
    DocumentUUID *m_cameraMotionUUID;
    DocumentUUID *m_lightMotionUUID;
    Document *m_lastDocument;
    DocumentErrorType m_errorType;
}

@property (readonly, nonatomic) nanoem::BaseApplicationService *application;
@property (readonly, nonatomic) nanoem::Project *project;
@property (readonly, nonatomic) NSFileWrapper *rootFileWrapper;
@property (assign, nonatomic) nanoem::plugin::DecoderPlugin *decoderPlugin;

- (instancetype)init UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithFileURL:(NSURL *)url UNAVAILABLE_ATTRIBUTE;
- (instancetype)initWithFileURL:(NSURL *)fileURL
                    application:(nanoem::BaseApplicationService *)applicationPtr
                  projectHolder:(nanoem::IProjectHolder *)projectHolderPtr;

- (BOOL)registerAccessory:(const nanoem::Accessory *)accessory uuid:(DocumentUUID *)uuid;
- (BOOL)registerModel:(const nanoem::Model *)model uuid:(DocumentUUID *)uuid;
- (BOOL)registerMotion:(const nanoem::Motion *)motion uuid:(DocumentUUID *)uuid;
- (void)removeAccessory:(const nanoem::Accessory *)accessory uuid:(DocumentUUID *)uuid;
- (void)removeModel:(const nanoem::Model *)model uuid:(DocumentUUID *)uuid;
- (void)removeMotion:(const nanoem::Motion *)motion uuid:(DocumentUUID *)uuid;
- (DocumentUUID *)resolveUUIDFromAccessory:(const nanoem::Accessory *)value;
- (DocumentUUID *)resolveUUIDFromModel:(const nanoem::Model *)value;
- (DocumentUUID *)resolveUUIDFromMotion:(const nanoem::Motion *)value;
- (nanoem::Accessory *)resolveAccessoryFromUUID:(DocumentUUID *)value;
- (nanoem::Model *)resolveModelFromUUID:(DocumentUUID *)value;
- (nanoem::Motion *)resolveMotionFromUUID:(DocumentUUID *)value;

- (void)load:(NSError **)error;
- (void)destroy;

@end
