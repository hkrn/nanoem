/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

using namespace nanoem;

namespace test {

namespace matcher {

EqualsVec2::EqualsVec2(const Vector2 &v)
    : m_data(v)
{
}

EqualsVec2::EqualsVec2(const EqualsVec2 &v)
    : m_data(v.m_data)
{
}

bool
EqualsVec2::match(const Vector2 &v) const
{
    return m_data == v;
}

std::string
EqualsVec2::describe() const
{
    return "== " + glm::to_string(m_data);
}

EqualsVec3::EqualsVec3(const Vector3 &v)
    : m_data(v)
{
}

EqualsVec3::EqualsVec3(const EqualsVec3 &v)
    : m_data(v.m_data)
{
}

bool
EqualsVec3::match(const Vector3 &v) const
{
    return m_data == v;
}

std::string
EqualsVec3::describe() const
{
    return "== " + glm::to_string(m_data);
}

EqualsVec4::EqualsVec4(const Vector4 &v)
    : m_data(v)
{
}

EqualsVec4::EqualsVec4(const EqualsVec4 &v)
    : m_data(v.m_data)
{
}

bool
EqualsVec4::match(const Vector4 &v) const
{
    return m_data == v;
}

std::string
EqualsVec4::describe() const
{
    return "== " + glm::to_string(m_data);
}

EqualsU8Vec4::EqualsU8Vec4(const glm::u8vec4 &v)
    : m_data(v)
{
}

EqualsU8Vec4::EqualsU8Vec4(const EqualsU8Vec4 &v)
    : m_data(v.m_data)
{
}

bool
EqualsU8Vec4::match(const glm::u8vec4 &v) const
{
    return m_data == v;
}

std::string
EqualsU8Vec4::describe() const
{
    return "== " + glm::to_string(m_data);
}

EqualsQuat::EqualsQuat(const Quaternion &v)
    : m_data(v)
{
}

EqualsQuat::EqualsQuat(const EqualsQuat &v)
    : m_data(v.m_data)
{
}

bool
EqualsQuat::match(const Quaternion &v) const
{
    return m_data == v;
}

std::string
EqualsQuat::describe() const
{
    return "== " + glm::to_string(m_data);
}
}
}
