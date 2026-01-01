#ifndef LOGICAL_SWAPCHAIN_HPP_INCLUDED
#define LOGICAL_SWAPCHAIN_HPP_INCLUDED
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include "effects/effect.hpp"

#include "vulkan_include.hpp"

#include "logical_device.hpp"

namespace vkBasalt
{
    class Config;

    // for each swapchain, we have the Images and the other stuff we need to execute the compute shader
    struct LogicalSwapchain
    {
        LogicalDevice*                       pLogicalDevice;
        VkSwapchainCreateInfoKHR             swapchainCreateInfo;
        VkExtent2D                           imageExtent;
        VkFormat                             format;
        uint32_t                             imageCount;
        std::vector<VkImage>                 images;
        std::vector<VkImageView>             imageViews;  // for overlay rendering
        std::vector<VkImage>                 fakeImages;
        size_t                               maxEffectSlots = 0;  // Max number of effects supported
        std::vector<VkCommandBuffer>         commandBuffersEffect;
        std::vector<VkCommandBuffer>         commandBuffersNoEffect;
        std::vector<VkSemaphore>             semaphores;
        std::vector<VkSemaphore>             overlaySemaphores;
        std::vector<std::shared_ptr<Effect>> effects;
        std::shared_ptr<Effect>              defaultTransfer;
        VkDeviceMemory                       fakeImageMemory;

        // Injection support: temp images for mid-frame effect application
        std::vector<VkImage>                 injectionTempImages;
        std::vector<VkImageView>             injectionTempImageViews;
        VkDeviceMemory                       injectionTempMemory = VK_NULL_HANDLE;
        VkSwapchainKHR                       swapchainHandle = VK_NULL_HANDLE;

        // Injection effects: apply effects and copy back to fake images
        std::vector<std::shared_ptr<Effect>> injectionEffects;
        std::shared_ptr<Effect>              injectionCopyBack;  // Copy temp → fake

        void destroy();
        void reloadEffects(Config* pConfig);
    };
} // namespace vkBasalt

#endif // LOGICAL_SWAPCHAIN_HPP_INCLUDED
