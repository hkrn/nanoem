/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/StringUtils.h"

using namespace nanoem;
using namespace test;

TEST_CASE("stringutils_length", "[emapp][misc]")
{
    CHECK(StringUtils::length("abcdef") == 6);
    CHECK(StringUtils::length("abcdef", 4) == 4);
    CHECK(StringUtils::length("") == 0);
    CHECK(StringUtils::length(nullptr) == 0);
}

TEST_CASE("stringutils_equals", "[emapp][misc]")
{
    CHECK(StringUtils::equals("abcdef", "abcdef"));
    CHECK_FALSE(StringUtils::equals("ABCDEF", "abcdef"));
    CHECK_FALSE(StringUtils::equals("abcdfe", "abcdef"));
    CHECK(StringUtils::equals("abcdfe", "abcdef", 4));
    CHECK_FALSE(StringUtils::equals("abcdef", nullptr));
    CHECK_FALSE(StringUtils::equals(nullptr, "abcdef"));
    CHECK(StringUtils::equals(nullptr, nullptr));
}

TEST_CASE("stringutils_equalsignorecase", "[emapp][misc]")
{
    CHECK(StringUtils::equalsIgnoreCase("abcdef", "abcdef"));
    CHECK(StringUtils::equalsIgnoreCase("ABCDEF", "abcdef"));
    CHECK_FALSE(StringUtils::equalsIgnoreCase("abcdfe", "abcdef"));
    CHECK(StringUtils::equalsIgnoreCase("abcdfe", "abcdef", 4));
    CHECK_FALSE(StringUtils::equalsIgnoreCase("abcdef", nullptr));
    CHECK_FALSE(StringUtils::equalsIgnoreCase(nullptr, "abcdef"));
    CHECK(StringUtils::equalsIgnoreCase(nullptr, nullptr));
}

TEST_CASE("stringutils_hasprefix", "[emapp][misc]")
{
    CHECK(StringUtils::hasPrefix("abcdef", "abc"));
    CHECK(StringUtils::hasPrefix("abc", "abc"));
    CHECK_FALSE(StringUtils::hasPrefix("ABCDEF", "abc"));
    CHECK_FALSE(StringUtils::hasPrefix("ab", "abc"));
}

TEST_CASE("stringutils_touppercase", "[emapp][misc]")
{
    CHECK(StringUtils::toUpperCase("abcdef") == String("ABCDEF"));
    CHECK(StringUtils::toUpperCase("ABCdef") == String("ABCDEF"));
    CHECK(StringUtils::toUpperCase("abcDEF") == String("ABCDEF"));
    CHECK(StringUtils::toUpperCase("ABCDEF") == String("ABCDEF"));
}

TEST_CASE("stringutils_tolowercase", "[emapp][misc]")
{
    CHECK(StringUtils::toLowerCase("abcdef") == String("abcdef"));
    CHECK(StringUtils::toLowerCase("ABCdef") == String("abcdef"));
    CHECK(StringUtils::toLowerCase("abcDEF") == String("abcdef"));
    CHECK(StringUtils::toLowerCase("ABCDEF") == String("abcdef"));
}

TEST_CASE("stringutils_format", "[emapp][misc]")
{
    char buffer[128];
    String sb;
    CHECK(StringUtils::format(buffer, sizeof(buffer), "%s:%s", "abc", "def") == 7);
    CHECK(StringUtils::equals(buffer, "abc:def"));
    CHECK(StringUtils::format(sb, "%s:%s", "abc", "def") == 7);
    CHECK(StringUtils::equals(sb.c_str(), "abc:def"));
}

TEST_CASE("stringutils_formatoverflow", "[emapp][misc]")
{
    String sb, result;
    for (int i = 0; i <= 16384; i++) {
        sb.append("0");
    }
    CHECK(StringUtils::format(result, "%s", sb.c_str()) == static_cast<int>(sb.size()));
    CHECK(result == sb);
}

TEST_CASE("stringutils_parseint", "[emapp][misc]")
{
    char *ptr = nullptr;
    CHECK(StringUtils::parseInteger("42", &ptr) == 42);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseInteger("42.0", &ptr) == 42);
    CHECK(StringUtils::equals(ptr, ".0"));
    CHECK(StringUtils::parseInteger("2147483647", &ptr) == 2147483647);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseInteger("2147483648", &ptr) == 2147483647);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseInteger("-2147483647", &ptr) == 2147483647);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseInteger("-2147483648", &ptr) == 2147483647);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseInteger("abcdef", &ptr) == 0);
    CHECK(StringUtils::equals(ptr, "abcdef"));
}

TEST_CASE("stringutils_parseunsignedint", "[emapp][misc]")
{
    char *ptr = nullptr;
    CHECK(StringUtils::parseUnsignedInteger("42", &ptr) == 42);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseUnsignedInteger("42.0", &ptr) == 42);
    CHECK(StringUtils::equals(ptr, ".0"));
    CHECK(StringUtils::parseUnsignedInteger("4294967295", &ptr) == 4294967295);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseUnsignedInteger("4294967296", &ptr) == 4294967295);
    CHECK(StringUtils::equals(ptr, ""));
    CHECK(StringUtils::parseUnsignedInteger("abcdef", &ptr) == 0);
    CHECK(StringUtils::equals(ptr, "abcdef"));
}
