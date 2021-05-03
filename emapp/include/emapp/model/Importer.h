/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_IMPORTER_H_
#define NANOEM_EMAPP_MODEL_IMPORTER_H_

#include "emapp/Model.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {
namespace model {

class Importer NANOEM_DECL_SEALED : private NonCopyable {
public:
    Importer(Model *model);
    ~Importer() NANOEM_DECL_NOEXCEPT;

    bool execute(const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error);

private:
    bool handleWavefrontObjDocument(
        const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error);
    bool handleDirectXMeshDocument(
        const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error);
    bool handleMetasequoiaDocument(
        const nanoem_u8_t *bytes, size_t length, const Model::ImportDescription &desc, Error &error);
    void setModelNameAndComment(
        nanoem_mutable_model_t *model, const Model::ImportDescription &desc, nanoem_status_t *status);
    void setMaterialTexture(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *texture,
        const char *texturePathPtr, nanoem_status_t *status);

    Model *m_model;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_IMPORTER_H_ */
