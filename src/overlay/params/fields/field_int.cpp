#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"

namespace vkBasalt
{
    class IntFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParameter& param) override
        {
            bool changed = false;

            if (!param.items.empty())
            {
                // Combo box mode
                std::string itemsStr;
                for (const auto& item : param.items)
                    itemsStr += item + '\0';
                itemsStr += '\0';

                if (ImGui::Combo(param.label.c_str(), &param.valueInt, itemsStr.c_str()))
                    changed = true;
            }
            else
            {
                // Slider mode
                if (ImGui::SliderInt(param.label.c_str(), &param.valueInt, param.minInt, param.maxInt))
                {
                    if (param.step > 0.0f)
                    {
                        int step = static_cast<int>(param.step);
                        if (step > 0)
                            param.valueInt = (param.valueInt / step) * step;
                    }
                    changed = true;
                }
            }

            // Double-click to reset
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            {
                resetToDefault(param);
                changed = true;
                ImGui::ClearActiveID();
            }

            return changed;
        }

        void resetToDefault(EffectParameter& param) override
        {
            param.valueInt = param.defaultInt;
        }
    };

    REGISTER_FIELD_EDITOR(ParamType::Int, IntFieldEditor)

} // namespace vkBasalt
