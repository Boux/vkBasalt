#ifndef IMGUI_OVERLAY_HPP_INCLUDED
#define IMGUI_OVERLAY_HPP_INCLUDED

#include <vector>
#include <memory>
#include <string>

#include "vulkan_include.hpp"
#include "logical_device.hpp"

namespace vkBasalt
{
    class Effect;

    struct OverlayState
    {
        std::vector<std::string> effectNames;
        std::string configPath;
        bool effectsEnabled = true;
    };

    class ImGuiOverlay
    {
    public:
        ImGuiOverlay(LogicalDevice* device, VkFormat swapchainFormat, uint32_t imageCount);
        ~ImGuiOverlay();

        void toggle() { visible = !visible; }
        bool isVisible() const { return visible; }

        void updateState(const OverlayState& state) { this->state = state; }

        VkCommandBuffer recordFrame(uint32_t imageIndex, VkImageView imageView, uint32_t width, uint32_t height);

    private:
        void initVulkanBackend(VkFormat swapchainFormat, uint32_t imageCount);

        LogicalDevice* pLogicalDevice;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers;
        VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;
        uint32_t imageCount = 0;
        OverlayState state;
        bool visible = false;
        bool initialized = false;
        bool backendInitialized = false;
    };

} // namespace vkBasalt

#endif // IMGUI_OVERLAY_HPP_INCLUDED
