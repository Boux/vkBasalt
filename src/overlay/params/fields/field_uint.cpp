#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"

namespace vkBasalt
{
    class UintFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParam& param) override
        {
            auto& p = static_cast<UintParam&>(param);
            bool changed = false;

            // Use SliderScalar for unsigned int
            if (ImGui::SliderScalar(p.label.c_str(), ImGuiDataType_U32, &p.value, &p.minValue, &p.maxValue))
            {
                if (p.step > 0.0f)
                {
                    uint32_t step = static_cast<uint32_t>(p.step);
                    if (step > 0)
                        p.value = (p.value / step) * step;
                }
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

        void resetToDefault(EffectParam& param) override
        {
            param.resetToDefault();
        }
    };

    REGISTER_FIELD_EDITOR(ParamType::Uint, UintFieldEditor)

} // namespace vkBasalt
