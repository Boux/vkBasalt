#include "imgui_overlay.hpp"
#include "logger.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_vulkan.h"

namespace vkBasalt
{
    // Function loader for ImGui - uses vkBasalt's dispatch tables with fallback to proc addr
    static PFN_vkVoidFunction imguiVulkanLoader(const char* function_name, void* user_data)
    {
        LogicalDevice* device = static_cast<LogicalDevice*>(user_data);

        // Try device functions first via GetDeviceProcAddr
        PFN_vkVoidFunction func = device->vkd.GetDeviceProcAddr(device->device, function_name);
        if (func != nullptr)
            return func;

        // Fall back to instance functions via GetInstanceProcAddr
        func = device->vki.GetInstanceProcAddr(device->instance, function_name);
        if (func != nullptr)
            return func;

        Logger::warn("ImGui requested unavailable Vulkan function: " + std::string(function_name));
        return nullptr;
    }

    ImGuiOverlay::ImGuiOverlay(LogicalDevice* device, VkFormat swapchainFormat, uint32_t imageCount)
        : pLogicalDevice(device)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // Make it semi-transparent
        ImGuiStyle& style = ImGui::GetStyle();
        style.Alpha = 0.9f;
        style.WindowRounding = 5.0f;

        initVulkanBackend(swapchainFormat, imageCount);

        initialized = true;
        Logger::info("ImGui overlay initialized");
    }

    ImGuiOverlay::~ImGuiOverlay()
    {
        if (!initialized) return;

        pLogicalDevice->vkd.QueueWaitIdle(pLogicalDevice->queue);

        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();

        if (renderPass != VK_NULL_HANDLE)
            pLogicalDevice->vkd.DestroyRenderPass(pLogicalDevice->device, renderPass, nullptr);
        if (descriptorPool != VK_NULL_HANDLE)
            pLogicalDevice->vkd.DestroyDescriptorPool(pLogicalDevice->device, descriptorPool, nullptr);

        Logger::info("ImGui overlay destroyed");
    }

    void ImGuiOverlay::initVulkanBackend(VkFormat swapchainFormat, uint32_t imageCount)
    {
        // Load Vulkan functions for ImGui using our dispatch tables
        bool loaded = ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_3, imguiVulkanLoader, pLogicalDevice);
        if (!loaded)
        {
            Logger::err("Failed to load Vulkan functions for ImGui");
            return;
        }

        // Create descriptor pool for ImGui
        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 }
        };

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 100;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = poolSizes;

        pLogicalDevice->vkd.CreateDescriptorPool(pLogicalDevice->device, &poolInfo, nullptr, &descriptorPool);

        // Create render pass for ImGui
        VkAttachmentDescription attachment = {};
        attachment.format = swapchainFormat;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef = {};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        pLogicalDevice->vkd.CreateRenderPass(pLogicalDevice->device, &renderPassInfo, nullptr, &renderPass);

        // Initialize ImGui Vulkan backend
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.ApiVersion = VK_API_VERSION_1_3;
        initInfo.Instance = pLogicalDevice->instance;
        initInfo.PhysicalDevice = pLogicalDevice->physicalDevice;
        initInfo.Device = pLogicalDevice->device;
        initInfo.QueueFamily = pLogicalDevice->queueFamilyIndex;
        initInfo.Queue = pLogicalDevice->queue;
        initInfo.DescriptorPool = descriptorPool;
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = 2;
        initInfo.PipelineInfoMain.RenderPass = renderPass;

        ImGui_ImplVulkan_Init(&initInfo);

        Logger::debug("ImGui Vulkan backend initialized");
    }

} // namespace vkBasalt
