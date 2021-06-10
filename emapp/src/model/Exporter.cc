/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Exporter.h"

namespace nanoem {
namespace model {

Exporter::Exporter(const Model *model)
    : m_model(model)
{
}

Exporter::~Exporter() NANOEM_DECL_NOEXCEPT
{
}

bool
Exporter::execute(IWriter *writer, const Model::ExportDescription &desc, Error &error)
{
    BX_UNUSED_4(m_model, writer, desc, error);
    return false;
}

} /* namespace model */
} /* namespace nanoem */
