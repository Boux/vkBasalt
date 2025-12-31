#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"

namespace vkBasalt
{
    class BoolFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParameter& param) override
        {
            bool changed = false;

            if (ImGui::Checkbox(param.label.c_str(), &param.valueBool))
                changed = true;

            return changed;
        }

        void resetToDefault(EffectParameter& param) override
        {
            param.valueBool = param.defaultBool;
        }
    };

    REGISTER_FIELD_EDITOR(ParamType::Bool, BoolFieldEditor)

} // namespace vkBasalt
