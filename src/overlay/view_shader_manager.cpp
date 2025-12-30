#include "imgui_overlay.hpp"
#include "config_serializer.hpp"
#include "logger.hpp"

#include <cstdlib>

#include "imgui/imgui.h"
#include "imgui/imfilebrowser.h"

namespace vkBasalt
{
    // Static file browser for adding directories
    static ImGui::FileBrowser dirBrowser(
        ImGuiFileBrowserFlags_SelectDirectory |
        ImGuiFileBrowserFlags_HideRegularFiles |
        ImGuiFileBrowserFlags_CloseOnEsc |
        ImGuiFileBrowserFlags_CreateNewDir);

    void ImGuiOverlay::renderShaderManagerView()
    {
        // Load config on first open
        if (!shaderMgrInitialized)
        {
            ShaderManagerConfig config = ConfigSerializer::loadShaderManagerConfig();
            shaderMgrParentDirs = config.parentDirectories;
            shaderMgrShaderPaths = config.discoveredShaderPaths;
            shaderMgrTexturePaths = config.discoveredTexturePaths;
            shaderMgrInitialized = true;
        }

        ImGui::BeginChild("ShaderMgrContent", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);

        // Parent Directories section
        ImGui::Text("Parent Directories");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add directories containing ReShade shader packs.\nThey will be scanned for Shaders/ and Textures/ subdirectories.");

        if (ImGui::Button("Browse..."))
        {
            dirBrowser.SetTitle("Select Parent Directory");
            const char* home = std::getenv("HOME");
            dirBrowser.SetPwd(home ? home : "/");
            dirBrowser.Open();
        }

        // List parent directories with remove buttons
        ImGui::BeginChild("ParentDirList", ImVec2(0, 120), true);
        int removeIdx = -1;
        for (size_t i = 0; i < shaderMgrParentDirs.size(); i++)
        {
            ImGui::PushID(static_cast<int>(i));
            if (ImGui::Button("X"))
                removeIdx = static_cast<int>(i);
            ImGui::SameLine();
            ImGui::TextUnformatted(shaderMgrParentDirs[i].c_str());
            ImGui::PopID();
        }
        if (shaderMgrParentDirs.empty())
            ImGui::TextDisabled("No directories added");
        ImGui::EndChild();

        if (removeIdx >= 0)
            shaderMgrParentDirs.erase(shaderMgrParentDirs.begin() + removeIdx);

        // Rescan button and stats
        ImGui::Spacing();
        if (ImGui::Button("Rescan All"))
        {
            // TODO: Implement directory scanning in milestone 4
            Logger::info("Shader Manager: Rescan requested (not yet implemented)");
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(%zu shader paths, %zu texture paths)",
            shaderMgrShaderPaths.size(), shaderMgrTexturePaths.size());

        ImGui::Separator();

        // Discovered Shader Paths (collapsible)
        if (ImGui::TreeNode("Discovered Shader Paths"))
        {
            if (shaderMgrShaderPaths.empty())
                ImGui::TextDisabled("None - click Rescan All");
            else
            {
                for (const auto& path : shaderMgrShaderPaths)
                    ImGui::BulletText("%s", path.c_str());
            }
            ImGui::TreePop();
        }

        // Discovered Texture Paths (collapsible)
        if (ImGui::TreeNode("Discovered Texture Paths"))
        {
            if (shaderMgrTexturePaths.empty())
                ImGui::TextDisabled("None - click Rescan All");
            else
            {
                for (const auto& path : shaderMgrTexturePaths)
                    ImGui::BulletText("%s", path.c_str());
            }
            ImGui::TreePop();
        }

        ImGui::EndChild();

        // Display file browser (must be called every frame when open)
        dirBrowser.Display();
        if (dirBrowser.HasSelected())
        {
            std::string selectedPath = dirBrowser.GetSelected().string();
            // Avoid duplicates
            bool exists = false;
            for (const auto& dir : shaderMgrParentDirs)
            {
                if (dir == selectedPath)
                {
                    exists = true;
                    break;
                }
            }
            if (!exists)
                shaderMgrParentDirs.push_back(selectedPath);
            dirBrowser.ClearSelected();
        }

        // Footer button
        if (ImGui::Button("Save"))
        {
            ShaderManagerConfig config;
            config.parentDirectories = shaderMgrParentDirs;
            config.discoveredShaderPaths = shaderMgrShaderPaths;
            config.discoveredTexturePaths = shaderMgrTexturePaths;
            ConfigSerializer::saveShaderManagerConfig(config);
        }
    }

} // namespace vkBasalt
