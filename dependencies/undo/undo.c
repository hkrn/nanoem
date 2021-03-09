/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include "undo.h"

#include <stdlib.h>
#include <string.h>

#ifndef UNDO_STACK_MAX_NUM
#define UNDO_STACK_MAX_NUM 64
#endif /* UNDO_STACK_MAX_NUM */

const undo_global_allocator_t *__undo_global_allocator = NULL;

UNDO_DECL_INLINE static void *
undoMalloc(undo_rsize_t size, const char *filename, int line)
{
    void *p = NULL;
    if (undo_likely(size <= UNDO_RSIZE_MAX)) {
        if (__undo_global_allocator != NULL) {
            p = __undo_global_allocator->malloc(__undo_global_allocator->opaque, size, filename, line);
        }
        else {
            p = malloc(size);
        }
    }
    return p;
}

UNDO_DECL_INLINE static void *
undoCalloc(undo_rsize_t length, undo_rsize_t size, const char *filename, int line)
{
    void *p = NULL;
    if (undo_likely(length > 0 && size > 0 && length <= (UNDO_RSIZE_MAX / size))) {
        if (__undo_global_allocator != NULL) {
            p = __undo_global_allocator->calloc(__undo_global_allocator->opaque, length, size, filename, line);
        }
        else {
            p = calloc(length, size);
        }
    }
    return p;
}

UNDO_DECL_INLINE static void
undoFree(void *ptr, const char *filename, int line)
{
    if (ptr) {
        if (__undo_global_allocator != NULL) {
            __undo_global_allocator->free(__undo_global_allocator->opaque, ptr, filename, line);
        }
        else {
            free(ptr);
        }
    }
}

/* allocation macros */
#ifdef UNDO_ENABLE_DEBUG_ALLOCATOR
#define undo_malloc(size) undoMalloc((size), __FILE__, __LINE__)
#define undo_calloc(length, size) undoCalloc((length), (size), __FILE__, __LINE__)
#define undo_free(ptr) undoFree((ptr), __FILE__, __LINE__)
#else
#define undo_malloc(size) undoMalloc((size), NULL, 0)
#define undo_calloc(length, size) undoCalloc((length), (size), NULL, 0)
#define undo_free(ptr) undoFree((ptr), NULL, 0)
#endif

struct undo_command_t {
    undo_command_on_destroy_t on_destroy;
    undo_command_on_persist_undo_t on_persist_undo;
    undo_command_on_persist_redo_t on_persist_redo;
    undo_command_on_redo_t on_redo;
    undo_command_on_undo_t on_undo;
    undo_command_can_undo_t can_undo;
    undo_command_can_redo_t can_redo;
    char *name;
    void *opaque_data;
};

struct undo_stack_t {
    undo_command_t *commands[UNDO_STACK_MAX_NUM];
    undo_bool_t is_dirty;
    int num_commands;
    int current_index;
    int saved_index;
    int soft_limit;
};

const undo_global_allocator_t *APIENTRY
undoGlobalGetCustomAllocator(void)
{
    return __undo_global_allocator;
}

void APIENTRY
undoGlobalSetCustomAllocator(const undo_global_allocator_t *allocator)
{
    if (undo_is_not_null(allocator)) {
        __undo_global_allocator = allocator;
    }
    else {
        __undo_global_allocator = NULL;
    }
}

UNDO_DECL_INLINE static void
undoUtilCopyString(char *dest, undo_rsize_t dest_size, const char *src, undo_rsize_t src_size)
{
#if defined(__STDC_LIB_EXT1__)
    strncpy_s(dest, dest_size, src, src_size);
#else
    strncpy(dest, src, src_size);
    dest[dest_size - 1] = '\0';
#endif
}

UNDO_DECL_INLINE static void
undoCommandDefaultVoidHandler(const undo_command_t *command)
{
    undo_mark_unused(command);
}

UNDO_DECL_INLINE static undo_bool_t
undoCommandDefaultPersistHandler(const undo_command_t *command)
{
    undo_mark_unused(command);
    return 1;
}

UNDO_DECL_INLINE static int
undoCommandDefaultIntHandler(const undo_command_t *command)
{
    undo_mark_unused(command);
    return 1;
}

UNDO_DECL_INLINE static undo_command_t **
undoStackGetPtr(undo_stack_t *stack, int index)
{
    int limit = undoStackGetSoftLimit(stack);
    int num_commands = stack->num_commands < limit ? stack->num_commands : limit;
    return index < num_commands ? stack->commands : NULL;
}

UNDO_DECL_INLINE static void
undoStackUpdateDirty(undo_stack_t *stack)
{
    stack->is_dirty = stack->current_index != stack->saved_index;
}

undo_stack_t * APIENTRY
undoStackCreate(void)
{
    return undoStackCreateWithSoftLimit(UNDO_STACK_MAX_NUM);
}

undo_stack_t * APIENTRY
undoStackCreateWithSoftLimit(int value)
{
    undo_stack_t *stack;
    stack = (undo_stack_t *) undo_calloc(1, sizeof(*stack));
    if (undo_is_not_null(stack)) {
        stack->soft_limit = value;
    }
    return stack;
}

void APIENTRY
undoStackPushCommand(undo_stack_t *stack, undo_command_t *command)
{
    undo_command_t **commands_in_stack, *c;
    int i, current_index, num_commands;
    if (undoStackCanPush(stack) && undo_is_not_null(command) && command->on_persist_redo(command)) {
        command->on_redo(command);
        current_index = stack->current_index;
        num_commands = stack->num_commands;
        commands_in_stack = stack->commands;
        if (current_index < num_commands) {
            for (i = num_commands - 1; i >= current_index; i--) {
                c = commands_in_stack[i];
                commands_in_stack[i] = NULL;
                undoCommandDestroy(c);
            }
            commands_in_stack[stack->current_index++] = command;
            stack->num_commands = current_index + 1;
        }
        else if (current_index == undoStackGetSoftLimit(stack)) {
            c = commands_in_stack[0];
            memmove(&commands_in_stack[0], &commands_in_stack[1], (current_index - 1) * sizeof(commands_in_stack[0]));
            commands_in_stack[current_index - 1] = command;
            undoCommandDestroy(c);
        }
        else {
            commands_in_stack[stack->current_index++] = command;
            stack->num_commands++;
        }
        undoStackUpdateDirty(stack);
    }
}

void APIENTRY
undoStackUndo(undo_stack_t *stack)
{
    undo_command_t **commands, *command;
    if (undoStackCanUndo(stack)) {
        commands = undoStackGetPtr(stack, stack->current_index - 1);
        if (commands) {
            command = commands[--stack->current_index];
            if (command->on_persist_undo(command)) {
                command->on_undo(command);
            }
            undoStackUpdateDirty(stack);
        }
    }
}

void APIENTRY
undoStackRedo(undo_stack_t *stack)
{
    undo_command_t **commands, *command;
    if (undoStackCanRedo(stack)) {
        commands = undoStackGetPtr(stack, stack->current_index);
        if (commands) {
            command = commands[stack->current_index++];
            if (command->on_persist_redo(command)) {
                command->on_redo(command);
            }
            undoStackUpdateDirty(stack);
        }
    }
}

int APIENTRY
undoStackGetOffset(const undo_stack_t *stack)
{
    return undo_is_not_null(stack) ? stack->current_index : 0;
}

void APIENTRY
undoStackSetOffset(undo_stack_t *stack, int value)
{
    int i, current_index;
    if (undo_is_not_null(stack)) {
        current_index = stack->current_index;
        if (value > current_index) {
            for (i = current_index; i <= value && undoStackCanRedo(stack); i++) {
                undoStackRedo(stack);
            }
        }
        else if (value < current_index) {
            for (i = current_index; i > value && undoStackCanUndo(stack); i--) {
                undoStackUndo(stack);
            }
        }
    }
}

void APIENTRY
undoStackClear(undo_stack_t *stack)
{
    undo_command_t *command;
    int num_commands = stack->num_commands, i;
    for (i = num_commands - 1; i >= 0; i--) {
        command = stack->commands[i];
        stack->commands[i] = NULL;
        undoCommandDestroy(command);
    }
    stack->num_commands = stack->saved_index = stack->current_index = 0;
    stack->is_dirty = 0;
}

int APIENTRY
undoStackCountCommands(const undo_stack_t *stack)
{
    return undo_is_not_null(stack) ? stack->num_commands : 0;
}

int APIENTRY
undoStackGetMaxStackSize(const undo_stack_t *stack)
{
    (void) stack;
    return undoStackGetHardLimit();
}

int APIENTRY
undoStackGetHardLimit(void)
{
    return UNDO_STACK_MAX_NUM;
}

int APIENTRY
undoStackGetSoftLimit(const undo_stack_t *stack)
{
    return undo_is_not_null(stack) ? stack->soft_limit : 0;
}

void APIENTRY
undoStackSetSoftLimit(undo_stack_t *stack, int value)
{
    undo_command_t *command;
    int actual_value, soft_limit, i;
    if (undo_is_not_null(stack) && value > 0 && value <= undoStackGetMaxStackSize(stack)) {
        soft_limit = stack->soft_limit;
        actual_value = value - 1;
        if (soft_limit > actual_value) {
            for (i = soft_limit - 1; i >= actual_value; i--) {
                command = stack->commands[i];
                stack->commands[i] = NULL;
                undoCommandDestroy(command);
            }
        }
        stack->soft_limit = actual_value;
    }
}

undo_bool_t APIENTRY
undoStackCanPush(const undo_stack_t *stack)
{
    return undo_is_not_null(stack) && stack->current_index <= undoStackGetSoftLimit(stack);
}

undo_bool_t APIENTRY
undoStackCanUndo(const undo_stack_t *stack)
{
    undo_command_t *command = NULL;
    undo_bool_t rc = 0;
    if (undo_is_not_null(stack) && stack->current_index > 0) {
        command = stack->commands[stack->current_index - 1];
        rc = command->can_undo(command) != 0;
    }
    return rc;
}

undo_bool_t APIENTRY
undoStackCanRedo(const undo_stack_t *stack)
{
    undo_command_t *command = NULL;
    undo_bool_t rc = 0;
    if (undo_is_not_null(stack) && stack->current_index < stack->num_commands) {
        command = stack->commands[stack->current_index];
        rc = command->can_redo(command) != 0;
    }
    return rc;
}

undo_bool_t APIENTRY
undoStackIsDirty(const undo_stack_t *stack)
{
    return undo_is_not_null(stack) ? stack->is_dirty : 0;
}

void APIENTRY
undoStackDestroy(undo_stack_t *stack)
{
    undo_command_t *command;
    int i;
    if (undo_is_not_null(stack)) {
        for (i = undoStackGetSoftLimit(stack) - 1; i >= 0; i--) {
            command = stack->commands[i];
            undoCommandDestroy(command);
        }
        stack->num_commands = stack->current_index = stack->saved_index = 0;
        undo_free(stack);
    }
}

undo_command_t * APIENTRY
undoCommandCreate(void)
{
    undo_command_t *command;
    command = (undo_command_t *) undo_calloc(1, sizeof(*command));
    if (undo_is_not_null(command)) {
        command->on_destroy = undoCommandDefaultVoidHandler;
        command->on_persist_undo = undoCommandDefaultPersistHandler;
        command->on_persist_redo = undoCommandDefaultPersistHandler;
        command->on_redo = undoCommandDefaultVoidHandler;
        command->on_undo = undoCommandDefaultVoidHandler;
        command->can_undo = undoCommandDefaultIntHandler;
        command->can_redo = undoCommandDefaultIntHandler;
    }
    return command;
}

undo_command_on_undo_t APIENTRY
undoCommandGetOnUndoCallback(undo_command_t *command)
{
    undo_command_on_undo_t callback = NULL;
    if (undo_is_not_null(command) && command->on_undo != undoCommandDefaultVoidHandler) {
        callback = command->on_undo;
    }
    return callback;
}

void APIENTRY
undoCommandSetOnUndoCallback(undo_command_t *command, undo_command_on_undo_t value)
{
    if (undo_is_not_null(command)) {
        command->on_undo = undo_is_not_null(value) ? value : undoCommandDefaultVoidHandler;
    }
}

undo_command_on_redo_t APIENTRY
undoCommandGetOnRedoCallback(undo_command_t *command)
{
    undo_command_on_undo_t callback = NULL;
    if (undo_is_not_null(command) && command->on_redo != undoCommandDefaultVoidHandler) {
        callback = command->on_redo;
    }
    return callback;
}

void APIENTRY
undoCommandSetOnRedoCallback(undo_command_t *command, undo_command_on_redo_t value)
{
    if (undo_is_not_null(command)) {
        command->on_redo = undo_is_not_null(value) ? value : undoCommandDefaultVoidHandler;
    }
}

undo_command_on_destroy_t APIENTRY
undoCommandGetOnDestroyCallback(undo_command_t *command)
{
    undo_command_on_destroy_t callback = NULL;
    if (undo_is_not_null(command) && command->on_destroy != undoCommandDefaultVoidHandler) {
        callback = command->on_destroy;
    }
    return callback;
}

void APIENTRY
undoCommandSetOnDestroyCallback(undo_command_t *command, undo_command_on_destroy_t value)
{
    if (undo_is_not_null(command)) {
        command->on_destroy = undo_is_not_null(value) ? value : undoCommandDefaultVoidHandler;
    }
}

undo_command_on_persist_undo_t APIENTRY
undoCommandGetOnPersistUndoCallback(undo_command_t *command)
{
    undo_command_on_persist_undo_t callback = NULL;
    if (undo_is_not_null(command) && command->on_persist_undo != undoCommandDefaultPersistHandler) {
        callback = command->on_persist_undo;
    }
    return callback;
}

void APIENTRY
undoCommandSetOnPersistUndoCallback(undo_command_t *command, undo_command_on_persist_undo_t value)
{
    if (undo_is_not_null(command)) {
        command->on_persist_undo = undo_is_not_null(value) ? value : undoCommandDefaultPersistHandler;
    }
}

undo_command_on_persist_redo_t APIENTRY
undoCommandGetOnPersistRedoCallback(undo_command_t *command)
{
    undo_command_on_persist_redo_t callback = NULL;
    if (undo_is_not_null(command) && command->on_persist_redo != undoCommandDefaultPersistHandler) {
        callback = command->on_persist_redo;
    }
    return callback;
}

void APIENTRY
undoCommandSetOnPersistRedoCallback(undo_command_t *command, undo_command_on_persist_redo_t value)
{
    if (undo_is_not_null(command)) {
        command->on_persist_redo = undo_is_not_null(value) ? value : undoCommandDefaultPersistHandler;
    }
}

undo_command_can_undo_t APIENTRY
undoCommandGetCanUndoCallback(undo_command_t *command)
{
    undo_command_can_undo_t callback = NULL;
    if (undo_is_not_null(command) && command->can_undo != undoCommandDefaultIntHandler) {
        callback = command->can_undo;
    }
    return callback;
}

void APIENTRY
undoCommandSetCanUndoCallback(undo_command_t *command, undo_command_can_undo_t value)
{
    if (undo_is_not_null(command)) {
        command->can_undo = undo_is_not_null(value) ? value : undoCommandDefaultIntHandler;
    }
}

undo_command_can_redo_t APIENTRY
undoCommandGetCanRedoCallback(undo_command_t *command)
{
    undo_command_can_redo_t callback = NULL;
    if (undo_is_not_null(command) && command->can_redo != undoCommandDefaultIntHandler) {
        callback = command->can_redo;
    }
    return callback;
}

UNDO_DECL_API void APIENTRY
undoCommandSetCanRedoCallback(undo_command_t *command, undo_command_can_redo_t value)
{
    if (undo_is_not_null(command)) {
        command->can_redo = undo_is_not_null(value) ? value : undoCommandDefaultIntHandler;
    }
}

const char * APIENTRY
undoCommandGetName(const undo_command_t *command)
{
    return undo_is_not_null(command) ? command->name : NULL;
}

void APIENTRY
undoCommandSetName(undo_command_t *command, const char *value)
{
    undo_rsize_t length, capacity;
    if (undo_is_not_null(command) && undo_is_not_null(value)) {
        length = strlen(value);
        capacity = length + 1;
        command->name = undo_malloc(capacity);
        undoUtilCopyString(command->name, capacity, value, length);
    }
}

void * APIENTRY
undoCommandGetOpaqueData(const undo_command_t *command)
{
    return undo_is_not_null(command) ? command->opaque_data : NULL;
}

void APIENTRY
undoCommandSetOpaqueData(undo_command_t *command, void *value)
{
    if (undo_is_not_null(command)) {
        command->opaque_data = value;
    }
}

void APIENTRY
undoCommandDestroy(undo_command_t *command)
{
    if (undo_is_not_null(command)) {
        command->on_destroy(command);
        if (undo_is_not_null(command->name)) {
            undo_free(command->name);
        }
        undo_free(command);
    }
}
