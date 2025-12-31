#include "../field_editor.hpp"
#include "../../../imgui/imgui.h"

namespace vkBasalt
{
    class BoolFieldEditor : public FieldEditor
    {
    public:
        bool render(EffectParam& param) override
        {
            auto& p = static_cast<BoolParam&>(param);
            bool changed = false;

            if (ImGui::Checkbox(p.label.c_str(), &p.value))
                changed = true;

            return changed;
        }

        void resetToDefault(EffectParam& param) override
        {
            param.resetToDefault();
        }
    };

    REGISTER_FIELD_EDITOR(ParamType::Bool, BoolFieldEditor)

} // namespace vkBasalt
