/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/ImageLoader.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include <dirent.h>

using namespace nanoem;
using namespace test;

#if defined(NANOEM_TEST_DXTEXMEDIA_PATH)

namespace {

class DDSFileURIGenerator : public Catch::Generators::IGenerator<URI> {
public:
    static bool isFailureExpected(const URI &fileURI) noexcept;

    DDSFileURIGenerator();
    ~DDSFileURIGenerator();

    const URI &get() const override;
    bool next() override;

private:
    DIR *m_dir = nullptr;
    URI m_fileURI;
};

bool
DDSFileURIGenerator::isFailureExpected(const URI &fileURI) noexcept
{
    return StringUtils::equals(fileURI.lastPathComponentConstString(), "texture0.dds");
}

DDSFileURIGenerator::DDSFileURIGenerator()
    : m_dir(::opendir(NANOEM_TEST_DXTEXMEDIA_PATH))
{
    next();
}

DDSFileURIGenerator::~DDSFileURIGenerator()
{
    ::closedir(m_dir);
}

const URI &
DDSFileURIGenerator::get() const
{
    return m_fileURI;
}

bool
DDSFileURIGenerator::next()
{
    const dirent *cursor = ::readdir(m_dir);
    bool hasNext = cursor != nullptr;
    if (hasNext) {
        String ddsFilePath(NANOEM_TEST_DXTEXMEDIA_PATH);
        ddsFilePath.append("/");
        ddsFilePath.append(cursor->d_name);
        m_fileURI = URI::createFromFilePath(ddsFilePath);
        if (!StringUtils::equalsIgnoreCase(m_fileURI.pathExtension().c_str(), "dds")) {
            hasNext = next();
        }
    }
    return hasNext;
}

} /* namespace anonoymous */

TEST_CASE("imageloader_dds", "[emapp][misc]")
{
    Error error;
    FileReaderScope scope(nullptr);
    const URI fileURI = GENERATE(Catch::Generators::take(UINT32_MAX,
        Catch::Generators::GeneratorWrapper<URI>(Catch::Generators::pf::make_unique<DDSFileURIGenerator>())));
    scope.open(fileURI, error);
    image::DDS *dds = ImageLoader::decodeDDS(scope.reader(), error);
    if (strstr(error.reasonConstString(), "Unsupported texture format") == 0) {
        if (DDSFileURIGenerator::isFailureExpected(fileURI)) {
            CHECK_FALSE(dds != nullptr);
        }
        else {
            CHECK(dds != nullptr);
        }
    }
    nanoem_delete(dds);
}
#endif /* NANOEM_TEST_DXTEXMEDIA_PATH */
