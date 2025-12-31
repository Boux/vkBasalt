#ifndef EFFECT_PARAMS_HPP_INCLUDED
#define EFFECT_PARAMS_HPP_INCLUDED

#include <vector>
#include <string>
#include <memory>

#include "effect_param.hpp"
#include "config.hpp"
#include "effect.hpp"

namespace vkBasalt
{
    // Collects parameters from all active effects (built-in and ReShade)
    std::vector<std::unique_ptr<EffectParam>> collectEffectParameters(
        std::shared_ptr<Config> pConfig,
        const std::vector<std::string>& effectNames,
        const std::vector<std::shared_ptr<Effect>>& effects);

} // namespace vkBasalt

#endif // EFFECT_PARAMS_HPP_INCLUDED
