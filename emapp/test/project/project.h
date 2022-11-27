/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#include "../common.h"

#include "emapp/Archiver.h"

using namespace nanoem;

namespace {

/* "Hatsune Miku" in Kanji and Katakana */
static const nanoem_u8_t kHatsuneMiku[] = { 0xe5, 0x88, 0x9d, 0xe9, 0x9f, 0xb3, 0xe3, 0x83, 0x9f, 0xe3, 0x82, 0xaf,
    0x0 };

nanoem_pragma_diagnostics_push()
nanoem_pragma_diagnostics_ignore_clang_gcc("-Wunused-function")

static String
modelPath(int index, const char *filename)
{
    String name;
    if (index > 0) {
        bx::stringPrintf(name, "Model/%s-%d/%s", kHatsuneMiku, index, filename);
    }
    else {
        bx::stringPrintf(name, "Model/%s/%s", kHatsuneMiku, filename);
    }
    return name;
}

static String
modelMotionPath(int index)
{
    String name;
    if (index > 0) {
        bx::stringPrintf(name, "Motion/%s-%d.nmd", kHatsuneMiku, index);
    }
    else {
        bx::stringPrintf(name, "Motion/%s.nmd", kHatsuneMiku);
    }
    return name;
}

static bool
validateMotion(const ByteArray &bytes, Project *project)
{
    Error error;
    Motion *motion = project->createMotion();
    motion->setFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
    bool loaded = motion->load(bytes, 0, error);
    project->destroyMotion(motion);
    return loaded;
}

static void
expectAllProjectResourcesSame(Archiver &archiver, Project *newProject)
{
    Archiver::Entry entry;
    ByteArray bytes;
    Error error;
    {
        CHECK(archiver.findEntry("Motion/Camera.nmd", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry("Motion/Light.nmd", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry("Motion/test.nmd", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry("Motion/test-1.nmd", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry("Motion/test-2.nmd", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry(modelMotionPath(0), entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry(modelMotionPath(1), entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
    {
        CHECK(archiver.findEntry(modelMotionPath(2), entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(validateMotion(bytes, newProject));
    }
}

static void
expectAllMainEffectResourcesSame(Archiver &archiver)
{
    Archiver::Entry entry;
    ByteArray bytes;
    Error error;
    {
        CHECK(archiver.findEntry("Accessory/main-1/main.fx", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 1157);
    }
    {
        CHECK(archiver.findEntry("Accessory/main-1/foo/bar/baz/config.fxsub", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 32);
    }
    {
        CHECK(archiver.findEntry("Accessory/main-1/textures/foo/bar/baz/1px.png", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 107);
    }
    {
        CHECK(archiver.findEntry("Model/main/main.fx", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 1157);
    }
    {
        CHECK(archiver.findEntry("Model/main/foo/bar/baz/config.fxsub", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 32);
    }
    {
        CHECK(archiver.findEntry("Model/main/textures/foo/bar/baz/1px.png", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 107);
    }
}

static void
expectAllOffscreenEffectResourcesSame(Archiver &archiver)
{
    Archiver::Entry entry;
    ByteArray bytes;
    Error error;
    {
        CHECK(archiver.findEntry("Accessory/offscreen-1/offscreen.fx", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 1696);
    }
    {
        CHECK(archiver.findEntry("Accessory/offscreen-1/foo/bar/baz/config.fxsub", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 32);
    }
    {
        CHECK(archiver.findEntry("Accessory/offscreen-1/textures/foo/bar/baz/1px.png", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 107);
    }
    {
        CHECK(archiver.findEntry("Model/offscreen/offscreen.fx", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 1696);
    }
    {
        CHECK(archiver.findEntry("Model/offscreen/foo/bar/baz/config.fxsub", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 32);
    }
    {
        CHECK(archiver.findEntry("Model/offscreen/textures/foo/bar/baz/1px.png", entry, error));
        CHECK(archiver.extract(entry, bytes, error));
        CHECK(bytes.size() == 107);
    }
}

static void
expectAllOffscreenEffectResourcesSame(ISeekableReader *reader)
{
    Archiver archiver(reader);
    Error error;
    REQUIRE(archiver.open(error));
    expectAllOffscreenEffectResourcesSame(archiver);
    REQUIRE(archiver.close(error));
}

nanoem_pragma_diagnostics_pop()

} /* namespace anonymous */
