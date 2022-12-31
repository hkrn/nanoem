/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is licensed under MIT license. for more details, see LICENSE.txt.
 */

#include <stdio.h>
#include <stdlib.h>

#include "greatest/greatest.h"
#include "undo.h"

typedef struct undo_command_state_t undo_command_state_t;
struct undo_command_state_t {
    int index;
    int *count_ptr;
};

static void
undoTestHandleOnUndo(const undo_command_t *command)
{
    undo_command_state_t *p = (undo_command_state_t *) undoCommandGetOpaqueData(command);
    fprintf(stderr, "on_undo: %d:%d:%s\n", p->index, (*p->count_ptr)--, undoCommandGetName(command));
}

static void
undoTestHandleOnRedo(const undo_command_t *command)
{
    undo_command_state_t *p = (undo_command_state_t *) undoCommandGetOpaqueData(command);
    fprintf(stderr, "on_redo: %d:%d:%s\n", p->index, (*p->count_ptr)++, undoCommandGetName(command));
}

static void
undoTestHandleOnDestroy(const undo_command_t *command)
{
    undo_command_state_t *p = (undo_command_state_t *) undoCommandGetOpaqueData(command);
    fprintf(stderr, "on_destroy: %d:%d:%s\n",  p->index, --(*p->count_ptr), undoCommandGetName(command));
    free(p);
}

static void
undoTestHandleOnDestroyOldest(const undo_command_t *command)
{
    int *p = (int *) undoCommandGetOpaqueData(command);
    *p += 1;
}

static void
undoTestFillCommands(undo_stack_t *stack, int *destroyed_count_ptr)
{
    undo_command_t *command = undoCommandCreate();
    int max_stack_size = undoStackGetMaxStackSize(stack), i;
    undoCommandSetOnDestroyCallback(command, undoTestHandleOnDestroyOldest);
    undoCommandSetOpaqueData(command, destroyed_count_ptr);
    undoStackPushCommand(stack, command);
    for (i = 1; i < max_stack_size; i++) {
        command = undoCommandCreate();
        undoCommandSetOnDestroyCallback(command, undoTestHandleOnDestroyOldest);
        undoCommandSetOpaqueData(command, destroyed_count_ptr);
        undoStackPushCommand(stack, command);
    }
}

GREATEST_TEST
undoTestNullStack(void)
{
    GREATEST_ASSERT_FALSE(undoStackCanPush(NULL));
    GREATEST_ASSERT_FALSE(undoStackCanUndo(NULL));
    GREATEST_ASSERT_FALSE(undoStackCanRedo(NULL));
    GREATEST_ASSERT_FALSE(undoStackIsDirty(NULL));
    GREATEST_ASSERT_EQ(0, undoStackCountCommands(NULL));
    undoStackPushCommand(NULL, NULL);
    undoStackUndo(NULL);
    undoStackRedo(NULL);
    undoStackRedo(NULL);
    undoStackDestroy(NULL);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestNullCommand(void)
{
    GREATEST_ASSERT_EQ(NULL, undoCommandGetOpaqueData(NULL));
    GREATEST_ASSERT_EQ(NULL, undoCommandGetName(NULL));
    undoCommandSetName(NULL, NULL);
    undoCommandSetOnDestroyCallback(NULL, NULL);
    undoCommandSetOnUndoCallback(NULL, NULL);
    undoCommandSetOnRedoCallback(NULL, NULL);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestPushNullCommand(void)
{
    undo_stack_t *stack = undoStackCreate();
    undoStackPushCommand(stack, NULL);
    GREATEST_ASSERT_EQ(0, undoStackCountCommands(stack));
    undoStackDestroy(stack);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestSetNullCommandName(void)
{
    undo_command_t *command = undoCommandCreate();
    undoCommandSetName(command, NULL);
    GREATEST_ASSERT_EQ(NULL, undoCommandGetName(command));
    undoCommandDestroy(command);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestSetNullCommandCallback(void)
{
    undo_command_t *command = undoCommandCreate();
    undoCommandSetOnDestroyCallback(command, NULL);
    undoCommandSetOnUndoCallback(command, NULL);
    undoCommandSetOnRedoCallback(command, NULL);
    undoCommandDestroy(command);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestUndoRedo(void)
{
    undo_stack_t *stack = undoStackCreate();
    undo_command_t *command = undoCommandCreate();
    undoStackPushCommand(stack, command);
    GREATEST_ASSERT_EQ(1, undoStackCountCommands(stack));
    GREATEST_ASSERT_EQ(undo_true,  undoStackIsDirty(stack));
    GREATEST_ASSERT_EQ(undo_true,  undoStackCanUndo(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackCanRedo(stack));
    undoStackUndo(stack);
    GREATEST_ASSERT_EQ(undo_false, undoStackIsDirty(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackCanUndo(stack));
    GREATEST_ASSERT_EQ(undo_true,  undoStackCanRedo(stack));
    undoStackRedo(stack);
    GREATEST_ASSERT_EQ(undo_true,  undoStackIsDirty(stack));
    GREATEST_ASSERT_EQ(undo_true,  undoStackCanUndo(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackCanRedo(stack));
    undoStackDestroy(stack);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestClearUndo(void)
{
    undo_stack_t *stack = undoStackCreate();
    undoStackPushCommand(stack, undoCommandCreate());
    undoStackPushCommand(stack, undoCommandCreate());
    undoStackPushCommand(stack, undoCommandCreate());
    GREATEST_ASSERT_EQ(3, undoStackCountCommands(stack));
    undoStackUndo(stack);
    undoStackUndo(stack);
    undoStackUndo(stack);
    undoStackPushCommand(stack, undoCommandCreate());
    GREATEST_ASSERT_EQ(1, undoStackCountCommands(stack));
    undoStackDestroy(stack);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestDeleteOldestCommandAtFill(void)
{
    undo_stack_t *stack = undoStackCreate();
    undo_command_t *command;
    int destroyed_count = 0;
    undoTestFillCommands(stack, &destroyed_count);
    command = undoCommandCreate();
    undoCommandSetOnDestroyCallback(command, undoTestHandleOnDestroyOldest);
    undoCommandSetOpaqueData(command, &destroyed_count);
    undoStackPushCommand(stack, command);
    GREATEST_ASSERT_EQ(1, destroyed_count);
    GREATEST_ASSERT_EQ(undoStackGetMaxStackSize(stack), undoStackCountCommands(stack));
    undoStackDestroy(stack);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestClearStack(void)
{
    undo_stack_t *stack = undoStackCreate();
    int destroyed_count = 0;
    undoTestFillCommands(stack, &destroyed_count);
    undoStackClear(stack);
    GREATEST_ASSERT_EQ(undoStackGetMaxStackSize(stack), destroyed_count);
    GREATEST_ASSERT_EQ(0, undoStackCountCommands(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackCanUndo(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackCanRedo(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackIsDirty(stack));
    undoStackDestroy(stack);
    GREATEST_PASS();
}

GREATEST_TEST
undoTestPushPop(void)
{
    undo_stack_t *stack = undoStackCreate();
    undo_command_t *command;
    int destroyed_count = 0;
    undoStackPushCommand(stack, undoCommandCreate());
    command = undoCommandCreate();
    undoCommandSetOnDestroyCallback(command, undoTestHandleOnDestroyOldest);
    undoCommandSetOpaqueData(command, &destroyed_count);
    undoStackPushCommand(stack, command);
    undoStackUndo(stack);
    undoStackPushCommand(stack, undoCommandCreate());
    GREATEST_ASSERT_EQ(2, undoStackCountCommands(stack));
    GREATEST_ASSERT_EQ(1, destroyed_count);
    undoStackUndo(stack);
    undoStackUndo(stack);
    GREATEST_ASSERT_EQ(2, undoStackCountCommands(stack));
    GREATEST_ASSERT_EQ(undo_true, undoStackCanRedo(stack));
    GREATEST_ASSERT_EQ(undo_false, undoStackCanUndo(stack));
    undoStackRedo(stack);
    undoStackRedo(stack);
    GREATEST_ASSERT_EQ(undo_false, undoStackCanRedo(stack));
    GREATEST_ASSERT_EQ(undo_true, undoStackCanUndo(stack));
    undoStackDestroy(stack);
    GREATEST_PASS();
}

GREATEST_SUITE(undo)
{
    GREATEST_RUN_TEST(undoTestNullStack);
    GREATEST_RUN_TEST(undoTestNullCommand);
    GREATEST_RUN_TEST(undoTestPushNullCommand);
    GREATEST_RUN_TEST(undoTestSetNullCommandName);
    GREATEST_RUN_TEST(undoTestSetNullCommandCallback);
    GREATEST_RUN_TEST(undoTestUndoRedo);
    GREATEST_RUN_TEST(undoTestClearUndo);
    GREATEST_RUN_TEST(undoTestDeleteOldestCommandAtFill);
    GREATEST_RUN_TEST(undoTestClearStack);
    GREATEST_RUN_TEST(undoTestPushPop);
}

GREATEST_MAIN_DEFS();

int
main(int argc, char *argv[])
{
    undo_command_state_t *state;
    undo_stack_t *stack;
    undo_command_t *command;
    int count = 0, i;
    undo_mark_unused(argc);
    undo_mark_unused(argv);
    stack = undoStackCreate();
    for (i = 0; i < 64; i++) {
        state = (undo_command_state_t *) calloc(1, sizeof(*state));
        state->count_ptr = &count;
        state->index = i;
        command = undoCommandCreate();
        undoCommandSetName(command, "This is a test.");
        undoCommandSetOnDestroyCallback(command, undoTestHandleOnDestroy);
        undoCommandSetOnUndoCallback(command, undoTestHandleOnUndo);
        undoCommandSetOnRedoCallback(command, undoTestHandleOnRedo);
        undoCommandSetOpaqueData(command, state);
        undoStackPushCommand(stack, command);
    }
    undoStackUndo(stack);
    undoStackUndo(stack);
    undoStackRedo(stack);
    undoStackRedo(stack);
    undoStackDestroy(stack);
    GREATEST_MAIN_BEGIN();
    GREATEST_RUN_SUITE(undo);
    GREATEST_MAIN_END();
}
