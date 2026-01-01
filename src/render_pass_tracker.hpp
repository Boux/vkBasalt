#ifndef RENDER_PASS_TRACKER_HPP_INCLUDED
#define RENDER_PASS_TRACKER_HPP_INCLUDED

#include <vector>
#include <mutex>
#include <map>
#include "vulkan_include.hpp"

namespace vkBasalt
{
    struct FramebufferInfo
    {
        std::vector<VkImageView> attachments;
        uint32_t width;
        uint32_t height;
    };

    struct RenderPassInfo
    {
        uint32_t index;
        VkRenderPass renderPass;
        VkFramebuffer framebuffer;
        uint32_t width;
        uint32_t height;
    };

    class RenderPassTracker
    {
    public:
        void beginFrame()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_passes.clear();
            m_passIndex = 0;
            m_currentPassIndex = -1;
            m_injectionPerformed = false;
        }

        // Track acquired swapchain image index
        void setAcquiredImageIndex(VkSwapchainKHR swapchain, uint32_t imageIndex)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_acquiredImageIndex[swapchain] = imageIndex;
        }

        uint32_t getAcquiredImageIndex(VkSwapchainKHR swapchain) const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_acquiredImageIndex.find(swapchain);
            return it != m_acquiredImageIndex.end() ? it->second : 0;
        }

        // Track whether injection was performed this frame
        void setInjectionPerformed(bool performed)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_injectionPerformed = performed;
        }

        bool wasInjectionPerformed() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_injectionPerformed;
        }

        // Cached injection pass index (updated when settings change)
        void setTargetPassIndex(int index)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_targetPassIndex = index;
        }

        int getTargetPassIndex() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_targetPassIndex;
        }

        void recordPass(const VkRenderPassBeginInfo* info)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            RenderPassInfo pass;
            pass.index = m_passIndex;
            pass.renderPass = info->renderPass;
            pass.framebuffer = info->framebuffer;
            pass.width = info->renderArea.extent.width;
            pass.height = info->renderArea.extent.height;
            m_passes.push_back(pass);
            m_currentPassIndex = static_cast<int>(m_passIndex);
            m_passIndex++;
        }

        // Called when a render pass ends, returns the index of the pass that just ended
        int endPass()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            int ended = m_currentPassIndex;
            m_currentPassIndex = -1;
            return ended;
        }

        // Get the current pass info (if in a render pass)
        bool getCurrentPass(RenderPassInfo& outInfo) const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_currentPassIndex < 0 || m_currentPassIndex >= static_cast<int>(m_passes.size()))
                return false;
            outInfo = m_passes[m_currentPassIndex];
            return true;
        }

        std::vector<RenderPassInfo> getPasses() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_passes;
        }

        // Framebuffer tracking
        void registerFramebuffer(VkFramebuffer fb, const VkFramebufferCreateInfo* info)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            FramebufferInfo fbInfo;
            fbInfo.width = info->width;
            fbInfo.height = info->height;
            for (uint32_t i = 0; i < info->attachmentCount; i++)
                fbInfo.attachments.push_back(info->pAttachments[i]);
            m_framebuffers[fb] = fbInfo;
        }

        void unregisterFramebuffer(VkFramebuffer fb)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_framebuffers.erase(fb);
        }

        bool getFramebufferInfo(VkFramebuffer fb, FramebufferInfo& outInfo) const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_framebuffers.find(fb);
            if (it == m_framebuffers.end())
                return false;
            outInfo = it->second;
            return true;
        }

    private:
        mutable std::mutex m_mutex;
        std::vector<RenderPassInfo> m_passes;
        uint32_t m_passIndex = 0;
        int m_currentPassIndex = -1;  // -1 when not in a render pass
        std::map<VkFramebuffer, FramebufferInfo> m_framebuffers;
        std::map<VkSwapchainKHR, uint32_t> m_acquiredImageIndex;
        bool m_injectionPerformed = false;
        int m_targetPassIndex = 0;
    };

} // namespace vkBasalt

#endif // RENDER_PASS_TRACKER_HPP_INCLUDED
