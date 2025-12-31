#ifndef EFFECT_PARAM_HPP_INCLUDED
#define EFFECT_PARAM_HPP_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <utility>

namespace vkBasalt
{
    enum class ParamType
    {
        Float,
        Int,
        Bool
    };

    // Base class for effect parameters
    class EffectParam
    {
    public:
        virtual ~EffectParam() = default;

        std::string effectName;  // Which effect this belongs to (e.g., "cas", "Clarity.fx")
        std::string name;        // Parameter name (e.g., "casSharpness")
        std::string label;       // Display label (from ui_label or name)
        std::string tooltip;     // ui_tooltip - hover description
        std::string uiType;      // ui_type - "slider", "drag", "combo", etc.

        virtual ParamType getType() const = 0;
        virtual bool hasChanged() const = 0;
        virtual void resetToDefault() = 0;
        virtual std::vector<std::pair<std::string, std::string>> serialize() const = 0;
        virtual std::unique_ptr<EffectParam> clone() const = 0;
    };

    // Float parameter
    class FloatParam : public EffectParam
    {
    public:
        float value = 0.0f;
        float defaultValue = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float step = 0.0f;

        ParamType getType() const override { return ParamType::Float; }

        bool hasChanged() const override
        {
            return value != defaultValue;
        }

        void resetToDefault() override
        {
            value = defaultValue;
        }

        std::vector<std::pair<std::string, std::string>> serialize() const override
        {
            return {{"", std::to_string(value)}};
        }

        std::unique_ptr<EffectParam> clone() const override
        {
            auto p = std::make_unique<FloatParam>();
            p->effectName = effectName;
            p->name = name;
            p->label = label;
            p->tooltip = tooltip;
            p->uiType = uiType;
            p->value = value;
            p->defaultValue = defaultValue;
            p->minValue = minValue;
            p->maxValue = maxValue;
            p->step = step;
            return p;
        }
    };

    // Int parameter
    class IntParam : public EffectParam
    {
    public:
        int value = 0;
        int defaultValue = 0;
        int minValue = 0;
        int maxValue = 100;
        float step = 0.0f;
        std::vector<std::string> items;  // ui_items - combo box options

        ParamType getType() const override { return ParamType::Int; }

        bool hasChanged() const override
        {
            return value != defaultValue;
        }

        void resetToDefault() override
        {
            value = defaultValue;
        }

        std::vector<std::pair<std::string, std::string>> serialize() const override
        {
            return {{"", std::to_string(value)}};
        }

        std::unique_ptr<EffectParam> clone() const override
        {
            auto p = std::make_unique<IntParam>();
            p->effectName = effectName;
            p->name = name;
            p->label = label;
            p->tooltip = tooltip;
            p->uiType = uiType;
            p->value = value;
            p->defaultValue = defaultValue;
            p->minValue = minValue;
            p->maxValue = maxValue;
            p->step = step;
            p->items = items;
            return p;
        }
    };

    // Bool parameter
    class BoolParam : public EffectParam
    {
    public:
        bool value = false;
        bool defaultValue = false;

        ParamType getType() const override { return ParamType::Bool; }

        bool hasChanged() const override
        {
            return value != defaultValue;
        }

        void resetToDefault() override
        {
            value = defaultValue;
        }

        std::vector<std::pair<std::string, std::string>> serialize() const override
        {
            return {{"", value ? "true" : "false"}};
        }

        std::unique_ptr<EffectParam> clone() const override
        {
            auto p = std::make_unique<BoolParam>();
            p->effectName = effectName;
            p->name = name;
            p->label = label;
            p->tooltip = tooltip;
            p->uiType = uiType;
            p->value = value;
            p->defaultValue = defaultValue;
            return p;
        }
    };

    // Helper to clone a vector of params
    inline std::vector<std::unique_ptr<EffectParam>> cloneParams(const std::vector<std::unique_ptr<EffectParam>>& params)
    {
        std::vector<std::unique_ptr<EffectParam>> result;
        result.reserve(params.size());
        for (const auto& p : params)
            result.push_back(p->clone());
        return result;
    }

} // namespace vkBasalt

#endif // EFFECT_PARAM_HPP_INCLUDED
