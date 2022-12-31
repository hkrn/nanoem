/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "../common.h"

#include "emapp/Effect.h"

using namespace nanoem;

TEST_CASE("effect_parse_empty_script", "[emapp][effect]")
{
    effect::ScriptCommandMap scripts;
    Effect::parseScript("", scripts);
    CHECK(scripts.empty());
}

TEST_CASE("effect_parse_script", "[emapp][effect]")
{
    effect::ScriptCommandMap scripts;
    bool scriptExternal;
    int clearColorScriptIndex, clearDepthScriptIndex;
    Effect::parseScript("RenderColorTarget=A;"
                        " RenderColorTarget0=B;"
                        "  RenderColorTarget1    =C;"
                        "   RenderColorTarget2=  D;"
                        "    RenderColorTarget3=E    ;"
                        "     RenderDepthStencilTarget   =   F   ;  "
                        "      ClearSetColor=  G;"
                        "      ClearSetDepth  =H   ;"
                        "      Clear=I;"
                        "      ScriptExternal=J;  "
                        "      Pass=K  ;"
                        "      LoopByCount=L; "
                        "        LoopGetIndex=M  ;"
                        "      LoopEnd=N ; "
                        "     Draw=O; "
                        "  This_is_just_ignored=P;"
                        "rendercolortarget=Q;",
        scripts, clearColorScriptIndex, clearDepthScriptIndex, scriptExternal);
    CHECK(scripts.size() == 15);
    CHECK(scripts[0].first == effect::kScriptCommandTypeSetRenderColorTarget0);
    CHECK(scripts[0].second == String("A"));
    CHECK(scripts[1].first == effect::kScriptCommandTypeSetRenderColorTarget0);
    CHECK(scripts[1].second == String("B"));
    CHECK(scripts[2].first == effect::kScriptCommandTypeSetRenderColorTarget1);
    CHECK(scripts[2].second == String("C"));
    CHECK(scripts[3].first == effect::kScriptCommandTypeSetRenderColorTarget2);
    CHECK(scripts[3].second == String("D"));
    CHECK(scripts[4].first == effect::kScriptCommandTypeSetRenderColorTarget3);
    CHECK(scripts[4].second == String("E"));
    CHECK(scripts[5].first == effect::kScriptCommandTypeSetRenderDepthStencilTarget);
    CHECK(scripts[5].second == String("F"));
    CHECK(scripts[6].first == effect::kScriptCommandTypeClearSetColor);
    CHECK(scripts[6].second == String("G"));
    CHECK(scripts[7].first == effect::kScriptCommandTypeClearSetDepth);
    CHECK(scripts[7].second == String("H"));
    CHECK(scripts[8].first == effect::kScriptCommandTypeClear);
    CHECK(scripts[8].second == String("I"));
    CHECK(scripts[9].first == effect::kScriptCommandTypeSetScriptExternal);
    CHECK(scripts[9].second == String("J"));
    CHECK(scripts[10].first == effect::kScriptCommandTypeExecutePass);
    CHECK(scripts[10].second == String("K"));
    CHECK(scripts[11].first == effect::kScriptCommandTypePushLoopCounter);
    CHECK(scripts[11].second == String("L"));
    CHECK(scripts[12].first == effect::kScriptCommandTypeGetLoopIndex);
    CHECK(scripts[12].second == String("M"));
    CHECK(scripts[13].first == effect::kScriptCommandTypePopLoopCounter);
    CHECK(scripts[13].second == String());
    CHECK(scripts[14].first == effect::kScriptCommandTypeDraw);
    CHECK(scripts[14].second == String("O"));
    CHECK(clearColorScriptIndex == -1);
    CHECK(clearDepthScriptIndex == -1);
    CHECK(scriptExternal);
}
