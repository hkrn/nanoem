/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_EXPORTER_H_
#define NANOEM_EMAPP_MODEL_EXPORTER_H_

#include "emapp/Model.h"

namespace nanoem {
namespace model {

class Exporter NANOEM_DECL_SEALED : private NonCopyable {
public:
    Exporter(const Model *model);
    ~Exporter() NANOEM_DECL_NOEXCEPT;

    bool execute(IWriter *writer, const Model::ExportDescription &desc, Error &error);

private:
    const Model *m_model;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_EXPORTER_H_ */
