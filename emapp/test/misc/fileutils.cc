/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/FileUtils.h"

using namespace nanoem;
using namespace test;

TEST_CASE("fileutils_canonicalize_path_separator", "[emapp][misc]")
{
    String result;
    FileUtils::canonicalizePathSeparator("", result);
    CHECK(result == String());
    FileUtils::canonicalizePathSeparator("C:\\path\\to\\windows", result);
    CHECK(result == String("C:/path/to/windows"));
    FileUtils::canonicalizePathSeparator("C:\\\\path\\\\to\\\\windows", result);
    CHECK(result == String("C:/path/to/windows"));
}

TEST_CASE("fileutils_canonicalize_path", "[emapp][misc]")
{
    CHECK(FileUtils::canonicalizePath("", "") == String());
    CHECK(FileUtils::canonicalizePath("/path/to", "file") == String("/path/to/file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "../file") == String("/path/file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "../../file") == String("file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "../../../file") == String("file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "./file") == String("/path/to/file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "././file") == String("/path/to/file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "./../file") == String("/path/file"));
    CHECK(FileUtils::canonicalizePath("/path/to", "./.././../file") == String("file"));
    CHECK(FileUtils::canonicalizePath("./path/to", "file") == String("path/to/file"));
    CHECK(FileUtils::canonicalizePath("./path/to", "../file") == String("path/file"));
    CHECK(FileUtils::canonicalizePath("./path/to", "../../file") == String("./file"));
    CHECK(FileUtils::canonicalizePath("./path/to", "../../../file") == String("file"));
}

TEST_CASE("fileutils_relative_path", "[emapp][misc]")
{
    CHECK(FileUtils::relativePath("", "") == String());
    CHECK(FileUtils::relativePath("/", "/") == String());
    CHECK(FileUtils::relativePath("/", "/path/to/base") == String("../../../"));
    CHECK(FileUtils::relativePath("/path/to/relative", "") == String());
    CHECK(FileUtils::relativePath("/path/to/relative", "/") == String("path/to/relative"));
    CHECK(FileUtils::relativePath("/path/to/relative", "/path/to/base") == String("../relative"));
    CHECK(FileUtils::relativePath("/relative", "/path/to/base") == String("../../../relative"));
    CHECK(FileUtils::relativePath("/path/to/relative", "/base") == String("../path/to/relative"));
    CHECK(FileUtils::relativePath("C:/", "C:/") == String());
    CHECK(FileUtils::relativePath("C:/", "C:/path/to/base") == String("../../../"));
    CHECK(FileUtils::relativePath("C:/path/to/relative", "") == String());
    CHECK(FileUtils::relativePath("C:/path/to/relative", "C:/") == String("path/to/relative"));
    CHECK(FileUtils::relativePath("D:/path/to/relative", "D:/path/to/base") == String("../relative"));
    CHECK(FileUtils::relativePath("D:/relative", "D:/path/to/base") == String("../../../relative"));
    CHECK(FileUtils::relativePath("D:/path/to/relative", "D:/base") == String("../path/to/relative"));
    CHECK(FileUtils::relativePath("D:/path/to/relative", "C:/base") == String());
}
