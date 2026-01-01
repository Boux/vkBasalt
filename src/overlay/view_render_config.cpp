#include "imgui_overlay.hpp"
#include "config_serializer.hpp"
#include "imgui/imgui.h"

namespace vkBasalt
{
    void ImGuiOverlay::renderRenderPassConfigWindow()
    {
        if (!settingsRenderPassInjection)
            return;

        ImGui::SetNextWindowSize(ImVec2(350, 280), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Render Pass Injection", &settingsRenderPassInjection))
        {
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Experimental Feature");
        ImGui::Separator();
        ImGui::Spacing();

        auto passes = pLogicalDevice->renderPassTracker.getPasses();
        ImGui::Text("Detected Render Passes: %zu", passes.size());
        ImGui::BeginChild("PassList", ImVec2(0, 100), true);
        if (passes.empty())
        {
            ImGui::TextDisabled("No render passes detected yet.");
        }
        else
        {
            for (const auto& pass : passes)
            {
                bool isTarget = (static_cast<int>(pass.index) == settingsInjectionPassIndex);
                if (isTarget)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                ImGui::Text("Pass %u: %ux%u%s", pass.index, pass.width, pass.height,
                    isTarget ? " <- inject after" : "");
                if (isTarget)
                    ImGui::PopStyleColor();
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();

        ImGui::Text("Apply effects after pass:");
        ImGui::SetNextItemWidth(100);
        if (ImGui::InputInt("##passIndex", &settingsInjectionPassIndex))
        {
            if (settingsInjectionPassIndex < 0)
                settingsInjectionPassIndex = 0;

            // Save to config
            VkBasaltSettings newSettings = ConfigSerializer::loadSettings();
            newSettings.injectionPassIndex = settingsInjectionPassIndex;
            ConfigSerializer::saveSettings(newSettings);
            settingsSaved = true;
        }

        // Status indicator
        ImGui::Spacing();
        ImGui::Separator();
        bool canInject = !passes.empty() && settingsInjectionPassIndex < static_cast<int>(passes.size());
        if (canInject)
        {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Status: Ready to inject");
            ImGui::TextDisabled("Effects will apply after pass %d", settingsInjectionPassIndex);
        }
        else if (passes.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Status: Waiting for passes...");
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Status: Invalid pass index");
            ImGui::TextDisabled("Max valid index: %zu", passes.size() - 1);
        }

        ImGui::End();
    }

} // namespace vkBasalt
