#ifndef EFFECT_HPP_INCLUDED
#define EFFECT_HPP_INCLUDED
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "vulkan_include.hpp"
#include "effect_param.hpp"

namespace vkBasalt
{
    class Effect
    {
    public:
        void virtual applyEffect(uint32_t imageIndex, VkCommandBuffer commandBuffer) = 0;
        void virtual updateEffect(){};
        void virtual useDepthImage(VkImageView depthImageView){};
        virtual std::vector<std::unique_ptr<EffectParam>> getParameters() const { return {}; }
        virtual ~Effect(){};

    private:
    };
} // namespace vkBasalt

#endif // EFFECT_HPP_INCLUDED
