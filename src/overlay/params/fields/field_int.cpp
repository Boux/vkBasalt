#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"

namespace vkBasalt
{
    class IntFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParam& param) override
        {
            auto& p = static_cast<IntParam&>(param);
            bool changed = false;

            if (!p.items.empty())
            {
                // Combo box mode
                std::string itemsStr;
                for (const auto& item : p.items)
                    itemsStr += item + '\0';
                itemsStr += '\0';

                if (ImGui::Combo(p.label.c_str(), &p.value, itemsStr.c_str()))
                    changed = true;
            }
            else
            {
                // Slider mode
                if (ImGui::SliderInt(p.label.c_str(), &p.value, p.minValue, p.maxValue))
                {
                    if (p.step > 0.0f)
                    {
                        int step = static_cast<int>(p.step);
                        if (step > 0)
                            p.value = (p.value / step) * step;
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

        void resetToDefault(EffectParam& param) override
        {
            param.resetToDefault();
        }
    };

    REGISTER_FIELD_EDITOR(ParamType::Int, IntFieldEditor)

} // namespace vkBasalt
