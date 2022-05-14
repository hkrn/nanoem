/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/project/PMM.h"

#include "emapp/AccessoryProgramBundle.h"
#include "emapp/Effect.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/IModelObjectSelection.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/ModelProgramBundle.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "nanoem/ext/document.h"

#define INI_IMPLEMENTATION
#include "../../ini.h"

namespace nanoem {
namespace internal {
namespace project {
namespace {

static const char kUserFileNeedleType1[] = "/UserFile/";
static const char kUserFileNeedleType2[] = "UserFile/";

static URI
concatFileURI(const URI &fileURI, const char *appendPath)
{
    String newPath(fileURI.absolutePathByDeletingLastPathComponent()), canonicalizedPath;
    newPath.append("/");
    newPath.append(appendPath);
    FileUtils::canonicalizePathSeparator(newPath, canonicalizedPath);
    return URI::createFromFilePath(canonicalizedPath);
}

} /* anonymous namespace */

struct PMM::Context {
    struct OrderedDrawable {
        OrderedDrawable(IDrawable *drawable, const nanoem_document_accessory_t *ao);
        OrderedDrawable(IDrawable *drawable, const nanoem_document_model_t *mo);
        static int sortByDraw(const void *left, const void *right);
        static int sortByTransform(const void *left, const void *right);
        IDrawable *m_drawable;
        int m_drawIndex;
        int m_transformIndex;
    };
    class EffectMap {
    public:
        typedef tinystl::unordered_map<String, StringMap, TinySTLAllocator> TreeMap;

        EffectMap(Project *project);
        ~EffectMap() NANOEM_DECL_NOEXCEPT;

        void load(const ByteArray &bytes, const Context *parent);
        bool attachEffect(IDrawable *drawable, const String &filePath, Progress &progress, Error &error);
        void attachAllOffscreenRenderTargetAttachmentEffects(Effect *effect, Progress &progress, Error &error);
        void attachAllOffscreenOwnerMainAttachmentEffects(Progress &progress, Error &error);
        String resolveFilePath(const String &filePath) const;
        String findByName(const String &name, const String &offscreenOwnerName) const;
        TreeMap dump() const;

    private:
        Effect *compileEffect(const String &filePath, effect::AttachmentType type, Progress &progress, Error &error);
        void loadAllObjectFilePaths(const ini_t *ini, const Context *parent);
        void loadAllOffscreenEffectsProperties(const ini_t *ini, const Context *parent);
        void loadOffscreenEffectProperties(
            const ini_t *ini, int sectionIndex, const String &offscreenOwnerName, const Context *parent);
        void attachOffscreenRenderTargetEffect(Effect *effect, const String &offscreenOwnerName, const String &filePath,
            const URI &fileURI, Progress &progress, Error &error);
        void resetOffscreenOwner(Effect *effect, const String &key);
        void setOffscreenEffectProperty(const String &offscreenOwnerName, const String &key, const char *p,
            const String &value, Progress &progress, Error &error);

        typedef tinystl::unordered_map<String, Effect *, TinySTLAllocator> CacheMap;
        Project *m_project;
        StringMap m_effectPropertyKey2FilePaths;
        StringMap m_filePath2EffectPropertyKeys;
        TreeMap m_offscreenEffectProperties;
    };
    typedef tinystl::vector<OrderedDrawable, TinySTLAllocator> OrderedDrawableList;
    typedef tinystl::unordered_map<int, Accessory *, TinySTLAllocator> PMMAccessoryHandleMap;
    typedef tinystl::unordered_map<int, Model *, TinySTLAllocator> PMMModelHandleMap;
    typedef tinystl::unordered_map<const Model *, nanoem_document_model_t *, TinySTLAllocator> ModelResolveMap;

    static nanoem_model_t *handleLoadingModel(void *user_data, const nanoem_unicode_string_t *path,
        nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

    Context(Project *project);
    ~Context() NANOEM_DECL_NOEXCEPT;

    bool load(const nanoem_u8_t *data, size_t size, Error &error, Project::IDiagnostics *diagnostics);
    bool save(ByteArray &bytes, Error &error);
    URI fileURI() const;
    void setFileURI(const URI &value);

    URI relativeFileURI() const;
    URI resolveFileURI(const nanoem_unicode_string_t *path) const;
    bool loadEffectMetadata(Error &error, EffectMap &effectMap);
    void loadDefaultEffect(Progress &progress, Error &error, EffectMap &effectMap);
    void loadAllAccessories(const nanoem_document_t *document, OrderedDrawableList &drawables,
        PMMAccessoryHandleMap &accessoryHandles, EffectMap &effectMap, Progress &progress, StringSet &reservedNameSet,
        Error &error, Project::IDiagnostics *diagnostics);
    void loadAccessory(const nanoem_document_accessory_t *ao, Accessory *accessory, StringSet &reservedNameSet,
        nanoem_status_t *mutableStatus);
    void loadAllModels(const nanoem_document_t *document, OrderedDrawableList &drawables,
        PMMModelHandleMap &modelHandles, EffectMap &effectMap, Progress &progress, StringSet &reservedNameSet,
        Error &error, Project::IDiagnostics *diagnostics);
    void loadModel(
        const nanoem_document_model_t *mo, Model *model, StringSet &reservedNameSet, nanoem_status_t *mutableStatus);
    void loadCamera(
        const nanoem_document_t *document, const PMMModelHandleMap &modelHandles, nanoem_status_t *mutableStatus);
    void loadLight(const nanoem_document_t *document, nanoem_status_t *mutableStatus);
    void loadSelfShadow(const nanoem_document_t *document, nanoem_status_t *mutableStatus);
    void configureAllAccessoryOutsideParents(const nanoem_document_t *document,
        const PMMAccessoryHandleMap &accessoryHandles, const PMMModelHandleMap &modelHandles,
        nanoem_status_t *mutableStatus);
    void configureAccessoryOutsideParent(const nanoem_document_accessory_t *ao, Accessory *accessory,
        const PMMModelHandleMap &modelHandles, nanoem_status_t *mutableStatus);
    void configureAllModelOutsideParents(
        const nanoem_document_t *document, const PMMModelHandleMap &modelHandles, nanoem_status_t *mutableStatus);
    void configureModelOutsideParent(
        const nanoem_document_model_t *mo, Model *subjectModel, nanoem_status_t *mutableStatus);
    void configureModelOutsideParentKeyframes(const nanoem_document_model_t *mo, Motion *subjectModelMotion,
        const nanoem_unicode_string_t *subjectBoneName, const StringPair &outsideParentStringPair,
        nanoem_status_t *mutableStatus);
    void loadAudio(
        const nanoem_document_t *document, Progress &progress, Error &error, Project::IDiagnostics *diagnostics);
    void loadBackgroundImage(
        const nanoem_document_t *document, Progress &progress, Error &error, Project::IDiagnostics *diagnostics);
    void loadBackgroundVideo(
        const nanoem_document_t *document, Progress &progress, Error &error, Project::IDiagnostics *diagnostics);

    void saveAudioSource(nanoem_mutable_document_t *document, nanoem_status_t *status);
    void saveBackgroundVideo(nanoem_mutable_document_t *document, nanoem_status_t *status);
    void saveCamera(nanoem_mutable_document_t *document, const ModelResolveMap &resolveMap, nanoem_status_t *status);
    void saveGravity(nanoem_mutable_document_t *document, nanoem_status_t *status);
    void saveLight(nanoem_mutable_document_t *document, nanoem_status_t *status);
    void saveSelfShadow(nanoem_mutable_document_t *document, nanoem_status_t *status);
    void saveAllAccessories(
        nanoem_mutable_document_t *document, const ModelResolveMap &resolveMap, nanoem_status_t *status);
    void saveAllModels(nanoem_mutable_document_t *document, ModelResolveMap &resolveMap, nanoem_status_t *status);
    void saveAllModelOutsideParents(
        nanoem_mutable_document_t *document, const ModelResolveMap &resolveMap, nanoem_status_t *status);
    void saveAccessory(const Accessory *accessory, const ModelResolveMap &resolveMap, int index,
        nanoem_mutable_document_t *document, nanoem_status_t *status);
    void saveAllAccessoryKeyframes(const Motion *motion, const ModelResolveMap &resolveMap,
        nanoem_mutable_document_accessory_t *ao, nanoem_status_t *status);
    void saveModel(const Model *model, int drawOrderIndex, int transformOrderIndex, nanoem_mutable_document_t *document,
        ModelResolveMap &resolveMap, nanoem_status_t *status);
    void saveAllBoneKeyframes(const Motion *motion, nanoem_mutable_document_model_t *mo, nanoem_status_t *status);
    void saveAllModelKeyframes(const Motion *motion, nanoem_mutable_document_model_t *mo, nanoem_status_t *status);
    void saveAllMorphKeyframes(const Motion *motion, nanoem_mutable_document_model_t *mo, nanoem_status_t *status);
    String makeRelativeFromProjectPath(const URI &fileURI) const;

    Project *m_project;
    URI m_fileURI;
    URI m_relativeFileURI;
};

PMM::Context::OrderedDrawable::OrderedDrawable(IDrawable *drawable, const nanoem_document_accessory_t *ao)
    : m_drawable(drawable)
    , m_drawIndex(nanoemDocumentAccessoryGetDrawOrderIndex(ao))
    , m_transformIndex(-1)
{
}

PMM::Context::OrderedDrawable::OrderedDrawable(IDrawable *drawable, const nanoem_document_model_t *mo)
    : m_drawable(drawable)
    , m_drawIndex(nanoemDocumentModelGetDrawOrderIndex(mo))
    , m_transformIndex(nanoemDocumentModelGetTransformOrderIndex(mo))
{
}

int
PMM::Context::OrderedDrawable::sortByDraw(const void *left, const void *right)
{
    const OrderedDrawable *lo = static_cast<const OrderedDrawable *>(left);
    const OrderedDrawable *ro = static_cast<const OrderedDrawable *>(right);
    return lo->m_drawIndex - ro->m_drawIndex;
}

int
PMM::Context::OrderedDrawable::sortByTransform(const void *left, const void *right)
{
    const OrderedDrawable *lo = static_cast<const OrderedDrawable *>(left);
    const OrderedDrawable *ro = static_cast<const OrderedDrawable *>(right);
    return lo->m_transformIndex - ro->m_transformIndex;
}

PMM::Context::EffectMap::EffectMap(Project *project)
    : m_project(project)
{
}

PMM::Context::EffectMap::~EffectMap() NANOEM_DECL_NOEXCEPT
{
}

void
PMM::Context::EffectMap::load(const ByteArray &bytes, const Context *parent)
{
    ini_t *ini = ini_load(reinterpret_cast<const char *>(bytes.data()), nullptr);
    loadAllObjectFilePaths(ini, parent);
    loadAllOffscreenEffectsProperties(ini, parent);
    ini_destroy(ini);
}

bool
PMM::Context::EffectMap::attachEffect(IDrawable *drawable, const String &filePath, Progress &progress, Error &error)
{
    const String &effectFilePath = resolveFilePath(filePath);
    bool succeeded = false;
    if (!effectFilePath.empty()) {
        if (Effect *effect = compileEffect(effectFilePath, effect::kAttachmentTypeNone, progress, error)) {
            m_project->attachActiveEffect(drawable, effect, progress, error);
            succeeded = !error.hasReason();
        }
    }
    return succeeded;
}

void
PMM::Context::EffectMap::attachAllOffscreenRenderTargetAttachmentEffects(
    Effect *effect, Progress &progress, Error &error)
{
    effect::OffscreenRenderTargetOptionList options;
    effect->getAllOffscreenRenderTargetOptions(options);
    bool continueable = true;
    for (effect::OffscreenRenderTargetOptionList::const_iterator it = options.begin(), end = options.end();
         continueable && it != end; ++it) {
        const String &offscreenOwnerName = it->m_name;
        TreeMap::const_iterator it2 = m_offscreenEffectProperties.find(offscreenOwnerName);
        if (it2 != m_offscreenEffectProperties.end()) {
            const StringMap &effectProperties = it2->second;
            for (StringMap::const_iterator it3 = effectProperties.begin(), end3 = effectProperties.end(); it3 != end3;
                 ++it3) {
                const String &key = it3->first;
                StringMap::const_iterator it4 = m_effectPropertyKey2FilePaths.find(key);
                if (it4 != m_effectPropertyKey2FilePaths.end()) {
                    const String &value = it3->second;
                    const URI &fileURI = URI::createFromFilePath(it4->second);
                    if (!StringUtils::equalsIgnoreCase(value.c_str(), "hide")) {
                        attachOffscreenRenderTargetEffect(effect, offscreenOwnerName, value, fileURI, progress, error);
                        if (error.hasReason()) {
                            continueable = false;
                            break;
                        }
                    }
                }
                else if (StringUtils::equalsIgnoreCase(key.c_str(), "owner")) {
                    resetOffscreenOwner(effect, it3->second);
                }
                else if (const char *p = StringUtils::indexOf(key, '.')) {
                    setOffscreenEffectProperty(offscreenOwnerName, key, p, it3->second, progress, error);
                }
            }
        }
    }
}

void
PMM::Context::EffectMap::attachAllOffscreenOwnerMainAttachmentEffects(Progress &progress, Error &error)
{
    TreeMap::const_iterator it2 = m_offscreenEffectProperties.find(Effect::kOffscreenOwnerNameMain);
    if (it2 != m_offscreenEffectProperties.end()) {
        const StringMap &effectProperties = it2->second;
        for (StringMap::const_iterator it3 = effectProperties.begin(), end3 = effectProperties.end(); it3 != end3;
             ++it3) {
            const String &key = it3->first;
            StringMap::const_iterator it4 = m_effectPropertyKey2FilePaths.find(key);
            if (it4 != m_effectPropertyKey2FilePaths.end()) {
                const String &value = it3->second;
                const URI &fileURI = URI::createFromFilePath(it4->second);
                if (!StringUtils::equalsIgnoreCase(value.c_str(), "hide")) {
                    attachOffscreenRenderTargetEffect(
                        nullptr, Effect::kOffscreenOwnerNameMain, value, fileURI, progress, error);
                }
            }
            else {
                const char *name = StringUtils::indexOf(key, '.');
                setOffscreenEffectProperty(Effect::kOffscreenOwnerNameMain, key, name, it3->second, progress, error);
            }
        }
    }
}

String
PMM::Context::EffectMap::resolveFilePath(const String &filePath) const
{
    StringMap::const_iterator it = m_filePath2EffectPropertyKeys.find(filePath);
    String effectPath;
    if (it != m_filePath2EffectPropertyKeys.end()) {
        const String &value = findByName(it->second, String());
        if (value.empty() || StringUtils::equalsIgnoreCase(value.c_str(), "none")) {
            effectPath = URI::stringByDeletingPathExtension(filePath);
            effectPath.append(".");
            effectPath.append(Effect::kSourceFileExtension);
        }
        else {
            effectPath = value;
        }
    }
    return effectPath;
}

String
PMM::Context::EffectMap::findByName(const String &name, const String &offscreenOwnerName) const
{
    TreeMap::const_iterator it = m_offscreenEffectProperties.find(offscreenOwnerName);
    String value;
    if (it != m_offscreenEffectProperties.end()) {
        const StringMap &properties = it->second;
        StringMap::const_iterator it2 = properties.find(name);
        if (it2 != properties.end()) {
            value = it2->second;
        }
    }
    return value;
}

PMM::Context::EffectMap::TreeMap
PMM::Context::EffectMap::dump() const
{
    return m_offscreenEffectProperties;
}

Effect *
PMM::Context::EffectMap::compileEffect(
    const String &filePath, effect::AttachmentType type, Progress &progress, Error &error)
{
    IFileManager *fileManager = m_project->fileManager();
    const URI effectFileURI(URI::createFromFilePath(filePath)),
        effectResolvedURI(Effect::resolveSourceURI(fileManager, effectFileURI));
    Effect *effect = nullptr;
    if (!effectResolvedURI.isEmpty()) {
        effect = m_project->findEffect(effectResolvedURI);
        if (!effect) {
            ByteArray bytes;
            if (Effect::compileFromSource(
                    effectResolvedURI, fileManager, m_project->isMipmapEnabled(), bytes, progress, error)) {
                Effect *innerEffect = m_project->createEffect();
                innerEffect->setName(effectResolvedURI.lastPathComponent());
                bool succeeded = false;
                if (innerEffect->load(bytes, progress, error)) {
                    innerEffect->setFileURI(effectResolvedURI);
                    if (innerEffect->upload(type, progress, error)) {
                        effect = innerEffect;
                        succeeded = !error.hasReason();
                    }
                }
                if (!succeeded) {
                    m_project->destroyEffect(innerEffect);
                    effect = nullptr;
                }
            }
        }
    }
    return effect;
}

void
PMM::Context::EffectMap::loadAllObjectFilePaths(const ini_t *ini, const Context *parent)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    int objectSectionIndex = ini_find_section(ini, "Object", 0);
    if (objectSectionIndex >= 0) {
        StringUtils::UnicodeStringScope us(factory);
        int numObjects = ini_property_count(ini, objectSectionIndex);
        for (int i = 0; i < numObjects; i++) {
            const char *name = ini_property_name(ini, objectSectionIndex, i);
            const char *filePath = ini_property_value(ini, objectSectionIndex, i);
            String effectPropertyKey, utf8FilePath;
            while (bx::isAlphaNum(*name)) {
                effectPropertyKey.append(name, name + 1);
                name++;
            }
            StringUtils::getUtf8String(
                filePath, StringUtils::length(filePath), NANOEM_CODEC_TYPE_SJIS, factory, utf8FilePath);
            if (StringUtils::tryGetString(factory, utf8FilePath, us)) {
                const URI &fileURI = parent->resolveFileURI(us.value());
                if (!fileURI.isEmpty()) {
                    const String &filePath = fileURI.absolutePath();
                    m_filePath2EffectPropertyKeys.insert(tinystl::make_pair(filePath, effectPropertyKey));
                    m_effectPropertyKey2FilePaths.insert(tinystl::make_pair(effectPropertyKey, filePath));
                }
            }
        }
    }
}

void
PMM::Context::EffectMap::loadAllOffscreenEffectsProperties(const ini_t *ini, const Context *parent)
{
    for (int i = 0, numSections = ini_section_count(ini); i < numSections; i++) {
        const char *sectionName = ini_section_name(ini, i);
        const char prefix[] = "Effect";
        if (StringUtils::hasPrefix(sectionName, prefix)) {
            String offscreenOwnerName(Effect::kOffscreenOwnerNameMain);
            const size_t prefixLength = sizeof(prefix) - 1;
            if (*(sectionName + prefixLength) == '@') {
                offscreenOwnerName = sectionName + prefixLength + 1;
            }
            loadOffscreenEffectProperties(ini, i, offscreenOwnerName, parent);
        }
    }
}

void
PMM::Context::EffectMap::loadOffscreenEffectProperties(
    const ini_t *ini, int sectionIndex, const String &offscreenOwnerName, const Context *parent)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    int numEffects = ini_property_count(ini, sectionIndex);
    for (int i = 0; i < numEffects; i++) {
        const char *name = ini_property_name(ini, sectionIndex, i);
        const char *value = ini_property_value(ini, sectionIndex, i);
        String utf8Value, key;
        while (!bx::isSpace(*name)) {
            key.append(name, name + 1);
            name++;
        }
        StringUtils::UnicodeStringScope us(factory);
        StringUtils::getUtf8String(value, StringUtils::length(value), NANOEM_CODEC_TYPE_SJIS, factory, utf8Value);
        if (StringUtils::tryGetString(factory, utf8Value, us)) {
            const URI &fileURI = parent->resolveFileURI(us.value());
            m_offscreenEffectProperties[offscreenOwnerName].insert(
                tinystl::make_pair(key, fileURI.isEmpty() ? utf8Value : fileURI.absolutePath()));
        }
    }
}

void
PMM::Context::EffectMap::attachOffscreenRenderTargetEffect(Effect *effect, const String &offscreenOwnerName,
    const String &filePath, const URI &fileURI, Progress &progress, Error &error)
{
    Error innerError;
    IDrawable *drawable = nullptr;
    if (Accessory *accessory = m_project->findAccessoryByURI(fileURI)) {
        drawable = accessory;
    }
    else if (Model *model = m_project->findModelByURI(fileURI)) {
        drawable = model;
    }
    if (drawable) {
        Effect *innerEffect = compileEffect(filePath, effect::kAttachmentTypeOffscreenPassive, progress, innerError);
        m_project->setOffscreenPassiveRenderTargetEffect(offscreenOwnerName, drawable, innerEffect);
    }
    if (effect && innerError.hasReason()) {
        effect->setEnabled(false);
        error = innerError;
    }
}

void
PMM::Context::EffectMap::resetOffscreenOwner(Effect *effect, const String &key)
{
    StringMap::const_iterator it = m_effectPropertyKey2FilePaths.find(key);
    if (it != m_effectPropertyKey2FilePaths.end()) {
        const URI &fileURI = URI::createFromFilePath(it->second);
        IDrawable *drawable = nullptr;
        if (Accessory *accessory = m_project->findAccessoryByURI(fileURI)) {
            drawable = accessory;
        }
        else if (Model *model = m_project->findModelByURI(fileURI)) {
            drawable = model;
        }
        if (drawable) {
            drawable->setActiveEffect(effect);
        }
    }
}

void
PMM::Context::EffectMap::setOffscreenEffectProperty(const String &offscreenOwnerName, const String &key,
    const char *name, const String &value, Progress &progress, Error &error)
{
    String newKey, propertyName;
    if (name) {
        newKey = String(key.c_str(), name - key.c_str());
        propertyName = String(name + 1);
    }
    else {
        newKey = key;
    }
    int materialOffset = -1;
    if (const char *blockStart = StringUtils::indexOf(newKey, '[')) {
        if (const char *blockEnd = StringUtils::indexOf(blockStart, ']')) {
            const char *offsetString = blockStart + 1;
            const String materialOffsetString(offsetString, blockEnd - offsetString);
            newKey = String(newKey.c_str(), blockStart - newKey.c_str());
            materialOffset = StringUtils::parseInteger(materialOffsetString, nullptr);
        }
    }
    StringMap::const_iterator it = m_effectPropertyKey2FilePaths.find(newKey);
    if (it != m_effectPropertyKey2FilePaths.end()) {
        const URI &fileURI = URI::createFromFilePath(it->second);
        Model *modelPtr = nullptr;
        IDrawable *drawable = nullptr;
        if (Accessory *accessory = m_project->findAccessoryByURI(fileURI)) {
            drawable = accessory;
        }
        else if (Model *model = m_project->findModelByURI(fileURI)) {
            drawable = model;
            modelPtr = model;
        }
        if (drawable) {
            if (StringUtils::equalsIgnoreCase(propertyName.c_str(), "show") && materialOffset == -1) {
                drawable->setOffscreenPassiveRenderTargetEffectEnabled(
                    offscreenOwnerName, StringUtils::equalsIgnoreCase(value.c_str(), "true"));
            }
            else if (materialOffset != -1 && modelPtr) {
                nanoem_rsize_t numMaterials;
                nanoem_model_material_t *const *materials =
                    nanoemModelGetAllMaterialObjects(modelPtr->data(), &numMaterials);
                if (materialOffset >= 0 && materialOffset < Inline::saturateInt32(numMaterials)) {
                    Error innerError;
                    if (Effect *innerEffect =
                            compileEffect(value, effect::kAttachmentTypeOffscreenPassive, progress, innerError)) {
                        model::Material *material = model::Material::cast(materials[materialOffset]);
                        m_project->attachModelMaterialEffect(material, innerEffect);
                    }
                    if (innerError.hasReason()) {
                        error = innerError;
                    }
                }
            }
        }
    }
}

nanoem_model_t *
PMM::Context::handleLoadingModel(void *user_data, const nanoem_unicode_string_t *path,
    nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    Context *self = static_cast<Context *>(user_data);
    nanoem_model_t *model = nullptr;
    FileReaderScope scope(self->m_project->translator());
    Error error;
    if (scope.open(self->resolveFileURI(path), error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        nanoem_buffer_t *buffer = nanoemBufferCreate(bytes.data(), bytes.size(), status);
        model = nanoemModelCreate(factory, status);
        nanoemModelLoadFromBuffer(model, buffer, status);
        nanoemBufferDestroy(buffer);
    }
    else {
        *status = NANOEM_STATUS_ERROR_NULL_OBJECT;
    }
    return model;
}

PMM::Context::Context(Project *project)
    : m_project(project)
{
}

PMM::Context::~Context() NANOEM_DECL_NOEXCEPT
{
}

bool
PMM::Context::load(const nanoem_u8_t *data, size_t size, Error &error, Project::IDiagnostics *diagnostics)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_buffer_t *buffer = nanoemBufferCreate(data, size, &status);
    nanoem_document_t *document = nanoemDocumentCreate(factory, &status);
    nanoemDocumentSetParseModelCallback(document, &PMM::Context::handleLoadingModel);
    nanoemDocumentSetParseModelCallbackUserData(document, this);
    if (nanoemDocumentLoadFromBuffer(document, buffer, &status)) {
        Context::OrderedDrawableList drawables;
        StringSet reservedNameSet;
        Context::PMMAccessoryHandleMap accessoryHandles;
        Context::PMMModelHandleMap modelHandles;
        nanoem_rsize_t numAccessories, numModels;
        bool isEffectPluginEnabled = m_project->isEffectPluginEnabled();
        m_project->setEffectPluginEnabled(false);
        nanoem_document_accessory_t *const *accessories =
            nanoemDocumentGetAllAccessoryObjects(document, &numAccessories);
        nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(document, &numModels);
        static const nanoem_u32_t kAdditionalProgressLoadingItems = 4;
        Progress progress(
            m_project, Inline::saturateInt32U(numAccessories + numModels + kAdditionalProgressLoadingItems));
        Context::EffectMap effectMap(m_project);
        isEffectPluginEnabled |= loadEffectMetadata(error, effectMap);
        loadAllAccessories(
            document, drawables, accessoryHandles, effectMap, progress, reservedNameSet, error, diagnostics);
        loadAllModels(document, drawables, modelHandles, effectMap, progress, reservedNameSet, error, diagnostics);
        loadDefaultEffect(progress, error, effectMap);
        loadCamera(document, modelHandles, &status);
        loadLight(document, &status);
        loadSelfShadow(document, &status);
        configureAllAccessoryOutsideParents(document, accessoryHandles, modelHandles, &status);
        configureAllModelOutsideParents(document, modelHandles, &status);
        qsort(drawables.data(), drawables.size(), sizeof(drawables[0]), Context::OrderedDrawable::sortByDraw);
        Project::DrawableList drawableOrderList;
        for (Context::OrderedDrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end;
             ++it) {
            IDrawable *drawable = it->m_drawable;
            drawableOrderList.push_back(drawable);
        }
        m_project->setDrawableOrderList(drawableOrderList);
        qsort(drawables.data(), drawables.size(), sizeof(drawables[0]), Context::OrderedDrawable::sortByTransform);
        Project::ModelList transformOrderList;
        for (Context::OrderedDrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end;
             ++it) {
            IDrawable *drawable = it->m_drawable;
            if (it->m_transformIndex >= 0) {
                transformOrderList.push_back(static_cast<Model *>(drawable));
            }
        }
        loadAudio(document, progress, error, diagnostics);
        loadBackgroundImage(document, progress, error, diagnostics);
        loadBackgroundVideo(document, progress, error, diagnostics);
        if (nanoemDocumentIsBlackBackgroundEnabled(document)) {
            m_project->setViewportBackgroundColor(Vector4(0, 0, 0, 1));
        }
        m_project->setPreferredMotionFPS(nanoem_u32_t(nanoemDocumentGetPreferredFPS(document)), false);
        m_project->setTransformOrderList(transformOrderList);
        m_project->setSelectionSegment(TimelineSegment(m_project));
        m_project->setViewportWithTransparentEnabled(false);
        effectMap.attachAllOffscreenOwnerMainAttachmentEffects(progress, error);
        for (Context::OrderedDrawableList::const_iterator it = drawables.begin(), end = drawables.end(); it != end;
             ++it) {
            IDrawable *drawable = it->m_drawable;
            if (Effect *effect = m_project->resolveEffect(drawable)) {
                effectMap.attachAllOffscreenRenderTargetAttachmentEffects(effect, progress, error);
            }
        }
        switch (nanoemDocumentGetPhysicsSimulationMode(document)) {
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_DISABLE:
        default:
            m_project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeDisable);
            break;
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_ANYTIME:
            m_project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableAnytime);
            break;
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_PLAYING:
            m_project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnablePlaying);
            break;
        case NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_TRACING:
            m_project->setPhysicsSimulationMode(PhysicsEngine::kSimulationModeEnableTracing);
            break;
        }
        switch (nanoemDocumentGetEditingMode(document)) {
        case NANOEM_DOCUMENT_EDITING_MODE_MOVE:
            m_project->setEditingMode(Project::kEditingModeMove);
            break;
        case NANOEM_DOCUMENT_EDITING_MODE_NONE:
            m_project->setEditingMode(Project::kEditingModeNone);
            break;
        case NANOEM_DOCUMENT_EDITING_MODE_ROTATE:
            m_project->setEditingMode(Project::kEditingModeRotate);
            break;
        case NANOEM_DOCUMENT_EDITING_MODE_SELECT:
        default:
            m_project->setEditingMode(Project::kEditingModeSelect);
            break;
        case NANOEM_DOCUMENT_EDITING_MODE_SELECT_BOX:
            m_project->setEditingMode(Project::kEditingModeSelect);
            break;
        }
        m_project->setEffectPluginEnabled(isEffectPluginEnabled);
        const nanoem_frame_index_t currentLocalFrameIndex(nanoemDocumentGetCurrentFrameIndex(document));
        TimelineSegment segment;
        segment.m_enableFrom = nanoemDocumentIsBeginFrameIndexEnabled(document) != 0;
        segment.m_from = nanoemDocumentGetBeginFrameIndex(document);
        segment.m_enableTo = nanoemDocumentIsEndFrameIndexEnabled(document) != 0;
        segment.m_to = nanoemDocumentGetEndFrameIndex(document);
        m_project->setPlayingSegment(segment);
        m_project->setLoopEnabled(nanoemDocumentIsLoopEnabled(document) != 0);
        m_project->setGroundShadowEnabled(nanoemDocumentIsGroundShadowShown(document) != 0);
        m_project->physicsEngine()->setGroundEnabled(nanoemDocumentIsPhysicsGroundEnabled(document) != 0);
        m_project->grid()->setVisible(nanoemDocumentIsGridAndAxisShown(document) != 0);
        m_project->seek(currentLocalFrameIndex, true);
        m_project->restart(currentLocalFrameIndex);
        m_project->update();
        const nanoem_u32_t viewportWidth = nanoemDocumentGetViewportWidth(document),
                           viewportHeight = nanoemDocumentGetViewportHeight(document);
        m_project->setViewportImageSize(Vector2UI16(viewportWidth, viewportHeight));
        for (Context::PMMAccessoryHandleMap::const_iterator it = accessoryHandles.begin(), end = accessoryHandles.end();
             it != end; ++it) {
            nanoem_rsize_t index = static_cast<nanoem_rsize_t>(it->first);
            if (index < numAccessories) {
                const nanoem_document_accessory_t *ao = accessories[index];
                Accessory *accessory = it->second;
                accessory->setTranslation(glm::make_vec3(nanoemDocumentAccessoryGetTranslation(ao)));
                accessory->setOrientation(glm::make_vec3(nanoemDocumentAccessoryGetOrientation(ao)));
                accessory->setScaleFactor(nanoemDocumentAccessoryGetScaleFactor(ao));
                accessory->setOpacity(nanoemDocumentAccessoryGetOpacity(ao));
                accessory->setVisible(nanoemDocumentAccessoryIsVisible(ao) != 0);
                accessory->setAddBlendEnabled(nanoemDocumentAccessoryIsAddBlendEnabled(ao) != 0);
                accessory->setGroundShadowEnabled(nanoemDocumentAccessoryIsShadowEnabled(ao) != 0);
            }
        }
        for (Context::PMMModelHandleMap::const_iterator it = modelHandles.begin(), end = modelHandles.end(); it != end;
             ++it) {
            nanoem_rsize_t index = static_cast<nanoem_rsize_t>(it->first);
            if (index < numModels) {
                const nanoem_document_model_t *mo = models[index];
                Model *model = it->second;
                model->setAddBlendEnabled(nanoemDocumentModelIsBlendEnabled(mo));
                model->setEdgeSizeScaleFactor(nanoemDocumentModelGetEdgeWidth(mo));
                model->setShadowMapEnabled(nanoemDocumentModelIsSelfShadowEnabled(mo));
            }
        }
        progress.complete();
    }
    nanoemDocumentDestroy(document);
    nanoemBufferDestroy(buffer);
    if (status != NANOEM_STATUS_SUCCESS) {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Loading PMM error: %s",
            Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message, status, Error::kDomainTypeNanoem);
    }
    return !error.isCancelled() && !error.hasReason();
}

bool
PMM::Context::save(ByteArray &bytes, Error &error)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_mutable_document_t *document = nanoemMutableDocumentCreate(factory, &status);
    ModelResolveMap resolveMap;
    saveAllModels(document, resolveMap, &status);
    saveAllModelOutsideParents(document, resolveMap, &status);
    saveAllAccessories(document, resolveMap, &status);
    saveAudioSource(document, &status);
    saveBackgroundVideo(document, &status);
    saveCamera(document, resolveMap, &status);
    saveGravity(document, &status);
    saveLight(document, &status);
    saveSelfShadow(document, &status);
    const Vector2UI16 viewportImageSize(m_project->viewportImageSize());
    nanoemMutableDocumentSetViewportWidth(document, viewportImageSize.x);
    nanoemMutableDocumentSetViewportHeight(document, viewportImageSize.y);
    const PhysicsEngine *engine = m_project->physicsEngine();
    switch (engine->simulationMode()) {
    case PhysicsEngine::kSimulationModeDisable:
    default: {
        nanoemMutableDocumentSetPhysicsSimulationMode(document, NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_DISABLE);
        break;
    }
    case PhysicsEngine::kSimulationModeEnableAnytime: {
        nanoemMutableDocumentSetPhysicsSimulationMode(document, NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_ANYTIME);
        break;
    }
    case PhysicsEngine::kSimulationModeEnablePlaying: {
        nanoemMutableDocumentSetPhysicsSimulationMode(document, NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_PLAYING);
        break;
    }
    case PhysicsEngine::kSimulationModeEnableTracing: {
        nanoemMutableDocumentSetPhysicsSimulationMode(document, NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_TRACING);
        break;
    }
    }
    switch (m_project->editingMode()) {
    case Project::kEditingModeMove: {
        nanoemMutableDocumentSetEditingMode(document, NANOEM_DOCUMENT_EDITING_MODE_MOVE);
        break;
    }
    case Project::kEditingModeRotate: {
        nanoemMutableDocumentSetEditingMode(document, NANOEM_DOCUMENT_EDITING_MODE_ROTATE);
        break;
    }
    case Project::kEditingModeSelect: {
        const Model *model = m_project->activeModel();
        if (model && model->selection()->isBoxSelectedBoneModeEnabled()) {
            nanoemMutableDocumentSetEditingMode(document, NANOEM_DOCUMENT_EDITING_MODE_SELECT_BOX);
        }
        else {
            nanoemMutableDocumentSetEditingMode(document, NANOEM_DOCUMENT_EDITING_MODE_SELECT);
        }
        break;
    }
    case Project::kEditingModeNone: {
    default:
        nanoemMutableDocumentSetEditingMode(document, NANOEM_DOCUMENT_EDITING_MODE_NONE);
        break;
    }
    }
    nanoem_frame_index_t currentLocalFrameIndex = m_project->currentLocalFrameIndex();
    nanoemMutableDocumentSetCurrentFrameIndex(document, currentLocalFrameIndex);
    const TimelineSegment segment(m_project->playingSegment());
    nanoemMutableDocumentSetBeginFrameIndex(document, segment.m_from);
    nanoemMutableDocumentSetBeginFrameIndexEnabled(document, segment.m_enableFrom);
    nanoemMutableDocumentSetEndFrameIndex(document, segment.m_to);
    nanoemMutableDocumentSetEndFrameIndexEnabled(document, segment.m_enableTo);
    nanoemMutableDocumentSetLoopEnabled(document, m_project->isLoopEnabled());
    nanoemMutableDocumentSetGroundShadowShown(document, m_project->isGroundShadowEnabled());
    nanoemMutableDocumentSetPhysicsGroundEnabled(document, engine->isGroundEnabled());
    nanoemMutableDocumentSetGridAndAxisShown(document, m_project->grid()->isVisible());
    nanoemMutableDocumentSetInformationShown(document, m_project->isFPSCounterEnabled());
    nanoem_mutable_buffer_t *mutableBuffer = nanoemMutableBufferCreate(&status);
    nanoemMutableDocumentSaveToBuffer(document, mutableBuffer, &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutableBuffer, &status);
    const nanoem_u8_t *dataPtr = nanoemBufferGetDataPtr(buffer);
    bytes.assign(dataPtr, dataPtr + nanoemBufferGetLength(buffer));
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mutableBuffer);
    nanoemMutableDocumentDestroy(document);
    if (status != NANOEM_STATUS_SUCCESS) {
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Saving PMM error: %s",
            Error::convertStatusToMessage(status, m_project->translator()));
        error = Error(message, status, Error::kDomainTypeNanoem);
    }
    return !error.isCancelled() && !error.hasReason();
}

URI
PMM::Context::fileURI() const
{
    return m_fileURI;
}

void
PMM::Context::setFileURI(const URI &value)
{
    m_fileURI = value;
    const char *basePtr = value.absolutePathConstString();
    const bx::StringView &view = bx::strFind(basePtr, kUserFileNeedleType1);
    if (!view.isEmpty()) {
        m_relativeFileURI = URI::createFromFilePath(String(basePtr, view.getTerm() - basePtr));
    }
}

URI
PMM::Context::relativeFileURI() const
{
    return m_relativeFileURI.isEmpty() ? m_fileURI : m_relativeFileURI;
}

URI
PMM::Context::resolveFileURI(const nanoem_unicode_string_t *path) const
{
    URI fileURI;
    String filePath;
    StringUtils::getUtf8String(path, m_project->unicodeStringFactory(), filePath);
    if (!filePath.empty()) {
        String canonicalizedPath;
        FileUtils::canonicalizePathSeparator(filePath, canonicalizedPath);
        if (FileUtils::exists(canonicalizedPath.c_str())) {
            fileURI = URI::createFromFilePath(canonicalizedPath);
        }
        else {
            String absoluteCanonicalizedPath(
                URI::stringByDeletingLastPathComponent(m_project->fileURI().absolutePath()));
            absoluteCanonicalizedPath.append("/");
            absoluteCanonicalizedPath.append(canonicalizedPath.c_str());
            if (FileUtils::exists(absoluteCanonicalizedPath.c_str())) {
                fileURI = URI::createFromFilePath(absoluteCanonicalizedPath);
            }
            else if (StringUtils::equals(
                         canonicalizedPath.c_str(), kUserFileNeedleType2, sizeof(kUserFileNeedleType2) - 1)) {
                fileURI =
                    concatFileURI(relativeFileURI(), canonicalizedPath.c_str() + sizeof(kUserFileNeedleType2) - 1);
            }
            else {
                const bx::StringView &view = bx::strFind(canonicalizedPath.c_str(), kUserFileNeedleType1);
                if (!view.isEmpty()) {
                    fileURI = concatFileURI(relativeFileURI(), view.getTerm());
                }
            }
        }
    }
    return fileURI;
}

bool
PMM::Context::loadEffectMetadata(Error &error, EffectMap &effectMap)
{
    String filePath(m_fileURI.absolutePathByDeletingPathExtension());
    filePath.append(".emm");
    bool loaded = false;
    if (FileUtils::exists(filePath.c_str())) {
        FileReaderScope scope(m_project->translator());
        if (scope.open(URI::createFromFilePath(filePath), error)) {
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            bytes.push_back(0);
            effectMap.load(bytes, this);
            loaded = true;
        }
    }
    return loaded;
}

void
PMM::Context::loadDefaultEffect(Progress &progress, Error &error, EffectMap &effectMap)
{
    const String &value = effectMap.findByName("Default", String());
    if (!value.empty()) {
        IFileManager *fileManager = m_project->fileManager();
        const URI &fileURI = URI::createFromFilePath(value),
                  &resolvedURI = Effect::resolveSourceURI(fileManager, fileURI);
        if (!resolvedURI.isEmpty()) {
            ByteArray bytes;
            if (Effect::compileFromSource(
                    resolvedURI, fileManager, m_project->isMipmapEnabled(), bytes, progress, error)) {
                Effect *effect = m_project->createEffect();
                bool attached = false;
                effect->setName(resolvedURI.lastPathComponent());
                if (effect->load(bytes, progress, error)) {
                    effect->setFileURI(resolvedURI);
                    if (effect->upload(effect::kAttachmentTypeMaterial, progress, error)) {
                        Project::ISharedResourceRepository *sharedResourceRepository =
                            m_project->sharedResourceRepository();
                        IEffect *defaultModelEffect = sharedResourceRepository->modelProgramBundle(),
                                *defaultAccessoryEffect = sharedResourceRepository->accessoryProgramBundle();
                        const Project::DrawableList *allDrawables = m_project->drawableOrderList();
                        for (Project::DrawableList::const_iterator it = allDrawables->begin(),
                                                                   end = allDrawables->end();
                             it != end; ++it) {
                            IDrawable *drawable = *it;
                            IEffect *activeEffect = drawable->activeEffect();
                            if (activeEffect == defaultModelEffect || activeEffect == defaultAccessoryEffect) {
                                m_project->attachActiveEffect(drawable, effect, progress, error);
                                attached = !error.isCancelled() && !error.hasReason();
                            }
                        }
                    }
                }
                if (!attached) {
                    m_project->destroyEffect(effect);
                }
            }
        }
    }
}

void
PMM::Context::loadAllAccessories(const nanoem_document_t *document, OrderedDrawableList &drawables,
    PMMAccessoryHandleMap &accessoryHandles, EffectMap &effectMap, Progress &progress, StringSet &reservedNameSet,
    Error &error, Project::IDiagnostics *diagnostics)
{
    nanoem_rsize_t numAccessories, numModels;
    nanoem_document_accessory_t *const *accessories = nanoemDocumentGetAllAccessoryObjects(document, &numAccessories);
    nanoemDocumentGetAllModelObjects(document, &numModels);
    IFileManager *fileManager = m_project->fileManager();
    Accessory *activeAccessory = nullptr;
    const nanoem_rsize_t selectedAccessoryIndex =
        static_cast<nanoem_rsize_t>(nanoemDocumentGetSelectedAccessoryIndex(document));
    const int accessoryIndexAfterModel = nanoemDocumentGetAccessoryIndexAfterModel(document);
    for (nanoem_rsize_t i = 0; i < numAccessories; i++) {
        const nanoem_document_accessory_t *ao = accessories[i];
        const URI &fileURI = resolveFileURI(nanoemDocumentAccessoryGetPath(ao));
        Error innerError;
        if (fileURI.isEmpty()) {
            /* do nothing */
        }
        else if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
        }
        else if (!FileUtils::exists(fileURI)) {
            if (diagnostics) {
                diagnostics->addNotFoundFileURI(fileURI);
            }
        }
        else if (fileManager->loadFromFile(fileURI, IFileManager::kDialogTypeLoadModelFile, m_project, innerError)) {
            const Project::AccessoryList *allAccessories = m_project->allAccessories();
            if (!allAccessories->empty()) {
                Accessory *accessory = allAccessories->back();
                accessoryHandles.insert(tinystl::make_pair(nanoemDocumentAccessoryGetIndex(ao), accessory));
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                loadAccessory(ao, accessory, reservedNameSet, &status);
                if (status == NANOEM_STATUS_SUCCESS) {
                    effectMap.attachEffect(accessory, fileURI.absolutePath(), progress, innerError);
                    OrderedDrawable ordered(accessory, ao);
                    if (accessoryIndexAfterModel > -1 && ordered.m_drawIndex >= accessoryIndexAfterModel) {
                        ordered.m_drawIndex += Inline::saturateInt32(numModels);
                    }
                    drawables.push_back(ordered);
                    if (selectedAccessoryIndex == i) {
                        activeAccessory = accessory;
                    }
                }
                else {
                    const char *message = Error::convertStatusToMessage(status, m_project->translator());
                    error = Error(message, status, Error::kDomainTypeNanoem);
                }
            }
            progress.increment();
        }
        if (innerError.hasReason()) {
            error = innerError;
        }
    }
    if (activeAccessory) {
        m_project->setActiveAccessory(activeAccessory);
    }
}

void
PMM::Context::loadAccessory(
    const nanoem_document_accessory_t *ao, Accessory *accessory, StringSet &reservedNameSet, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    String name;
    StringUtils::getUtf8String(nanoemDocumentAccessoryGetName(ao), factory, name);
    accessory->setName(Project::resolveNameConfliction(name, reservedNameSet));
    Motion *accessoryMotion = m_project->resolveMotion(accessory);
    nanoem_motion_t *originAccessoryMotion = accessoryMotion->data();
    nanoem_mutable_motion_t *mutableAccessoryMotion =
        nanoemMutableMotionCreateAsReference(originAccessoryMotion, status);
    nanoem_document_accessory_keyframe_t *const *accessoryKeyframes =
        nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(ao, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_accessory_keyframe_t *ko = accessoryKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_accessory_keyframe_t *keyframe =
            nanoemMutableMotionAccessoryKeyframeCreateByFound(originAccessoryMotion, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionAccessoryKeyframeCreate(originAccessoryMotion, status);
            nanoemMutableMotionAddAccessoryKeyframe(mutableAccessoryMotion, keyframe, frameIndex, status);
        }
        const glm::quat orientation(glm::make_vec3(nanoemDocumentAccessoryKeyframeGetOrientation(ko)));
        nanoemMutableMotionAccessoryKeyframeSetTranslation(keyframe, nanoemDocumentAccessoryKeyframeGetTranslation(ko));
        nanoemMutableMotionAccessoryKeyframeSetOrientation(keyframe, glm::value_ptr(orientation));
        nanoemMutableMotionAccessoryKeyframeSetScaleFactor(keyframe, nanoemDocumentAccessoryKeyframeGetScaleFactor(ko));
        nanoemMutableMotionAccessoryKeyframeSetOpacity(keyframe, nanoemDocumentAccessoryKeyframeGetOpacity(ko));
        nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(
            keyframe, nanoemDocumentAccessoryKeyframeIsShadowEnabled(ko));
        nanoemMutableMotionAccessoryKeyframeSetVisible(keyframe, nanoemDocumentAccessoryKeyframeIsVisible(ko));
        nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(keyframe, nanoemDocumentAccessoryIsAddBlendEnabled(ao));
        nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(keyframe, nanoemDocumentAccessoryIsShadowEnabled(ao));
        nanoemMutableMotionAccessoryKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionSortAllKeyframes(mutableAccessoryMotion);
    nanoemMutableMotionDestroy(mutableAccessoryMotion);
    m_project->setBaseDuration(nanoemMotionGetMaxFrameIndex(originAccessoryMotion));
}

void
PMM::Context::loadAllModels(const nanoem_document_t *document, OrderedDrawableList &drawables,
    PMMModelHandleMap &modelHandles, EffectMap &effectMap, Progress &progress, StringSet &reservedNameSet, Error &error,
    Project::IDiagnostics *diagnostics)
{
    nanoem_rsize_t numModels;
    nanoem_document_model_t *const *modelItems = nanoemDocumentGetAllModelObjects(document, &numModels);
    IFileManager *fileManager = m_project->fileManager();
    Model *activeModel = nullptr;
    const nanoem_rsize_t selectedModelIndex = nanoemDocumentIsEditingCLAEnabled(document)
        ? NANOEM_RSIZE_MAX
        : static_cast<nanoem_rsize_t>(nanoemDocumentGetSelectedModelIndex(document));
    const int accessoryIndexAfterModel = nanoemDocumentGetAccessoryIndexAfterModel(document);
    for (nanoem_rsize_t i = 0; i < numModels; i++) {
        const nanoem_document_model_t *mo = modelItems[i];
        const URI &fileURI = resolveFileURI(nanoemDocumentModelGetPath(mo));
        Error innerError;
        if (fileURI.isEmpty()) {
        }
        else if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
        }
        else if (!FileUtils::exists(fileURI)) {
            if (diagnostics) {
                diagnostics->addNotFoundFileURI(fileURI);
            }
        }
        else if (fileManager->loadFromFile(fileURI, IFileManager::kDialogTypeLoadModelFile, m_project, innerError)) {
            const Project::ModelList *allModels = m_project->allModels();
            if (!allModels->empty()) {
                Model *model = allModels->back();
                modelHandles.insert(tinystl::make_pair(nanoemDocumentModelGetIndex(mo), model));
                nanoem_status_t status = NANOEM_STATUS_SUCCESS;
                loadModel(mo, model, reservedNameSet, &status);
                if (status == NANOEM_STATUS_SUCCESS) {
                    effectMap.attachEffect(model, fileURI.absolutePath(), progress, innerError);
                    OrderedDrawable ordered(model, mo);
                    if (accessoryIndexAfterModel > -1) {
                        ordered.m_drawIndex += accessoryIndexAfterModel;
                    }
                    drawables.push_back(ordered);
                    if (selectedModelIndex == i) {
                        activeModel = model;
                    }
                    /* reset dirty morph state at initializing model to apply morph motion correctly */
                    model->updateStagingVertexBuffer();
                    model->setDirty(false);
                }
                else {
                    const char *message = Error::convertStatusToMessage(status, m_project->translator());
                    innerError = Error(message, status, Error::kDomainTypeNanoem);
                }
            }
            progress.increment();
        }
        if (innerError.hasReason()) {
            error = innerError;
        }
    }
    m_project->setActiveModel(activeModel);
}

void
PMM::Context::loadModel(
    const nanoem_document_model_t *mo, Model *model, StringSet &reservedNameSet, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    model->setName(Project::resolveNameConfliction(model, reservedNameSet));
    model->setActiveBone(model->findBone(nanoemDocumentModelGetSelectedBoneName(mo)));
    for (nanoem_rsize_t i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
        model->setActiveMorph(category, model->findMorph(nanoemDocumentModelGetSelectedMorphName(mo, category)));
    }
    Motion *motion = m_project->resolveMotion(model);
    IMotionKeyframeSelection *selection = motion->selection();
    nanoem_motion_t *originModelMotion = motion->data();
    nanoem_mutable_motion_t *mutableModelMotion = nanoemMutableMotionCreateAsReference(originModelMotion, status);
    nanoem_document_model_bone_keyframe_t *const *boneKeyframes =
        nanoemDocumentModelGetAllBoneKeyframeObjects(mo, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_bone_keyframe_t *ko = boneKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(ko);
        const nanoem_unicode_string_t *name = nanoemDocumentModelBoneKeyframeGetName(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_bone_keyframe_t *keyframe =
            nanoemMutableMotionBoneKeyframeCreateByFound(originModelMotion, name, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionBoneKeyframeCreate(originModelMotion, status);
            nanoemMutableMotionAddBoneKeyframe(mutableModelMotion, keyframe, name, frameIndex, status);
        }
        if (nanoemDocumentBaseKeyframeIsSelected(base)) {
            selection->add(nanoemMutableMotionBoneKeyframeGetOriginObject(keyframe));
        }
        nanoemMutableMotionBoneKeyframeSetTranslation(keyframe, nanoemDocumentModelBoneKeyframeGetTranslation(ko));
        nanoemMutableMotionBoneKeyframeSetOrientation(keyframe, nanoemDocumentModelBoneKeyframeGetOrientation(ko));
        nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(
            keyframe, nanoemDocumentModelBoneKeyframeIsPhysicsSimulationDisabled(ko) ? 0 : 1);
        for (nanoem_rsize_t j = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             j < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
            nanoem_motion_bone_keyframe_interpolation_type_t type =
                static_cast<nanoem_motion_bone_keyframe_interpolation_type_t>(j);
            nanoemMutableMotionBoneKeyframeSetInterpolation(
                keyframe, type, nanoemDocumentModelBoneKeyframeGetInterpolation(ko, type));
        }
        nanoemMutableMotionBoneKeyframeDestroy(keyframe);
    }
    nanoem_document_model_keyframe_t *const *modelKeyframes =
        nanoemDocumentModelGetAllModelKeyframeObjects(mo, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_keyframe_t *ko = modelKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_model_keyframe_t *keyframe =
            nanoemMutableMotionModelKeyframeCreateByFound(originModelMotion, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionModelKeyframeCreate(originModelMotion, status);
            nanoemMutableMotionAddModelKeyframe(mutableModelMotion, keyframe, frameIndex, status);
        }
        if (nanoemDocumentBaseKeyframeIsSelected(base)) {
            selection->add(nanoemMutableMotionModelKeyframeGetOriginObject(keyframe));
        }
        nanoemMutableMotionModelKeyframeSetVisible(keyframe, nanoemDocumentModelKeyframeIsVisible(ko));
        nanoemMutableMotionModelKeyframeDestroy(keyframe);
    }
    nanoem_document_model_morph_keyframe_t *const *morphKeyframes =
        nanoemDocumentModelGetAllMorphKeyframeObjects(mo, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_morph_keyframe_t *ko = morphKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(ko);
        const nanoem_unicode_string_t *name = nanoemDocumentModelMorphKeyframeGetName(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_morph_keyframe_t *keyframe =
            nanoemMutableMotionMorphKeyframeCreateByFound(originModelMotion, name, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionMorphKeyframeCreate(originModelMotion, status);
            nanoemMutableMotionAddMorphKeyframe(mutableModelMotion, keyframe, name, frameIndex, status);
        }
        if (nanoemDocumentBaseKeyframeIsSelected(base)) {
            selection->add(nanoemMutableMotionMorphKeyframeGetOriginObject(keyframe));
        }
        nanoemMutableMotionMorphKeyframeSetWeight(keyframe, nanoemDocumentModelMorphKeyframeGetWeight(ko));
        nanoemMutableMotionMorphKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionSortAllKeyframes(mutableModelMotion);
    nanoemMutableMotionDestroy(mutableModelMotion);
    m_project->setBaseDuration(nanoemMotionGetMaxFrameIndex(originModelMotion));
}

void
PMM::Context::loadCamera(
    const nanoem_document_t *document, const PMMModelHandleMap &modelHandles, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *originCameraMotion = m_project->cameraMotion()->data();
    nanoem_mutable_motion_t *mutableCameraMotion = nanoemMutableMotionCreateAsReference(originCameraMotion, status);
    const nanoem_document_camera_t *camera = nanoemDocumentGetCameraObject(document);
    ICamera *globalCamera = m_project->globalCamera();
    globalCamera->setLookAt(glm::make_vec3(nanoemDocumentCameraGetLookAt(camera)));
    globalCamera->setAngle(glm::make_vec3(nanoemDocumentCameraGetAngle(camera)));
    globalCamera->setDistance(nanoemDocumentCameraGetDistance(camera));
    globalCamera->setFov(int(nanoemDocumentCameraGetFov(camera)));
    globalCamera->setPerspective(nanoemDocumentCameraIsPerspectiveEnabled(camera) != 0);
    nanoem_document_camera_keyframe_t *const *cameraKeyframes =
        nanoemDocumentCameraGetAllCameraKeyframeObjects(camera, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_camera_keyframe_t *ko = cameraKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentCameraKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_camera_keyframe_t *keyframe =
            nanoemMutableMotionCameraKeyframeCreateByFound(originCameraMotion, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionCameraKeyframeCreate(originCameraMotion, status);
            nanoemMutableMotionAddCameraKeyframe(mutableCameraMotion, keyframe, frameIndex, status);
        }
        nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, nanoemDocumentCameraKeyframeGetLookAt(ko));
        nanoemMutableMotionCameraKeyframeSetAngle(
            keyframe, glm::value_ptr(glm::make_vec4(nanoemDocumentCameraKeyframeGetAngle(ko))));
        nanoemMutableMotionCameraKeyframeSetFov(keyframe, nanoemDocumentCameraKeyframeGetFov(ko));
        nanoemMutableMotionCameraKeyframeSetDistance(keyframe, nanoemDocumentCameraKeyframeGetDistance(ko));
        nanoemMutableMotionCameraKeyframeSetPerspectiveView(
            keyframe, nanoemDocumentCameraKeyframeIsPerspectiveView(ko));
        for (nanoem_rsize_t j = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             j < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
            nanoem_motion_camera_keyframe_interpolation_type_t type =
                static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(j);
            nanoemMutableMotionCameraKeyframeSetInterpolation(
                keyframe, type, nanoemDocumentCameraKeyframeGetInterpolation(ko, type));
        }
        if (const nanoem_unicode_string_t *boneName = nanoemDocumentCameraKeyframeGetParentModelBoneName(ko)) {
            const nanoem_document_model_t *mo = nanoemDocumentCameraKeyframeGetParentModelObject(ko);
            PMMModelHandleMap::const_iterator it2 = modelHandles.find(nanoemDocumentModelGetIndex(mo));
            if (it2 != modelHandles.end()) {
                nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
                nanoem_mutable_motion_outside_parent_t *op = nanoemMutableMotionOutsideParentCreateFromCameraKeyframe(
                    nanoemMutableMotionCameraKeyframeGetOriginObject(keyframe), status);
                StringUtils::UnicodeStringScope scope(factory);
                if (StringUtils::tryGetString(factory, it2->second->name(), scope)) {
                    nanoemMutableMotionOutsideParentSetTargetObjectName(op, scope.value(), status);
                }
                nanoemMutableMotionOutsideParentSetTargetBoneName(op, boneName, status);
                nanoemMutableMotionCameraKeyframeSetOutsideParent(keyframe, op, status);
                nanoemMutableMotionOutsideParentDestroy(op);
            }
        }
        nanoemMutableMotionCameraKeyframeDestroy(keyframe);
    }
    if (numKeyframes == 0) {
        const Vector4 lookAt(globalCamera->lookAt(), 1), angle(globalCamera->angle(), 0);
        nanoem_mutable_motion_camera_keyframe_t *keyframe =
            nanoemMutableMotionCameraKeyframeCreateByFound(originCameraMotion, 0, status);
        nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, glm::value_ptr(lookAt));
        nanoemMutableMotionCameraKeyframeSetAngle(keyframe, glm::value_ptr(angle));
        nanoemMutableMotionCameraKeyframeSetFov(keyframe, globalCamera->fov());
        nanoemMutableMotionCameraKeyframeSetDistance(keyframe, globalCamera->distance());
        nanoemMutableMotionCameraKeyframeSetPerspectiveView(keyframe, globalCamera->isPerspective() ? 1 : 0);
        nanoemMutableMotionCameraKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionSortAllKeyframes(mutableCameraMotion);
    nanoemMutableMotionDestroy(mutableCameraMotion);
    m_project->setBaseDuration(nanoemMotionGetMaxFrameIndex(originCameraMotion));
}

void
PMM::Context::loadLight(const nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *originLightMotion = m_project->lightMotion()->data();
    nanoem_mutable_motion_t *mutableLightMotion = nanoemMutableMotionCreateAsReference(originLightMotion, status);
    const nanoem_document_light_t *light = nanoemDocumentGetLightObject(document);
    ILight *globalLight = m_project->globalLight();
    globalLight->setColor(glm::make_vec3(nanoemDocumentLightGetColor(light)));
    globalLight->setDirection(glm::make_vec3(nanoemDocumentLightGetDirection(light)));
    globalLight->setTranslucentGroundShadowEnabled(nanoemDocumentIsTranslucentGroundShadowEnabled(document) != 0);
    nanoem_document_light_keyframe_t *const *lightKeyframes =
        nanoemDocumentLightGetAllLightKeyframeObjects(light, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_light_keyframe_t *ko = lightKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentLightKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_light_keyframe_t *keyframe =
            nanoemMutableMotionLightKeyframeCreateByFound(originLightMotion, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionLightKeyframeCreate(originLightMotion, status);
            nanoemMutableMotionAddLightKeyframe(mutableLightMotion, keyframe, frameIndex, status);
        }
        nanoemMutableMotionLightKeyframeSetColor(keyframe, nanoemDocumentLightKeyframeGetColor(ko));
        nanoemMutableMotionLightKeyframeSetDirection(keyframe, nanoemDocumentLightKeyframeGetDirection(ko));
        nanoemMutableMotionLightKeyframeDestroy(keyframe);
    }
    if (numKeyframes == 0) {
        const Vector4 color(globalLight->color(), 1), direction(globalLight->direction(), 0);
        nanoem_mutable_motion_light_keyframe_t *keyframe =
            nanoemMutableMotionLightKeyframeCreateByFound(originLightMotion, 0, status);
        nanoemMutableMotionLightKeyframeSetColor(keyframe, glm::value_ptr(color));
        nanoemMutableMotionLightKeyframeSetDirection(keyframe, glm::value_ptr(direction));
        nanoemMutableMotionLightKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionSortAllKeyframes(mutableLightMotion);
    nanoemMutableMotionDestroy(mutableLightMotion);
    m_project->setBaseDuration(nanoemMotionGetMaxFrameIndex(originLightMotion));
}

void
PMM::Context::loadSelfShadow(const nanoem_document_t *document, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_t *originSelfShadowMotion = m_project->selfShadowMotion()->data();
    nanoem_mutable_motion_t *mutableSelfShadowMotion =
        nanoemMutableMotionCreateAsReference(originSelfShadowMotion, status);
    const nanoem_document_self_shadow_t *selfShadow = nanoemDocumentGetSelfShadowObject(document);
    ShadowCamera *shadowCamera = m_project->shadowCamera();
    shadowCamera->setDistance(
        ShadowCamera::kMaximumDistance - nanoemDocumentSelfShadowGetDistance(selfShadow) * 100000);
    shadowCamera->setEnabled(nanoemDocumentSelfShadowIsEnabled(selfShadow) != 0);
    nanoem_document_self_shadow_keyframe_t *const *selfShadowKeyframes =
        nanoemDocumentSelfShadowGetAllSelfShadowKeyframeObjects(selfShadow, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_self_shadow_keyframe_t *ko = selfShadowKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_self_shadow_keyframe_t *keyframe =
            nanoemMutableMotionSelfShadowKeyframeCreateByFound(originSelfShadowMotion, 0, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionSelfShadowKeyframeCreate(originSelfShadowMotion, status);
            nanoemMutableMotionAddSelfShadowKeyframe(mutableSelfShadowMotion, keyframe, frameIndex, status);
        }
        nanoemMutableMotionSelfShadowKeyframeSetDistance(
            keyframe, 10000 - nanoemDocumentSelfShadowKeyframeGetDistance(ko) * 100000);
        nanoemMutableMotionSelfShadowKeyframeSetMode(keyframe, nanoemDocumentSelfShadowKeyframeGetMode(ko));
        nanoemMutableMotionSelfShadowKeyframeDestroy(keyframe);
    }
    if (numKeyframes == 0) {
        nanoem_mutable_motion_self_shadow_keyframe_t *keyframe =
            nanoemMutableMotionSelfShadowKeyframeCreateByFound(originSelfShadowMotion, 0, status);
        nanoemMutableMotionSelfShadowKeyframeSetDistance(keyframe, shadowCamera->distance());
        nanoemMutableMotionSelfShadowKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionSortAllKeyframes(mutableSelfShadowMotion);
    nanoemMutableMotionDestroy(mutableSelfShadowMotion);
}

void
PMM::Context::configureAllAccessoryOutsideParents(const nanoem_document_t *document,
    const PMMAccessoryHandleMap &accessoryHandles, const PMMModelHandleMap &modelHandles, nanoem_status_t *status)
{
    nanoem_rsize_t numAccessories;
    nanoem_document_accessory_t *const *accessories = nanoemDocumentGetAllAccessoryObjects(document, &numAccessories);
    for (nanoem_rsize_t i = 0; i < numAccessories; i++) {
        const nanoem_document_accessory_t *ao = accessories[i];
        PMMAccessoryHandleMap::const_iterator it = accessoryHandles.find(nanoemDocumentAccessoryGetIndex(ao));
        if (it != accessoryHandles.end()) {
            Accessory *accessory = it->second;
            configureAccessoryOutsideParent(ao, accessory, modelHandles, status);
        }
    }
}

void
PMM::Context::configureAccessoryOutsideParent(const nanoem_document_accessory_t *ao, Accessory *accessory,
    const PMMModelHandleMap &modelHandles, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    if (const nanoem_unicode_string_t *bn = nanoemDocumentAccessoryGetParentModelBoneName(ao)) {
        const nanoem_document_model_t *mo = nanoemDocumentAccessoryGetParentModelObject(ao);
        PMMModelHandleMap::const_iterator it2 = modelHandles.find(nanoemDocumentModelGetIndex(mo));
        if (it2 != modelHandles.end()) {
            String boneNameString;
            StringUtils::getUtf8String(bn, factory, boneNameString);
            accessory->setOutsideParent(tinystl::make_pair(it2->second->name(), boneNameString));
        }
    }
    Motion *accessoryMotion = m_project->resolveMotion(accessory);
    nanoem_motion_t *originAccessoryMotion = accessoryMotion->data();
    nanoem_mutable_motion_t *mutableAccessoryMotion =
        nanoemMutableMotionCreateAsReference(originAccessoryMotion, status);
    nanoem_document_accessory_keyframe_t *const *accessoryKeyframes =
        nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(ao, &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_accessory_keyframe_t *ko = accessoryKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_accessory_keyframe_t *keyframe =
            nanoemMutableMotionAccessoryKeyframeCreateByFound(originAccessoryMotion, frameIndex, status);
        if (const nanoem_unicode_string_t *boneName = nanoemDocumentAccessoryKeyframeGetParentModelBoneName(ko)) {
            const nanoem_document_model_t *mo = nanoemDocumentAccessoryKeyframeGetParentModelObject(ko);
            PMMModelHandleMap::const_iterator it2 = modelHandles.find(nanoemDocumentModelGetIndex(mo));
            if (it2 != modelHandles.end()) {
                nanoem_mutable_motion_outside_parent_t *op =
                    nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(
                        nanoemMutableMotionAccessoryKeyframeGetOriginObject(keyframe), status);
                StringUtils::UnicodeStringScope scope(factory);
                if (StringUtils::tryGetString(factory, it2->second->name(), scope)) {
                    nanoemMutableMotionOutsideParentSetTargetObjectName(op, scope.value(), status);
                }
                nanoemMutableMotionOutsideParentSetTargetBoneName(op, boneName, status);
                nanoemMutableMotionAccessoryKeyframeSetOutsideParent(keyframe, op, status);
                nanoemMutableMotionOutsideParentDestroy(op);
            }
        }
        nanoemMutableMotionAccessoryKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionDestroy(mutableAccessoryMotion);
}

void
PMM::Context::configureAllModelOutsideParents(
    const nanoem_document_t *document, const PMMModelHandleMap &modelHandles, nanoem_status_t *status)
{
    nanoem_rsize_t numModels;
    nanoem_document_model_t *const *models = nanoemDocumentGetAllModelObjects(document, &numModels);
    for (nanoem_rsize_t i = 0; i < numModels; i++) {
        const nanoem_document_model_t *mo = models[i];
        PMMModelHandleMap::const_iterator it = modelHandles.find(nanoemDocumentModelGetIndex(mo));
        if (it != modelHandles.end()) {
            Model *subjectModel = it->second;
            configureModelOutsideParent(mo, subjectModel, status);
        }
    }
}

void
PMM::Context::configureModelOutsideParent(
    const nanoem_document_model_t *mo, Model *subjectModel, nanoem_status_t *status)
{
    nanoem_rsize_t numStates;
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_document_model_outside_parent_state_t *const *states =
        nanoemDocumentModelGetAllOutsideParentStateObjects(mo, &numStates);
    for (nanoem_rsize_t i = 0; i < numStates; i++) {
        const nanoem_document_model_outside_parent_state_t *state = states[i];
        const nanoem_unicode_string_t *subjectBoneName = nanoemDocumentModelGetOutsideParentSubjectBoneName(mo, i);
        const nanoem_model_bone_t *subjectBone = subjectModel->findBone(subjectBoneName);
        const nanoem_document_model_t *targetModelPtr =
            nanoemDocumentModelOutsideParentStateGetTargetModelObject(state);
        const nanoem_unicode_string_t *targetBoneName = nanoemDocumentModelOutsideParentStateGetTargetBoneName(state);
        if (subjectBone && targetModelPtr && targetBoneName) {
            nanoem_frame_index_t currentFrameIndex = m_project->currentLocalFrameIndex();
            nanoem_frame_index_t beginFrameIndex = nanoemDocumentModelOutsideParentStateGetBeginFrameIndex(state),
                                 endFrameIndex = nanoemDocumentModelOutsideParentStateGetEndFrameIndex(state);
            if (currentFrameIndex >= beginFrameIndex && (endFrameIndex == 0 || currentFrameIndex <= endFrameIndex)) {
                StringPair outsideParentStringPair;
                StringUtils::getUtf8String(nanoemDocumentModelGetName(targetModelPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM),
                    factory, outsideParentStringPair.first);
                StringUtils::getUtf8String(targetBoneName, factory, outsideParentStringPair.second);
                subjectModel->setOutsideParent(subjectBone, outsideParentStringPair);
                if (Motion *subjectModelMotion = m_project->resolveMotion(subjectModel)) {
                    configureModelOutsideParentKeyframes(
                        mo, subjectModelMotion, subjectBoneName, outsideParentStringPair, status);
                }
            }
        }
    }
}

void
PMM::Context::configureModelOutsideParentKeyframes(const nanoem_document_model_t *mo, Motion *subjectModelMotion,
    const nanoem_unicode_string_t *subjectBoneName, const StringPair &outsideParentStringPair, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_rsize_t numKeyframes;
    nanoem_document_model_keyframe_t *const *modelKeyframes =
        nanoemDocumentModelGetAllModelKeyframeObjects(mo, &numKeyframes);
    nanoem_motion_t *originSubjectModelMotion = subjectModelMotion->data();
    nanoem_mutable_motion_t *mutableSubjectModelMotion =
        nanoemMutableMotionCreateAsReference(originSubjectModelMotion, status);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_document_model_keyframe_t *ko = modelKeyframes[i];
        const nanoem_document_base_keyframe_t *base = nanoemDocumentModelKeyframeGetBaseKeyframeObject(ko);
        nanoem_frame_index_t frameIndex = nanoemDocumentBaseKeyframeGetFrameIndex(base);
        nanoem_mutable_motion_model_keyframe_t *keyframe =
            nanoemMutableMotionModelKeyframeCreateByFound(originSubjectModelMotion, frameIndex, status);
        if (!keyframe) {
            keyframe = nanoemMutableMotionModelKeyframeCreate(originSubjectModelMotion, status);
            nanoemMutableMotionAddModelKeyframe(mutableSubjectModelMotion, keyframe, frameIndex, status);
        }
        nanoem_rsize_t numObjects;
        nanoem_document_outside_parent_t *const *outsideParents =
            nanoemDocumentModelKeyframeGetAllOutsideParentObjects(modelKeyframes[i], &numObjects);
        for (nanoem_rsize_t j = 0; j < numObjects; j++) {
            const nanoem_document_outside_parent_t *outsideParent = outsideParents[j];
            const nanoem_document_model_t *targetModel = nanoemDocumentOutsideParentGetModelObject(outsideParent);
            const nanoem_unicode_string_t *targetBoneName = nanoemDocumentOutsideParentGetBoneName(outsideParent);
            if (targetModel && targetBoneName) {
                StringPair innerOutsideParentStringPair;
                const nanoem_unicode_string_t *targetModelName =
                    nanoemDocumentModelGetName(targetModel, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                StringUtils::getUtf8String(targetModelName, factory, innerOutsideParentStringPair.first);
                StringUtils::getUtf8String(targetBoneName, factory, innerOutsideParentStringPair.second);
                if (outsideParentStringPair.first == innerOutsideParentStringPair.first &&
                    outsideParentStringPair.second == innerOutsideParentStringPair.second) {
                    nanoem_motion_model_keyframe_t *origin = nanoemMutableMotionModelKeyframeGetOriginObject(keyframe);
                    nanoem_mutable_motion_outside_parent_t *op =
                        nanoemMutableMotionOutsideParentCreateFromModelKeyframe(origin, status);
                    nanoemMutableMotionOutsideParentSetSubjectBoneName(op, subjectBoneName, status);
                    nanoemMutableMotionOutsideParentSetTargetBoneName(op, targetBoneName, status);
                    nanoemMutableMotionOutsideParentSetTargetObjectName(op, targetModelName, status);
                    nanoemMutableMotionModelKeyframeAddOutsideParent(keyframe, op, status);
                    nanoemMutableMotionOutsideParentDestroy(op);
                }
            }
        }
        nanoemMutableMotionModelKeyframeDestroy(keyframe);
    }
    nanoemMutableMotionDestroy(mutableSubjectModelMotion);
}

void
PMM::Context::loadAudio(
    const nanoem_document_t *document, Progress &progress, Error &error, Project::IDiagnostics *diagnostics)
{
    if (nanoemDocumentIsAudioEnabled(document)) {
        FileReaderScope scope(m_project->translator());
        const URI &fileURI = resolveFileURI(nanoemDocumentGetAudioPath(document));
        IFileManager *fileManager = m_project->fileManager();
        if (fileURI.isEmpty()) {
        }
        else if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
        }
        else if (!FileUtils::exists(fileURI)) {
            if (diagnostics) {
                diagnostics->addNotFoundFileURI(fileURI);
            }
        }
        else if (fileManager->loadAudioFile(fileURI, m_project, error)) {
        }
    }
    progress.increment();
}

void
PMM::Context::loadBackgroundImage(
    const nanoem_document_t *document, Progress &progress, Error &error, Project::IDiagnostics *diagnostics)
{
    if (nanoemDocumentIsBackgroundImageEnabled(document)) {
        const URI &fileURI = resolveFileURI(nanoemDocumentGetBackgroundImagePath(document));
        IFileManager *fileManager = m_project->fileManager();
        if (fileURI.isEmpty()) {
        }
        else if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
        }
        else if (!FileUtils::exists(fileURI)) {
            if (diagnostics) {
                diagnostics->addNotFoundFileURI(fileURI);
            }
        }
        else if (fileManager->loadVideoFile(fileURI, m_project, error)) {
            const Vector4SI32 rect(nanoemDocumentGetBackgroundVideoOffsetX(document),
                nanoemDocumentGetBackgroundVideoOffsetY(document), 0, 0);
            m_project->setBackgroundVideoRect(rect);
        }
    }
    progress.increment();
}

void
PMM::Context::loadBackgroundVideo(
    const nanoem_document_t *document, Progress &progress, Error &error, Project::IDiagnostics *diagnostics)
{
    if (nanoemDocumentIsBackgroundVideoEnabled(document)) {
        const URI &fileURI = resolveFileURI(nanoemDocumentGetBackgroundVideoPath(document));
        IFileManager *fileManager = m_project->fileManager();
        if (fileURI.isEmpty()) {
        }
        else if (!progress.tryLoadingItem(fileURI)) {
            error = Error::cancelled();
        }
        else if (!FileUtils::exists(fileURI)) {
            if (diagnostics) {
                diagnostics->addNotFoundFileURI(fileURI);
            }
        }
        else if (fileManager->loadVideoFile(fileURI, m_project, error)) {
            int x = nanoemDocumentGetBackgroundVideoOffsetX(document),
                y = nanoemDocumentGetBackgroundVideoOffsetY(document);
            const Vector4SI32 rect(x, y, 0, 0);
            m_project->setBackgroundVideoRect(rect);
            m_project->setBackgroundVideoScaleFactor(nanoemDocumentGetBackgroundVideoScaleFactor(document));
        }
    }
    progress.increment();
}

void
PMM::Context::saveAudioSource(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    const IAudioPlayer *player = m_project->audioPlayer();
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    nanoemMutableDocumentSetAudioEnabled(document, player->isLoaded());
    if (StringUtils::tryGetString(factory, makeRelativeFromProjectPath(player->fileURI()), scope)) {
        nanoemMutableDocumentSetAudioPath(document, scope.value(), status);
    }
}

void
PMM::Context::saveBackgroundVideo(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    const IBackgroundVideoRenderer *renderer = m_project->backgroundVideoRenderer();
    const URI fileURI(renderer->fileURI());
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    nanoemMutableDocumentSetBackgroundVideoEnabled(document, !fileURI.isEmpty());
    if (StringUtils::tryGetString(factory, makeRelativeFromProjectPath(fileURI), scope)) {
        const Vector4SI32 rect(m_project->backgroundVideoRect());
        nanoemMutableDocumentSetBackgroundVideoPath(document, scope.value(), status);
        nanoemMutableDocumentSetBackgroundVideoOffsetX(document, rect.x);
        nanoemMutableDocumentSetBackgroundVideoOffsetY(document, rect.y);
        nanoemMutableDocumentSetBackgroundVideoScaleFactor(document, m_project->backgroundVideoScaleFactor());
    }
}

void
PMM::Context::saveCamera(
    nanoem_mutable_document_t *document, const ModelResolveMap &resolveMap, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    nanoem_mutable_document_camera_t *co = nanoemMutableDocumentCameraCreate(document, status);
    const ICamera *globalCamera = m_project->globalCamera();
    nanoemMutableDocumentCameraSetLookAt(co, glm::value_ptr(glm::vec4(globalCamera->lookAt(), 0)));
    nanoemMutableDocumentCameraSetAngle(co, glm::value_ptr(glm::vec4(globalCamera->angle(), 0)));
    nanoemMutableDocumentCameraSetPosition(co, glm::value_ptr(glm::vec4(0, 0, -globalCamera->distance(), 0)));
    nanoemMutableDocumentCameraSetFov(co, globalCamera->fov());
    nanoemMutableDocumentCameraSetPerspectiveEnabled(co, globalCamera->isPerspective());
    nanoem_rsize_t numKeyframes;
    nanoem_motion_camera_keyframe_t *const *keyframes =
        nanoemMotionGetAllCameraKeyframeObjects(m_project->cameraMotion()->data(), &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_camera_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
        nanoem_mutable_document_camera_keyframe_t *ko = nanoemMutableDocumentCameraKeyframeCreate(co, status);
        nanoemMutableDocumentCameraKeyframeSetLookAt(ko, nanoemMotionCameraKeyframeGetLookAt(keyframe));
        nanoemMutableDocumentCameraKeyframeSetAngle(
            ko, glm::value_ptr(glm::make_vec4(nanoemMotionCameraKeyframeGetAngle(keyframe))));
        nanoemMutableDocumentCameraKeyframeSetFov(ko, nanoemMotionCameraKeyframeGetFov(keyframe));
        nanoemMutableDocumentCameraKeyframeSetDistance(ko, nanoemMotionCameraKeyframeGetDistance(keyframe));
        if (const nanoem_motion_outside_parent_t *outsideParent =
                nanoemMotionCameraKeyframeGetOutsideParent(keyframe)) {
            const nanoem_unicode_string_t *modelName = nanoemMotionOutsideParentGetTargetObjectName(outsideParent);
            String s;
            StringUtils::getUtf8String(modelName, factory, status, s);
            ModelResolveMap::const_iterator it = resolveMap.find(m_project->findModelByName(s));
            if (it != resolveMap.end()) {
                const nanoem_unicode_string_t *boneName = nanoemMotionOutsideParentGetTargetBoneName(outsideParent);
                nanoemMutableDocumentCameraKeyframeSetParentModelBoneName(ko, boneName, status);
                nanoemMutableDocumentCameraKeyframeSetParentModelObject(ko, it->second);
            }
        }
        nanoemMutableDocumentCameraKeyframeSetPerspectiveView(
            ko, nanoemMotionCameraKeyframeIsPerspectiveView(keyframe));
        for (nanoem_rsize_t j = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             j < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
            nanoem_motion_camera_keyframe_interpolation_type_t type =
                static_cast<nanoem_motion_camera_keyframe_interpolation_type_t>(j);
            nanoemMutableDocumentCameraKeyframeSetInterpolation(
                ko, type, nanoemMotionCameraKeyframeGetInterpolation(keyframe, type));
        }
        nanoemMutableDocumentCameraAddCameraKeyframeObject(co, ko, frameIndex, status);
        nanoemMutableDocumentCameraKeyframeDestroy(ko);
    }
    nanoemMutableDocumentSetCameraObject(document, co);
    nanoemMutableDocumentCameraDestroy(co);
}

void
PMM::Context::saveGravity(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    const PhysicsEngine *engine = m_project->physicsEngine();
    nanoem_mutable_document_gravity_t *go = nanoemMutableDocumentGravityCreate(document, status);
    nanoemMutableDocumentGravitySetAcceleration(go, engine->acceleration());
    nanoemMutableDocumentGravitySetDirection(go, glm::value_ptr(glm::vec4(engine->direction(), 0)));
    nanoemMutableDocumentGravitySetNoise(go, engine->noise());
    nanoemMutableDocumentGravitySetNoiseEnabled(go, engine->isNoiseEnabled());
    nanoemMutableDocumentSetGravityObject(document, go);
    nanoemMutableDocumentGravityDestroy(go);
}

void
PMM::Context::saveLight(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_light_t *lo = nanoemMutableDocumentLightCreate(document, status);
    const ILight *globalLight = m_project->globalLight();
    nanoemMutableDocumentLightSetColor(lo, glm::value_ptr(glm::vec4(globalLight->color(), 1)));
    nanoemMutableDocumentLightSetDirection(lo, glm::value_ptr(glm::vec4(globalLight->direction(), 0)));
    nanoemMutableDocumentSetTranslucentGroundShadowEnabled(document, globalLight->isTranslucentGroundShadowEnabled());
    nanoem_rsize_t numKeyframes;
    nanoem_motion_light_keyframe_t *const *keyframes =
        nanoemMotionGetAllLightKeyframeObjects(m_project->lightMotion()->data(), &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_light_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(keyframe));
        nanoem_mutable_document_light_keyframe_t *ko = nanoemMutableDocumentLightKeyframeCreate(lo, status);
        nanoemMutableDocumentLightKeyframeSetColor(ko, nanoemMotionLightKeyframeGetColor(keyframe));
        nanoemMutableDocumentLightKeyframeSetDirection(ko, nanoemMotionLightKeyframeGetDirection(keyframe));
        nanoemMutableDocumentLightAddLightKeyframeObject(lo, ko, frameIndex, status);
        nanoemMutableDocumentLightKeyframeDestroy(ko);
    }
    nanoemMutableDocumentSetLightObject(document, lo);
    nanoemMutableDocumentLightDestroy(lo);
}

void
PMM::Context::saveSelfShadow(nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_mutable_document_self_shadow_t *so = nanoemMutableDocumentSelfShadowCreate(document, status);
    const ShadowCamera *shadowCamera = m_project->shadowCamera();
    nanoemMutableDocumentSelfShadowSetDistance(
        so, (ShadowCamera::kMaximumDistance - shadowCamera->distance()) * 0.00001);
    nanoemMutableDocumentSelfShadowSetEnabled(so, shadowCamera->isEnabled());
    nanoem_rsize_t numKeyframes;
    nanoem_motion_self_shadow_keyframe_t *const *keyframes =
        nanoemMotionGetAllSelfShadowKeyframeObjects(m_project->selfShadowMotion()->data(), &numKeyframes);
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_self_shadow_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(keyframe));
        nanoem_mutable_document_self_shadow_keyframe_t *ko = nanoemMutableDocumentSelfShadowKeyframeCreate(so, status);
        nanoemMutableDocumentSelfShadowKeyframeSetDistance(
            ko, (ShadowCamera::kMaximumDistance - nanoemMotionSelfShadowKeyframeGetDistance(keyframe)) * 0.00001);
        nanoemMutableDocumentSelfShadowKeyframeSetMode(ko, nanoemMotionSelfShadowKeyframeGetMode(keyframe));
        nanoemMutableDocumentSelfShadowAddSelfShadowKeyframeObject(so, ko, frameIndex, status);
        nanoemMutableDocumentSelfShadowKeyframeDestroy(ko);
    }
    nanoemMutableDocumentSetSelfShadowObject(document, so);
    nanoemMutableDocumentSelfShadowDestroy(so);
}

void
PMM::Context::saveAllAccessories(
    nanoem_mutable_document_t *document, const ModelResolveMap &resolveMap, nanoem_status_t *status)
{
    const Project::DrawableList *drawables = m_project->drawableOrderList();
    tinystl::unordered_set<Accessory *, TinySTLAllocator> allAccessories(
        ListUtils::toSetFromList(*m_project->allAccessories()));
    Project::AccessoryList accessories;
    for (Project::DrawableList::const_iterator it = drawables->begin(), end = drawables->end(); it != end; ++it) {
        Accessory *accessory = static_cast<Accessory *>(*it);
        if (allAccessories.find(accessory) != allAccessories.end()) {
            accessories.push_back(accessory);
        }
    }
    const Accessory *activeAccessory = m_project->activeAccessory();
    int drawOrderIndex = 0, selectedAccessoryIndex = 0;
    for (Project::AccessoryList::const_iterator it = accessories.begin(), end = accessories.end(); it != end; ++it) {
        if (activeAccessory == *it) {
            selectedAccessoryIndex = it - accessories.begin();
        }
        saveAccessory(*it, resolveMap, drawOrderIndex++, document, status);
    }
    if (!accessories.empty()) {
        nanoemMutableDocumentSetSelectedAccessoryIndex(document, selectedAccessoryIndex);
    }
}

void
PMM::Context::saveAllModels(nanoem_mutable_document_t *document, ModelResolveMap &resolveMap, nanoem_status_t *status)
{
    const Project::DrawableList *drawables = m_project->drawableOrderList();
    const Project::ModelList *transforms = m_project->transformOrderList();
    tinystl::unordered_set<Model *, TinySTLAllocator> allModels(ListUtils::toSetFromList(*m_project->allModels()));
    Project::ModelList models;
    int drawOrderIndex = 0, selectedModelIndex = 0;
    for (Project::DrawableList::const_iterator it = drawables->begin(), end = drawables->end(); it != end; ++it) {
        Model *model = static_cast<Model *>(*it);
        if (allModels.find(model) != allModels.end()) {
            models.push_back(model);
        }
    }
    const Model *activeModel = m_project->activeModel();
    for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
        Model *model = *it;
        int transformOrder = ListUtils::indexOf(model, *transforms);
        if (activeModel == model) {
            selectedModelIndex = it - models.begin();
        }
        saveModel(model, drawOrderIndex++, transformOrder, document, resolveMap, status);
        model->setDirty(false);
    }
    nanoemMutableDocumentSetAccessoryIndexAfterModel(document, drawOrderIndex);
    if (!models.empty()) {
        nanoemMutableDocumentSetSelectedModelIndex(document, selectedModelIndex);
    }
}

void
PMM::Context::saveAllModelOutsideParents(
    nanoem_mutable_document_t *document, const ModelResolveMap &resolveMap, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    for (ModelResolveMap::const_iterator it = resolveMap.begin(), end = resolveMap.end(); it != end; ++it) {
        const Model *model = it->first;
        if (const Motion *motion = m_project->resolveMotion(model)) {
            nanoem_document_model_t *mo = it->second;
            nanoem_mutable_document_model_t *mmo = nanoemMutableDocumentModelCreateAsReference(document, mo, status);
            {
                const model::Bone::OutsideParentMap ops(model->allOutsideParents());
                nanoem_rsize_t numBones;
                nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    const nanoem_unicode_string_t *boneName =
                        nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
                    nanoem_mutable_document_model_outside_parent_state_t *state =
                        nanoemMutableDocumentModelOutsideParentStateCreate(mmo, status);
                    model::Bone::OutsideParentMap::const_iterator it = ops.find(bonePtr);
                    if (it != ops.end()) {
                        const StringPair &op = it->second;
                        ModelResolveMap::const_iterator it2 = resolveMap.find(m_project->findModelByName(op.first));
                        if (it2 != resolveMap.end()) {
                            if (StringUtils::tryGetString(factory, op.second, scope)) {
                                nanoemMutableDocumentModelOutsideParentStateSetTargetModelObject(state, it2->second);
                                nanoemMutableDocumentModelOutsideParentStateSetTargetBoneName(state, scope.value());
                                nanoemMutableDocumentModelAddOutsideParentSubjectBone(mmo, boneName, status);
                                nanoemMutableDocumentModelInsertModelOutsideParentStateObject(mmo, state, -1, status);
                            }
                        }
                    }
                    nanoemMutableDocumentModelOutsideParentStateDestroy(state);
                }
            }
            nanoem_rsize_t numSourceKeyframes, numDestKeyframes;
            nanoem_motion_model_keyframe_t *const *sourceKeyframes =
                nanoemMotionGetAllModelKeyframeObjects(motion->data(), &numSourceKeyframes);
            nanoem_document_model_keyframe_t *const *destKeyframes =
                nanoemDocumentModelGetAllModelKeyframeObjects(mo, &numDestKeyframes);
            if (numSourceKeyframes == numDestKeyframes) {
                for (nanoem_rsize_t i = 0; i < numDestKeyframes; i++) {
                    const nanoem_motion_model_keyframe_t *sourceKeyframe = sourceKeyframes[i];
                    nanoem_document_model_keyframe_t *destKeyframe = destKeyframes[i];
                    nanoem_mutable_document_model_keyframe_t *ko =
                        nanoemMutableDocumentModelKeyframeCreateAsReference(mmo, destKeyframe, status);
                    nanoem_rsize_t numOutsideParents = 0;
                    nanoem_motion_outside_parent_t *const *outsideParents =
                        nanoemMotionModelKeyframeGetAllOutsideParentObjects(sourceKeyframe, &numOutsideParents);
                    for (nanoem_rsize_t j = 0; j < numOutsideParents; j++) {
                        const nanoem_motion_outside_parent_t *outsideParent = outsideParents[j];
                        const nanoem_unicode_string_t *modelName =
                            nanoemMotionOutsideParentGetTargetObjectName(outsideParent);
                        String s;
                        StringUtils::getUtf8String(modelName, factory, status, s);
                        ModelResolveMap::const_iterator it = resolveMap.find(m_project->findModelByName(s));
                        if (it != resolveMap.end()) {
                            const nanoem_unicode_string_t *boneName =
                                nanoemMotionOutsideParentGetTargetBoneName(outsideParent);
                            nanoem_mutable_document_outside_parent_t *op =
                                nanoemMutableDocumentOutsideParentCreate(document, status);
                            nanoemMutableDocumentOutsideParentSetBoneName(op, boneName, status);
                            nanoemMutableDocumentOutsideParentSetModelObject(op, it->second);
                            nanoemMutableDocumentModelKeyframeInsertOutsideParentObject(ko, op, -1, status);
                            nanoemMutableDocumentOutsideParentDestroy(op);
                        }
                    }
                    nanoemMutableDocumentModelKeyframeDestroy(ko);
                }
            }
            nanoemMutableDocumentModelDestroy(mmo);
        }
    }
}

void
PMM::Context::saveAccessory(const Accessory *accessory, const ModelResolveMap &resolveMap, int drawOrderIndex,
    nanoem_mutable_document_t *document, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    nanoem_mutable_document_accessory_t *ao = nanoemMutableDocumentAccessoryCreate(document, status);
    nanoemMutableDocumentAccessorySetAddBlendEnabled(ao, accessory->isAddBlendEnabled());
    nanoemMutableDocumentAccessorySetDrawOrderIndex(ao, drawOrderIndex);
    if (StringUtils::tryGetString(factory, accessory->name(), scope)) {
        nanoemMutableDocumentAccessorySetName(ao, scope.value(), status);
    }
    nanoemMutableDocumentAccessorySetOpacity(ao, accessory->opacity());
    nanoemMutableDocumentAccessorySetOrientation(ao, glm::value_ptr(Vector4(accessory->orientation(), 0)));
    if (StringUtils::tryGetString(factory, makeRelativeFromProjectPath(accessory->fileURI()), scope)) {
        nanoemMutableDocumentAccessorySetPath(ao, scope.value(), status);
    }
    nanoemMutableDocumentAccessorySetScaleFactor(ao, accessory->scaleFactor());
    nanoemMutableDocumentAccessorySetShadowEnabled(ao, accessory->isShadowMapEnabled());
    nanoemMutableDocumentAccessorySetTranslation(ao, glm::value_ptr(Vector4(accessory->translation(), 0)));
    nanoemMutableDocumentAccessorySetVisible(ao, accessory->isVisible());
    const StringPair outsideParent(accessory->outsideParent());
    if (const Model *model = m_project->findModelByName(outsideParent.first)) {
        ModelResolveMap::const_iterator it = resolveMap.find(model);
        if (it != resolveMap.end()) {
            nanoemMutableDocumentAccessorySetParentModelObject(ao, it->second);
            if (StringUtils::tryGetString(factory, outsideParent.second, scope)) {
                nanoemMutableDocumentAccessorySetParentModelBoneName(ao, scope.value(), status);
            }
        }
    }
    if (const Motion *motion = m_project->resolveMotion(accessory)) {
        saveAllAccessoryKeyframes(motion, resolveMap, ao, status);
    }
    nanoemMutableDocumentInsertAccessoryObject(document, ao, -1, status);
    nanoemMutableDocumentAccessoryDestroy(ao);
}

void
PMM::Context::saveAllAccessoryKeyframes(const Motion *motion, const ModelResolveMap &resolveMap,
    nanoem_mutable_document_accessory_t *ao, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_accessory_keyframe_t *const *keyframes =
        nanoemMotionGetAllAccessoryKeyframeObjects(motion->data(), &numKeyframes);
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_accessory_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(keyframe));
        nanoem_mutable_document_accessory_keyframe_t *ko = nanoemMutableDocumentAccessoryKeyframeCreate(ao, status);
        nanoemMutableDocumentAccessoryKeyframeSetTranslation(ko, nanoemMotionAccessoryKeyframeGetTranslation(keyframe));
        nanoemMutableDocumentAccessoryKeyframeSetOrientation(ko, nanoemMotionAccessoryKeyframeGetOrientation(keyframe));
        nanoemMutableDocumentAccessoryKeyframeSetScaleFactor(ko, nanoemMotionAccessoryKeyframeGetScaleFactor(keyframe));
        nanoemMutableDocumentAccessoryKeyframeSetOpacity(ko, nanoemMotionAccessoryKeyframeGetOpacity(keyframe));
        nanoemMutableDocumentAccessoryKeyframeSetShadowEnabled(
            ko, nanoemMotionAccessoryKeyframeIsShadowEnabled(keyframe));
        nanoemMutableDocumentAccessoryKeyframeSetVisible(ko, nanoemMotionAccessoryKeyframeIsVisible(keyframe));
        nanoemMutableDocumentAccessoryKeyframeSetShadowEnabled(
            ko, nanoemMotionAccessoryKeyframeIsShadowEnabled(keyframe));
        if (const nanoem_motion_outside_parent_t *outsideParent =
                nanoemMotionAccessoryKeyframeGetOutsideParent(keyframe)) {
            const nanoem_unicode_string_t *modelName = nanoemMotionOutsideParentGetTargetObjectName(outsideParent);
            String s;
            StringUtils::getUtf8String(modelName, factory, status, s);
            ModelResolveMap::const_iterator it = resolveMap.find(m_project->findModelByName(s));
            if (it != resolveMap.end()) {
                const nanoem_unicode_string_t *boneName = nanoemMotionOutsideParentGetTargetBoneName(outsideParent);
                nanoemMutableDocumentAccessoryKeyframeSetParentModelObject(ko, it->second);
                nanoemMutableDocumentAccessoryKeyframeSetParentModelBoneName(ko, boneName, status);
            }
        }
        nanoemMutableDocumentAccessoryAddAccessoryKeyframeObject(ao, ko, frameIndex, status);
        nanoemMutableDocumentAccessoryKeyframeDestroy(ko);
    }
}

void
PMM::Context::saveModel(const Model *model, int drawOrderIndex, int transformOrderIndex,
    nanoem_mutable_document_t *document, ModelResolveMap &resolveMap, nanoem_status_t *status)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    nanoem_mutable_document_model_t *mo = nanoemMutableDocumentModelCreate(document, status);
    nanoemMutableDocumentModelSetBlendEnabled(mo, model->isAddBlendEnabled());
    nanoemMutableDocumentModelSetDrawOrderIndex(mo, drawOrderIndex);
    nanoemMutableDocumentModelSetEdgeWidth(mo, model->edgeSize());
    nanoemMutableDocumentModelSetLastFrameIndex(mo, m_project->currentLocalFrameIndex());
    nanoemMutableDocumentModelSetSelfShadowEnabled(mo, model->isShadowMapEnabled());
    nanoemMutableDocumentModelSetTransformOrderIndex(mo, transformOrderIndex);
    for (nanoem_rsize_t i = NANOEM_LANGUAGE_TYPE_FIRST_ENUM; i < NANOEM_LANGUAGE_TYPE_MAX_ENUM; i++) {
        const nanoem_language_type_t language = static_cast<nanoem_language_type_t>(i);
        const nanoem_unicode_string_t *name = nanoemModelGetName(model->data(), language);
        nanoemMutableDocumentModelSetName(mo, language, name, status);
    }
    const URI fileURI(model->fileURI());
    if (StringUtils::tryGetString(factory, makeRelativeFromProjectPath(fileURI), scope)) {
        nanoemMutableDocumentModelSetPath(mo, scope.value(), status);
    }
    if (const model::Bone *bone = model::Bone::cast(model->activeBone())) {
        if (StringUtils::tryGetString(factory, bone->name(), scope)) {
            nanoemMutableDocumentModelSetSelectedBoneName(mo, scope.value(), status);
        }
    }
    for (nanoem_rsize_t i = NANOEM_MODEL_MORPH_CATEGORY_FIRST_ENUM; i < NANOEM_MODEL_MORPH_CATEGORY_MAX_ENUM; i++) {
        nanoem_model_morph_category_t category = static_cast<nanoem_model_morph_category_t>(i);
        if (const model::Morph *morph = model::Morph::cast(model->activeMorph(category))) {
            if (StringUtils::tryGetString(factory, morph->name(), scope)) {
                nanoemMutableDocumentModelSetSelectedMorphName(mo, category, scope.value(), status);
            }
        }
    }
    nanoem_rsize_t numBones, numMorphs;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        nanoemMutableDocumentModelRegisterBone(mo, name, status);
        if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
            nanoem_mutable_document_model_bone_state_t *state = nanoemMutableDocumentModelBoneStateCreate(mo, status);
            nanoemMutableDocumentModelBoneStateSetTranslation(
                state, glm::value_ptr(glm::vec4(bone->localUserTranslation(), 0)));
            nanoemMutableDocumentModelBoneStateSetOrientation(state, glm::value_ptr(bone->localUserOrientation()));
            nanoemMutableDocumentModelBoneStateSetPhysicsSimulationDisabled(state, false);
            nanoemMutableDocumentModelAddModelBoneStateObject(mo, state, name, status);
            nanoemMutableDocumentModelBoneStateDestroy(state);
        }
        if (const model::Constraint *constraint =
                model::Constraint::cast(nanoemModelBoneGetConstraintObject(bonePtr))) {
            nanoem_mutable_document_model_constraint_state_t *state =
                nanoemMutableDocumentModelConstraintStateCreate(mo, status);
            nanoemMutableDocumentModelConstraintStateSetEnabled(state, constraint->isEnabled());
            nanoemMutableDocumentModelAddModelConstraintStateObject(mo, state, name, status);
            nanoemMutableDocumentModelConstraintStateDestroy(state);
        }
    }
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    if (numMorphs > 0 && fileURI.pathExtension() == String("pmd") &&
        StringUtils::tryGetString(factory, "base", scope)) {
        nanoemMutableDocumentModelRegisterMorph(mo, scope.value(), status);
        nanoem_mutable_document_model_morph_state_t *state = nanoemMutableDocumentModelMorphStateCreate(mo, status);
        nanoemMutableDocumentModelMorphStateSetWeight(state, 0.0f);
        nanoemMutableDocumentModelAddModelMorphStateObject(mo, state, scope.value(), status);
        nanoemMutableDocumentModelMorphStateDestroy(state);
    }
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        nanoemMutableDocumentModelRegisterMorph(mo, name, status);
        if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
            nanoem_mutable_document_model_morph_state_t *state = nanoemMutableDocumentModelMorphStateCreate(mo, status);
            nanoemMutableDocumentModelMorphStateSetWeight(state, morph->weight());
            nanoemMutableDocumentModelAddModelMorphStateObject(mo, state, name, status);
            nanoemMutableDocumentModelMorphStateDestroy(state);
        }
    }
    if (const Motion *motion = m_project->resolveMotion(model)) {
        saveAllBoneKeyframes(motion, mo, status);
        saveAllModelKeyframes(motion, mo, status);
        saveAllMorphKeyframes(motion, mo, status);
    }
    nanoemMutableDocumentInsertModelObject(document, mo, -1, status);
    resolveMap.insert(tinystl::make_pair(model, nanoemMutableDocumentModelGetOrigin(mo)));
    nanoemMutableDocumentModelDestroy(mo);
}

void
PMM::Context::saveAllBoneKeyframes(const Motion *motion, nanoem_mutable_document_model_t *mo, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_bone_keyframe_t *const *keyframes =
        nanoemMotionGetAllBoneKeyframeObjects(motion->data(), &numKeyframes);
    const IMotionKeyframeSelection *selection = motion->selection();
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_bone_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(keyframe));
        const nanoem_unicode_string_t *name = nanoemMotionBoneKeyframeGetName(keyframe);
        nanoem_mutable_document_model_bone_keyframe_t *ko =
            nanoemMutableDocumentModelBoneKeyframeCreate(mo, name, status);
        nanoemMutableDocumentModelBoneKeyframeSetOrientation(ko, nanoemMotionBoneKeyframeGetOrientation(keyframe));
        nanoemMutableDocumentModelBoneKeyframeSetTranslation(ko, nanoemMotionBoneKeyframeGetTranslation(keyframe));
        nanoemMutableDocumentModelBoneKeyframeSetPhysicsSimulationDisabled(
            ko, nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(keyframe));
        nanoemDocumentBaseKeyframeSetSelected(nanoemDocumentModelBoneKeyframeGetBaseKeyframeObjectMutable(
                                                  nanoemMutableDocumentModelBoneKeyframeGetOrigin(ko)),
            selection->contains(keyframe));
        for (nanoem_rsize_t j = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM;
             j < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; j++) {
            nanoem_motion_bone_keyframe_interpolation_type_t type =
                static_cast<nanoem_motion_bone_keyframe_interpolation_type_t>(j);
            nanoemMutableDocumentModelBoneKeyframeSetInterpolation(
                ko, type, nanoemMotionBoneKeyframeGetInterpolation(keyframe, type));
        }
        nanoemMutableDocumentModelAddBoneKeyframeObject(mo, ko, name, frameIndex, status);
        nanoemMutableDocumentModelBoneKeyframeDestroy(ko);
    }
}

void
PMM::Context::saveAllModelKeyframes(const Motion *motion, nanoem_mutable_document_model_t *mo, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_model_keyframe_t *const *keyframes =
        nanoemMotionGetAllModelKeyframeObjects(motion->data(), &numKeyframes);
    const IMotionKeyframeSelection *selection = motion->selection();
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_model_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(keyframe));
        nanoem_mutable_document_model_keyframe_t *ko = nanoemMutableDocumentModelKeyframeCreate(mo, status);
        nanoem_rsize_t numStates;
        nanoem_motion_model_keyframe_constraint_state_t *const *states =
            nanoemMotionModelKeyframeGetAllConstraintStateObjects(keyframe, &numStates);
        for (nanoem_rsize_t j = 0; j < numStates; j++) {
            const nanoem_motion_model_keyframe_constraint_state_t *state = states[j];
            const nanoem_unicode_string_t *name = nanoemMotionModelKeyframeConstraintStateGetBoneName(state);
            nanoem_bool_t enabled = nanoemMotionModelKeyframeConstraintStateIsEnabled(state);
            nanoemMutableDocumentModelKeyframeSetConstraintEnabled(ko, name, enabled, status);
        }
        nanoemMutableDocumentModelKeyframeSetVisible(ko, nanoemMotionModelKeyframeIsVisible(keyframe));
        nanoemDocumentBaseKeyframeSetSelected(
            nanoemDocumentModelKeyframeGetBaseKeyframeObjectMutable(nanoemMutableDocumentModelKeyframeGetOrigin(ko)),
            selection->contains(keyframe));
        nanoemMutableDocumentModelAddModelKeyframeObject(mo, ko, frameIndex, status);
        nanoemMutableDocumentModelKeyframeDestroy(ko);
    }
}

void
PMM::Context::saveAllMorphKeyframes(const Motion *motion, nanoem_mutable_document_model_t *mo, nanoem_status_t *status)
{
    nanoem_rsize_t numKeyframes;
    nanoem_motion_morph_keyframe_t *const *keyframes =
        nanoemMotionGetAllMorphKeyframeObjects(motion->data(), &numKeyframes);
    const IMotionKeyframeSelection *selection = motion->selection();
    for (nanoem_rsize_t i = 0; i < numKeyframes; i++) {
        const nanoem_motion_morph_keyframe_t *keyframe = keyframes[i];
        const nanoem_frame_index_t frameIndex =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(keyframe));
        const nanoem_unicode_string_t *name = nanoemMotionMorphKeyframeGetName(keyframe);
        nanoem_mutable_document_model_morph_keyframe_t *ko =
            nanoemMutableDocumentModelMorphKeyframeCreate(mo, name, status);
        nanoemMutableDocumentModelMorphKeyframeSetWeight(ko, nanoemMotionMorphKeyframeGetWeight(keyframe));
        nanoemDocumentBaseKeyframeSetSelected(nanoemDocumentModelMorphKeyframeGetBaseKeyframeObjectMutable(
                                                  nanoemMutableDocumentModelMorphKeyframeGetOrigin(ko)),
            selection->contains(keyframe));
        nanoemMutableDocumentModelAddMorphKeyframeObject(mo, ko, name, frameIndex, status);
        nanoemMutableDocumentModelMorphKeyframeDestroy(ko);
    }
}

String
PMM::Context::makeRelativeFromProjectPath(const URI &fileURI) const
{
    const String absolutePath(fileURI.absolutePath());
    if (!absolutePath.empty()) {
        MutableString s(absolutePath.c_str(), absolutePath.c_str() + absolutePath.size() + 1);
        char *p = s.data();
        while (*p) {
            if (*p == '/') {
                *p = '\\';
            }
            p++;
        }
        return s.data();
    }
    return absolutePath;
}

PMM::PMM(Project *project)
    : m_context(nanoem_new(Context(project)))
{
    setFileURI(project->fileURI());
}

PMM::~PMM() NANOEM_DECL_NOEXCEPT
{
    nanoem_delete_safe(m_context);
}

bool
PMM::load(const nanoem_u8_t *data, size_t size, Error &error, Project::IDiagnostics *diagnostics)
{
    return m_context->load(data, size, error, diagnostics);
}

bool
PMM::save(ByteArray &bytes, Error &error)
{
    return m_context->save(bytes, error);
}

URI
PMM::fileURI() const
{
    return m_context->fileURI();
}

void
PMM::setFileURI(const URI &value)
{
    m_context->setFileURI(value);
}

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */
