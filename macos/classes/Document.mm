/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "Document.h"

#include "emapp/Accessory.h"
#include "emapp/Archiver.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/Effect.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/IProjectHolder.h"
#include "emapp/Model.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"
#include "glm/gtc/type_ptr.hpp"

#include "stb/stb_image.h"

#if TARGET_OS_MAC && !TARGET_OS_IOS
#import "./CommonPolyfill.h"
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
#import <AVFoundation/AVFoundation.h>
#else
#import <QTKit/QTKit.h>
#endif /* MAC_OS_X_VERSION_MAX_ALLOWED */
#else
#define nanoem_objc_domain(name) @"com.github.nanoem." #name
#endif /* TARGET_OS_MAC */

namespace {

using namespace nanoem;

class JSONUtils {
public:
    static inline JSON_Value *
    writeVector3(const Vector3 &value)
    {
        JSON_Value *v = json_value_init_array();
        JSON_Array *a = json_array(v);
        json_array_append_number(a, value.x);
        json_array_append_number(a, value.y);
        json_array_append_number(a, value.z);
        return v;
    }
    static inline JSON_Value *
    writeVector4(const Vector4 &value)
    {
        JSON_Value *v = json_value_init_array();
        JSON_Array *a = json_array(v);
        json_array_append_number(a, value.x);
        json_array_append_number(a, value.y);
        json_array_append_number(a, value.z);
        json_array_append_number(a, value.w);
        return v;
    }
    static inline JSON_Value *
    writeQuaternion(const Vector4 &value)
    {
        JSON_Value *v = json_value_init_array();
        JSON_Array *a = json_array(v);
        json_array_append_number(a, value.x);
        json_array_append_number(a, value.y);
        json_array_append_number(a, value.z);
        json_array_append_number(a, value.w);
        return v;
    }
    static inline Vector3
    readVector3(const JSON_Array *value)
    {
        Vector3 v;
        if (json_array_get_count(value) >= 3) {
            v.x = Vector3::value_type(json_array_get_number(value, 0));
            v.y = Vector3::value_type(json_array_get_number(value, 1));
            v.z = Vector3::value_type(json_array_get_number(value, 2));
        }
        return v;
    }
    static inline Vector4
    readVector4(const JSON_Array *value)
    {
        Vector4 v;
        if (json_array_get_count(value) >= 4) {
            v.x = Vector3::value_type(json_array_get_number(value, 0));
            v.y = Vector3::value_type(json_array_get_number(value, 1));
            v.z = Vector3::value_type(json_array_get_number(value, 2));
            v.w = Vector3::value_type(json_array_get_number(value, 3));
        }
        return v;
    }
    static inline Quaternion
    readQuaternion(const JSON_Array *value)
    {
        Quaternion v;
        if (json_array_get_count(value) >= 4) {
            v.x = Quaternion::value_type(json_array_get_number(value, 0));
            v.y = Quaternion::value_type(json_array_get_number(value, 1));
            v.z = Quaternion::value_type(json_array_get_number(value, 2));
            v.w = Quaternion::value_type(json_array_get_number(value, 3));
        }
        return v;
    }
    static inline bool
    booleanValue(const JSON_Value *root, const char *name, bool defv)
    {
        int value = json_object_dotget_boolean(json_value_get_object(root), name);
        return value < 0 ? defv : value > 0;
    }

private:
    JSONUtils();
    ~JSONUtils();
};

class Allocator : public bx::AllocatorI {
    void *
    realloc(void *ptr, size_t size, size_t align, const char *file, uint32_t line)
    {
        BX_UNUSED_3(align, file, line)
        void *p = 0;
        if (size == 0 && ptr != 0) {
            ::free(ptr);
        }
        else if (ptr == 0) {
            p = ::malloc(size);
        }
        else {
            p = ::realloc(ptr, size);
        }
        return p;
    }
};

class NSDataReader : public ISeekableReader {
public:
    NSDataReader(NSData *data)
        : m_data(data)
        , m_offset(0)
    {
    }
    ~NSDataReader()
    {
    }

    int32_t
    read(void *data, int32_t size, Error & /* error */) override
    {
        [m_data getBytes:data range:NSMakeRange(m_offset, size)];
        m_offset += size;
        return size;
    }
    nanoem_rsize_t
    size() override
    {
        return m_data.length;
    }
    int64_t
    seek(int64_t offset, SeekType whence, Error & /* error */) override
    {
        switch (whence) {
        case kSeekTypeBegin: {
            m_offset = offset;
            break;
        }
        case kSeekTypeCurrent: {
            m_offset += offset;
            break;
        }
        case kSeekTypeEnd: {
            m_offset = glm::min(int64_t(m_data.length) + offset, int64_t(m_data.length));
            break;
        }
        default:
            break;
        }
        return m_offset;
    }

private:
    NSData *m_data;
    int64_t m_offset;
};

} /* namespace anonymous */

namespace nanoem {
namespace osx {

static NSString *kPMDExtension = @"pmd";
static NSString *kPMXExtension = @"pmx";
static NSString *kNMDExtension = @"nmd";
static NSString *kVMDExtension = @"vmd";
static NSString *kXExtension = @"x";
static NSString *kLoadingProjectErrorDomain = nanoem_objc_domain("LoadingProjectErrorDomain");

static NSError *
createNSError(const Error &error)
{
    NSError *err = nil;
    if (error.hasReason()) {
        NSString *reason = [NSString stringWithFormat:@"Loading project error (status=%d): %s",
                                     Inline::saturateInt32(error.code()), error.reasonConstString()];
        NSString *suggestion = [NSString stringWithUTF8String:error.recoverySuggestionConstString()];
        NSDictionary *userInfo =
            [NSDictionary dictionaryWithObjectsAndKeys:suggestion, NSLocalizedRecoverySuggestionErrorKey, reason,
                          NSLocalizedFailureReasonErrorKey, nil];
        err = [[NSError alloc] initWithDomain:NSCocoaErrorDomain code:error.code() userInfo:userInfo];
    }
    return err;
}

} /* namespace osx */
} /* namespace nanoem */

using namespace nanoem;
using namespace osx;

@implementation DocumentUUID

@synthesize UUID = m_uuid;
@synthesize string = m_string;

+ (BOOL)isValid:(NSString *)value
{
    CFUUIDRef result = CFUUIDCreateFromString(kCFAllocatorDefault, (__bridge CFStringRef) value);
    BOOL ok = result != NULL;
    CFRelease(result);
    return ok;
}

- (instancetype)init
{
    if (self = [super init]) {
        m_uuid = CFUUIDCreate(kCFAllocatorDefault);
        m_string = (__bridge_transfer NSString *) CFUUIDCreateString(kCFAllocatorDefault, m_uuid);
    }
    return self;
}

- (instancetype)initWithCFUUID:(CFUUIDRef)value
{
    if (self = [super init]) {
        m_uuid = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, CFUUIDGetUUIDBytes(value));
        m_string = (__bridge_transfer NSString *) CFUUIDCreateString(kCFAllocatorDefault, m_uuid);
    }
    return self;
}

- (instancetype)initWithString:(NSString *)string
{
    if (self = [super init]) {
        m_uuid = CFUUIDCreateFromString(kCFAllocatorDefault, (__bridge CFStringRef) string);
        m_string = (__bridge_transfer NSString *) CFUUIDCreateString(kCFAllocatorDefault, m_uuid);
    }
    return self;
}

- (instancetype)initWithUTF8String:(const char *)string
{
    if (self = [super init]) {
        CFStringRef uuidString = CFStringCreateWithCString(kCFAllocatorDefault, string, kCFStringEncodingUTF8);
        m_uuid = CFUUIDCreateFromString(kCFAllocatorDefault, uuidString);
        CFRelease(uuidString);
        m_string = (__bridge_transfer NSString *) CFUUIDCreateString(kCFAllocatorDefault, m_uuid);
    }
    return self;
}

- (NSString *)description
{
    return self.string;
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[DocumentUUID allocWithZone:zone] initWithCFUUID:m_uuid];
}

- (void)dealloc
{

    CFRelease(m_uuid);
    m_uuid = NULL;
}

- (BOOL)isEqual:(id)object
{
    BOOL result = NO;
    if ([object isKindOfClass:[DocumentUUID class]]) {
        DocumentUUID *uuid = static_cast<DocumentUUID *>(object);
        result = [m_string isEqualToString:uuid.string];
    }
    return result;
}

- (NSUInteger)hash
{
    return [m_string hash];
}

@end

@interface
Document ()

+ (NSFileWrapper *)newFileWrapperWithFileURL:(const URI &)fileURL error:(NSError **)error;
+ (DocumentUUID *)newUUIDFromJson:(const JSON_Object *)root forKey:(const char *)key;
+ (void)setUUID:(DocumentUUID *)uuid toJson:(JSON_Object *)root forKey:(const char *)key;
+ (JSON_Value *)addJsonObject:(JSON_Value *)target uuid:(DocumentUUID *)uuid filename:(NSString *)filename;
+ (NSFileWrapper *)newModelFileWrapper:(Model *)model withFilePath:(NSString *)filePath error:(NSError **)outError;

typedef NSString * (^SavingMotionCallback)(Motion *, ByteArray &);
- (NSFileWrapper *)saveMotion:(Motion *)motion
                       parent:(NSFileWrapper *)parent
                        block:(SavingMotionCallback)savingMotionCallbackApply;
- (void)aggregateEffectSources:(NSMutableDictionary *)effectSourceFileWrappers
                    pathPrefix:(NSMutableString *)pathPrefix
              directoryWrapper:(NSFileWrapper *)directoryWrapper
                    extensions:(NSSet *)extensions;

- (void)createFileWrapperError:(NSError **)outError;
- (void)serializeAccessorySettingAsJson:(JSON_Object *)root;
- (void)serializeModelSettingAsJson:(JSON_Object *)root;
- (void)serializeDrawableOrderSettingAsJson:(JSON_Object *)root;
- (void)serializeTransformOrderSettingAsJson:(JSON_Object *)root;
- (void)serializeCameraAsJson:(JSON_Object *)root target:(const ICamera *)camera;
- (void)serializeLightAsJson:(JSON_Object *)root target:(const ILight *)light;
- (void)serializeGlobalCameraAsJson:(JSON_Object *)root;
- (void)serializeGlobalLightAsJson:(JSON_Object *)root;
- (void)serializeMotionSettingAsJson:(JSON_Object *)root;

- (void)setFileWrapper:(NSFileWrapper *)textureFileWrapper
               keyPath:(NSString *)keyPath
                parent:(NSFileWrapper *)parentFileWrapper;
- (NSFileWrapper *)addDirectoryFileWrapper:(NSString *)name parent:(NSFileWrapper *)parentFileWrapper;
- (NSFileWrapper *)addEffectSourceFileWrapper:(NSString *)keyPath
                          parentEffectFileURI:(const URI &)fileURI
                                       parent:(NSFileWrapper *)parentFileWrapper;
- (NSFileWrapper *)addTextureFileWrapper:(NSString *)keyPath
                                 fileURI:(const URI &)fileURI
                                  parent:(NSFileWrapper *)parentFileWrapper;
- (void)addAllEffectResourceFileWrappers:(const Effect *)effect
                                 fileURI:(const URI &)fileURI
                                  parent:(NSFileWrapper *)parentFileWrapper;
- (NSFileWrapper *)addEffectFileWrapper:(const IDrawable *)drawable parent:(NSFileWrapper *)parentFileWrapper;
- (NSFileWrapper *)addAccessoryFileWrapper:(const Accessory *)accessory
                                    parent:(NSFileWrapper *)parent
                                     error:(NSError **)outError;
- (NSFileWrapper *)addModelFileWrapper:(Model *)model parent:(NSFileWrapper *)parent error:(NSError **)outError;
- (NSFileWrapper *)addMotionFileWrapper:(Motion *)motion
                              accessory:(const Accessory *)accessory
                                 parent:(NSFileWrapper *)parent;
- (NSFileWrapper *)addMotionFileWrapper:(Motion *)motion model:(const Model *)model parent:(NSFileWrapper *)parent;
- (NSFileWrapper *)addCameraMotionFileWrapperWithParent:(NSFileWrapper *)parent firstSave:(BOOL)isFirstSave;
- (NSFileWrapper *)addLightMotionFileWrapperWithParent:(NSFileWrapper *)parent firstSave:(BOOL)isFirstSave;
- (NSFileWrapper *)addDataFileWrapper:(NSData *)data filename:(NSString *)filename parent:(NSFileWrapper *)parent;
- (NSFileWrapper *)addProjectJsonFileWrapper:(NSFileWrapper *)parent;

- (void)setAccessoryFileWrapper:(NSFileWrapper *)accessoryFileWrapper
                           uuid:(DocumentUUID *)uuid
           fromDirectoryWrapper:(NSFileWrapper *)accessoryDirectoryWrapper;
- (void)setModelFileWrapper:(NSFileWrapper *)modelFileWrapper
                       uuid:(DocumentUUID *)uuid
       fromDirectoryWrapper:(NSFileWrapper *)modelDirectoryWrapper;
- (BOOL)readAccessoryFromFileWrapper:(NSDictionary *)rootDirectoryWrapper error:(NSError **)outError;
- (BOOL)readModelFromFileWrapper:(NSDictionary *)rootDirectoryWrapper error:(NSError **)outError;
- (BOOL)readMotionFromFileWrapper:(NSDictionary *)rootDirectoryWrapper error:(NSError **)outError;

- (void)loadAllAccessories:(NSError **)error;
- (void)loadAllModels:(NSError **)error;
- (void)loadAllMotions:(NSError **)error;
- (void)loadAllAudios:(NSError **)error;
- (void)loadAllVideos:(NSError **)error;
- (void)configureAllModels:(const JSON_Array *)root;
- (void)configureAllAccessories:(const JSON_Array *)root;
- (void)configureDrawOrder:(const JSON_Array *)root;
- (void)configureTransformOrder:(const JSON_Array *)root;
- (void)resetCameraMotion;
- (void)configureCamera:(const JSON_Object *)root target:(ICamera *)camera;
- (void)configureCamera:(const JSON_Object *)root;
- (void)resetLightMotion;
- (void)configureLight:(const JSON_Object *)root target:(ILight *)light;
- (void)configureLight:(const JSON_Object *)root;

- (BOOL)loadDocument:(NSFileWrapper *)fileWrapper ofType:(NSString *)typeName error:(NSError **)outError;
- (NSFileWrapper *)saveDocument:(NSString *)typeName error:(NSError **)outError;

#if TARGET_OS_IPHONE
- (BOOL)loadFromContents:(id)contents ofType:(NSString *)typeName error:(NSError **)outError;
- (id)contentsForType:(NSString *)typeName error:(NSError **)outError;
#else
- (BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper ofType:(NSString *)typeName error:(NSError **)outError;
- (NSFileWrapper *)fileWrapperOfType:(NSString *)typeName error:(NSError **)outError;
#endif /* TARGET_OS_IPHONE */

@end

@implementation Document

@synthesize application = m_applicationPtr;
@synthesize rootFileWrapper = m_rootFileWrapper;
@synthesize decoderPlugin = m_decoderPlugin;

+ (NSSet *)supportedMediaExtensions:(CFStringRef)requiredType
{
#ifdef AVF_EXPORT
    NSArray *supportedUTITypes = [AVURLAsset audiovisualTypes];
    NSMutableSet *supportedExtensions = [NSMutableSet setWithCapacity:supportedUTITypes.count];
    for (NSString *supportedUTI in supportedUTITypes) {
        if (UTTypeConformsTo((__bridge CFStringRef) supportedUTI, requiredType)) {
            if (CFStringRef cfext = UTTypeCopyPreferredTagWithClass(
                    (__bridge CFStringRef) supportedUTI, kUTTagClassFilenameExtension)) {
                NSString *extension = (__bridge NSString *) cfext;
                if (extension != nil) {
                    [supportedExtensions addObject:extension];
                }
                CFRelease(cfext);
            }
        }
    }
#elif 0
    NSArray *supportedFileTypes = [QTMovie movieFileTypes:QTIncludeCommonTypes];
    NSMutableSet *supportedExtensions = [NSMutableSet setWithCapacity:supportedFileTypes.count];
    for (NSString *supportedFileType in supportedFileTypes) {
        if (CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(
                kUTTagClassFilenameExtension, (__bridge CFStringRef) supportedFileType),
            NULL) {
            if (UTTypeConformsTo(uti, requiredType)) {
                NSString *extension = (__bridge NSString *) supportedFileType;
                if (extension != nil) {
                    [supportedExtensions addObject:extension];
                }
            }
            CFRelease(uti);
        }
    }
#else
    NSSet *supportedExtensions = [[NSSet alloc] init];
#endif /* AVF_EXPORT */
    return supportedExtensions;
}

+ (NSFileWrapper *)newFileWrapperWithFileURL:(const URI &)fileURI error:(NSError **)outError
{
    NSFileWrapper *fileWrapper = nil;
    if (Project::isArchiveURI(fileURI)) {
        FileReaderScope scope(nullptr);
        Error err;
        if (scope.open(fileURI, err)) {
            Archiver archiver(scope.reader());
            if (archiver.open(err)) {
                Archiver::Entry entry;
                ByteArray bytes;
                if (archiver.findEntry(fileURI.fragment(), entry, err) && archiver.extract(entry, bytes, err)) {
                    NSData *data = [[NSData alloc] initWithBytes:bytes.data() length:bytes.size()];
                    fileWrapper = [[NSFileWrapper alloc] initRegularFileWithContents:data];
                    NSString *filename =
                        [[NSString alloc] initWithUTF8String:URI::lastPathComponent(fileURI.fragment()).c_str()];
                    fileWrapper.preferredFilename = filename;
                }
                archiver.close(err);
            }
        }
    }
    else {
        NSString *filePath = [[NSString alloc] initWithUTF8String:fileURI.absolutePath().c_str()];
        NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
        fileWrapper = [[NSFileWrapper alloc] initWithURL:fileURL options:NSFileWrapperReadingImmediate error:outError];
        if (fileWrapper.symbolicLink) {
            NSData *data = [[NSData alloc] initWithContentsOfURL:fileURL options:NSDataReadingUncached error:outError];
            if (data) {
                fileWrapper = [[NSFileWrapper alloc] initRegularFileWithContents:data];
                fileWrapper.preferredFilename = fileURL.lastPathComponent;
            }
        }
    }
    return fileWrapper;
}

+ (DocumentUUID *)newUUIDFromJson:(const JSON_Object *)root forKey:(const char *)key
{
    DocumentUUID *uuid = nil;
    if (const char *value = json_object_dotget_string(root, key)) {
        uuid = [[DocumentUUID alloc] initWithUTF8String:value];
    }
    return uuid;
}

+ (void)setUUID:(DocumentUUID *)uuid toJson:(JSON_Object *)root forKey:(const char *)key
{
    json_object_dotset_string(root, key, uuid.string.UTF8String);
}

+ (JSON_Value *)addJsonObject:(JSON_Value *)target uuid:(DocumentUUID *)uuid filename:(NSString *)filename
{
    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_object(value);
    json_object_set_string(object, "uuid", uuid.string.UTF8String);
    json_object_set_string(object, "filename", filename.UTF8String);
    json_array_append_value(json_array(target), value);
    return value;
}

+ (NSFileWrapper *)newModelFileWrapper:(Model *)model withFilePath:(NSString *)filePath error:(NSError **)outError
{
    ByteArray bytes;
    Error error;
    NSFileWrapper *modelFileWrapper = nil;
    if (model->save(bytes, error)) {
        NSData *data = [[NSData alloc] initWithBytes:bytes.data() length:bytes.size()];
        modelFileWrapper = [[NSFileWrapper alloc] initRegularFileWithContents:data];
        modelFileWrapper.preferredFilename = filePath.lastPathComponent;
    }
    else if (outError) {
        *outError = createNSError(error);
    }
    return modelFileWrapper;
}

- (NSFileWrapper *)saveMotion:(Motion *)motion
                       parent:(NSFileWrapper *)parent
                        block:(SavingMotionCallback)savingMotionCallbackApply
{
    ByteArray bytes;
    DocumentUUID *motionUUID = [self resolveUUIDFromMotion:motion];
    if (!motionUUID) {
        motionUUID = [[DocumentUUID alloc] init];
        [self registerMotion:motion uuid:motionUUID];
    }
    NSFileWrapper *motionFileWrapper = [[parent fileWrappers] objectForKey:motionUUID.string];
    if (motionFileWrapper && !motion->isDirty()) {
        return motionFileWrapper;
    }
    NSFileWrapper *motionDirectoryWrapper = [self addDirectoryFileWrapper:motionUUID.string parent:parent];
    motionFileWrapper = [m_allMotionDictionary objectForKey:motionUUID];
    if (motionFileWrapper) {
        [motionDirectoryWrapper removeFileWrapper:motionFileWrapper];
    }
    motion->setFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
    NSString *filename = savingMotionCallbackApply(motion, bytes);
    NSData *data = [[NSData alloc] initWithBytes:bytes.data() length:bytes.size()];
    motionFileWrapper = [self addDataFileWrapper:data
                                        filename:[filename stringByAppendingPathExtension:kNMDExtension]
                                          parent:motionDirectoryWrapper];

    motion->setDirty(false);
    [m_allMotionDictionary setObject:motionFileWrapper forKey:motionUUID];
    return motionFileWrapper;
}

- (void)loadEffectBinaryFromData:(NSData *)data
                     baseFileURL:(NSURL *)fileURL
                directoryWrapper:(NSFileWrapper *)directoryWrapper
             drawableFileWrapper:(NSFileWrapper *)fileWrapper
                        drawable:(IDrawable *)drawable
                           error:(NSError **)outError
{
    NSString *fileWrapperFilename = fileWrapper.preferredFilename;
    Project *projectPtr = m_projectHolderPtr->currentProject();
    Effect *effect = projectPtr->createEffect();
    Error error;
    Progress progress(projectPtr, 0);
    effect->setName(fileWrapperFilename.UTF8String);
    if (effect->load(static_cast<const uint8_t *>(data.bytes), data.length, progress, error)) {
        NSString *filename = fileURL.path;
        filename = [filename stringByAppendingPathComponent:directoryWrapper.filename];
        filename = [filename stringByAppendingPathComponent:fileWrapperFilename];
        effect->setFileURI(URI::createFromFilePath(filename.UTF8String));
        if (effect->upload(effect::kAttachmentTypeNone, progress, error)) {
            NSDictionary *fileWrappers = [directoryWrapper fileWrappers];
            Project::IncludeEffectSourceMap effectSourceMap;
            NSString *fileWrapperFilenameWithoutExtension = [fileWrapperFilename stringByDeletingPathExtension];
            for (NSString *filename in fileWrappers) {
                NSString *filenameWithoutExtension = [filename.lastPathComponent stringByDeletingPathExtension];
                if (![fileWrapperFilenameWithoutExtension isEqualToString:filenameWithoutExtension]) {
                    NSFileWrapper *childFileWrapper = [fileWrappers objectForKey:filename];
                    if (childFileWrapper.regularFile) {
                        NSData *content = childFileWrapper.regularFileContents;
                        const nanoem_u8_t *ptr = static_cast<const nanoem_u8_t *>(content.bytes);
                        const ByteArray bytes(ptr, ptr + content.length);
                        effectSourceMap.insert(
                            tinystl::make_pair(String(filename.lastPathComponent.UTF8String), bytes));
                    }
                }
            }
            projectPtr->attachActiveEffect(drawable, effect, effectSourceMap, progress, error);
            if (outError) {
                *outError = createNSError(error);
            }
        }
        else if (outError) {
            *outError = createNSError(error);
        }
    }
    else {
        projectPtr->destroyEffect(effect);
        if (outError) {
            *outError = createNSError(error);
        }
    }
}

- (void)aggregateEffectSources:(NSMutableDictionary *)effectSourceFileWrappers
                    pathPrefix:(NSMutableString *)pathPrefix
              directoryWrapper:(NSFileWrapper *)directoryWrapper
                    extensions:(NSSet *)extensions
{
    NSDictionary *fileWrappers = directoryWrapper.fileWrappers;
    for (NSString *key in fileWrappers) {
        NSFileWrapper *fileWrapper = [fileWrappers objectForKey:key];
        if (fileWrapper.directory) {
            [pathPrefix appendString:fileWrapper.filename];
            [pathPrefix appendString:@"/"];
            [self aggregateEffectSources:effectSourceFileWrappers
                              pathPrefix:pathPrefix
                        directoryWrapper:fileWrapper
                              extensions:extensions];
        }
        else if (fileWrapper.regularFile && [extensions containsObject:fileWrapper.filename.pathExtension]) {
            NSString *key = [[NSString alloc] initWithFormat:@"%@%@", pathPrefix, fileWrapper.filename];
            [effectSourceFileWrappers setValue:fileWrapper forKey:key];
        }
    }
}

- (void)createEffectFromFileWrapper:(NSFileWrapper *)fileWrapper
                        baseFileURL:(NSURL *)fileURL
                   directoryWrapper:(NSFileWrapper *)directoryWrapper
                           drawable:(IDrawable *)drawable
                              error:(NSError **)outError
{
    bool succeeded = false;
    if (m_projectHolderPtr->currentProject()->isEffectPluginEnabled()) {
        IFileManager *fileManager = m_applicationPtr->fileManager();
        if (plugin::EffectPlugin *plugin = fileManager->sharedEffectPlugin()) {
            PluginFactory::EffectPluginProxy proxy(plugin);
            const StringList &extensionsList = proxy.availableExtensions();
            NSMutableSet *extensionsSet = [[NSMutableSet alloc] init];
            for (StringList::const_iterator it = extensionsList.begin(), end = extensionsList.end(); it != end; ++it) {
                NSString *extension = [[NSString alloc] initWithUTF8String:it->c_str()];
                [extensionsSet addObject:extension];
            }
            NSMutableDictionary *effectSourceFileWrappers = [[NSMutableDictionary alloc] init];
            NSMutableString *pathPrefix = [[NSMutableString alloc] initWithString:@""];
            [self aggregateEffectSources:effectSourceFileWrappers
                              pathPrefix:pathPrefix
                        directoryWrapper:directoryWrapper
                              extensions:extensionsSet];

            for (StringList::const_iterator it = extensionsList.begin(), end = extensionsList.end(); it != end; ++it) {
                const URI &effectFileURL =
                    URI::createFromFilePath(Effect::resolveFilePath(fileWrapper.filename.UTF8String, it->c_str()));
                NSString *absoluteFilePath = [[NSString alloc] initWithUTF8String:effectFileURL.absolutePath().c_str()];
                if (NSFileWrapper *effectFileWrapper = [[directoryWrapper fileWrappers]
                        objectForKey:[absoluteFilePath precomposedStringWithCanonicalMapping]]) {
                    for (NSString *path in effectSourceFileWrappers) {
                        NSFileWrapper *fileWrapper = [effectSourceFileWrappers objectForKey:path];
                        NSData *data = fileWrapper.regularFileContents;
                        const nanoem_u8_t *ptr = static_cast<const nanoem_u8_t *>(data.bytes);
                        const ByteArray source(ptr, ptr + data.length);
                        proxy.addIncludeSource(path.UTF8String, source);
                    }
                    NSData *data = effectFileWrapper.regularFileContents;
                    ByteArray output;
                    const String originSource(static_cast<const char *>(data.bytes), data.length);
                    if (proxy.compile(originSource, output)) {
                        NSData *bytes = [[NSData alloc] initWithBytes:output.data() length:output.size()];
                        [self loadEffectBinaryFromData:bytes
                                           baseFileURL:fileURL
                                      directoryWrapper:directoryWrapper
                                   drawableFileWrapper:fileWrapper
                                              drawable:drawable
                                                 error:outError];
                        succeeded = outError == nil;
                        break;
                    }
                }
            }
        }
    }
    if (!succeeded) {
        const URI &effectFileURL = URI::createFromFilePath(
            Effect::resolveFilePath(fileWrapper.filename.UTF8String, Effect::kBinaryFileExtension));
        NSString *absoluteFilePath = [[NSString alloc] initWithUTF8String:effectFileURL.absolutePath().c_str()];
        if (NSFileWrapper *effectFileWrapper = [[directoryWrapper fileWrappers]
                objectForKey:[absoluteFilePath precomposedStringWithCanonicalMapping]]) {
            NSData *data = effectFileWrapper.regularFileContents;
            [self loadEffectBinaryFromData:data
                               baseFileURL:fileURL
                          directoryWrapper:directoryWrapper
                       drawableFileWrapper:fileWrapper
                                  drawable:drawable
                                     error:outError];
        }
    }
}

- (Accessory *)readAccessoryFromFileWrapper:(NSFileWrapper *)fileWrapper
                           directoryWrapper:(NSFileWrapper *)directoryWrapper
                                       uuid:(DocumentUUID *)uuid
                                      error:(NSError **)outError
{
    Accessory *accessory = [self resolveAccessoryFromUUID:uuid];
    if (!accessory && fileWrapper.regularFile) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        accessory = projectPtr->createAccessory();
        NSData *data = fileWrapper.regularFileContents;
        NSString *suffix =
            [NSString stringWithFormat:@"/Accessory/%@/%@", directoryWrapper.filename, fileWrapper.filename];
        NSString *path = [self.fileURL.path stringByAppendingString:suffix];
        Progress progress(projectPtr, 0);
        Error error;
        accessory->setFileURI(URI::createFromFilePath(path.UTF8String));
        if (accessory->load(static_cast<const nanoem_u8_t *>(data.bytes), data.length, error) &&
            [self registerAccessory:accessory uuid:uuid]) {
            accessory->upload();
            accessory->loadAllImages(progress, error);
            [self createEffectFromFileWrapper:fileWrapper
                                  baseFileURL:[self.fileURL URLByAppendingPathComponent:@"Accessory"]
                             directoryWrapper:directoryWrapper
                                     drawable:accessory
                                        error:outError];
            accessory->setName(fileWrapper.preferredFilename.UTF8String);
            accessory->setFileURI(URI::createFromFilePath(self.fileURL.path.UTF8String, suffix.UTF8String));
            projectPtr->addAccessory(accessory);
        }
        else {
            projectPtr->destroyAccessory(accessory);
            accessory = 0;
            if (outError) {
                *outError = createNSError(error);
            }
        }
    }
    return accessory;
}

- (Model *)readModelFromFileWrapper:(NSFileWrapper *)fileWrapper
                   directoryWrapper:(NSFileWrapper *)directoryWrapper
                               uuid:(DocumentUUID *)uuid
                              error:(NSError **)outError
{
    Model *model = [self resolveModelFromUUID:uuid];
    if (!model && fileWrapper.regularFile) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        model = projectPtr->createModel();
        NSData *data = fileWrapper.regularFileContents;
        NSString *suffix = [NSString stringWithFormat:@"/Model/%@/%@", directoryWrapper.filename, fileWrapper.filename];
        NSString *path = [self.fileURL.path stringByAppendingString:suffix];
        Error error;
        model->setFileURI(URI::createFromFilePath(path.UTF8String));
        if (model->load(static_cast<const nanoem_u8_t *>(data.bytes), data.length, error) &&
            [self registerModel:model uuid:uuid]) {
            model->setupAllBindings();
            Progress progress(projectPtr, model->createAllImages());
            model->upload();
            model->loadAllImages(progress, error);
            [self createEffectFromFileWrapper:fileWrapper
                                  baseFileURL:[self.fileURL URLByAppendingPathComponent:@"Model"]
                             directoryWrapper:directoryWrapper
                                     drawable:model
                                        error:outError];
            model->setFileURI(URI::createFromFilePath(self.fileURL.path.UTF8String, suffix.UTF8String));
            model->setDirty(false);
            projectPtr->addModel(model);
        }
        else {
            projectPtr->destroyModel(model);
            model = 0;
            if (outError) {
                *outError = createNSError(error);
            }
        }
    }
    return model;
}

- (Motion *)readMotionFromFileWrapper:(NSFileWrapper *)fileWrapper uuid:(DocumentUUID *)uuid error:(NSError **)outError
{
    Project *projectPtr = m_projectHolderPtr->currentProject();
    NSData *data = fileWrapper.regularFileContents;
    Motion *motion = projectPtr->createMotion();
    motion->setFormat(fileWrapper.preferredFilename.pathExtension.UTF8String);
    Error error;
    bool loaded = motion->load(static_cast<const nanoem_u8_t *>(data.bytes), data.length, 0, error);
    if (!(loaded && [self registerMotion:motion uuid:uuid])) {
        projectPtr->destroyMotion(motion);
        motion = 0;
        if (outError) {
            *outError = createNSError(error);
        }
    }
    return motion;
}

- (void)createFileWrapperError:(NSError **)outError
{
    if (outError != nil) {
        NSDictionary *userInfo =
            [[NSDictionary alloc] initWithObjectsAndKeys:@"", NSLocalizedRecoverySuggestionErrorKey, nil];
        *outError = [[NSError alloc] initWithDomain:NSPOSIXErrorDomain code:ENOTDIR userInfo:userInfo];
    }
}

- (void)serializeAccessorySettingAsJson:(JSON_Object *)root
{
    JSON_Value *accessories = json_value_init_array();
    for (DocumentUUID *uuid in m_allAccessoryFileWrapperDictionary) {
        NSFileWrapper *fileWrapper = [m_allAccessoryFileWrapperDictionary objectForKey:uuid];
        [Document addJsonObject:accessories uuid:uuid filename:fileWrapper.preferredFilename];
    }
    json_object_dotset_value(root, "project.accessories", accessories);
}

- (void)serializeModelSettingAsJson:(JSON_Object *)root
{
    JSON_Value *models = json_value_init_array();
    for (DocumentUUID *uuid in m_allModelFileWrapperDictionary) {
        NSFileWrapper *fileWrapper = [m_allModelFileWrapperDictionary objectForKey:uuid];
        [Document addJsonObject:models uuid:uuid filename:fileWrapper.preferredFilename];
    }
    json_object_dotset_value(root, "project.models", models);
}

- (void)serializeDrawableOrderSettingAsJson:(JSON_Object *)root
{
    const Project::DrawableList *allDrawables = m_projectHolderPtr->currentProject()->drawableOrderList();
    JSON_Value *drawables = json_value_init_array();
    for (Project::DrawableList::const_iterator it = allDrawables->begin(), end = allDrawables->end(); it != end; ++it) {
        const IDrawable *drawable = *it;
        NSValue *value = [NSValue valueWithPointer:drawable];
        if (DocumentUUID *uuid = [m_modelUUIDDictionary objectForKey:value]) {
            json_array_append_string(json_array(drawables), uuid.string.UTF8String);
        }
        else if (DocumentUUID *uuid = [m_accessoryUUIDDictionary objectForKey:value]) {
            json_array_append_string(json_array(drawables), uuid.string.UTF8String);
        }
    }
    json_object_dotset_value(root, "project.order.draw", drawables);
}

- (void)serializeTransformOrderSettingAsJson:(JSON_Object *)root
{
    const Project::ModelList *allModels = m_projectHolderPtr->currentProject()->transformOrderList();
    JSON_Value *transformables = json_value_init_array();
    for (Project::ModelList::const_iterator it = allModels->begin(), end = allModels->end(); it != end; ++it) {
        const Model *model = *it;
        if (DocumentUUID *uuid = [self resolveUUIDFromModel:model]) {
            json_array_append_string(json_array(transformables), uuid.string.UTF8String);
        }
    }
    json_object_dotset_value(root, "project.order.transform", transformables);
}

- (void)serializeCameraAsJson:(JSON_Object *)root target:(const ICamera *)camera
{
    json_object_set_value(root, "angle", JSONUtils::writeVector3(camera->angle()));
    json_object_set_value(root, "lookAt", JSONUtils::writeVector3(camera->lookAt()));
    json_object_set_number(root, "distance", camera->distance());
    json_object_set_number(root, "fov", camera->fov());
    json_object_set_boolean(root, "perspective", camera->isPerspective());
}

- (void)serializeLightAsJson:(JSON_Object *)root target:(const ILight *)light
{
    json_object_set_value(root, "color", JSONUtils::writeVector3(light->color()));
    json_object_set_value(root, "direction", JSONUtils::writeVector3(light->direction()));
}

- (void)serializeGlobalCameraAsJson:(JSON_Object *)root
{
    Project *projectPtr = m_projectHolderPtr->currentProject();
    JSON_Value *camera = json_value_init_object();
    json_object_set_boolean(json_object(camera), "shared", projectPtr->isCameraShared());
    json_object_set_string(json_object(camera), "motion", m_cameraMotionUUID.string.UTF8String);
    [self serializeCameraAsJson:json_object(camera) target:projectPtr->globalCamera()];
    json_object_dotset_value(root, "project.camera", camera);
}

- (void)serializeGlobalLightAsJson:(JSON_Object *)root
{
    JSON_Value *light = json_value_init_object();
    [self serializeLightAsJson:json_object(light) target:m_projectHolderPtr->currentProject()->globalLight()];
    json_object_dotset_string(json_object(light), "motion", m_lightMotionUUID.string.UTF8String);
    json_object_dotset_value(root, "project.light", light);
}

- (void)serializeMotionSettingAsJson:(JSON_Object *)root
{
    JSON_Value *motions = json_value_init_array();
    for (DocumentUUID *uuid in m_allMotionDictionary) {
        NSFileWrapper *fileWrapper = [m_allMotionDictionary objectForKey:uuid];
        [Document addJsonObject:motions uuid:uuid filename:fileWrapper.preferredFilename];
    }
    json_object_dotset_value(root, "project.motions", motions);
}

- (void)setFileWrapper:(NSFileWrapper *)fileWrapper
               keyPath:(NSString *)keyPath
                parent:(NSFileWrapper *)parentFileWrapper
{
    NSString *standardizedKeyPath = [keyPath stringByStandardizingPath];
    NSArray *keyPathComponents = standardizedKeyPath.pathComponents;
    if (keyPathComponents.count > 1) {
        NSFileWrapper *directoryWrapper = parentFileWrapper;
        int depth = int(keyPathComponents.count) - 1;
        for (int i = 0; i < depth; i++) {
            NSString *directory = [keyPathComponents objectAtIndex:i];
            directoryWrapper = [self addDirectoryFileWrapper:directory parent:directoryWrapper];
        }
        fileWrapper.preferredFilename = standardizedKeyPath.lastPathComponent;
        [directoryWrapper addFileWrapper:fileWrapper];
    }
    else {
        fileWrapper.preferredFilename = standardizedKeyPath;
        [parentFileWrapper addFileWrapper:fileWrapper];
    }
}

- (NSFileWrapper *)addDirectoryFileWrapper:(NSString *)name parent:(NSFileWrapper *)parentFileWrapper
{
    NSFileWrapper *directoryWrapper = [[parentFileWrapper fileWrappers] objectForKey:name];
    if (!directoryWrapper) {
        directoryWrapper = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:[NSDictionary dictionary]];
        directoryWrapper.preferredFilename = name;
        [parentFileWrapper addFileWrapper:directoryWrapper];
    }
    return directoryWrapper;
}

- (NSFileWrapper *)addEffectSourceFileWrapper:(NSString *)keyPath
                          parentEffectFileURI:(const URI &)fileURI
                                       parent:(NSFileWrapper *)parentFileWrapper
{
    URI effectSourceURI;
    if (fileURI.hasFragment()) {
        String fragment(URI::stringByDeletingLastPathComponent(fileURI.fragment()));
        fragment.append("/");
        fragment.append([keyPath UTF8String]);
        effectSourceURI = URI::createFromFilePath(fileURI.absolutePath(), fragment);
    }
    else {
        String path(fileURI.absolutePathByDeletingLastPathComponent());
        path.append("/");
        path.append([keyPath UTF8String]);
        effectSourceURI = URI::createFromFilePath(path);
    }
    NSFileWrapper *sourceFileWrapper = [Document newFileWrapperWithFileURL:effectSourceURI error:nil];
    if (sourceFileWrapper) {
        [self setFileWrapper:sourceFileWrapper keyPath:keyPath parent:parentFileWrapper];
    }
    return sourceFileWrapper;
}

- (NSFileWrapper *)addTextureFileWrapper:(NSString *)keyPath
                                 fileURI:(const URI &)fileURI
                                  parent:(NSFileWrapper *)parentFileWrapper
{
    NSFileWrapper *textureFileWrapper = [Document newFileWrapperWithFileURL:fileURI error:nil];
    if (textureFileWrapper) {
        [self setFileWrapper:textureFileWrapper keyPath:keyPath parent:parentFileWrapper];
    }
    return textureFileWrapper;
}

- (void)addAllEffectResourceFileWrappers:(const Effect *)effect
                                 fileURI:(const URI &)fileURI
                                  parent:(NSFileWrapper *)parentFileWrapper
{
    const StringList &includePaths = effect->allIncludePaths();
    for (StringList::const_iterator it = includePaths.begin(), end = includePaths.end(); it != end; ++it) {
        NSString *keyPath = [[NSString alloc] initWithUTF8String:it->c_str()];
        [self addEffectSourceFileWrapper:keyPath parentEffectFileURI:fileURI parent:parentFileWrapper];
    }
    const IEffect::ImageResourceList &allParameters = effect->allImageResources();
    for (IEffect::ImageResourceList::const_iterator it = allParameters.begin(), end = allParameters.end(); it != end;
         ++it) {
        const IEffect::ImageResourceParameter &parameter = *it;
        NSString *keyPath = [[NSString alloc] initWithUTF8String:parameter.m_filename.c_str()];
        [self addTextureFileWrapper:keyPath fileURI:parameter.m_fileURI parent:parentFileWrapper];
    }
}

- (NSFileWrapper *)addEffectFileWrapper:(const IDrawable *)drawable parent:(NSFileWrapper *)parentFileWrapper
{
    NSFileWrapper *effectFileWrapper = nil;
    Project *projectPtr = m_projectHolderPtr->currentProject();
    if (const Effect *effect = projectPtr->resolveEffect(drawable)) {
        const URI &fileURI = effect->fileURI();
        effectFileWrapper = [Document newFileWrapperWithFileURL:fileURI error:nil];
        if (effectFileWrapper) {
            [self addAllEffectResourceFileWrappers:effect fileURI:fileURI parent:parentFileWrapper];
            Project::LoadedEffectSet allRenderTargetEffects;
            projectPtr->getAllOffscreenRenderTargetEffects(effect, allRenderTargetEffects);
            String baseDirectory;
            if (Project::isArchiveURI(fileURI)) {
                baseDirectory.append(URI::stringByDeletingLastPathComponent(fileURI.fragment()).c_str());
            }
            else {
                baseDirectory.append(fileURI.absolutePathByDeletingLastPathComponent().c_str());
            }
            for (Project::LoadedEffectSet::const_iterator it = allRenderTargetEffects.begin(),
                                                          end = allRenderTargetEffects.end();
                 it != end; ++it) {
                NSError *error = nil;
                Effect *subEffect = *it;
                String fullPathString(baseDirectory);
                fullPathString.append("/");
                fullPathString.append(subEffect->fileURI().lastPathComponentConstString());
                const URI subEffectURI = Project::isArchiveURI(fileURI)
                    ? URI::createFromFilePath(fileURI.absolutePath(), fullPathString)
                    : URI::createFromFilePath(fullPathString);
                if (NSFileWrapper *fileWrapper = [Document newFileWrapperWithFileURL:subEffectURI error:&error]) {
                    NSString *filePath =
                        [[NSString alloc] initWithUTF8String:subEffectURI.lastPathComponentConstString()];
                    [self setFileWrapper:fileWrapper keyPath:filePath parent:parentFileWrapper];
                }
                [self addAllEffectResourceFileWrappers:subEffect fileURI:subEffectURI parent:parentFileWrapper];
            }
            NSString *filePath = [[NSString alloc] initWithUTF8String:fileURI.absolutePath().c_str()];
            [self setFileWrapper:effectFileWrapper keyPath:filePath.lastPathComponent parent:parentFileWrapper];
        }
    }
    return effectFileWrapper;
}

- (NSFileWrapper *)addAccessoryFileWrapper:(const Accessory *)accessory
                                    parent:(NSFileWrapper *)parent
                                     error:(NSError **)outError
{
    Accessory::ImageViewMap textures;
    accessory->getAllImageViews(textures);
    DocumentUUID *uuid = [self resolveUUIDFromAccessory:accessory];
    if (!uuid) {
        uuid = [[DocumentUUID alloc] init];
        [self registerAccessory:accessory uuid:uuid];
    }
    NSFileWrapper *accessoryDirectoryWrapper = [m_allAccessoryDirectoryWrapperDictionary objectForKey:uuid];
    if (accessoryDirectoryWrapper) {
        return accessoryDirectoryWrapper;
    }
    accessoryDirectoryWrapper = [self addDirectoryFileWrapper:uuid.string parent:parent];
    for (Accessory::ImageViewMap::const_iterator it = textures.begin(), end = textures.end(); it != end; ++it) {
        const String &key = it->first;
        const URI &textureURI = accessory->resolveImageURI(it->first.c_str());
        NSString *keyPath = [[NSString alloc] initWithUTF8String:key.c_str()];
        [self addTextureFileWrapper:keyPath fileURI:textureURI parent:accessoryDirectoryWrapper];
    }
    NSFileWrapper *accessoryFileWrapper = [Document newFileWrapperWithFileURL:accessory->fileURI() error:outError];
    if (accessoryFileWrapper) {
        [accessoryDirectoryWrapper addFileWrapper:accessoryFileWrapper];
        [m_allAccessoryFileWrapperDictionary setObject:accessoryFileWrapper forKey:uuid];
        [self addEffectFileWrapper:accessory parent:accessoryDirectoryWrapper];
    }
    else {
        return nil;
    }
    [m_allAccessoryDirectoryWrapperDictionary setObject:accessoryDirectoryWrapper forKey:uuid];
    return accessoryDirectoryWrapper;
}

- (NSFileWrapper *)addModelFileWrapper:(Model *)model parent:(NSFileWrapper *)parent error:(NSError **)outError
{
    Model::ImageViewMap textures;
    model->getAllImageViews(textures);
    DocumentUUID *uuid = [self resolveUUIDFromModel:model];
    if (!uuid) {
        uuid = [[DocumentUUID alloc] init];
        [self registerModel:model uuid:uuid];
    }
    NSFileWrapper *modelDirectoryWrapper = [m_allModelDirectoryWrapperDictionary objectForKey:uuid];
    if (modelDirectoryWrapper) {
        if (model->isDirty()) {
            NSFileWrapper *modelFileWrapper = [m_allModelFileWrapperDictionary objectForKey:uuid];
            NSString *filePath = modelFileWrapper.preferredFilename;
            if (NSFileWrapper *newModelFileWrapper = [Document newModelFileWrapper:model
                                                                      withFilePath:filePath
                                                                             error:outError]) {
                [m_allModelFileWrapperDictionary setObject:newModelFileWrapper forKey:uuid];
            }
        }
        return modelDirectoryWrapper;
    }
    modelDirectoryWrapper = [self addDirectoryFileWrapper:uuid.string parent:parent];
    for (Model::ImageViewMap::const_iterator it = textures.begin(), end = textures.end(); it != end; ++it) {
        const String &key = it->first;
        const URI &textureURI = model->resolveImageURI(key);
        NSString *keyPath = [[NSString alloc] initWithUTF8String:key.c_str()];
        [self addTextureFileWrapper:keyPath fileURI:textureURI parent:modelDirectoryWrapper];
    }
    NSFileWrapper *modelFileWrapper = nil;
    if (model->isDirty()) {
        const URI &fileURI = model->fileURI();
        const String &filename = fileURI.hasFragment() ? fileURI.fragment() : fileURI.absolutePath();
        NSString *filePath = [[NSString alloc] initWithUTF8String:filename.c_str()];
        modelFileWrapper = [Document newModelFileWrapper:model withFilePath:filePath error:outError];
    }
    else {
        modelFileWrapper = [Document newFileWrapperWithFileURL:model->fileURI() error:outError];
    }
    if (modelFileWrapper) {
        [modelDirectoryWrapper addFileWrapper:modelFileWrapper];
        [m_allModelFileWrapperDictionary setObject:modelFileWrapper forKey:uuid];
        [self addEffectFileWrapper:model parent:modelDirectoryWrapper];
    }
    else {
        return nil;
    }
    [m_allModelDirectoryWrapperDictionary setObject:modelDirectoryWrapper forKey:uuid];
    return modelDirectoryWrapper;
}

- (NSFileWrapper *)addMotionFileWrapper:(Motion *)motion
                              accessory:(const Accessory *)accessory
                                 parent:(NSFileWrapper *)parent
{
    NSFileWrapper *motionFileWrapper = nil;
    if (DocumentUUID *modelUUID = [self resolveUUIDFromAccessory:accessory]) {
        motionFileWrapper = [self saveMotion:motion
                                      parent:parent
                                       block:^(Motion *motion, ByteArray &bytes) {
                                           Error error;
                                           motion->save(bytes, 0, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, error);
                                           return modelUUID.string;
                                       }];
    }
    return motionFileWrapper;
}

- (NSFileWrapper *)addMotionFileWrapper:(Motion *)motion model:(const Model *)model parent:(NSFileWrapper *)parent
{
    NSFileWrapper *motionFileWrapper = nil;
    if (DocumentUUID *modelUUID = [self resolveUUIDFromModel:model]) {
        motionFileWrapper =
            [self saveMotion:motion
                      parent:parent
                       block:^(Motion *motion, ByteArray &bytes) {
                           Error error;
                           motion->save(bytes, model,
                               NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE | NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH |
                                   NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL,
                               error);
                           return modelUUID.string;
                       }];
    }
    return motionFileWrapper;
}

- (NSFileWrapper *)addCameraMotionFileWrapperWithParent:(NSFileWrapper *)parent firstSave:(BOOL)isFirstSave
{
    [self resetCameraMotion];
    Motion *motion = m_projectHolderPtr->currentProject()->cameraMotion();
    if (isFirstSave) {
        motion->setDirty(true);
    }
    [self addDirectoryFileWrapper:m_cameraMotionUUID.string parent:parent];
    NSFileWrapper *motionFileWrapper =
        [self saveMotion:motion
                  parent:parent
                   block:^(Motion *motion, ByteArray &bytes) {
                       Error error;
                       motion->save(bytes, 0, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, error);
                       return @"camera";
                   }];
    return motionFileWrapper;
}

- (NSFileWrapper *)addLightMotionFileWrapperWithParent:(NSFileWrapper *)parent firstSave:(BOOL)isFirstSave
{
    [self resetLightMotion];
    Motion *motion = m_projectHolderPtr->currentProject()->lightMotion();
    [self addDirectoryFileWrapper:m_lightMotionUUID.string parent:parent];
    if (isFirstSave) {
        motion->setDirty(true);
    }
    NSFileWrapper *motionFileWrapper =
        [self saveMotion:motion
                  parent:parent
                   block:^(Motion *motion, ByteArray &bytes) {
                       Error error;
                       motion->save(bytes, 0, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, error);
                       return @"light";
                   }];
    return motionFileWrapper;
}

- (NSFileWrapper *)addDataFileWrapper:(NSData *)data filename:(NSString *)filename parent:(NSFileWrapper *)parent
{
    NSFileWrapper *dataFileWrapper = [[NSFileWrapper alloc] initRegularFileWithContents:data];
    dataFileWrapper.preferredFilename = filename;
    [parent addFileWrapper:dataFileWrapper];

    return dataFileWrapper;
}

- (NSFileWrapper *)addProjectJsonFileWrapper:(NSFileWrapper *)parent
{
    NSFileWrapper *projectJsonFileWrapper = [[parent fileWrappers] objectForKey:@"project.json"];
    if (projectJsonFileWrapper) {
        [parent removeFileWrapper:projectJsonFileWrapper];
    }
    Project *projectPtr = m_projectHolderPtr->currentProject();
    projectPtr->saveAsJSON(m_json);
    JSON_Object *root = json_object(m_json);
    if (DocumentUUID *uuid = [self resolveUUIDFromAccessory:projectPtr->activeAccessory()]) {
        [Document setUUID:uuid toJson:root forKey:"project.active.accessory"];
    }
    if (DocumentUUID *uuid = [self resolveUUIDFromModel:projectPtr->activeModel()]) {
        [Document setUUID:uuid toJson:root forKey:"project.active.model"];
    }
    [self serializeAccessorySettingAsJson:root];
    [self serializeModelSettingAsJson:root];
    [self serializeDrawableOrderSettingAsJson:root];
    [self serializeTransformOrderSettingAsJson:root];
    [self serializeGlobalCameraAsJson:root];
    [self serializeGlobalLightAsJson:root];
    [self serializeMotionSettingAsJson:root];
    size_t length = json_serialization_size(m_json);
    NSMutableData *data = [[NSMutableData alloc] initWithLength:length];
    json_serialize_to_buffer(m_json, static_cast<char *>(data.mutableBytes), length);
    projectJsonFileWrapper = [[NSFileWrapper alloc] initRegularFileWithContents:data];
    projectJsonFileWrapper.preferredFilename = @"project.json";
    [parent addFileWrapper:projectJsonFileWrapper];

    return projectJsonFileWrapper;
}

- (void)setAccessoryFileWrapper:(NSFileWrapper *)accessoryFileWrapper
                           uuid:(DocumentUUID *)uuid
           fromDirectoryWrapper:(NSFileWrapper *)accessoryDirectoryWrapper
{
    if (accessoryFileWrapper) {
        [m_allAccessoryDirectoryWrapperDictionary setObject:accessoryDirectoryWrapper forKey:uuid];
        [m_allAccessoryFileWrapperDictionary setObject:accessoryFileWrapper forKey:uuid];
    }
}

- (void)setModelFileWrapper:(NSFileWrapper *)modelFileWrapper
                       uuid:(DocumentUUID *)uuid
       fromDirectoryWrapper:(NSFileWrapper *)modelDirectoryWrapper
{
    if (modelFileWrapper) {
        [m_allModelDirectoryWrapperDictionary setObject:modelDirectoryWrapper forKey:uuid];
        [m_allModelFileWrapperDictionary setObject:modelFileWrapper forKey:uuid];
    }
}

- (BOOL)readAccessoryFromFileWrapper:(NSDictionary *)rootDirectoryWrapper error:(NSError **)outError
{
    NSFileWrapper *accessoryDirectoryWrapper = [rootDirectoryWrapper objectForKey:@"Accessory"];
    if (!accessoryDirectoryWrapper.directory) {
        [self createFileWrapperError:outError];
        return NO;
    }
    NSDictionary *allAccessoryFileWrappers = [accessoryDirectoryWrapper fileWrappers];
    const JSON_Object *root = json_object(m_json);
    if (m_json && json_value_get_type(json_object_dotget_value(root, "project.accessories")) == JSONArray) {
        const JSON_Array *accessories = json_object_dotget_array(root, "project.accessories");
        const size_t numAccessories = json_array_get_count(accessories);
        for (size_t i = 0; i < numAccessories; i++) {
            const JSON_Object *object = json_array_get_object(accessories, i);
            const char *filename = json_object_get_string(object, "filename");
            NSString *filenameString = [NSString stringWithUTF8String:filename];
            NSString *uuidString = [NSString stringWithUTF8String:json_object_get_string(object, "uuid")];
            NSFileWrapper *accessoryDirectoryWrapper = [allAccessoryFileWrappers objectForKey:uuidString];
            if (!accessoryDirectoryWrapper) {
                accessoryDirectoryWrapper = [allAccessoryFileWrappers objectForKey:[uuidString lowercaseString]];
            }
            if (!accessoryDirectoryWrapper.directory) {
                continue;
            }
            NSFileWrapper *accessoryFileWrapper =
                [[accessoryDirectoryWrapper fileWrappers] objectForKey:filenameString];
            if (!accessoryFileWrapper) {
                accessoryFileWrapper =
                    [[accessoryFileWrapper fileWrappers] objectForKey:[filenameString lowercaseString]];
            }
            DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
            [self setAccessoryFileWrapper:accessoryFileWrapper
                                     uuid:uuid
                     fromDirectoryWrapper:accessoryDirectoryWrapper];
        }
    }
    else {
        for (DocumentUUID *uuid in allAccessoryFileWrappers) {
            NSFileWrapper *accessoryDirectoryWrapper = [allAccessoryFileWrappers objectForKey:uuid];
            if (!accessoryDirectoryWrapper.directory) {
                continue;
            }
            NSDictionary *fileWrappers = [accessoryDirectoryWrapper fileWrappers];
            NSFileWrapper *accessoryFileWrapper = nil;
            for (NSString *filename in fileWrappers) {
                NSFileWrapper *fileWrapper = [fileWrappers objectForKey:filename];
                NSString *extension = filename.pathExtension;
                if (fileWrapper.regularFile && [extension isEqualToString:kXExtension]) {
                    accessoryFileWrapper = fileWrapper;
                    break;
                }
            }
            [self setAccessoryFileWrapper:accessoryFileWrapper
                                     uuid:uuid
                     fromDirectoryWrapper:accessoryDirectoryWrapper];
        }
    }
    return YES;
}

- (BOOL)readModelFromFileWrapper:(NSDictionary *)rootDirectoryWrapper error:(NSError **)outError
{
    NSFileWrapper *modelDirectoryWrapper = [rootDirectoryWrapper objectForKey:@"Model"];
    if (!modelDirectoryWrapper.directory) {
        [self createFileWrapperError:outError];
        return NO;
    }
    NSDictionary *allModelFileWrappers = [modelDirectoryWrapper fileWrappers];
    const JSON_Object *root = json_object(m_json);
    if (m_json && json_value_get_type(json_object_dotget_value(root, "project.models")) == JSONArray) {
        const JSON_Array *models = json_object_dotget_array(root, "project.models");
        const size_t numModels = json_array_get_count(models);
        for (size_t i = 0; i < numModels; i++) {
            const JSON_Object *object = json_array_get_object(models, i);
            const char *filename = json_object_get_string(object, "filename");
            NSString *filenameString = [NSString stringWithUTF8String:filename];
            NSString *uuidString = [NSString stringWithUTF8String:json_object_get_string(object, "uuid")];
            NSFileWrapper *modelDirectoryWrapper = [allModelFileWrappers objectForKey:uuidString];
            if (!modelDirectoryWrapper) {
                modelDirectoryWrapper = [allModelFileWrappers objectForKey:[uuidString lowercaseString]];
            }
            if (!modelDirectoryWrapper.directory) {
                continue;
            }
            NSFileWrapper *modelFileWrapper = [[modelDirectoryWrapper fileWrappers] objectForKey:filenameString];
            if (!modelFileWrapper) {
                modelFileWrapper = [[modelDirectoryWrapper fileWrappers] objectForKey:[filenameString lowercaseString]];
            }
            DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
            [self setModelFileWrapper:modelFileWrapper uuid:uuid fromDirectoryWrapper:modelDirectoryWrapper];
        }
    }
    else {
        for (DocumentUUID *uuid in allModelFileWrappers) {
            NSFileWrapper *modelDirectoryWrapper = [allModelFileWrappers objectForKey:uuid.string];
            if (!modelDirectoryWrapper.directory) {
                continue;
            }
            NSDictionary *fileWrappers = [modelDirectoryWrapper fileWrappers];
            NSFileWrapper *modelFileWrapper = nil;
            for (NSString *filename in fileWrappers) {
                NSFileWrapper *fileWrapper = [fileWrappers objectForKey:filename];
                NSString *extension = filename.pathExtension;
                if (fileWrapper.regularFile &&
                    ([extension isEqualToString:kPMXExtension] || [extension isEqualToString:kPMDExtension])) {
                    modelFileWrapper = fileWrapper;
                    break;
                }
            }
            [self setModelFileWrapper:modelFileWrapper uuid:uuid fromDirectoryWrapper:modelDirectoryWrapper];
        }
    }
    return YES;
}

- (BOOL)readMotionFromFileWrapper:(NSDictionary *)rootDirectoryWrapper error:(NSError **)outError
{
    NSFileWrapper *motionDirectoryWrapper = [rootDirectoryWrapper objectForKey:@"Motion"];
    if (!motionDirectoryWrapper.directory) {
        [self createFileWrapperError:outError];
        return NO;
    }
    NSDictionary *allMotionFileWrappers = [motionDirectoryWrapper fileWrappers];
    for (NSString *uuidString in allMotionFileWrappers) {
        NSFileWrapper *motionDirectoryWrapper = [allMotionFileWrappers objectForKey:uuidString];
        if (!motionDirectoryWrapper.directory) {
            continue;
        }
        DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
        NSDictionary *fileWrappers = [motionDirectoryWrapper fileWrappers];
        NSFileWrapper *motionFileWrapper = nil;
        for (NSString *filename in fileWrappers) {
            NSFileWrapper *fileWrapper = [fileWrappers objectForKey:filename];
            NSString *extension = filename.pathExtension;
            if (fileWrapper.regularFile &&
                ([extension isEqualToString:kNMDExtension] || [extension isEqualToString:kVMDExtension])) {
                motionFileWrapper = fileWrapper;
                break;
            }
        }
        if (motionFileWrapper) {
            [m_allMotionDictionary setObject:motionFileWrapper forKey:uuid];
        }
    }
    return YES;
}

- (void)loadAllAccessories:(NSError **)outError
{
    if (m_allAccessoryFileWrapperDictionary.count > 0) {
        for (DocumentUUID *uuid in m_allAccessoryFileWrapperDictionary) {
            NSFileWrapper *accessoryDirectoryWrapper = [m_allAccessoryDirectoryWrapperDictionary objectForKey:uuid];
            NSFileWrapper *accessoryFileWrapper = [m_allAccessoryFileWrapperDictionary objectForKey:uuid];
            [self readAccessoryFromFileWrapper:accessoryFileWrapper
                              directoryWrapper:accessoryDirectoryWrapper
                                          uuid:uuid
                                         error:outError];
        }
    }
}

- (void)loadAllModels:(NSError **)outError
{
    if (m_allModelFileWrapperDictionary.count > 0) {
        for (DocumentUUID *uuid in m_allModelFileWrapperDictionary) {
            NSFileWrapper *modelDirectoryWrapper = [m_allModelDirectoryWrapperDictionary objectForKey:uuid];
            NSFileWrapper *modelFileWrapper = [m_allModelFileWrapperDictionary objectForKey:uuid];
            [self readModelFromFileWrapper:modelFileWrapper
                          directoryWrapper:modelDirectoryWrapper
                                      uuid:uuid
                                     error:outError];
        }
    }
}

- (void)loadAllMotions:(NSError **)outError
{
    Project *projectPtr = m_projectHolderPtr->currentProject();
    if (m_allMotionDictionary.count > 0) {
        for (DocumentUUID *motionUUID in m_allMotionDictionary) {
            NSFileWrapper *motionFileWrapper = [m_allMotionDictionary objectForKey:motionUUID];
            NSString *drawableUUIDString = [motionFileWrapper.preferredFilename stringByDeletingPathExtension];
            DocumentUUID *drawableUUID = [[DocumentUUID alloc] initWithString:drawableUUIDString];
            if (Model *model = [self resolveModelFromUUID:drawableUUID]) {
                if (Motion *modelMotion = [self readMotionFromFileWrapper:motionFileWrapper
                                                                     uuid:motionUUID
                                                                    error:outError]) {
                    Motion *oldModelMotion = projectPtr->resolveMotion(model);
                    projectPtr->removeMotion(oldModelMotion);
                    projectPtr->addModelMotion(modelMotion, model);
                    projectPtr->destroyMotion(oldModelMotion);
                }
            }
            else if (Accessory *accessory = [self resolveAccessoryFromUUID:drawableUUID]) {
                if (Motion *accessoryMotion = [self readMotionFromFileWrapper:motionFileWrapper
                                                                         uuid:motionUUID
                                                                        error:outError]) {
                    Motion *oldAccessoryMotion = projectPtr->resolveMotion(accessory);
                    projectPtr->removeMotion(oldAccessoryMotion);
                    projectPtr->addAccessoryMotion(accessoryMotion, accessory);
                    projectPtr->destroyMotion(oldAccessoryMotion);
                }
            }
        }
    }
}

- (void)loadAllAudios:(NSError **)outError
{
    if (m_allAudioURLSet.count > 0) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        IFileManager *fileManager = m_applicationPtr->fileManager();
        const JSON_Value *v = json_object_dotget_value(json_object(m_json), "project.audio.filename");
        if (json_value_get_type(v) == JSONString) {
            if (const char *filename = json_value_get_string(v)) {
                for (NSURL *audioURL in m_allAudioURLSet) {
                    if (StringUtils::equals(filename, audioURL.lastPathComponent.UTF8String)) {
                        const URI &fileURI = URI::createFromFilePath(audioURL.path.UTF8String);
                        Error error;
                        if (!fileManager->loadAudioFile(fileURI, projectPtr, error) && outError) {
                            *outError = createNSError(error);
                        }
                        break;
                    }
                }
            }
        }
    }
}

- (void)loadAllVideos:(NSError **)outError
{
    BX_UNUSED_1(outError);
    if (m_allVideoURLSet.count > 0) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        IFileManager *fileManager = m_applicationPtr->fileManager();
        const JSON_Value *v = json_object_dotget_value(json_object(m_json), "project.background.filename");
        if (json_value_get_type(v) == JSONString) {
            if (const char *filename = json_value_get_string(v)) {
                for (NSURL *audioURL in m_allVideoURLSet) {
                    if (StringUtils::equals(filename, audioURL.lastPathComponent.UTF8String)) {
                        const URI &fileURI = URI::createFromFilePath(audioURL.path.UTF8String);
                        Error error;
                        if (!fileManager->loadVideoFile(fileURI, projectPtr, error) && outError) {
                            *outError = createNSError(error);
                        }
                        break;
                    }
                }
            }
        }
    }
}

- (void)configureAllModels:(const JSON_Array *)models
{
    const size_t numModels = json_array_get_count(models);
    Project *projectPtr = m_projectHolderPtr->currentProject();
    nanoem_unicode_string_factory_t *factory = projectPtr->unicodeStringFactory();
    for (size_t i = 0; i < numModels; i++) {
        const JSON_Object *object = json_array_get_object(models, i);
        NSString *uuidString = [NSString stringWithUTF8String:json_object_get_string(object, "uuid")];
        DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
        if (Model *targetModel = [self resolveModelFromUUID:uuid]) {
            [self configureCamera:json_object_get_object(object, "camera") target:targetModel->localCamera()];
            const JSON_Object *external = json_object_get_object(object, "external");
            if (const char *targetBoneName = json_object_dotget_string(external, "target.bone")) {
                if (const nanoem_model_bone_t *bone = targetModel->findBone(targetBoneName)) {
                    const char *parentBoneName = json_object_dotget_string(external, "parent.bone");
                    NSString *parentModelUUIDString =
                        [NSString stringWithUTF8String:json_object_dotget_string(external, "parent.model.uuid")];
                    DocumentUUID *parentModelUUID = [[DocumentUUID alloc] initWithString:parentModelUUIDString];
                    if (Model *parentModel = [self resolveModelFromUUID:parentModelUUID]) {
                        if (parentModel->findBone(parentBoneName)) {
                            targetModel->setOutsideParent(
                                bone, tinystl::make_pair(parentModel->canonicalName(), String(parentBoneName)));
                            Motion *motion = projectPtr->resolveMotion(targetModel);
                            nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                            nanoem_mutable_motion_model_keyframe_t *keyframe =
                                nanoemMutableMotionModelKeyframeCreateByFound(motion->data(), 0, &status);
                            nanoem_mutable_motion_outside_parent_t *outsideParent =
                                nanoemMutableMotionOutsideParentCreateFromModelKeyframe(
                                    nanoemMutableMotionModelKeyframeGetOriginObject(keyframe), &status);
                            nanoem_unicode_string_t *baseBoneName = nanoemUnicodeStringFactoryCreateString(factory,
                                reinterpret_cast<const nanoem_u8_t *>(targetBoneName), strlen(targetBoneName), &status);
                            nanoemMutableMotionOutsideParentSetSubjectBoneName(outsideParent, baseBoneName, &status);
                            nanoemUnicodeStringFactoryDestroyString(factory, baseBoneName);
                            nanoem_unicode_string_t *boneName = nanoemUnicodeStringFactoryCreateString(factory,
                                reinterpret_cast<const nanoem_u8_t *>(parentBoneName), strlen(parentBoneName), &status);
                            nanoemMutableMotionOutsideParentSetTargetBoneName(outsideParent, boneName, &status);
                            nanoemUnicodeStringFactoryDestroyString(factory, boneName);
                            nanoem_unicode_string_t *objectName = nanoemUnicodeStringFactoryCreateString(factory,
                                reinterpret_cast<const nanoem_u8_t *>(parentModel->name().c_str()),
                                parentModel->name().size(), &status);
                            nanoemMutableMotionOutsideParentSetTargetObjectName(outsideParent, objectName, &status);
                            nanoemUnicodeStringFactoryDestroyString(factory, objectName);
                            nanoemMutableMotionModelKeyframeAddOutsideParent(keyframe, outsideParent, &status);
                            nanoemMutableMotionOutsideParentDestroy(outsideParent);
                            nanoemMutableMotionModelKeyframeDestroy(keyframe);
                        }
                    }
                }
            }
        }
    }
}

- (void)configureAllAccessories:(const JSON_Array *)accessories
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    Project *projectPtr = m_projectHolderPtr->currentProject();
    const size_t numAccessories = json_array_get_count(accessories);
    for (size_t i = 0; i < numAccessories; i++) {
        const JSON_Object *object = json_array_get_object(accessories, i);
        NSString *uuidString = [NSString stringWithUTF8String:json_object_get_string(object, "uuid")];
        DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
        if (Accessory *accessory = [self resolveAccessoryFromUUID:uuid]) {
            int findCount = 0;
            const JSON_Value *translation = json_object_get_value(object, "translation");
            if (translation && json_value_get_type(translation) == JSONArray) {
                accessory->setTranslation(JSONUtils::readVector3(json_array(translation)));
                findCount++;
            }
            const JSON_Value *orientation = json_object_get_value(object, "orientation");
            if (orientation && json_value_get_type(orientation) == JSONArray) {
                accessory->setOrientation(JSONUtils::readVector3(json_array(orientation)));
                findCount++;
            }
            const JSON_Value *scaleFactor = json_object_get_value(object, "scale");
            if (scaleFactor && json_value_get_type(scaleFactor) == JSONNumber) {
                accessory->setScaleFactor(nanodxm_float32_t(json_value_get_number(scaleFactor)));
                findCount++;
            }
            const JSON_Value *opacity = json_object_get_value(object, "opacity");
            if (opacity && json_value_get_type(opacity) == JSONNumber) {
                accessory->setOpacity(nanodxm_float32_t(json_value_get_number(opacity)));
                findCount++;
            }
            if (findCount >= 4) {
                Motion *motion = projectPtr->resolveMotion(accessory);
                nanoem_mutable_motion_accessory_keyframe_t *keyframe =
                    nanoemMutableMotionAccessoryKeyframeCreateByFound(motion->data(), 0, &status);
                nanoemMutableMotionAccessoryKeyframeSetTranslation(keyframe, glm::value_ptr(accessory->translation()));
                nanoemMutableMotionAccessoryKeyframeSetOrientation(
                    keyframe, glm::value_ptr(accessory->orientationQuaternion()));
                nanoemMutableMotionAccessoryKeyframeSetScaleFactor(keyframe, accessory->scaleFactor());
                nanoemMutableMotionAccessoryKeyframeSetOpacity(keyframe, accessory->opacity());
                nanoemMutableMotionAccessoryKeyframeDestroy(keyframe);
            }
        }
    }
}

- (void)configureDrawOrder:(const JSON_Array *)root
{
    const size_t numAllDrawables = json_array_get_count(root);
    Project::DrawableList drawables;
    for (size_t i = 0; i < numAllDrawables; i++) {
        if (const char *value = json_array_get_string(root, i)) {
            NSString *uuidString = [NSString stringWithUTF8String:value];
            DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
            if (Model *model = [self resolveModelFromUUID:uuid]) {
                drawables.push_back(model);
            }
            else if (Accessory *accessory = [self resolveAccessoryFromUUID:uuid]) {
                drawables.push_back(accessory);
            }
        }
    }
    Project *projectPtr = m_projectHolderPtr->currentProject();
    const Project::AccessoryList *allAccessories = projectPtr->allAccessories();
    const Project::ModelList *allModels = projectPtr->allModels();
    if (allModels->size() + allAccessories->size() == drawables.size()) {
        projectPtr->setDrawableOrderList(drawables);
    }
    else {
        drawables.clear();
        for (Project::AccessoryList::const_iterator it = allAccessories->begin(), end = allAccessories->end();
             it != end; ++it) {
            drawables.push_back(*it);
        }
        for (Project::ModelList::const_iterator it = allModels->begin(), end = allModels->end(); it != end; ++it) {
            drawables.push_back(*it);
        }
        projectPtr->setDrawableOrderList(drawables);
    }
}

- (void)configureTransformOrder:(const JSON_Array *)root
{
    const size_t numAllTransformables = json_array_get_count(root);
    Project::ModelList transformables;
    for (size_t i = 0; i < numAllTransformables; i++) {
        if (const char *value = json_array_get_string(root, i)) {
            NSString *uuidString = [NSString stringWithUTF8String:value];
            DocumentUUID *uuid = [[DocumentUUID alloc] initWithString:uuidString];
            if (Model *model = [self resolveModelFromUUID:uuid]) {
                transformables.push_back(model);
            }
        }
    }
    Project *projectPtr = m_projectHolderPtr->currentProject();
    const Project::ModelList *allModels = projectPtr->allModels();
    if (allModels->size() == transformables.size()) {
        projectPtr->setTransformOrderList(transformables);
    }
    else {
        projectPtr->setTransformOrderList(*allModels);
    }
}

- (void)resetCameraMotion
{
    Project *projectPtr = m_projectHolderPtr->currentProject();
    if (!projectPtr->cameraMotion()) {
        Motion *newMotion = projectPtr->createMotion();
        newMotion->initialize(projectPtr->globalCamera());
        projectPtr->setCameraMotion(newMotion);
    }
    if (!m_cameraMotionUUID) {
        m_cameraMotionUUID = [[DocumentUUID alloc] init];
        [self registerMotion:projectPtr->cameraMotion() uuid:m_cameraMotionUUID];
    }
}

- (void)configureCamera:(const JSON_Object *)root target:(ICamera *)camera
{
    if (const JSON_Array *angle = json_object_get_array(root, "angle")) {
        camera->setAngle(JSONUtils::readVector3(angle));
    }
    if (const JSON_Array *lookAt = json_object_get_array(root, "lookAt")) {
        camera->setLookAt(JSONUtils::readVector3(lookAt));
    }
    double distance = json_object_get_number(root, "distance");
    bool needCompatible = m_projectHolderPtr->currentProject()->coordinationSystem() == GLM_RIGHT_HANDED;
    if (distance != 0) {
        camera->setDistance(distance * (needCompatible ? -1 : 1));
    }
    double fov = json_object_get_number(root, "fov");
    if (fov != 0) {
        needCompatible ? camera->setFovRadians(fov) : camera->setFov(fov);
    }
    camera->setPerspective(json_object_get_boolean(root, "perspective") != 0);
}

- (void)configureCamera:(const JSON_Object *)root
{
    if (const char *uuidString = json_object_get_string(root, "motion")) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        m_cameraMotionUUID = [[DocumentUUID alloc] initWithUTF8String:uuidString];
        NSFileWrapper *motionFileWrapper = [m_allMotionDictionary objectForKey:m_cameraMotionUUID];
        if (Motion *motion = [self readMotionFromFileWrapper:motionFileWrapper uuid:m_cameraMotionUUID error:nil]) {
            if (Motion *oldMotion = projectPtr->cameraMotion()) {
                projectPtr->setCameraMotion(0);
                projectPtr->destroyMotion(oldMotion);
            }
            projectPtr->setCameraMotion(motion);
        }
        else {
        }
    }
    [self resetCameraMotion];
}

- (void)resetLightMotion
{
    Project *projectPtr = m_projectHolderPtr->currentProject();
    if (!projectPtr->lightMotion()) {
        Motion *newMotion = projectPtr->createMotion();
        newMotion->initialize(projectPtr->globalLight());
        projectPtr->setLightMotion(newMotion);
    }
    if (!m_lightMotionUUID) {
        m_lightMotionUUID = [[DocumentUUID alloc] init];
        [self registerMotion:projectPtr->lightMotion() uuid:m_lightMotionUUID];
    }
}

- (void)configureLight:(const JSON_Object *)root target:(ILight *)light
{
    if (const JSON_Array *color = json_object_get_array(root, "color")) {
        light->setColor(JSONUtils::readVector3(color));
    }
    if (const JSON_Array *direction = json_object_get_array(root, "direction")) {
        light->setDirection(JSONUtils::readVector3(direction));
    }
}

- (void)configureLight:(const JSON_Object *)root
{
    if (const char *uuidString = json_object_get_string(root, "motion")) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        m_lightMotionUUID = [[DocumentUUID alloc] initWithUTF8String:uuidString];
        NSFileWrapper *motionFileWrapper = [m_allMotionDictionary objectForKey:m_lightMotionUUID];
        if (Motion *motion = [self readMotionFromFileWrapper:motionFileWrapper uuid:m_lightMotionUUID error:nil]) {
            if (Motion *oldMotion = projectPtr->lightMotion()) {
                projectPtr->setLightMotion(0);
                projectPtr->destroyMotion(oldMotion);
            }
            projectPtr->setLightMotion(motion);
        }
        else {
        }
    }
    [self resetLightMotion];
}

- (NSError *)outError
{
    if (m_errorType != DocumentErrorTypeNone) {
        NSDictionary *details = [NSDictionary
            dictionaryWithObjectsAndKeys:NSLocalizedString(@"Failed Loading a project", nil), NSLocalizedDescriptionKey,
            NSLocalizedString(@"Loaded project seems corrupted", nil), NSLocalizedFailureReasonErrorKey,
            NSLocalizedString(@"Create a new project", nil), NSLocalizedRecoverySuggestionErrorKey, nil];
        NSError *error = [[NSError alloc] initWithDomain:kLoadingProjectErrorDomain code:0 userInfo:details];
        return error;
    }
    else {
        return nil;
    }
}

- (void)load:(NSError **)outError
{
    // load project first due to activation of effect plugin
    if (m_json) {
        Project *projectPtr = m_projectHolderPtr->currentProject();
        projectPtr->loadFromJSON(m_json);
    }
    [self loadAllAccessories:outError];
    [self loadAllModels:outError];
    [self loadAllMotions:outError];
    [self loadAllAudios:outError];
    [self loadAllVideos:outError];
    const JSON_Object *root = json_object(m_json);
    const JSON_Object *project = json_object_get_object(root, "project");
    Accessory *activeAccessory = 0;
    Model *activeModel = 0;
    if (DocumentUUID *activeAccessoryUUID = [Document newUUIDFromJson:project forKey:"active.accessory"]) {
        activeAccessory = [self resolveAccessoryFromUUID:activeAccessoryUUID];
    }
    if (DocumentUUID *activeModelUUID = [Document newUUIDFromJson:project forKey:"active.model"]) {
        activeModel = [self resolveModelFromUUID:activeModelUUID];
    }
    [self configureAllModels:json_object_get_array(project, "models")];
    [self configureAllAccessories:json_object_get_array(project, "accessories")];
    const JSON_Object *order = json_object_get_object(project, "order");
    [self configureDrawOrder:json_object_get_array(order, "draw")];
    [self configureTransformOrder:json_object_get_array(order, "transform")];
    const JSON_Object *camera = json_object_get_object(project, "camera");
    const JSON_Object *light = json_object_get_object(project, "light");
    [self configureCamera:camera];
    [self configureLight:light];
    nanoem_frame_index_t seekFrameIndex = json_object_get_number(project, "seek");
    Project *projectPtr = m_projectHolderPtr->currentProject();
    projectPtr->setActiveAccessory(activeAccessory);
    projectPtr->setActiveModel(activeModel);
    projectPtr->seek(glm::clamp(seekFrameIndex, nanoem_frame_index_t(0), projectPtr->duration()), true);
    projectPtr->restart();
    ICamera *cameraPtr = projectPtr->globalCamera();
    ILight *lightPtr = projectPtr->globalLight();
    [self configureCamera:camera target:cameraPtr];
    [self configureLight:light target:lightPtr];
    cameraPtr->setDirty(false);
    lightPtr->setDirty(false);
    projectPtr->activeCamera()->setDirty(false);
    projectPtr->activeLight()->setDirty(false);
}

- (BOOL)loadDocument:(NSFileWrapper *)fileWrapper ofType:(NSString *)typeName error:(NSError **)outError
{
    BX_UNUSED_1(typeName);
    if (!fileWrapper.directory) {
        [self createFileWrapperError:outError];
    }
    NSDictionary *rootDirectoryWrapper = [fileWrapper fileWrappers];
    if (outError && !*outError) {
        if (NSFileWrapper *projectJsonFileWrapper = [rootDirectoryWrapper objectForKey:@"project.json"]) {
            NSData *data = projectJsonFileWrapper.regularFileContents;
            NSString *json = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
            if (m_json) {
                json_value_free(m_json);
                m_json = 0;
            }
            m_json = json_parse_string(json.UTF8String);
        }
        if (!m_json) {
            m_json = json_value_init_object();
            JSON_Object *root = json_object(m_json);
            json_object_dotset_boolean(root, "project.grid.visible", 1);
            json_object_dotset_number(root, "project.grid.cell.x", 5);
            json_object_dotset_number(root, "project.grid.cell.y", 5);
            json_object_dotset_number(root, "project.grid.opacity", 1);
            json_object_dotset_number(root, "project.screen.sample", 1);
        }
    }
    if (outError && !*outError) {
#if TARGET_OS_MAC && !TARGET_OS_IOS
        NSFileWrapper *backgroundDirectoryWrapper = [rootDirectoryWrapper objectForKey:@"BackGround"];
        NSDictionary *allVideoFileWrappers = [backgroundDirectoryWrapper fileWrappers];
        NSSet *videoFileExtensions = [Document supportedMediaExtensions:kUTTypeMovie];
        for (NSString *filename in allVideoFileWrappers) {
            NSFileWrapper *videoFileWrapper = [allVideoFileWrappers objectForKey:filename];
            NSString *extension = filename.pathExtension;
            if (videoFileWrapper.symbolicLink && [videoFileExtensions containsObject:extension]) {
                [m_allVideoURLSet addObject:videoFileWrapper.symbolicLinkDestinationURL];
            }
        }
#endif
    }
    [self readAccessoryFromFileWrapper:rootDirectoryWrapper error:outError];
    [self readModelFromFileWrapper:rootDirectoryWrapper error:outError];
    [self readMotionFromFileWrapper:rootDirectoryWrapper error:outError];
    NSFileWrapper *waveDirectoryWrapper = [rootDirectoryWrapper objectForKey:@"Wave"];
    if (outError && !*outError) {
#if TARGET_OS_MAC && !TARGET_OS_IOS
        NSDictionary *allAudioFileWrappers = [waveDirectoryWrapper fileWrappers];
        NSSet *audioFileExtensions = [Document supportedMediaExtensions:kUTTypeAudio];
        for (NSString *filename in allAudioFileWrappers) {
            NSFileWrapper *audioFileWrapper = [allAudioFileWrappers objectForKey:filename];
            NSString *extension = filename.pathExtension;
            if (audioFileWrapper.symbolicLink && [audioFileExtensions containsObject:extension]) {
                [m_allAudioURLSet addObject:audioFileWrapper.symbolicLinkDestinationURL];
            }
        }
#endif
    }
    if (m_rootFileWrapper != fileWrapper) {
        m_rootFileWrapper = fileWrapper;
    }
    return !outError || (outError && !*outError);
}

- (NSFileWrapper *)saveDocument:(NSString *)typeName error:(NSError **)outError
{
    BX_UNUSED_1(typeName);
    NSFileWrapper *rootFileWrapper = m_rootFileWrapper;
    BOOL isFirstSave = NO;
    if (!rootFileWrapper) {
        rootFileWrapper = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:[NSDictionary dictionary]];
        m_rootFileWrapper = rootFileWrapper;
        isFirstSave = YES;
    }
    if (!m_json) {
        m_json = json_value_init_object();
        json_object_set_value(json_object(m_json), "project", json_value_init_object());
    }
    Project *projectPtr = m_projectHolderPtr->currentProject();
    NSFileWrapper *accessoryDirectoryWrapper = [self addDirectoryFileWrapper:@"Accessory" parent:rootFileWrapper];
    const Project::AccessoryList *accessories = projectPtr->allAccessories();
    for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end; ++it) {
        Accessory *accessory = *it;
        if ([self addAccessoryFileWrapper:accessory parent:accessoryDirectoryWrapper error:outError] == nil) {
            return nil;
        }
    }
    PluginFactory::DecoderPluginProxy proxy(self.decoderPlugin);
    NSFileWrapper *backgroundDirectoryWrapper = [self addDirectoryFileWrapper:@"BackGround" parent:rootFileWrapper];
    URI videoURI = projectPtr->backgroundVideoRenderer()->fileURI();
    if (!videoURI.isEmpty()) {
        NSURL *videoURL = [[NSURL alloc]
            initFileURLWithPath:[[NSString alloc] initWithUTF8String:videoURI.absolutePathConstString()]];
        NSFileWrapper *videoFileWrapper = [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:videoURL];
        const String filename(videoURI.lastPathComponent());
        videoFileWrapper.preferredFilename = [[NSString alloc] initWithUTF8String:filename.c_str()];
        [backgroundDirectoryWrapper addFileWrapper:videoFileWrapper];
        json_object_dotset_string(json_object(m_json), "project.background.filename", filename.c_str());
    }
    [self addDirectoryFileWrapper:@"Data" parent:rootFileWrapper];
    [self addDirectoryFileWrapper:@"Effect" parent:rootFileWrapper];
    NSFileWrapper *modelDirectoryWrapper = [self addDirectoryFileWrapper:@"Model" parent:rootFileWrapper];
    const Project::ModelList *models = projectPtr->allModels();
    for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
        Model *model = *it;
        if ([self addModelFileWrapper:model parent:modelDirectoryWrapper error:outError] == nil) {
            return nil;
        }
    }
    NSFileWrapper *motionDirectoryWrapper = [self addDirectoryFileWrapper:@"Motion" parent:rootFileWrapper];
    for (Project::AccessoryList::const_iterator it = accessories->begin(), end = accessories->end(); it != end; ++it) {
        Accessory *accessory = *it;
        Motion *motion = projectPtr->resolveMotion(accessory);
        [self addMotionFileWrapper:motion accessory:accessory parent:motionDirectoryWrapper];
    }
    for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
        Model *model = *it;
        Motion *motion = projectPtr->resolveMotion(model);
        [self addMotionFileWrapper:motion model:model parent:motionDirectoryWrapper];
    }
    [self addCameraMotionFileWrapperWithParent:motionDirectoryWrapper firstSave:isFirstSave];
    [self addLightMotionFileWrapperWithParent:motionDirectoryWrapper firstSave:isFirstSave];
    [self addDirectoryFileWrapper:@"Pose" parent:rootFileWrapper];
    [self addDirectoryFileWrapper:@"Vsq" parent:rootFileWrapper];
    NSFileWrapper *waveDirectoryWrapper = [self addDirectoryFileWrapper:@"Wave" parent:rootFileWrapper];
    const URI &audioURI = projectPtr->audioPlayer()->fileURI();
    if (!audioURI.isEmpty()) {
        NSURL *audioURL = [[NSURL alloc]
            initFileURLWithPath:[[NSString alloc] initWithUTF8String:audioURI.absolutePathConstString()]];
        NSFileWrapper *videoFileWrapper = [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:audioURL];
        const String filename(audioURI.lastPathComponent());
        videoFileWrapper.preferredFilename = [[NSString alloc] initWithUTF8String:filename.c_str()];
        [waveDirectoryWrapper addFileWrapper:videoFileWrapper];
        json_object_dotset_string(json_object(m_json), "project.audio.filename", filename.c_str());
    }
    [self addProjectJsonFileWrapper:rootFileWrapper];
    projectPtr->globalCamera()->setDirty(false);
    projectPtr->globalLight()->setDirty(false);
    return rootFileWrapper;
}

- (BOOL)registerAccessory:(const Accessory *)accessory uuid:(DocumentUUID *)uuid
{
    NSCAssert(uuid, @"must not be NULL");
    NSCAssert(accessory, @"must not be NULL");
    BOOL registered = NO;
    NSValue *key = [NSValue valueWithPointer:accessory];
    if (![m_accessoryUUIDDictionary objectForKey:key]) {
        [m_accessoryUUIDDictionary setObject:uuid forKey:key];
        [m_uuidAccessoryDictionary setObject:key forKey:uuid];
        registered = YES;
    }
    return registered;
}

- (BOOL)registerModel:(const Model *)model uuid:(DocumentUUID *)uuid
{
    NSCAssert(uuid, @"must not be NULL");
    NSCAssert(model, @"must not be NULL");
    NSValue *key = [NSValue valueWithPointer:model];
    BOOL registered = NO;
    if (![m_modelUUIDDictionary objectForKey:key]) {
        [m_modelUUIDDictionary setObject:uuid forKey:key];
        [m_uuidModelDictionary setObject:key forKey:uuid];
        registered = YES;
    }
    return registered;
}

- (BOOL)registerMotion:(const Motion *)motion uuid:(DocumentUUID *)uuid
{
    NSCAssert(uuid, @"must not be NULL");
    NSCAssert(motion, @"must not be NULL");
    NSValue *key = [NSValue valueWithPointer:motion];
    BOOL registered = NO;
    if (![m_motionUUIDDictionary objectForKey:uuid]) {
        [m_motionUUIDDictionary setObject:uuid forKey:key];
        [m_uuidMotionDictionary setObject:key forKey:uuid];
        registered = YES;
    }
    return registered;
}

- (void)removeAccessory:(const Accessory *)accessory uuid:(DocumentUUID *)uuid
{
    NSValue *key = [NSValue valueWithPointer:accessory];
    NSFileWrapper *accessoryDirectoryFileWrapper = [m_allAccessoryDirectoryWrapperDictionary objectForKey:uuid];
    NSFileWrapper *rootAccessoryDirectoryFileWrapper = [[m_rootFileWrapper fileWrappers] objectForKey:@"Accessory"];
    if (rootAccessoryDirectoryFileWrapper.directory) {
        [rootAccessoryDirectoryFileWrapper removeFileWrapper:accessoryDirectoryFileWrapper];
    }
    [m_uuidAccessoryDictionary removeObjectForKey:uuid];
    [m_accessoryUUIDDictionary removeObjectForKey:key];
}

- (void)removeModel:(const Model *)model uuid:(DocumentUUID *)uuid
{
    NSValue *key = [NSValue valueWithPointer:model];
    NSFileWrapper *modelDirectoryFileWrapper = [m_allModelDirectoryWrapperDictionary objectForKey:uuid];
    NSFileWrapper *rootModelDirectoryFileWrapper = [[m_rootFileWrapper fileWrappers] objectForKey:@"Model"];
    if (rootModelDirectoryFileWrapper.directory) {
        [rootModelDirectoryFileWrapper removeFileWrapper:modelDirectoryFileWrapper];
    }
    [m_uuidModelDictionary removeObjectForKey:uuid];
    [m_modelUUIDDictionary removeObjectForKey:key];
}

- (void)removeMotion:(const Motion *)motion uuid:(DocumentUUID *)uuid
{
    NSValue *key = [NSValue valueWithPointer:motion];
    NSFileWrapper *rootMotionDirectoryFileWrapper = [[m_rootFileWrapper fileWrappers] objectForKey:@"Motion"];
    if (rootMotionDirectoryFileWrapper.directory) {
        NSFileWrapper *motionDirectoryFileWrapper =
            [[rootMotionDirectoryFileWrapper fileWrappers] objectForKey:uuid.string];
        [rootMotionDirectoryFileWrapper removeFileWrapper:motionDirectoryFileWrapper];
    }
    [m_uuidMotionDictionary removeObjectForKey:uuid];
    [m_motionUUIDDictionary removeObjectForKey:key];
}

- (DocumentUUID *)resolveUUIDFromAccessory:(const Accessory *)value
{
    NSValue *accessoryValue = [NSValue valueWithPointer:value];
    return [m_accessoryUUIDDictionary objectForKey:accessoryValue];
}

- (DocumentUUID *)resolveUUIDFromModel:(const Model *)value
{
    NSValue *modelValue = [NSValue valueWithPointer:value];
    return [m_modelUUIDDictionary objectForKey:modelValue];
}

- (DocumentUUID *)resolveUUIDFromMotion:(const Motion *)value
{
    NSValue *motionValue = [NSValue valueWithPointer:value];
    return [m_motionUUIDDictionary objectForKey:motionValue];
}

- (Accessory *)resolveAccessoryFromUUID:(DocumentUUID *)value
{
    NSValue *v = [m_uuidAccessoryDictionary objectForKey:value];
    return static_cast<Accessory *>([v pointerValue]);
}

- (Model *)resolveModelFromUUID:(DocumentUUID *)value
{
    NSValue *v = [m_uuidModelDictionary objectForKey:value];
    return static_cast<Model *>([v pointerValue]);
}

- (Motion *)resolveMotionFromUUID:(DocumentUUID *)value
{
    NSValue *v = [m_uuidMotionDictionary objectForKey:value];
    return static_cast<Motion *>([v pointerValue]);
}

- (instancetype)initWithFileURL:(NSURL *)fileURL
                    application:(BaseApplicationService *)applicationPtr
                  projectHolder:(IProjectHolder *)projectHolderPtr
{
#if TARGET_OS_IPHONE
    if (self = [super initWithFileURL:fileURL]) {
        self.undoManager = nil;
#else
    if (self = [super init]) {
        self.fileURL = fileURL;
        self.hasUndoManager = NO;
#endif
        self.undoManager = nil;
        m_applicationPtr = applicationPtr;
        m_projectHolderPtr = projectHolderPtr;
        m_json = NULL;
        m_allAccessoryDirectoryWrapperDictionary = [[NSMutableDictionary alloc] init];
        m_allAccessoryFileWrapperDictionary = [[NSMutableDictionary alloc] init];
        m_allModelDirectoryWrapperDictionary = [[NSMutableDictionary alloc] init];
        m_allModelFileWrapperDictionary = [[NSMutableDictionary alloc] init];
        m_allMotionDictionary = [[NSMutableDictionary alloc] init];
        m_allAudioURLSet = [[NSMutableSet alloc] init];
        m_allVideoURLSet = [[NSMutableSet alloc] init];
        m_accessoryUUIDDictionary = [[NSMutableDictionary alloc] init];
        m_modelUUIDDictionary = [[NSMutableDictionary alloc] init];
        m_motionUUIDDictionary = [[NSMutableDictionary alloc] init];
        m_uuidAccessoryDictionary = [[NSMutableDictionary alloc] init];
        m_uuidModelDictionary = [[NSMutableDictionary alloc] init];
        m_uuidMotionDictionary = [[NSMutableDictionary alloc] init];
        m_cameraMotionUUID = nil;
        m_lightMotionUUID = nil;
        m_errorType = DocumentErrorTypeNone;
        Project *projectPtr = m_projectHolderPtr->currentProject();
        projectPtr->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
    }
    return self;
}

- (void)destroy
{
    if (m_json) {
        json_value_free(m_json);
        m_json = NULL;
    }
}

#if TARGET_OS_IPHONE

- (BOOL)loadFromContents:(id)contents ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL loaded = NO;
    if ([contents isKindOfClass:[NSFileWrapper class]]) {
        NSFileWrapper *fileWrapper = (NSFileWrapper *) contents;
        @try {
            loaded = [self loadDocument:fileWrapper ofType:typeName error:outError];
        } @catch (NSException *exception) {
            NSDictionary *userInfo =
                [[NSDictionary alloc] initWithObjectsAndKeys:exception.reason, NSLocalizedFailureReasonErrorKey, nil];
            *outError = [[NSError alloc] initWithDomain:NSCocoaErrorDomain code:0 userInfo:userInfo];
        }
    }
    return loaded;
}

- (id)contentsForType:(NSString *)typeName error:(NSError **)outError
{
    NSFileWrapper *fileWrapper = nil;
    @try {
        fileWrapper = [self saveDocument:typeName error:outError];
    } @catch (NSException *exception) {
        NSDictionary *userInfo =
            [[NSDictionary alloc] initWithObjectsAndKeys:exception.reason, NSLocalizedFailureReasonErrorKey, nil];
        *outError = [[NSError alloc] initWithDomain:NSCocoaErrorDomain code:0 userInfo:userInfo];
    }
    return fileWrapper;
}

#else /* TARGET_OS_IPHONE */

static void
handleNSException(NSException *exception, NSError **outError)
{
    if (outError) {
        NSMutableString *s = [NSMutableString string];
        for (NSString *symbol in exception.callStackSymbols) {
            [s appendString:symbol];
            [s appendString:@"\n"];
        }
        NSDictionary *userInfo =
            [NSDictionary dictionaryWithObjectsAndKeys:exception.reason, NSLocalizedFailureReasonErrorKey, s,
                          NSLocalizedRecoverySuggestionErrorKey, nil];
        *outError = [[NSError alloc] initWithDomain:NSCocoaErrorDomain code:0 userInfo:userInfo];
    }
}

+ (BOOL)autosavesInPlace
{
    return NO;
}

- (BOOL)readFromData:(NSData *)data ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL result = NO;
    const URI fileURI(URI::createFromFilePath(self.fileURL.path.UTF8String));
    NSDataReader reader(data);
    @autoreleasepool {
        @try {
            Error error;
            Project *projectPtr = m_projectHolderPtr->currentProject();
            if ([typeName isEqualToString:@"nma"]) {
                self.fileType = typeName;
                projectPtr->setFileURI(fileURI);
                result = projectPtr->loadFromArchive(&reader, fileURI, error) ? YES : NO;
            }
            else if ([typeName isEqualToString:@"pmm"]) {
                self.fileType = typeName;
                projectPtr->setFileURI(fileURI);
                result = projectPtr->loadFromBinary(static_cast<const uint8_t *>(data.bytes), data.length,
                             Project::kBinaryFormatPMM, error, nullptr)
                    ? YES
                    : NO;
            }
            if (!result && outError) {
                *outError = createNSError(error);
            }
        } @catch (NSException *exception) {
            handleNSException(exception, outError);
        }
    }
    return result;
}

- (BOOL)readFromFileWrapper:(NSFileWrapper *)fileWrapper ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL loaded = NO;
    @autoreleasepool {
        @try {
            loaded = [self loadDocument:fileWrapper ofType:typeName error:outError];
            self.fileType = typeName;
        } @catch (NSException *exception) {
            handleNSException(exception, outError);
            loaded = NO;
        }
    }
    return loaded;
}

- (BOOL)readFromURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL loaded = NO;
    if ([typeName isEqualToString:@"nanoem"]) {
        if (NSFileWrapper *fileWrapper = [[NSFileWrapper alloc] initWithURL:absoluteURL options:0 error:outError]) {
            loaded = [self readFromFileWrapper:fileWrapper ofType:typeName error:outError];
        }
    }
    else if ([typeName isEqualToString:@"nma"] || [typeName isEqualToString:@"pmm"]) {
        NSData *data = [[NSData alloc] initWithContentsOfURL:absoluteURL];
        loaded = [self readFromData:data ofType:typeName error:outError];
    }
    return loaded;
}

- (NSData *)dataOfType:(NSString *)typeName error:(NSError **)outError
{
    BX_UNUSED_1(typeName);
    NSData *data = nil;
    ByteArray bytes;
    MemoryWriter writer(&bytes);
    @autoreleasepool {
        @try {
            Error error;
            Project *projectPtr = m_projectHolderPtr->currentProject();
            if (projectPtr->saveAsArchive(&writer, error)) {
                data = [[NSData alloc] initWithBytes:bytes.data() length:bytes.size()];
            }
            else if (outError) {
                *outError = createNSError(error);
            }
        } @catch (NSException *exception) {
            handleNSException(exception, outError);
        }
    }
    return data;
}

- (NSFileWrapper *)fileWrapperOfType:(NSString *)typeName error:(NSError **)outError
{
    NSFileWrapper *fileWrapper = nil;
    @autoreleasepool {
        @try {
            fileWrapper = [self saveDocument:typeName error:outError];
        } @catch (NSException *exception) {
            handleNSException(exception, outError);
        }
    }
    return fileWrapper;
}

- (BOOL)writeToURL:(NSURL *)absoluteURL ofType:(NSString *)typeName error:(NSError **)outError
{
    BOOL saved = NO;
    if ([typeName isEqualToString:@"nanoem"]) {
        if (NSFileWrapper *fileWrapper = [self fileWrapperOfType:typeName error:outError]) {
            saved = [fileWrapper writeToURL:absoluteURL
                                    options:NSFileWrapperWritingAtomic
                        originalContentsURL:self.fileURL
                                      error:outError];
        }
    }
    else if ([typeName isEqualToString:@"nma"]) {
        if (NSData *data = [self dataOfType:typeName error:outError]) {
            saved = [data writeToURL:absoluteURL options:NSDataWritingAtomic error:outError];
        }
    }
    return saved;
}

- (BOOL)canAsynchronouslyWriteToURL:(NSURL *)url
                             ofType:(NSString *)typeName
                   forSaveOperation:(NSSaveOperationType)saveOperation
{
    BX_UNUSED_2(url, typeName);
    return NSAutosaveInPlaceOperation <= saveOperation;
}

#endif /* TARGET_OS_IPHONE */

@end

extern "C" {

struct nanoem_application_document_t {
    Document *m_opaque;
};

nanoem_application_document_t *
nanoemApplicationDocumentCreate(BaseApplicationService *application, IProjectHolder *holder, const char *path)
{
    nanoem_application_document_t *document = new nanoem_application_document_t;
    NSString *filePath = [[NSString alloc] initWithUTF8String:path];
    NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
    Document *opaque = [[Document alloc] initWithFileURL:fileURL application:application projectHolder:holder];
    document->m_opaque = opaque;
    return document;
}

nanoem_bool_t
nanoemApplicationDocumentLoad(nanoem_application_document_t *document, Error *error)
{
    BOOL result = NO;
    if (document) {
        NSError *err = nil;
        Document *opaque = document->m_opaque;
#if TARGET_OS_IOS
        result = [opaque readFromURL:opaque.fileURL error:&err];
#else
        result = [opaque readFromURL:opaque.fileURL ofType:@"nanoem" error:&err];
#endif
        if (result) {
            [opaque load:&err];
            result = err == nil;
            if (err) {
                *error = Error(err.localizedDescription.UTF8String, err.localizedRecoverySuggestion.UTF8String,
                    Error::kDomainTypeOS);
            }
        }
        else if (err) {
            *error = Error(
                err.localizedDescription.UTF8String, err.localizedRecoverySuggestion.UTF8String, Error::kDomainTypeOS);
        }
    }
    return result == YES;
}

nanoem_bool_t
nanoemApplicationDocumentSave(nanoem_application_document_t *document, const char *path, Error *error)
{
    bool saved = false;
    if (document) {
        NSString *filePath = [[NSString alloc] initWithUTF8String:path];
        NSURL *fileURL = [[NSURL alloc] initFileURLWithPath:filePath];
        NSError *err = nil;
        Document *opaque = document->m_opaque;
#if TARGET_OS_IOS
        __block BOOL result = NO;
        __block dispatch_semaphore_t sema = dispatch_semaphore_create(1);
        [opaque saveToURL:fileURL
             forSaveOperation:UIDocumentSaveForOverwriting
            completionHandler:^(BOOL success) {
                result = success;
                dispatch_semaphore_signal(sema);
            }];
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
        saved = result == YES;
#else
        BOOL result = [opaque writeSafelyToURL:fileURL ofType:@"nanoem" forSaveOperation:NSSaveAsOperation error:&err];
        saved = result == YES;
#endif
        if (err) {
            *error = Error(
                err.localizedDescription.UTF8String, err.localizedRecoverySuggestion.UTF8String, Error::kDomainTypeOS);
        }
    }
    else {
        *error = Error("document data seems NULL", "report this incident to the developer.", Error::kDomainTypeOS);
    }
    return saved;
}

void
nanoemApplicationDocumentDestroy(nanoem_application_document_t *document)
{
    if (document) {
        Document *opaque = document->m_opaque;
        [opaque destroy];
        delete document;
    }
}

void
nanoemApplicationDocumentDeleteFile(nanoem_application_document_t *document)
{
    if (document) {
        NSError *err = nil;
        [[NSFileManager defaultManager] removeItemAtURL:document->m_opaque.fileURL error:&err];
    }
}

nanoem_bool_t
nanoemApplicationDocumentIsSupported()
{
    return 1;
}

} /* extern "C" */
