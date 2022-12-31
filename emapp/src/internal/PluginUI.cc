/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/PluginUI.h"

#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "emapp/src/protoc/plugin.pb-c.h"

namespace nanoem {
namespace internal {
namespace {

static nanoem_rsize_t
requiredByteSize(Nanoem__Application__Plugin__UIComponent__DataType from, nanoem_rsize_t numComponents)
{
    switch (from) {
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S8:
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__U8: {
        return sizeof(nanoem_u8_t) * numComponents;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S16:
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__U16: {
        return sizeof(nanoem_u16_t) * numComponents;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S32:
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__U32: {
        return sizeof(nanoem_u32_t) * numComponents;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__FLOAT: {
        return sizeof(nanoem_f32_t) * numComponents;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__DOUBLE: {
        return sizeof(nanoem_f64_t) * numComponents;
    }
    default:
        return ~0;
    }
}

static bool
validate(Nanoem__Application__Plugin__UIComponent__DataType from, nanoem_rsize_t numComponents,
    const ProtobufCBinaryData &value, ImGuiDataType &to)
{
    bool result = numComponents > 0 && value.len >= requiredByteSize(from, numComponents);
    switch (from) {
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S8: {
        to = ImGuiDataType_S8;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__U8: {
        to = ImGuiDataType_U8;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S16: {
        to = ImGuiDataType_S16;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__U16: {
        to = ImGuiDataType_U16;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__S32: {
        to = ImGuiDataType_S32;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__U32: {
        to = ImGuiDataType_U32;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__FLOAT: {
        to = ImGuiDataType_Float;
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__DATA_TYPE__DOUBLE: {
        to = ImGuiDataType_Double;
        break;
    }
    default:
        result = false;
        break;
    }
    return result;
}

} /* namespace anonymous */

PluginUI::PluginUI(
    IReactor *reactor, Nanoem__Application__Plugin__UIWindow *window, const char *title, float devicePixelRatio)
    : m_reactor(reactor)
    , m_window(window)
    , m_title((window && window->title) ? window->title : title)
    , m_devicePixelRatio(devicePixelRatio)
{
}

PluginUI::~PluginUI()
{
    clear();
}

void
PluginUI::draw()
{
    const nanoem_rsize_t numItems = m_window ? m_window->n_items : 0;
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        Nanoem__Application__Plugin__UIComponent *component = m_window->items[i];
        drawComponent(component);
    }
}

bool
PluginUI::reload(Error &error)
{
    ByteArray bytes;
    bool reloaded = false;
    if (m_reactor->reloadUILayout(bytes, error)) {
        if (Nanoem__Application__Plugin__UIWindow *window =
                nanoem__application__plugin__uiwindow__unpack(g_protobufc_allocator, bytes.size(), bytes.data())) {
            nanoem__application__plugin__uiwindow__free_unpacked(m_window, g_protobufc_allocator);
            m_window = window;
            reloaded = true;
        }
    }
    return reloaded;
}

void
PluginUI::clear()
{
    if (m_window) {
        nanoem__application__plugin__uiwindow__free_unpacked(m_window, g_protobufc_allocator);
        m_window = nullptr;
    }
}

const char *
PluginUI::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title;
}

void
PluginUI::drawComponent(const Nanoem__Application__Plugin__UIComponent *component)
{
    char label[Inline::kLongNameStackBufferSize];
    switch (component->type_case) {
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_LABEL: {
        const Nanoem__Application__Plugin__UIComponent__Label *label = component->label;
        if (label->has_color) {
            const ImColor color(label->color);
            ImGui::TextColored(color.Value, "%s", label->text);
        }
        else {
            ImGui::TextUnformatted(label->text);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_BUTTON: {
        const Nanoem__Application__Plugin__UIComponent__Button *button = component->button;
        StringUtils::format(label, sizeof(label), "%s##%s", button->text, button->id);
        if (ImGui::Button(label)) {
            m_reactor->handleUIComponent(button->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHECK_BOX: {
        Nanoem__Application__Plugin__UIComponent__CheckBox *checkbox = component->check_box;
        StringUtils::format(label, sizeof(label), "%s##%s", checkbox->text, checkbox->id);
        bool value = checkbox->value != 0;
        if (ImGui::Checkbox(label, &value)) {
            checkbox->value = value ? 1 : 0;
            m_reactor->handleUIComponent(checkbox->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_COMBO_BOX: {
        Nanoem__Application__Plugin__UIComponent__ComboBox *combobox = component->combo_box;
        const nanoem_rsize_t numItems = combobox->n_selectables;
        if (numItems > 0) {
            const nanoem_rsize_t index = combobox->selected_index < numItems ? combobox->selected_index : 0;
            StringUtils::format(label, sizeof(label), "%s##%s", combobox->text, combobox->id);
            if (ImGui::BeginCombo(label, combobox->selectables[index]->text)) {
                for (nanoem_rsize_t i = 0; i < numItems; i++) {
                    const Nanoem__Application__Plugin__UIComponent__Selectable *selectable = combobox->selectables[i];
                    StringUtils::format(label, sizeof(label), "%s##%s", selectable->text, selectable->id);
                    if (ImGui::Selectable(label)) {
                        combobox->selected_index = Inline::saturateInt32U(i);
                        m_reactor->handleUIComponent(combobox->id, component);
                    }
                }
                ImGui::EndCombo();
            }
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SEPARATOR: {
        ImGui::Separator();
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_RADIO_BUTTON: {
        const Nanoem__Application__Plugin__UIComponent__RadioButton *radio = component->radio_button;
        StringUtils::format(label, sizeof(label), "%s##%s", radio->text, radio->id);
        bool active = radio->active != 0;
        if (ImGui::RadioButton(label, active)) {
            m_reactor->handleUIComponent(radio->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_DRAG_SCALAR_N: {
        Nanoem__Application__Plugin__UIComponent__DragScalarN *drag = component->drag_scalar_n;
        StringUtils::format(label, sizeof(label), "%s##%s", drag->text, drag->id);
        ImGuiDataType imguiDataType;
        const ProtobufCBinaryData &min = drag->min, &max = drag->max;
        const Nanoem__Application__Plugin__UIComponent__DataType &dataType = drag->data_type;
        const nanoem_rsize_t numComponents = drag->num_components;
        ProtobufCBinaryData &value = drag->value;
        if (validate(dataType, numComponents, value, imguiDataType) &&
            (min.data && min.len >= requiredByteSize(dataType, numComponents)) &&
            (max.data && max.len >= requiredByteSize(dataType, numComponents)) &&
            ImGui::DragScalarN(label, imguiDataType, value.data, Inline::saturateInt32(numComponents), drag->speed,
                min.data, max.data, drag->format)) {
            m_reactor->handleUIComponent(drag->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_INPUT_SCALAR_N: {
        Nanoem__Application__Plugin__UIComponent__InputScalarN *input = component->input_scalar_n;
        StringUtils::format(label, sizeof(label), "%s##%s", input->text, input->id);
        ImGuiDataType imguiDataType;
        const ProtobufCBinaryData &step = input->step, &stepFast = input->step_fast;
        const Nanoem__Application__Plugin__UIComponent__DataType &dataType = input->data_type;
        const nanoem_rsize_t numComponents = input->num_components;
        ProtobufCBinaryData &value = input->value;
        if (validate(dataType, numComponents, value, imguiDataType) &&
            (step.data && step.len >= requiredByteSize(dataType, numComponents)) &&
            (stepFast.data && stepFast.len >= requiredByteSize(dataType, numComponents)) &&
            ImGui::InputScalarN(label, imguiDataType, value.data, Inline::saturateInt32(numComponents), step.data,
                stepFast.data, input->format)) {
            m_reactor->handleUIComponent(input->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SLIDER_SCALAR_N: {
        Nanoem__Application__Plugin__UIComponent__SliderScalarN *slider = component->slider_scalar_n;
        StringUtils::format(label, sizeof(label), "%s##%s", slider->text, slider->id);
        ImGuiDataType imguiDataType;
        const ProtobufCBinaryData &min = slider->min, &max = slider->max;
        const Nanoem__Application__Plugin__UIComponent__DataType &dataType = slider->data_type;
        const nanoem_rsize_t numComponents = slider->num_components;
        ProtobufCBinaryData &value = slider->value;
        if (validate(dataType, numComponents, value, imguiDataType) &&
            (min.data && min.len >= requiredByteSize(dataType, numComponents)) &&
            (max.data && max.len >= requiredByteSize(dataType, numComponents)) &&
            ImGui::SliderScalarN(label, imguiDataType, value.data, Inline::saturateInt32(numComponents), min.data,
                max.data, slider->format)) {
            m_reactor->handleUIComponent(slider->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SELECTABLE: {
        const Nanoem__Application__Plugin__UIComponent__Selectable *selectable = component->selectable;
        StringUtils::format(label, sizeof(label), "%s##%s", selectable->text, selectable->id);
        if (ImGui::Selectable(label)) {
            m_reactor->handleUIComponent(selectable->id, component);
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_SAME_LINE: {
        const Nanoem__Application__Plugin__UIComponent__SameLine *sameLine = component->same_line;
        ImGui::SameLine(sameLine->offset_from_start_x, sameLine->has_spacing_w ? sameLine->spacing_w : -1.0f);
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CHILD_WINDOW: {
        Nanoem__Application__Plugin__UIComponent__ChildWindow *window = component->child_window;
        const ImVec2 windowSize(window->has_width ? window->width * m_devicePixelRatio : 0,
            window->has_height ? window->height * m_devicePixelRatio : 0);
        ImGui::BeginChild(window->id, windowSize, window->border, window->flags);
        for (nanoem_rsize_t i = 0, numItems = window->n_components; i < numItems; i++) {
            Nanoem__Application__Plugin__UIComponent *item = window->components[i];
            drawComponent(item);
        }
        ImGui::EndChild();
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_DUMMY: {
        const Nanoem__Application__Plugin__UIComponent__ChildWindow *dummy = component->child_window;
        const ImVec2 size(dummy->has_width ? dummy->width * m_devicePixelRatio : 0,
            dummy->has_height ? dummy->height * m_devicePixelRatio : 0);
        ImGui::Dummy(size);
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_TREE: {
        Nanoem__Application__Plugin__UIComponent__Tree *tree = component->tree;
        if (ImGui::TreeNode(tree->id)) {
            for (nanoem_rsize_t i = 0, numItems = tree->n_components; i < numItems; i++) {
                Nanoem__Application__Plugin__UIComponent *item = tree->components[i];
                drawComponent(item);
            }
            ImGui::TreePop();
        }
        break;
    }
    case NANOEM__APPLICATION__PLUGIN__UICOMPONENT__TYPE_CANVAS: {
        break;
    }
    default:
        break;
    }
}

} /* namespace internal */
} /* namespace nanoem */
