/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ModelEffectSetting.h"

#include "emapp/Effect.h"
#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "../ini.h"

namespace nanoem {
namespace internal {

ModelEffectSetting::ModelEffectSetting(Project *project)
    : m_project(project)
    , m_fileManager(project->fileManager())
{
}

ModelEffectSetting::ModelEffectSetting(Project *project, IFileManager *fileManager)
    : m_project(project)
    , m_fileManager(fileManager)
{
}

ModelEffectSetting::~ModelEffectSetting() NANOEM_DECL_NOEXCEPT
{
}

void
ModelEffectSetting::load(const char *data, Model *model, Progress &progress, Error &error)
{
    const URI fileURI(model->fileURI());
    ini_t *ini = ini_load(data, nullptr);
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    for (int i = 0, numSections = ini_section_count(ini); i < numSections; i++) {
        const char *section = ini_section_name(ini, i);
        if (StringUtils::equals(section, "Effect")) {
            for (int j = 0, numProperties = ini_property_count(ini, i); j < numProperties; j++) {
                const char *name = ini_property_name(ini, i, j);
                const char *value = ini_property_value(ini, i, j);
                if (StringUtils::equals(name, "Obj", 3) && !StringUtils::equalsIgnoreCase(value, "none")) {
                    String s;
                    FileUtils::canonicalizePathSeparator(StringUtils::skipWhiteSpaces(value), s);
                    if (!s.empty()) {
                        String effectFilePath(fileURI.absolutePathByDeletingLastPathComponent());
                        effectFilePath.append("/");
                        effectFilePath.append(s.c_str());
                        const URI fileURI(URI::createFromFilePath(effectFilePath));
                        const char *left = strchr(name + 3, '['), *right = strchr(name + 3, ']');
                        if (left && right) {
                            char *term = nullptr;
                            int materialIndex = StringUtils::parseInteger(left + 1, &term);
                            if (materialIndex >= 0 && materialIndex < Inline::saturateInt32(numMaterials) &&
                                term == right) {
                                if (Effect *effect = compileEffect(fileURI, progress, error)) {
                                    model::Material *material = model::Material::cast(materials[materialIndex]);
                                    m_project->attachModelMaterialEffect(material, effect);
                                }
                            }
                        }
                        else if (Effect *effect = compileEffect(fileURI, progress, error)) {
                            m_project->attachActiveEffect(model, effect, progress, error);
                        }
                    }
                }
            }
        }
    }
    ini_destroy(ini);
}

void
ModelEffectSetting::save(const Model *model, MutableString &data)
{
    ini_t *ini = ini_create(nullptr);
    {
        int infoSection = ini_section_add(ini, "Info", -1);
        ini_property_add(ini, infoSection, "Version", -1, "3", -1);
        int effectSection = ini_section_add(ini, "Effect", -1);
        const URI fileURI(model->fileURI());
        const String baseModelDirectory(fileURI.absolutePathByDeletingLastPathComponent());
        if (const Effect *effect = m_project->resolveEffect(model)) {
            String relativePath(FileUtils::relativePath(effect->fileURI().absolutePath(), baseModelDirectory));
            ini_property_add(ini, effectSection, "Obj", -1, relativePath.c_str(), relativePath.size());
        }
        else {
            ini_property_add(ini, effectSection, "Obj", -1, "none", -1);
        }
        nanoem_rsize_t numMaterials;
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            if (const model::Material *material = model::Material::cast(materialPtr)) {
                if (const Effect *effect = material->effect()) {
                    String keyName,
                        relativePath(FileUtils::relativePath(effect->fileURI().absolutePath(), baseModelDirectory));
                    StringUtils::format(keyName, "Obj[%d]", Inline::saturateInt32(i));
                    ini_property_add(
                        ini, effectSection, keyName.c_str(), keyName.size(), relativePath.c_str(), relativePath.size());
                }
            }
        }
    }
    int size = ini_save(ini, 0, 0);
    data.resize(size + 1);
    ini_save(ini, data.data(), size);
    data[size] = 0;
    ini_destroy(ini);
}

Effect *
ModelEffectSetting::compileEffect(const URI &fileURI, Progress &progress, Error &error)
{
    ByteArray bytes;
    Effect *effect = nullptr;
    if (Effect::compileFromSource(fileURI, m_fileManager, m_project->isMipmapEnabled(), bytes, progress, error)) {
        Effect *innerEffect = m_project->createEffect();
        innerEffect->setName(fileURI.lastPathComponent());
        if (innerEffect->load(bytes, progress, error)) {
            innerEffect->setFileURI(fileURI);
            if (innerEffect->upload(effect::kAttachmentTypeMaterial, progress, error)) {
                effect = innerEffect;
            }
        }
        if (error.hasReason()) {
            m_project->destroyEffect(innerEffect);
        }
    }
    return effect;
}

} /* namespace internal */
} /* namespace nanoem */
