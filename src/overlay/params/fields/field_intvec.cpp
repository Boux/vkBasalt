#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"

namespace vkBasalt
{
    class IntVecFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParam& param) override
        {
            auto& p = static_cast<IntVecParam&>(param);
            bool changed = false;

            switch (p.componentCount)
            {
                case 2:
                    changed = ImGui::SliderInt2(p.label.c_str(), p.value, p.minValue[0], p.maxValue[0]);
                    break;
                case 3:
                    changed = ImGui::SliderInt3(p.label.c_str(), p.value, p.minValue[0], p.maxValue[0]);
                    break;
                case 4:
                    changed = ImGui::SliderInt4(p.label.c_str(), p.value, p.minValue[0], p.maxValue[0]);
                    break;
            }

            if (changed && p.step > 0.0f)
            {
                int step = static_cast<int>(p.step);
                if (step > 0)
                {
                    for (uint32_t i = 0; i < p.componentCount; i++)
                        p.value[i] = (p.value[i] / step) * step;
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

    REGISTER_FIELD_EDITOR(ParamType::IntVec, IntVecFieldEditor)

} // namespace vkBasalt
