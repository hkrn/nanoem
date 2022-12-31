/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

float4
nanoemVSMain(float4 position: SV_POSITION) : SV_POSITION
{
    return position;
}

float4
nanoemPSMain() : SV_TARGET0
{
    return float4(0, 0, 0, 0);
}
