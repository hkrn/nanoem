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

namespace {

static bool
loadAPNG(const char *filename, Error &error)
{
    FileReaderScope scope(nullptr);
    bool loaded = false;
    error = Error();
    if (scope.open(URI::createFromFilePath(filename), error)) {
        auto apng = ImageLoader::decodeAPNG(scope.reader(), error);
        loaded = apng != nullptr;
        nanoem_delete(apng);
    }
    return loaded && !error.hasReason();
}

} /* namespace anonymous */

// based on https://philip.html5.org/tests/apng/tests.html
TEST_CASE("imageloader_apng", "[emapp][misc]")
{
    Error error;
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/000.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/001.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/002.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/003.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/004.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/005.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/006.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/007.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/008.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/009.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/010.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/011.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/012.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/013.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/014.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/015.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/016.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/017.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/018.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/019.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/020.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/021.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/022.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/023.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/024.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/025.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/026.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/027.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/028.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/029.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/030.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/031.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/032.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/033.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/034.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/035.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/036.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/037.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/038.png", error));
    // Invalid APNG images
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/039.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/040.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/041.png", error));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/042.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Frame count (1 != 0) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/043.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Empty image sequence"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/044.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Empty image sequence"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/045.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Empty animation control"));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/046.png", error));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/047.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Frame count (1 != 2) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/048.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Frame count (3 != 2) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/049.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Frame count (4 != 2) doesn't match"));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/050.png", error));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/051.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (0 != 1) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/052.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (3 != 4) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/053.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (3 != 2) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/054.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (4 != 3) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/055.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (3 != 4) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/056.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (3 != 4) doesn't match"));
    CHECK_FALSE(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/057.png", error));
    CHECK_THAT(error.reasonConstString(), Catch::Equals("APNG: Incorrect sequence number (1 != 0) doesn't match"));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/058.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/059.png", error));
    CHECK(loadAPNG(NANOEM_TEST_FIXTURE_PATH "/apngs/060.png", error));
}

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
