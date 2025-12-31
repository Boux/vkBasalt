#include "effect_params.hpp"

namespace vkBasalt
{
    namespace
    {
        std::unique_ptr<FloatParam> makeFloatParam(
            const std::string& effectName,
            const std::string& name,
            const std::string& label,
            float defaultVal,
            float minVal,
            float maxVal,
            std::shared_ptr<Config> pConfig)
        {
            auto p = std::make_unique<FloatParam>();
            p->effectName = effectName;
            p->name = name;
            p->label = label;
            p->defaultValue = defaultVal;
            p->value = pConfig->getOption<float>(name, defaultVal);
            p->minValue = minVal;
            p->maxValue = maxVal;
            return p;
        }

        std::unique_ptr<IntParam> makeIntParam(
            const std::string& effectName,
            const std::string& name,
            const std::string& label,
            int defaultVal,
            int minVal,
            int maxVal,
            std::shared_ptr<Config> pConfig)
        {
            auto p = std::make_unique<IntParam>();
            p->effectName = effectName;
            p->name = name;
            p->label = label;
            p->defaultValue = defaultVal;
            p->value = pConfig->getOption<int32_t>(name, defaultVal);
            p->minValue = minVal;
            p->maxValue = maxVal;
            return p;
        }
    }

    std::vector<std::unique_ptr<EffectParam>> collectEffectParameters(
        std::shared_ptr<Config> pConfig,
        const std::vector<std::string>& effectNames,
        const std::vector<std::shared_ptr<Effect>>& effects)
    {
        std::vector<std::unique_ptr<EffectParam>> parameters;

        for (const auto& effectName : effectNames)
        {
            if (effectName == "cas")
            {
                parameters.push_back(
                    makeFloatParam("cas", "casSharpness", "Sharpness", 0.4f, 0.0f, 1.0f, pConfig));
            }
            else if (effectName == "dls")
            {
                parameters.push_back(
                    makeFloatParam("dls", "dlsSharpness", "Sharpness", 0.5f, 0.0f, 1.0f, pConfig));
                parameters.push_back(
                    makeFloatParam("dls", "dlsDenoise", "Denoise", 0.17f, 0.0f, 1.0f, pConfig));
            }
            else if (effectName == "fxaa")
            {
                parameters.push_back(
                    makeFloatParam("fxaa", "fxaaQualitySubpix", "Quality Subpix", 0.75f, 0.0f, 1.0f, pConfig));
                parameters.push_back(
                    makeFloatParam("fxaa", "fxaaQualityEdgeThreshold", "Edge Threshold", 0.125f, 0.0f, 0.5f, pConfig));
                parameters.push_back(
                    makeFloatParam("fxaa", "fxaaQualityEdgeThresholdMin", "Edge Threshold Min", 0.0312f, 0.0f, 0.1f, pConfig));
            }
            else if (effectName == "smaa")
            {
                parameters.push_back(
                    makeFloatParam("smaa", "smaaThreshold", "Threshold", 0.05f, 0.0f, 0.5f, pConfig));
                parameters.push_back(
                    makeIntParam("smaa", "smaaMaxSearchSteps", "Max Search Steps", 32, 0, 112, pConfig));
                parameters.push_back(
                    makeIntParam("smaa", "smaaMaxSearchStepsDiag", "Max Search Steps Diag", 16, 0, 20, pConfig));
                parameters.push_back(
                    makeIntParam("smaa", "smaaCornerRounding", "Corner Rounding", 25, 0, 100, pConfig));
            }
            else if (effectName == "deband")
            {
                parameters.push_back(
                    makeFloatParam("deband", "debandAvgdiff", "Avg Diff", 3.4f, 0.0f, 255.0f, pConfig));
                parameters.push_back(
                    makeFloatParam("deband", "debandMaxdiff", "Max Diff", 6.8f, 0.0f, 255.0f, pConfig));
                parameters.push_back(
                    makeFloatParam("deband", "debandMiddiff", "Mid Diff", 3.3f, 0.0f, 255.0f, pConfig));
                parameters.push_back(
                    makeFloatParam("deband", "debandRange", "Range", 16.0f, 1.0f, 64.0f, pConfig));
                parameters.push_back(
                    makeIntParam("deband", "debandIterations", "Iterations", 4, 1, 16, pConfig));
            }
            else if (effectName == "lut")
            {
                auto p = std::make_unique<FloatParam>();
                p->effectName = "lut";
                p->name = "lutFile";
                p->label = "LUT File";
                p->value = 0;
                parameters.push_back(std::move(p));
            }
            else
            {
                // ReShade effect - get parameters from the effect itself
                for (const auto& effect : effects)
                {
                    auto params = effect->getParameters();
                    for (auto& param : params)
                    {
                        if (param->effectName == effectName)
                            parameters.push_back(std::move(param));
                    }
                }
            }
        }

        return parameters;
    }

} // namespace vkBasalt
