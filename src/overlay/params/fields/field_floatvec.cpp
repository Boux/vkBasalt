#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"
#include <cmath>

namespace vkBasalt
{
    class FloatVecFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParam& param) override
        {
            auto& p = static_cast<FloatVecParam&>(param);
            bool changed = false;

            switch (p.componentCount)
            {
                case 2:
                    changed = ImGui::SliderFloat2(p.label.c_str(), p.value, p.minValue[0], p.maxValue[0]);
                    break;
                case 3:
                    changed = ImGui::SliderFloat3(p.label.c_str(), p.value, p.minValue[0], p.maxValue[0]);
                    break;
                case 4:
                    changed = ImGui::SliderFloat4(p.label.c_str(), p.value, p.minValue[0], p.maxValue[0]);
                    break;
            }

            if (changed && p.step > 0.0f)
            {
                for (uint32_t i = 0; i < p.componentCount; i++)
                    p.value[i] = std::round(p.value[i] / p.step) * p.step;
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

    REGISTER_FIELD_EDITOR(ParamType::FloatVec, FloatVecFieldEditor)

} // namespace vkBasalt
