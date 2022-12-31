/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

using namespace nanoem::test;

TEST_CASE("null_buffer_basic", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    CHECK_FALSE(nanoemBufferCanReadLength(NULL, 0));
    CHECK_FALSE(nanoemBufferCanReadLength(NULL, 1));
    CHECK_FALSE(nanoemBufferGetDataPtr(NULL));
    CHECK(nanoemBufferGetLength(NULL) == 0);
    CHECK(nanoemBufferGetOffset(NULL) == 0);
    nanoemBufferSkip(NULL, 0, &status);
    CHECK(status == NANOEM_STATUS_ERROR_BUFFER_END);
    nanoemBufferDestroy(NULL);
}

TEST_CASE("mutable_buffer_null", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    SECTION("write_byte")
    {
        nanoemMutableBufferWriteByte(NULL, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    SECTION("write_byte_array")
    {
        nanoemMutableBufferWriteByteArray(NULL, 0, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    SECTION("write_float32")
    {
        nanoemMutableBufferWriteFloat32LittleEndian(NULL, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    SECTION("write_int16")
    {
        nanoemMutableBufferWriteInt16LittleEndian(NULL, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    SECTION("write_int16_unsigned")
    {
        nanoemMutableBufferWriteInt16LittleEndianUnsigned(NULL, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    SECTION("write_int32")
    {
        nanoemMutableBufferWriteInt32LittleEndian(NULL, 0, &status);
        CHECK(status == NANOEM_STATUS_ERROR_NULL_OBJECT);
    }
    CHECK_FALSE(nanoemMutableBufferCreateBufferObject(NULL, NULL));
    nanoemMutableBufferDestroy(NULL);
}

TEST_CASE("mutable_buffer_basic", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreateWithReservedSize(0, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoemMutableBufferWriteInt16LittleEndian(mutable_buffer, INT16_MAX, &status);
    nanoemMutableBufferWriteInt16LittleEndianUnsigned(mutable_buffer, UINT16_MAX, &status);
    nanoemMutableBufferWriteInt32LittleEndian(mutable_buffer, INT32_MAX, &status);
    nanoemMutableBufferWriteFloat32LittleEndian(mutable_buffer, 0.42f, &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
    CHECK(nanoemBufferGetLength(buffer) == 13);
    CHECK(nanoemBufferReadByte(buffer, &status) == INT8_MAX);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemBufferGetOffset(buffer) == 1);
    CHECK(nanoemBufferReadInt16LittleEndian(buffer, &status) == INT16_MAX);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemBufferGetOffset(buffer) == 3);
    CHECK(nanoemBufferReadInt16LittleEndianUnsigned(buffer, &status) == UINT16_MAX);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemBufferGetOffset(buffer) == 5);
    CHECK(nanoemBufferReadInt32LittleEndian(buffer, &status) == INT32_MAX);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemBufferGetOffset(buffer) == 9);
    nanoem_f32_t value = nanoemBufferReadFloat32LittleEndian(buffer, &status);
    CHECK(value == Approx(0.42f));
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemBufferGetOffset(buffer) == 13);
    CHECK_FALSE(nanoemBufferCanReadLength(buffer, 1));
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mutable_buffer);
}

TEST_CASE("mutable_buffer_byte_array", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreateWithReservedSize(0, &status);
    const char string[] = "This is a test.";
    nanoemMutableBufferWriteByteArray(mutable_buffer, (const nanoem_u8_t *) string, strlen(string), &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
    CHECK(nanoemBufferGetLength(buffer) == 15);
    char *actual = nanoemBufferReadBuffer(buffer, strlen(string), &status);
    CHECK_THAT((const char *) actual, Catch::Equals("This is a test."));
    nanoem_free(actual);
    CHECK(status == NANOEM_STATUS_SUCCESS);
    CHECK(nanoemBufferGetOffset(buffer) == 15);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mutable_buffer);
}

TEST_CASE("mutable_buffer_uint16_insufficient", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreateWithReservedSize(0, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
    CHECK(nanoemBufferGetLength(buffer) == 1);
    CHECK(nanoemBufferReadInt16LittleEndian(buffer, &status) == 0);
    CHECK(status == NANOEM_STATUS_ERROR_BUFFER_END);
    CHECK(nanoemBufferGetOffset(buffer) == 0);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mutable_buffer);
}

TEST_CASE("mutable_buffer_uint32_insufficient", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreateWithReservedSize(0, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
    CHECK(nanoemBufferGetLength(buffer) == 3);
    CHECK(nanoemBufferReadInt32LittleEndian(buffer, &status) == 0);
    CHECK(status == NANOEM_STATUS_ERROR_BUFFER_END);
    CHECK(nanoemBufferGetOffset(buffer) == 0);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mutable_buffer);
}

TEST_CASE("mutable_buffer_float32_insufficient", "[nanoem]")
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreateWithReservedSize(0, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoemMutableBufferWriteByte(mutable_buffer, INT8_MAX, &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
    CHECK(nanoemBufferGetLength(buffer) == 3);
    nanoem_f32_t value = nanoemBufferReadFloat32LittleEndian(buffer, &status);
    CHECK(value == Approx(0));
    CHECK(status == NANOEM_STATUS_ERROR_BUFFER_END);
    CHECK(nanoemBufferGetOffset(buffer) == 0);
    nanoemBufferDestroy(buffer);
    nanoemMutableBufferDestroy(mutable_buffer);
}
