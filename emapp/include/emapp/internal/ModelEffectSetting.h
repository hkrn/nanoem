/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Forward.h"

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_MODELEFFECTSETTING_H_
#define NANOEM_EMAPP_INTERNAL_MODELEFFECTSETTING_H_

namespace nanoem {

class Effect;
class Error;
class IFileManager;
class Model;
class Project;
class Progress;
class URI;

namespace internal {

class ModelEffectSetting : private NonCopyable {
public:
    ModelEffectSetting(Project *project);
    ModelEffectSetting(Project *project, IFileManager *fileManager);
    ~ModelEffectSetting() NANOEM_DECL_NOEXCEPT;

    void load(const char *data, Model *model, Progress &progress, Error &error);
    void save(const Model *model, MutableString &data);

private:
    Effect *compileEffect(const URI &fileURI, Progress &progress, Error &error);

    Project *m_project;
    IFileManager *m_fileManager;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_MODELEFFECTSETTING_H_ */
