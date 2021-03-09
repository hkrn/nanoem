/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_UI_H_
#define EMAPP_PLUGIN_SDK_UI_H_

#include "emapp/src/protoc/common.pb-c.h"
#include "emapp/src/protoc/plugin.pb-c.h"

#include <stdlib.h>
#include <string.h>

namespace nanoem {
namespace application {
namespace plugin {

static inline void
copyString(char *&dest, const char *text)
{
    size_t length = strlen(text);
    dest = new char[length + 1];
    strcpy(dest, text);
    dest[length] = 0;
}

template <typename TPrimitive>
static inline void
serializeValue(ProtobufCBinaryData &dest, const TPrimitive &value)
{
    dest.len = sizeof(value);
    dest.data = new nanoem_u8_t[sizeof(value)];
    memcpy(dest.data, &value, sizeof(value));
}

static inline Nanoem__Application__Plugin__UIComponent *
createLabel(const char *text)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_LABEL;
    auto label = item->label = new Nanoem__Application__Plugin__UIComponent__Label;
    nanoem__application__plugin__uicomponent__label__init(label);
    copyString(label->text, text);
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createButton(const char *id, const char *text)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_BUTTON;
    auto button = item->button = new Nanoem__Application__Plugin__UIComponent__Button;
    nanoem__application__plugin__uicomponent__button__init(button);
    copyString(button->id, id);
    copyString(button->text, text);
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createCheckbox(const char *id, const char *text, bool checked)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX;
    auto check = item->check_box = new Nanoem__Application__Plugin__UIComponent__CheckBox;
    nanoem__application__plugin__uicomponent__check_box__init(check);
    copyString(check->id, id);
    copyString(check->text, text);
    check->value = checked ? 1 : 0;
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createSeparator()
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SEPARATOR;
    auto separator = item->separator = new Nanoem__Application__Plugin__UIComponent__Separator;
    nanoem__application__plugin__uicomponent__separator__init(separator);
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createCombobox(const char *id, const char *const *candidates, size_t numCandididates, nanoem_u32_t selectedIndex)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_COMBO_BOX;
    auto combo = item->combo_box = new Nanoem__Application__Plugin__UIComponent__ComboBox;
    nanoem__application__plugin__uicomponent__combo_box__init(combo);
    copyString(combo->id, id);
    if (numCandididates > 0) {
        combo->n_selectables = numCandididates;
        combo->selectables = new Nanoem__Application__Plugin__UIComponent__Selectable *[numCandididates];
        for (size_t i = 0; i < numCandididates; i++) {
            auto selectable = combo->selectables[i] = new Nanoem__Application__Plugin__UIComponent__Selectable;
            nanoem__application__plugin__uicomponent__selectable__init(selectable);
            copyString(selectable->text, candidates[i]);
        }
        combo->selected_index = selectedIndex;
    }
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createRadiobox(const char *id, const char *text, bool active)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_RADIO_BUTTON;
    auto radio = item->radio_button = new Nanoem__Application__Plugin__UIComponent__RadioButton;
    nanoem__application__plugin__uicomponent__radio_button__init(radio);
    copyString(radio->id, id);
    copyString(radio->text, text);
    radio->active = active ? 1 : 0;
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createSelectable(const char *id, const char *text)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SELECTABLE;
    auto selectable = item->selectable = new Nanoem__Application__Plugin__UIComponent__Selectable;
    nanoem__application__plugin__uicomponent__selectable__init(selectable);
    copyString(selectable->id, id);
    copyString(selectable->text, text);
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createDragScalarN(const char *id, const char *text, int value, int min, int max)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_DRAG_SCALAR_N;
    auto drag = item->drag_scalar_n = new Nanoem__Application__Plugin__UIComponent__DragScalarN;
    nanoem__application__plugin__uicomponent__drag_scalar_n__init(drag);
    copyString(drag->id, id);
    copyString(drag->text, text);
    drag->data_type = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S32;
    drag->num_components = 1;
    serializeValue(drag->value, value);
    serializeValue(drag->min, min);
    serializeValue(drag->max, max);
    drag->has_min = 1;
    drag->has_max = 1;
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createInputScalarN(const char *id, const char *text, int value, int step, int stepFast)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_INPUT_SCALAR_N;
    auto input = item->input_scalar_n = new Nanoem__Application__Plugin__UIComponent__InputScalarN;
    nanoem__application__plugin__uicomponent__input_scalar_n__init(input);
    copyString(input->id, id);
    copyString(input->text, text);
    input->data_type = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S32;
    input->num_components = 1;
    serializeValue(input->value, value);
    serializeValue(input->step, step);
    serializeValue(input->step_fast, stepFast);
    input->has_step = 1;
    input->has_step_fast = 1;
    return item;
}

static inline Nanoem__Application__Plugin__UIComponent *
createSliderScalarN(const char *id, const char *text, int value, int min, int max)
{
    auto *item = new Nanoem__Application__Plugin__UIComponent;
    nanoem__application__plugin__uicomponent__init(item);
    item->type_case = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SLIDER_SCALAR_N;
    auto slider = item->slider_scalar_n = new Nanoem__Application__Plugin__UIComponent__SliderScalarN;
    nanoem__application__plugin__uicomponent__slider_scalar_n__init(slider);
    copyString(slider->id, id);
    copyString(slider->text, text);
    slider->data_type = NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S32;
    slider->num_components = 1;
    serializeValue(slider->value, value);
    serializeValue(slider->min, min);
    serializeValue(slider->max, max);
    slider->has_min = 1;
    slider->has_max = 1;
    return item;
}

static inline void
destroyComponent(Nanoem__Application__Plugin__UIComponent *component)
{
    switch (component->type_case) {
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_LABEL: {
        auto label = component->label;
        delete[] label->text;
        delete label;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_BUTTON: {
        auto button = component->button;
        delete[] button->id;
        delete[] button->text;
        delete button;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX: {
        auto checkbox = component->check_box;
        delete[] checkbox->id;
        delete[] checkbox->text;
        delete checkbox;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_COMBO_BOX: {
        auto combobox = component->combo_box;
        for (size_t i = 0, numItems = combobox->n_selectables; i < numItems; i++) {
            auto selectable = combobox->selectables[i];
            delete[] selectable->id;
            delete[] selectable->text;
            delete selectable;
        }
        delete[] combobox->selectables;
        delete combobox;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SEPARATOR: {
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_RADIO_BUTTON: {
        auto radiobox = component->radio_button;
        delete[] radiobox->id;
        delete[] radiobox->text;
        delete radiobox;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_DRAG_SCALAR_N: {
        auto drag = component->drag_scalar_n;
        delete[] drag->id;
        delete[] drag->text;
        delete[] drag->max.data;
        delete[] drag->min.data;
        delete[] drag->value.data;
        delete[] drag->format;
        delete drag;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_INPUT_SCALAR_N: {
        auto input = component->input_scalar_n;
        delete[] input->id;
        delete[] input->text;
        delete[] input->step.data;
        delete[] input->step_fast.data;
        delete[] input->value.data;
        delete[] input->format;
        delete input;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SLIDER_SCALAR_N: {
        auto slider = component->slider_scalar_n;
        delete[] slider->id;
        delete[] slider->text;
        delete[] slider->max.data;
        delete[] slider->min.data;
        delete[] slider->value.data;
        delete[] slider->format;
        delete slider;
        break;
    }
    default:
        break;
    }
    delete component;
}

} /* namespace plugin */
} /* namespace application */
} /* namespace nanoem */

#endif /* EMAPP_PLUGIN_SDK_UI_H_ */
