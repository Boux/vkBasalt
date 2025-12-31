#include "field_editor.hpp"
#include "../../imgui/imgui.h"

namespace vkBasalt
{
    FieldEditorFactory& FieldEditorFactory::instance()
    {
        static FieldEditorFactory factory;
        return factory;
    }

    void FieldEditorFactory::registerEditor(ParamType type, CreatorFunc creator)
    {
        creators[type] = creator;
    }

    FieldEditor* FieldEditorFactory::getEditor(ParamType type)
    {
        // Lazy initialization - create editor on first use
        auto it = editors.find(type);
        if (it != editors.end())
            return it->second.get();

        auto creatorIt = creators.find(type);
        if (creatorIt == creators.end())
            return nullptr;

        editors[type] = creatorIt->second();
        return editors[type].get();
    }

    bool renderFieldEditor(EffectParam& param)
    {
        FieldEditor* editor = FieldEditorFactory::instance().getEditor(param.getType());
        if (!editor)
            return false;

        bool changed = editor->render(param);

        // Show tooltip if present
        if (!param.tooltip.empty() && ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", param.tooltip.c_str());

        return changed;
    }

} // namespace vkBasalt
