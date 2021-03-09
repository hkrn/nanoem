/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>

#define LEFT_ORDER

namespace {

static void
createUnitAxes(const glm::vec3 &radians, glm::quat &x, glm::quat &y, glm::quat &z)
{
    x = glm::angleAxis(radians.x, glm::vec3(1, 0, 0));
    y = glm::angleAxis(radians.y, glm::vec3(0, 1, 0));
    z = glm::angleAxis(radians.z, glm::vec3(0, 0, 1));
}

static glm::quat
rotateZXY(const glm::vec3 &radians)
{
    glm::quat x, y, z;
    createUnitAxes(radians, x, y, z);
#ifdef LEFT_ORDER
    return z * x * y;
#else
    return y * x * z;
#endif
}

static glm::quat
rotateXYZ(const glm::vec3 &radians)
{
    glm::quat x, y, z;
    createUnitAxes(radians, x, y, z);
#ifdef LEFT_ORDER
    return x * y * z;
#else
    return z * y * x;
#endif
}

static glm::quat
rotateYZX(const glm::vec3 &radians)
{
    glm::quat x, y, z;
    createUnitAxes(radians, x, y, z);
#ifdef LEFT_ORDER
    return y * z * x;
#else
    return x * z * y;
#endif
}

static glm::vec3
extractEulerZXY(const glm::mat3 &matrix)
{
    glm::vec3 radians;
#ifdef LEFT_ORDER
    radians.x = glm::asin(matrix[1][2]);
    radians.y = glm::atan(-matrix[0][2], matrix[2][2]);
    radians.z = glm::atan(-matrix[1][0], matrix[1][1]);
#elif 0
    const float *v = glm::value_ptr(matrix);
    radians.x = glm::asin(-v[7]);
    radians.y = glm::atan(v[6], v[8]);
    radians.z = glm::atan(v[1], v[4]);
#else
    radians.x = glm::asin(-matrix[2][1]);
    radians.y = glm::atan(matrix[2][0], matrix[2][2]);
    radians.z = glm::atan(matrix[0][1], matrix[1][1]);
#endif
    return radians;
}

static glm::vec3
extractEulerXYZ(const glm::mat3 &matrix)
{
    glm::vec3 radians;
#ifdef LEFT_ORDER
    radians.x = glm::atan(-matrix[2][1], matrix[2][2]);
    radians.y = glm::asin(matrix[2][0]);
    radians.z = glm::atan(-matrix[1][0], matrix[0][0]);
#elif 0
    const float *v = glm::value_ptr(matrix);
    radians.x = glm::atan(v[5], v[8]);
    radians.y = glm::asin(-v[2]);
    radians.z = glm::atan(v[1], v[0]);
#else
    radians.x = glm::atan(matrix[1][2], matrix[2][2]);
    radians.y = glm::asin(-matrix[0][2]);
    radians.z = glm::atan(matrix[0][1], matrix[0][0]);
#endif
    return radians;
}

static glm::vec3
extractEulerYZX(const glm::mat3 &matrix)
{
    glm::vec3 radians;
#ifdef LEFT_ORDER
    radians.x = glm::atan(-matrix[2][1], matrix[1][1]);
    radians.y = glm::atan(-matrix[0][2], matrix[0][0]);
    radians.z = glm::asin(matrix[0][1]);
#elif 0
    const float *v = glm::value_ptr(matrix);
    radians.x = glm::atan(v[5], v[4]);
    radians.y = glm::atan(v[6], v[0]);
    radians.z = glm::asin(-v[3]);
#else
    radians.x = glm::atan(matrix[1][2], matrix[1][1]);
    radians.y = glm::atan(matrix[2][0], matrix[0][0]);
    radians.z = glm::asin(-matrix[1][0]);
#endif
    return radians;
}
}

int
main(void)
{
    static const glm::vec3 input(glm::radians(glm::vec3(150, 150, 30)));
    {
        glm::quat q = rotateXYZ(input);
        glm::vec3 output = extractEulerXYZ(glm::mat3_cast(q));
        std::cerr << "XYZ: " << glm::degrees(output) << " "
                  << glm::all(glm::lessThan(glm::abs(output - input), glm::vec3(glm::epsilon<glm::vec3::value_type>())))
                  << std::endl;
    }
    {
        glm::quat q = rotateYZX(input);
        glm::vec3 output = extractEulerYZX(glm::mat3_cast(q));
        std::cerr << "YZX: " << glm::degrees(output) << " "
                  << glm::all(glm::lessThan(glm::abs(output - input), glm::vec3(glm::epsilon<glm::vec3::value_type>())))
                  << std::endl;
    }
    {
        glm::quat q = rotateZXY(input);
        glm::vec3 output = extractEulerZXY(glm::mat3_cast(q));
        std::cerr << "ZXY: " << glm::degrees(output) << " "
                  << glm::all(glm::lessThan(glm::abs(output - input), glm::vec3(glm::epsilon<glm::vec3::value_type>())))
                  << std::endl;
        std::flush(std::cerr);
    }
    return 0;
}
