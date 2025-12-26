#ifndef IMGUI_OVERLAY_HPP_INCLUDED
#define IMGUI_OVERLAY_HPP_INCLUDED

#include <vector>
#include <memory>

#include "vulkan_include.hpp"
#include "logical_device.hpp"

namespace vkBasalt
{
    class Effect;

    class ImGuiOverlay
    {
    public:
        ImGuiOverlay(LogicalDevice* device, VkFormat swapchainFormat, uint32_t imageCount);
        ~ImGuiOverlay();

        void toggle() { visible = !visible; }
        bool isVisible() const { return visible; }

    private:
        void initVulkanBackend(VkFormat swapchainFormat, uint32_t imageCount);

        LogicalDevice* pLogicalDevice;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        bool visible = false;
        bool initialized = false;
    };

} // namespace vkBasalt

#endif // IMGUI_OVERLAY_HPP_INCLUDED
