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

    enum class ParamType
    {
        Float,
        Int,
        Bool
    };

    struct EffectParameter
    {
        std::string effectName;   // Which effect this belongs to (e.g., "cas", "Clarity.fx")
        std::string name;         // Parameter name (e.g., "casSharpness")
        std::string label;        // Display label (from ui_label or name)
        ParamType type = ParamType::Float;
        float valueFloat = 0.0f;
        int valueInt = 0;
        bool valueBool = false;
        float minFloat = 0.0f;
        float maxFloat = 1.0f;
        int minInt = 0;
        int maxInt = 100;
    };

    struct OverlayState
    {
        std::vector<std::string> effectNames;
        std::string configPath;
        bool effectsEnabled = true;
        std::vector<EffectParameter> parameters;
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
