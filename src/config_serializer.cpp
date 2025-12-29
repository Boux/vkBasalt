#include "config_serializer.hpp"
#include "logger.hpp"

#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

namespace vkBasalt
{
    std::string ConfigSerializer::getBaseConfigDir()
    {
        const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
        if (xdgConfig)
            return std::string(xdgConfig) + "/vkBasalt";

        const char* home = std::getenv("HOME");
        if (home)
            return std::string(home) + "/.config/vkBasalt";

        return "";
    }

    std::string ConfigSerializer::getConfigsDir()
    {
        std::string baseDir = getBaseConfigDir();
        if (baseDir.empty())
            return "";
        return baseDir + "/configs";
    }

    std::vector<std::string> ConfigSerializer::listConfigs()
    {
        std::vector<std::string> configs;
        std::string dir = getConfigsDir();

        DIR* d = opendir(dir.c_str());
        if (!d)
            return configs;

        struct dirent* entry;
        while ((entry = readdir(d)) != nullptr)
        {
            std::string name = entry->d_name;
            if (name.size() > 5 && name.substr(name.size() - 5) == ".conf")
                configs.push_back(name.substr(0, name.size() - 5));
        }
        closedir(d);

        std::sort(configs.begin(), configs.end());
        return configs;
    }

    static std::string joinEffects(const std::vector<std::string>& effects)
    {
        std::string result;
        for (size_t i = 0; i < effects.size(); i++)
        {
            if (i > 0)
                result += ":";
            result += effects[i];
        }
        return result;
    }

    bool ConfigSerializer::saveConfig(
        const std::string& configName,
        const std::vector<std::string>& effects,
        const std::vector<std::string>& disabledEffects,
        const std::vector<EffectParam>& params,
        const std::map<std::string, std::string>& effectPaths)
    {
        std::string configsDir = getConfigsDir();
        if (configsDir.empty())
        {
            Logger::err("Could not determine configs directory");
            return false;
        }

        mkdir(configsDir.c_str(), 0755);

        std::string filePath = configsDir + "/" + configName + ".conf";
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            Logger::err("Could not open config file for writing: " + filePath);
            return false;
        }

        // Group params by effect
        std::map<std::string, std::vector<const EffectParam*>> paramsByEffect;
        for (const auto& param : params)
            paramsByEffect[param.effectName].push_back(&param);

        // Write params grouped by effect (always prefix with effectName.paramName)
        // Also write effect path before params for each effect
        for (const auto& [effectName, effectParams] : paramsByEffect)
        {
            file << "# " << effectName << "\n";
            // Write effect path if available (for ReShade effects)
            auto pathIt = effectPaths.find(effectName);
            if (pathIt != effectPaths.end() && !pathIt->second.empty())
                file << effectName << " = " << pathIt->second << "\n";
            for (const auto* param : effectParams)
                file << param->effectName << "." << param->paramName << " = " << param->value << "\n";
            file << "\n";
        }

        // Write paths for effects that have no params but do have paths
        for (const auto& [effectName, path] : effectPaths)
        {
            if (!path.empty() && paramsByEffect.find(effectName) == paramsByEffect.end())
            {
                file << "# " << effectName << "\n";
                file << effectName << " = " << path << "\n\n";
            }
        }

        // Write effects list (all effects, enabled + disabled)
        file << "effects = " << joinEffects(effects) << "\n";

        // Write disabled effects if any
        if (!disabledEffects.empty())
            file << "disabledEffects = " << joinEffects(disabledEffects) << "\n";

        file.close();
        Logger::info("Saved config to: " + filePath);
        return true;
    }

    bool ConfigSerializer::deleteConfig(const std::string& configName)
    {
        std::string configsDir = getConfigsDir();
        if (configsDir.empty())
            return false;

        std::string filePath = configsDir + "/" + configName + ".conf";
        if (std::remove(filePath.c_str()) == 0)
        {
            Logger::info("Deleted config: " + filePath);
            return true;
        }
        Logger::err("Failed to delete config: " + filePath);
        return false;
    }

    std::string ConfigSerializer::getDefaultConfigPath()
    {
        const char* home = std::getenv("HOME");
        if (home)
            return std::string(home) + "/.config/vkBasalt/default_config";
        return "";
    }

    bool ConfigSerializer::setDefaultConfig(const std::string& configName)
    {
        std::string path = getDefaultConfigPath();
        if (path.empty())
            return false;

        std::ofstream file(path);
        if (!file.is_open())
        {
            Logger::err("Could not write default config file: " + path);
            return false;
        }

        file << configName;
        file.close();
        Logger::info("Set default config: " + configName);
        return true;
    }

    std::string ConfigSerializer::getDefaultConfig()
    {
        std::string path = getDefaultConfigPath();
        if (path.empty())
            return "";

        std::ifstream file(path);
        if (!file.is_open())
            return "";

        std::string configName;
        std::getline(file, configName);
        return configName;
    }

} // namespace vkBasalt
