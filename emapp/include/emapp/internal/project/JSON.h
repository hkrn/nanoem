/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#pragma once
#ifndef NANOEM_EMAPP_PROJECT_JSON_H_
#define NANOEM_EMAPP_PROJECT_JSON_H_

#include "emapp/Forward.h"

#include "nanoem/ext/parson/parson.h"

namespace nanoem {

class Project;

namespace internal {
namespace project {

class JSON NANOEM_DECL_SEALED : private NonCopyable {
public:
    JSON(Project *project);
    ~JSON() NANOEM_DECL_NOEXCEPT;

    bool load(const JSON_Value *value);
    void save(JSON_Value *value);

private:
    Project *m_project;
};

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PROJECT_JSON_H_ */
