/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_COMMAND_BASEMODELOBJECTCOMMAND_H_
#define NANOEM_EMAPP_COMMAND_BASEMODELOBJECTCOMMAND_H_

#include "emapp/command/BaseUndoCommand.h"

namespace nanoem {

class Model;

namespace command {

class BaseModelObjectCommand : public BaseUndoCommand {
public:
    typedef void(destructor_t)(void *opaque, void *context);
    struct Function {
        enum Type {
            kTypeFirstEnum,
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
            kTypeBoolean = kTypeFirstEnum,
            kTypeInteger,
#else
            kTypeBoolean = kTypeFirstEnum,
            kTypeInteger = kTypeBoolean,
#endif
            kTypeFloat = kTypeFirstEnum + 2,
            kTypeFloatAtIndex,
            kTypeObject,
            kTypeObjectAtIndex,
            kTypeMaxEnum
        };
        typedef void (*set_integer_value_t)(void *opaque, int value);
        typedef void (*set_float_value_t)(void *opaque, nanoem_f32_t value);
        typedef void (*set_float_value_at_index_t)(void *opaque, nanoem_f32_t value, nanoem_rsize_t index);
        typedef void (*set_object_value_t)(void *opaque, const void *value);
        typedef void (*set_object_value_at_index_t)(void *opaque, const void *value, nanoem_rsize_t index);
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
        typedef void (*set_boolean_value_t)(void *opaque, nanoem_bool_t value);
#else
        typedef set_integer_value_t set_boolean_value_t;
#endif
        union {
            set_boolean_value_t setBoolean;
            set_integer_value_t setInteger;
            set_float_value_t setFloat;
            set_float_value_at_index_t setFloatAtIndex;
            set_object_value_t setObject;
            set_object_value_at_index_t setObjectAtIndex;
        } u;
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
        Function(set_boolean_value_t v)
            : type(kTypeBoolean)
        {
            u.setBoolean = v;
        }
#endif
        Function(set_integer_value_t v)
            : type(kTypeInteger)
        {
            u.setInteger = v;
        }
        Function(set_float_value_t v)
            : type(kTypeFloat)
        {
            u.setFloat = v;
        }
        Function(set_float_value_at_index_t v)
            : type(kTypeFloatAtIndex)
        {
            u.setFloatAtIndex = v;
        }
        Function(set_object_value_t v)
            : type(kTypeObject)
        {
            u.setObject = v;
        }
        Function(set_object_value_at_index_t v)
            : type(kTypeObjectAtIndex)
        {
            u.setObjectAtIndex = v;
        }
        const Type type;
    };
    class Object {
    public:
        static Object *create(nanoem_model_t *model, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_vertex_t *vertex, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_material_t *material, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_bone_t *bone, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_morph_t *morph, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_label_t *label, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_rigid_body_t *rigidBody, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_joint_t *joint, const Function &undoFunc, const Function &redoFunc);
        static Object *create(nanoem_model_t *model, const Function &func);
        static Object *create(nanoem_model_vertex_t *vertex, const Function &func);
        static Object *create(nanoem_model_material_t *material, const Function &func);
        static Object *create(nanoem_model_bone_t *bone, const Function &func);
        static Object *create(nanoem_model_morph_t *morph, const Function &func);
        static Object *create(nanoem_model_label_t *label, const Function &func);
        static Object *create(nanoem_model_rigid_body_t *rigidBody, const Function &func);
        static Object *create(nanoem_model_joint_t *joint, const Function &func);
        virtual ~Object()
        {
        }
        virtual Function undoFunction() const = 0;
        virtual Function redoFunction() const = 0;
        virtual void *createOpaque() = 0;
        virtual void destroyOpaque(void *opaque) = 0;
    };

    virtual ~BaseModelObjectCommand() NANOEM_DECL_NOEXCEPT;

protected:
    struct ExecutionBlock {
        union Value {
            bool booleanValue;
            int integerValue;
            nanoem_f32_t nanoem_f32_tValue;
            void *mutableObjectValue;
            const void *immutableObjectValue;
        };
        ExecutionBlock(Object *o, Model *m);
        ExecutionBlock(Object *o, Model *m, const tinystl::pair<bool, bool> &v);
        ExecutionBlock(Object *o, Model *m, const tinystl::pair<int, int> &v);
        ExecutionBlock(
            Object *o, Model *m, const tinystl::pair<nanoem_f32_t, nanoem_f32_t> &v, nanoem_rsize_t index = 0);
        ExecutionBlock(Object *o, Model *m, const tinystl::pair<void *, void *> &v, destructor_t destructor,
            void *userData, nanoem_rsize_t index = 0);
        ExecutionBlock(
            Object *o, Model *m, const tinystl::pair<const void *, const void *> &v, nanoem_rsize_t index = 0);
        ~ExecutionBlock() NANOEM_DECL_NOEXCEPT;
        void execute(const Value &v, const Function &function);
        void undo(Error &error);
        void redo(Error &error);
        Model *m_model;
        Object *m_object;
        destructor_t *m_destructor;
        void *m_userData;
        Value m_newValue;
        Value m_oldValue;
        const nanoem_rsize_t m_index;
    };
    Model *m_model;
    ExecutionBlock *m_block;
    BaseModelObjectCommand(Model *model, ExecutionBlock *block);
};

} /* namespace command */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_COMMAND_BASEMODELOBJECTCOMMAND_H_ */
