#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"
#include "../../../imgui/imgui_internal.h"

namespace vkBasalt
{
    class UintVecFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParam& param) override
        {
            auto& p = static_cast<UintVecParam&>(param);
            bool changed = false;

            // Use SliderScalarN for unsigned int vectors
            if (ImGui::SliderScalarN(p.label.c_str(), ImGuiDataType_U32, p.value, p.componentCount, &p.minValue[0], &p.maxValue[0]))
            {
                if (p.step > 0.0f)
                {
                    uint32_t step = static_cast<uint32_t>(p.step);
                    if (step > 0)
                    {
                        for (uint32_t i = 0; i < p.componentCount; i++)
                            p.value[i] = (p.value[i] / step) * step;
                    }
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

    REGISTER_FIELD_EDITOR(ParamType::UintVec, UintVecFieldEditor)

} // namespace vkBasalt
