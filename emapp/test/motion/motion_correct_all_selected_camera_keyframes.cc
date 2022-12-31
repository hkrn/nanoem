/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

using namespace nanoem;
using namespace test;

struct emapp_motion_correct_all_selected_camera_keyframes_parameter_t {
    const Vector3 base;
    const Vector3 mul;
    const Vector3 add;
    const Vector3 expected;
};

#if 0
ParameterizedTestParameters(emapp, motion_correct_all_selected_camera_keyframes_translation)
{
    static emapp_motion_correct_all_selected_camera_keyframes_parameter_t parameters[] = {
        { Vector3(1), Vector3(0.5f), Vector3(0.3f), Vector3(0.8f) },
        { Vector3(1), Vector3(-0.5f), Vector3(0.3f), Vector3(-0.2f) },
        { Vector3(1), Vector3(0.5f), Vector3(-0.3f), Vector3(0.2f) },
        { Vector3(1), Vector3(-0.5f), Vector3(-0.3f), Vector3(-0.8f) }
    };
    return criterion_test_params(parameters);
}

ParameterizedTest(emapp_motion_correct_all_selected_camera_keyframes_parameter_t *parameter, emapp,
    motion_correct_all_selected_camera_keyframes_translation)
{
    cr_skip("not implemented");
}
#endif
