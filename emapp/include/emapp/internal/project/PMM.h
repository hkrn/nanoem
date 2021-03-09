/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_PROJECT_PMM_H_
#define NANOEM_EMAPP_INTERNAL_PROJECT_PMM_H_

#include "emapp/Forward.h"
#include "emapp/URI.h"

struct nanoem_document_t;
struct nanoem_document_accessory_t;
struct nanoem_document_model_t;
struct ini_t;

namespace nanoem {

class Accessory;
class Effect;
class Error;
class IDrawable;
class Model;
class Motion;
class Progress;
class Project;

namespace internal {
namespace project {

class PMM NANOEM_DECL_SEALED : private NonCopyable {
public:
    PMM(Project *project);
    ~PMM() NANOEM_DECL_NOEXCEPT;

    bool load(const nanoem_u8_t *data, size_t size, Error &error);
    bool save(ByteArray &bytes, Error &error);

    URI fileURI() const;
    void setFileURI(const URI &value);

private:
    struct Context;
    Context *m_context;
};

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROJECT_PMM_H_ */
