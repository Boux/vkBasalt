#ifndef EFFECT_LUT_HPP_INCLUDED
#define EFFECT_LUT_HPP_INCLUDED
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "vulkan_include.hpp"

#include "../effect_simple.hpp"
#include "config.hpp"

namespace vkBasalt
{
    class LutEffect : public SimpleEffect
    {
    public:
        LutEffect(LogicalDevice*       pLogicalDevice,
                  VkFormat             format,
                  VkExtent2D           imageExtent,
                  std::vector<VkImage> inputImages,
                  std::vector<VkImage> outputImages,
                  Config*              pConfig);
        ~LutEffect();
        void applyEffect(uint32_t imageIndex, VkCommandBuffer commandBuffer) override;

    private:
        VkImage               lutImage = VK_NULL_HANDLE;
        VkDeviceMemory        lutMemory = VK_NULL_HANDLE;
        VkImageView           lutImageView = VK_NULL_HANDLE;
        VkDescriptorSetLayout lutDescriptorSetLayout = VK_NULL_HANDLE;
        VkDescriptorPool      lutDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet       lutDescriptorSet = VK_NULL_HANDLE;
    };
} // namespace vkBasalt

#endif // EFFECT_LUT_HPP_INCLUDED
