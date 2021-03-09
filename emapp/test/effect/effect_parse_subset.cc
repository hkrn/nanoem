/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Effect.h"

#include <sstream>

using namespace nanoem;

namespace {

static std::string
stringify(const Effect::MaterialIndexSet &value)
{
    std::ostringstream s;
    s << "(";
    s << value.size();
    s << ") { ";
    for (Effect::MaterialIndexSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        s << *it;
        s << ", ";
    }
    s << " }";
    return s.str();
}

struct EqualsSubset : Catch::Matchers::Impl::MatcherBase<Effect::MaterialIndexSet> {
    EqualsSubset(const Effect::MaterialIndexSet &v);
    EqualsSubset(const EqualsSubset &v);
    bool match(const Effect::MaterialIndexSet &v) const override;
    std::string describe() const override;
    Effect::MaterialIndexSet m_data;
};

EqualsSubset::EqualsSubset(const Effect::MaterialIndexSet &v)
    : m_data(v)
{
}

EqualsSubset::EqualsSubset(const EqualsSubset &v)
    : m_data(v.m_data)
{
}

bool
EqualsSubset::match(const Effect::MaterialIndexSet &v) const
{
    bool result = m_data.size() == v.size();
    if (result) {
        for (Effect::MaterialIndexSet::const_iterator it = v.begin(), end = v.end(); it != end; ++it) {
            result &= m_data.find(*it) != m_data.end();
        }
    }
    return result;
}

std::string
EqualsSubset::describe() const
{
    return "== " + stringify(m_data);
}

static inline EqualsSubset
Equals(const Effect::MaterialIndexSet &v)
{
    return EqualsSubset(v);
}

} /* namespace anonymous */

namespace Catch {

template <> struct StringMaker<Effect::MaterialIndexSet> {
    static std::string convert(const Effect::MaterialIndexSet &value);
};

std::string
StringMaker<Effect::MaterialIndexSet>::convert(const Effect::MaterialIndexSet &value)
{
    return stringify(value);
}

} /* namespace Catch */

TEST_CASE("effect_parse_subset_comma", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(1);
    expected.insert(3);
    Effect::parseSubset("1,3", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_comma_with_spaces", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(1);
    expected.insert(3);
    Effect::parseSubset(" 1 , 3 ", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    Effect::parseSubset("1-3", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen_with_spaces", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    Effect::parseSubset(" 1 - 3 ", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen_swapped", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(2);
    expected.insert(3);
    expected.insert(4);
    Effect::parseSubset("4-2", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen_begin", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(0);
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    Effect::parseSubset("-3", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen_begin_with_spaces", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(0);
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    Effect::parseSubset(" - 3 ", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen_end", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(3);
    expected.insert(4);
    Effect::parseSubset("3-", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_hyphen_end_with_spaces", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(3);
    expected.insert(4);
    Effect::parseSubset(" 3 - ", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_mixed", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(0);
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    expected.insert(4);
    Effect::parseSubset("-1,2,3-", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_mixed_unordered", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(0);
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    expected.insert(4);
    Effect::parseSubset("2,4-,-1,3", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_empty", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    Effect::parseSubset("", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_empty_comma", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    Effect::parseSubset(",", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_empty_hyphen", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    expected.insert(0);
    expected.insert(1);
    expected.insert(2);
    expected.insert(3);
    expected.insert(4);
    Effect::parseSubset("-", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}

TEST_CASE("effect_parse_subset_invalid_subset", "[emapp][effect]")
{
    Effect::MaterialIndexSet subsets, expected;
    Effect::parseSubset("6,,7,,,8,12-", 5, subsets);
    CHECK_THAT(subsets, Equals(expected));
}
