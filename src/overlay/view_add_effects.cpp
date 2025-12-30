#include "imgui_overlay.hpp"
#include "effect_registry.hpp"

#include <algorithm>

#include "imgui/imgui.h"

namespace vkBasalt
{
    void ImGuiOverlay::renderAddEffectsView()
    {
        // Add Effects mode - two column layout
        if (insertPosition >= 0)
            ImGui::Text("Insert Effects at position %d (max %zu)", insertPosition, maxEffects);
        else
            ImGui::Text("Add Effects (max %zu)", maxEffects);
        ImGui::Separator();

        size_t currentCount = selectedEffects.size();
        size_t pendingCount = pendingAddEffects.size();
        size_t totalCount = currentCount + pendingCount;

        // Built-in effects
        std::vector<std::string> builtinEffects = {"cas", "dls", "fxaa", "smaa", "deband", "lut"};

        // Helper to check if instance name is used
        auto isNameUsed = [&](const std::string& name) {
            if (std::find(selectedEffects.begin(), selectedEffects.end(), name) != selectedEffects.end())
                return true;
            for (const auto& p : pendingAddEffects)
                if (p.first == name)
                    return true;
            return false;
        };

        // Helper to get next instance name for an effect type
        auto getNextInstanceName = [&](const std::string& effectType) -> std::string {
            if (!isNameUsed(effectType))
                return effectType;
            for (int n = 2; n <= 99; n++)
            {
                std::string candidate = effectType + "." + std::to_string(n);
                if (!isNameUsed(candidate))
                    return candidate;
            }
            return effectType + ".99";
        };

        // Helper to render add button for an effect
        auto renderAddButton = [&](const std::string& effectType, const std::string& tooltip = "") {
            bool atLimit = totalCount >= maxEffects;
            if (atLimit)
                ImGui::BeginDisabled();

            if (ImGui::Button(effectType.c_str(), ImVec2(-1, 0)))
            {
                std::string instanceName = getNextInstanceName(effectType);
                pendingAddEffects.push_back({instanceName, effectType});
            }

            // Show tooltip with shader path on hover
            if (!tooltip.empty() && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip.c_str());

            if (atLimit)
                ImGui::EndDisabled();
        };

        // Two column layout
        float footerHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
        float contentHeight = -footerHeight;
        float columnWidth = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f;

        // Left column: Available effects
        ImGui::BeginChild("EffectList", ImVec2(columnWidth, contentHeight), true);
        ImGui::Text("Available:");
        ImGui::Separator();

        // Sort effects for each category
        std::vector<std::string> sortedCurrentConfig = state.currentConfigEffects;
        std::vector<std::string> sortedDefaultConfig = state.defaultConfigEffects;
        std::sort(sortedCurrentConfig.begin(), sortedCurrentConfig.end());
        std::sort(sortedDefaultConfig.begin(), sortedDefaultConfig.end());

        // Built-in effects
        ImGui::Text("Built-in:");
        for (const auto& effectType : builtinEffects)
            renderAddButton(effectType);

        // ReShade effects from current config
        if (!sortedCurrentConfig.empty())
        {
            ImGui::Separator();
            ImGui::Text("ReShade (%s):", state.configName.c_str());
            for (const auto& effectType : sortedCurrentConfig)
            {
                auto it = state.effectPaths.find(effectType);
                std::string path = (it != state.effectPaths.end()) ? it->second : "";
                renderAddButton(effectType, path);
            }
        }

        // ReShade effects from default config
        if (!sortedDefaultConfig.empty())
        {
            ImGui::Separator();
            ImGui::Text("ReShade (all):");
            for (const auto& effectType : sortedDefaultConfig)
            {
                auto it = state.effectPaths.find(effectType);
                std::string path = (it != state.effectPaths.end()) ? it->second : "";
                renderAddButton(effectType, path);
            }
        }

        ImGui::EndChild();

        ImGui::SameLine();

        // Right column: Pending effects
        ImGui::BeginChild("PendingList", ImVec2(columnWidth, contentHeight), true);
        ImGui::Text("Will add (%zu):", pendingCount);
        ImGui::Separator();

        for (size_t i = 0; i < pendingAddEffects.size(); i++)
        {
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::SmallButton("x"))
            {
                pendingAddEffects.erase(pendingAddEffects.begin() + i);
                ImGui::PopID();
                continue;
            }
            ImGui::SameLine();
            // Show instanceName (effectType) if they differ
            const auto& [instanceName, effectType] = pendingAddEffects[i];
            if (instanceName != effectType)
                ImGui::Text("%s (%s)", instanceName.c_str(), effectType.c_str());
            else
                ImGui::Text("%s", instanceName.c_str());
            ImGui::PopID();
        }

        if (pendingAddEffects.empty())
            ImGui::TextDisabled("Click effects to add...");

        ImGui::EndChild();

        ImGui::Separator();

        if (ImGui::Button("Done"))
        {
            // Apply pending effects - insert at position or append
            int pos = (insertPosition >= 0 && insertPosition <= static_cast<int>(selectedEffects.size()))
                      ? insertPosition : static_cast<int>(selectedEffects.size());
            for (const auto& [instanceName, effectType] : pendingAddEffects)
            {
                selectedEffects.insert(selectedEffects.begin() + pos, instanceName);
                pos++;  // Insert subsequent effects after the previous one
                if (pEffectRegistry)
                {
                    pEffectRegistry->ensureEffect(instanceName, effectType);
                    pEffectRegistry->setEffectEnabled(instanceName, true);
                }
            }
            if (!pendingAddEffects.empty())
            {
                applyRequested = true;
                saveToPersistentState();
            }
            pendingAddEffects.clear();
            insertPosition = -1;
            inSelectionMode = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            pendingAddEffects.clear();
            insertPosition = -1;
            inSelectionMode = false;
        }
    }

} // namespace vkBasalt
