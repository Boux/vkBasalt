#include "imgui_overlay.hpp"
#include "reshade_parser.hpp"
#include "logger.hpp"

#include <filesystem>
#include <set>

#include "imgui/imgui.h"

namespace vkBasalt
{
    void ImGuiOverlay::renderShaderTestSection()
    {
        // Test All Shaders button
        ImGui::Spacing();
        if (shaderTestRunning)
        {
            // Show progress while testing
            float progress = shaderTestQueue.empty() ? 1.0f :
                static_cast<float>(shaderTestCurrentIndex) / static_cast<float>(shaderTestQueue.size());
            ImGui::ProgressBar(progress, ImVec2(-1, 0),
                ("Testing " + std::to_string(shaderTestCurrentIndex) + "/" +
                 std::to_string(shaderTestQueue.size())).c_str());

            // Process one shader per frame to avoid blocking
            if (shaderTestCurrentIndex < shaderTestQueue.size())
            {
                const auto& [name, path] = shaderTestQueue[shaderTestCurrentIndex];
                ShaderTestResult result = testShaderCompilation(name, path);
                shaderTestResults.emplace_back(result.effectName, result.filePath,
                    result.success, result.errorMessage);
                shaderTestCurrentIndex++;
            }
            else
            {
                shaderTestRunning = false;
                shaderTestComplete = true;
                Logger::info("Shader test complete: tested " +
                    std::to_string(shaderTestResults.size()) + " shaders");
            }
        }
        else
        {
            if (ImGui::Button("Test All Shaders"))
            {
                // Build test queue from all .fx files in discovered shader paths
                shaderTestQueue.clear();
                shaderTestResults.clear();
                shaderTestCurrentIndex = 0;
                shaderTestComplete = false;
                shaderTestDuplicateCount = 0;

                std::set<std::string> seenNames;
                for (const auto& shaderPath : shaderMgrShaderPaths)
                {
                    try
                    {
                        for (const auto& entry : std::filesystem::directory_iterator(shaderPath))
                        {
                            if (!entry.is_regular_file())
                                continue;
                            std::string ext = entry.path().extension().string();
                            if (ext != ".fx" && ext != ".FX")
                                continue;

                            std::string effectName = entry.path().stem().string();
                            // Skip duplicates (first occurrence wins)
                            if (seenNames.count(effectName))
                            {
                                shaderTestDuplicateCount++;
                                continue;
                            }
                            seenNames.insert(effectName);
                            shaderTestQueue.emplace_back(effectName, entry.path().string());
                        }
                    }
                    catch (const std::filesystem::filesystem_error&)
                    {
                        // Skip inaccessible directories
                    }
                }

                if (!shaderTestQueue.empty())
                    shaderTestRunning = true;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Test all .fx shaders for compilation errors");
        }

        // Show test results summary if complete
        if (shaderTestComplete && !shaderTestResults.empty())
        {
            ImGui::SameLine();
            int passCount = 0, failCount = 0;
            for (const auto& [name, path, success, error] : shaderTestResults)
            {
                if (success)
                    passCount++;
                else
                    failCount++;
            }
            if (failCount == 0)
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "All %d passed!", passCount);
            else
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "%d passed, %d failed", passCount, failCount);

            // Show duplicate warning if any were skipped
            if (shaderTestDuplicateCount > 0)
            {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "(%d duplicates skipped)", shaderTestDuplicateCount);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Shaders with the same name found in multiple paths.\nFirst occurrence was tested, others were skipped.");
            }
        }
    }

    // Render detailed test results in collapsible sections
    // Call this separately after the main shader manager content
    static void renderShaderTestResults(
        const std::vector<std::tuple<std::string, std::string, bool, std::string>>& results)
    {
        if (results.empty())
            return;

        // Count failures for header
        int failCount = 0;
        for (const auto& [name, path, success, error] : results)
        {
            if (!success)
                failCount++;
        }

        // Show failed shaders first (if any)
        if (failCount > 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.3f, 1.0f));
            bool failedOpen = ImGui::TreeNode("FailedShaders", "Failed Shaders (%d)", failCount);
            ImGui::PopStyleColor();

            if (failedOpen)
            {
                for (const auto& [name, path, success, error] : results)
                {
                    if (success)
                        continue;

                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                    // Use path as unique ID to avoid conflicts with duplicate names
                    if (ImGui::TreeNode(path.c_str(), "%s", name.c_str()))
                    {
                        ImGui::PopStyleColor();
                        ImGui::TextDisabled("Path: %s", path.c_str());
                        ImGui::TextWrapped("Error: %s", error.c_str());
                        ImGui::TreePop();
                    }
                    else
                    {
                        ImGui::PopStyleColor();
                        if (ImGui::IsItemHovered() && !error.empty())
                            ImGui::SetTooltip("%s", error.c_str());
                    }
                }
                ImGui::TreePop();
            }
        }

        // Show passed shaders
        int passCount = static_cast<int>(results.size()) - failCount;
        if (passCount > 0)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            bool passedOpen = ImGui::TreeNode("PassedShaders", "Passed Shaders (%d)", passCount);
            ImGui::PopStyleColor();

            if (passedOpen)
            {
                for (const auto& [name, path, success, error] : results)
                {
                    if (!success)
                        continue;

                    ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", name.c_str());
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", path.c_str());

                    // Show warnings if any
                    if (!error.empty())
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "(warnings)");
                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("%s", error.c_str());
                    }
                }
                ImGui::TreePop();
            }
        }
    }

} // namespace vkBasalt

// Expose renderShaderTestResults for use by view_shader_manager.cpp
namespace vkBasalt
{
    void renderShaderTestResultsUI(
        const std::vector<std::tuple<std::string, std::string, bool, std::string>>& results)
    {
        renderShaderTestResults(results);
    }
}
