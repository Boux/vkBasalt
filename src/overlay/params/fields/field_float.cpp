#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"
#include <cmath>

namespace vkBasalt
{
    class FloatFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParameter& param) override
        {
            bool changed = false;

            if (ImGui::SliderFloat(param.label.c_str(), &param.valueFloat, param.minFloat, param.maxFloat))
            {
                if (param.step > 0.0f)
                    param.valueFloat = std::round(param.valueFloat / param.step) * param.step;
                changed = true;
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
            param.valueFloat = param.defaultFloat;
        }
    };

    REGISTER_FIELD_EDITOR(ParamType::Float, FloatFieldEditor)

} // namespace vkBasalt
