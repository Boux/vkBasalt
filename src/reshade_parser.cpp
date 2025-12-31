#include "reshade_parser.hpp"

#include <climits>
#include <algorithm>
#include <filesystem>
#include <set>

#include "reshade/effect_parser.hpp"
#include "reshade/effect_codegen.hpp"
#include "reshade/effect_preprocessor.hpp"

#include "logger.hpp"
#include "config_serializer.hpp"

namespace vkBasalt
{
    namespace
    {
        // Helper to find annotation by name
        template<typename T>
        auto findAnnotation(const T& annotations, const std::string& name)
        {
            return std::find_if(annotations.begin(), annotations.end(),
                [&name](const auto& a) { return a.name == name; });
        }

        // Helper to check if annotation exists
        template<typename T>
        bool hasAnnotation(const T& annotations, const std::string& name)
        {
            return findAnnotation(annotations, name) != annotations.end();
        }

        // Helper to get float value from annotation (handles int->float conversion)
        template<typename T>
        float getAnnotationFloat(const T& annotation)
        {
            return annotation.type.is_floating_point()
                ? annotation.value.as_float[0]
                : static_cast<float>(annotation.value.as_int[0]);
        }

        // Helper to get int value from annotation (handles float->int conversion)
        template<typename T>
        int getAnnotationInt(const T& annotation)
        {
            return annotation.type.is_integral()
                ? annotation.value.as_int[0]
                : static_cast<int>(annotation.value.as_float[0]);
        }

        // Parse null-separated string into vector
        std::vector<std::string> parseNullSeparatedString(const std::string& str)
        {
            std::vector<std::string> items;
            size_t start = 0;

            for (size_t i = 0; i <= str.size(); i++)
            {
                bool atEnd = (i == str.size() || str[i] == '\0');
                if (!atEnd)
                    continue;

                if (i > start)
                    items.push_back(str.substr(start, i - start));
                start = i + 1;
            }

            return items;
        }

        void setupPreprocessor(reshadefx::preprocessor& pp)
        {
            pp.add_macro_definition("__RESHADE__", std::to_string(INT_MAX));
            pp.add_macro_definition("__RESHADE_PERFORMANCE_MODE__", "1");
            pp.add_macro_definition("__RENDERER__", "0x20000");
            pp.add_macro_definition("BUFFER_WIDTH", "1920");
            pp.add_macro_definition("BUFFER_HEIGHT", "1080");
            pp.add_macro_definition("BUFFER_RCP_WIDTH", "(1.0 / BUFFER_WIDTH)");
            pp.add_macro_definition("BUFFER_RCP_HEIGHT", "(1.0 / BUFFER_HEIGHT)");
            pp.add_macro_definition("BUFFER_COLOR_DEPTH", "8");

            // Add all discovered shader paths from shader manager
            ShaderManagerConfig shaderMgrConfig = ConfigSerializer::loadShaderManagerConfig();
            for (const auto& path : shaderMgrConfig.discoveredShaderPaths)
                pp.add_include_path(path);
        }

        void applyFloatRange(FloatParam& p, const auto& annotations)
        {
            auto minIt = findAnnotation(annotations, "ui_min");
            auto maxIt = findAnnotation(annotations, "ui_max");

            if (minIt != annotations.end())
                p.minValue = getAnnotationFloat(*minIt);
            if (maxIt != annotations.end())
                p.maxValue = getAnnotationFloat(*maxIt);
        }

        void applyIntRange(IntParam& p, const auto& annotations)
        {
            auto minIt = findAnnotation(annotations, "ui_min");
            auto maxIt = findAnnotation(annotations, "ui_max");

            if (minIt != annotations.end())
                p.minValue = getAnnotationInt(*minIt);
            if (maxIt != annotations.end())
                p.maxValue = getAnnotationInt(*maxIt);
        }

        std::unique_ptr<EffectParam> convertSpecConstant(
            const reshadefx::uniform_info& spec,
            const std::string& effectName,
            Config* pConfig)
        {
            // Label (common to all types)
            auto labelIt = findAnnotation(spec.annotations, "ui_label");
            std::string label = (labelIt != spec.annotations.end()) ? labelIt->value.string_data : spec.name;

            // Tooltip (common to all types)
            auto tooltipIt = findAnnotation(spec.annotations, "ui_tooltip");
            std::string tooltip = (tooltipIt != spec.annotations.end()) ? tooltipIt->value.string_data : "";

            // UI type (common to all types)
            auto typeIt = findAnnotation(spec.annotations, "ui_type");
            std::string uiType = (typeIt != spec.annotations.end()) ? typeIt->value.string_data : "";

            // Helper lambda to populate float vector parameters
            auto populateFloatVector = [&](FloatVecParam& p, uint32_t componentCount) {
                p.effectName = effectName;
                p.name = spec.name;
                p.label = label;
                p.tooltip = tooltip;
                p.uiType = uiType;
                p.componentCount = componentCount;

                auto minIt = findAnnotation(spec.annotations, "ui_min");
                auto maxIt = findAnnotation(spec.annotations, "ui_max");
                for (uint32_t c = 0; c < componentCount; c++)
                {
                    std::string suffix = "[" + std::to_string(c) + "]";
                    p.defaultValue[c] = spec.initializer_value.as_float[c];
                    p.value[c] = pConfig->getInstanceOption<float>(effectName, spec.name + suffix, p.defaultValue[c]);
                    if (minIt != spec.annotations.end())
                        p.minValue[c] = getAnnotationFloat(*minIt);
                    if (maxIt != spec.annotations.end())
                        p.maxValue[c] = getAnnotationFloat(*maxIt);
                }

                auto stepIt = findAnnotation(spec.annotations, "ui_step");
                if (stepIt != spec.annotations.end())
                    p.step = getAnnotationFloat(*stepIt);
            };

            // Helper lambda to populate int vector parameters
            auto populateIntVector = [&](IntVecParam& p, uint32_t componentCount) {
                p.effectName = effectName;
                p.name = spec.name;
                p.label = label;
                p.tooltip = tooltip;
                p.uiType = uiType;
                p.componentCount = componentCount;

                auto minIt = findAnnotation(spec.annotations, "ui_min");
                auto maxIt = findAnnotation(spec.annotations, "ui_max");
                for (uint32_t c = 0; c < componentCount; c++)
                {
                    std::string suffix = "[" + std::to_string(c) + "]";
                    p.defaultValue[c] = spec.initializer_value.as_int[c];
                    p.value[c] = pConfig->getInstanceOption<int32_t>(effectName, spec.name + suffix, p.defaultValue[c]);
                    if (minIt != spec.annotations.end())
                        p.minValue[c] = getAnnotationInt(*minIt);
                    if (maxIt != spec.annotations.end())
                        p.maxValue[c] = getAnnotationInt(*maxIt);
                }

                auto stepIt = findAnnotation(spec.annotations, "ui_step");
                if (stepIt != spec.annotations.end())
                    p.step = getAnnotationFloat(*stepIt);
            };

            // Helper lambda to populate uint vector parameters
            auto populateUintVector = [&](UintVecParam& p, uint32_t componentCount) {
                p.effectName = effectName;
                p.name = spec.name;
                p.label = label;
                p.tooltip = tooltip;
                p.uiType = uiType;
                p.componentCount = componentCount;

                auto minIt = findAnnotation(spec.annotations, "ui_min");
                auto maxIt = findAnnotation(spec.annotations, "ui_max");
                for (uint32_t c = 0; c < componentCount; c++)
                {
                    std::string suffix = "[" + std::to_string(c) + "]";
                    p.defaultValue[c] = spec.initializer_value.as_uint[c];
                    p.value[c] = pConfig->getInstanceOption<uint32_t>(effectName, spec.name + suffix, p.defaultValue[c]);
                    if (minIt != spec.annotations.end())
                        p.minValue[c] = static_cast<uint32_t>(getAnnotationInt(*minIt));
                    if (maxIt != spec.annotations.end())
                        p.maxValue[c] = static_cast<uint32_t>(getAnnotationInt(*maxIt));
                }

                auto stepIt = findAnnotation(spec.annotations, "ui_step");
                if (stepIt != spec.annotations.end())
                    p.step = getAnnotationFloat(*stepIt);
            };

            // Create appropriate subclass based on spec type
            if (spec.type.is_floating_point() && spec.type.rows >= 2 && spec.type.rows <= 4)
            {
                // float2/float3/float4 vector types
                auto p = std::make_unique<FloatVecParam>();
                populateFloatVector(*p, spec.type.rows);
                return p;
            }
            else if (spec.type.is_floating_point() && spec.type.rows == 1)
            {
                // scalar float
                auto p = std::make_unique<FloatParam>();
                p->effectName = effectName;
                p->name = spec.name;
                p->label = label;
                p->tooltip = tooltip;
                p->uiType = uiType;
                p->defaultValue = spec.initializer_value.as_float[0];
                p->value = pConfig->getInstanceOption<float>(effectName, spec.name, p->defaultValue);
                applyFloatRange(*p, spec.annotations);

                auto stepIt = findAnnotation(spec.annotations, "ui_step");
                if (stepIt != spec.annotations.end())
                    p->step = getAnnotationFloat(*stepIt);

                return p;
            }
            else if (spec.type.is_boolean())
            {
                auto p = std::make_unique<BoolParam>();
                p->effectName = effectName;
                p->name = spec.name;
                p->label = label;
                p->tooltip = tooltip;
                p->uiType = uiType;
                p->defaultValue = (spec.initializer_value.as_uint[0] != 0);
                p->value = pConfig->getInstanceOption<bool>(effectName, spec.name, p->defaultValue);
                return p;
            }
            else if (spec.type.is_integral() && spec.type.is_signed() && spec.type.rows >= 2 && spec.type.rows <= 4)
            {
                // int2/int3/int4 vector types
                auto p = std::make_unique<IntVecParam>();
                populateIntVector(*p, spec.type.rows);
                return p;
            }
            else if (spec.type.is_integral() && spec.type.is_signed() && spec.type.rows == 1)
            {
                // Scalar signed int
                auto p = std::make_unique<IntParam>();
                p->effectName = effectName;
                p->name = spec.name;
                p->label = label;
                p->tooltip = tooltip;
                p->uiType = uiType;
                p->defaultValue = spec.initializer_value.as_int[0];
                p->value = pConfig->getInstanceOption<int32_t>(effectName, spec.name, p->defaultValue);
                applyIntRange(*p, spec.annotations);

                auto stepIt = findAnnotation(spec.annotations, "ui_step");
                if (stepIt != spec.annotations.end())
                    p->step = getAnnotationFloat(*stepIt);

                auto itemsIt = findAnnotation(spec.annotations, "ui_items");
                if (itemsIt != spec.annotations.end())
                    p->items = parseNullSeparatedString(itemsIt->value.string_data);

                return p;
            }
            else if (spec.type.is_integral() && !spec.type.is_signed() && spec.type.rows >= 2 && spec.type.rows <= 4)
            {
                // uint2/uint3/uint4 vector types
                auto p = std::make_unique<UintVecParam>();
                populateUintVector(*p, spec.type.rows);
                return p;
            }
            else if (spec.type.is_integral() && !spec.type.is_signed() && spec.type.rows == 1)
            {
                // Scalar unsigned int
                auto p = std::make_unique<UintParam>();
                p->effectName = effectName;
                p->name = spec.name;
                p->label = label;
                p->tooltip = tooltip;
                p->uiType = uiType;
                p->defaultValue = spec.initializer_value.as_uint[0];
                p->value = pConfig->getInstanceOption<uint32_t>(effectName, spec.name, p->defaultValue);

                auto minIt = findAnnotation(spec.annotations, "ui_min");
                auto maxIt = findAnnotation(spec.annotations, "ui_max");
                if (minIt != spec.annotations.end())
                    p->minValue = static_cast<uint32_t>(getAnnotationInt(*minIt));
                if (maxIt != spec.annotations.end())
                    p->maxValue = static_cast<uint32_t>(getAnnotationInt(*maxIt));

                auto stepIt = findAnnotation(spec.annotations, "ui_step");
                if (stepIt != spec.annotations.end())
                    p->step = getAnnotationFloat(*stepIt);

                return p;
            }

            return nullptr;
        }

        bool shouldSkipSpecConstant(const reshadefx::uniform_info& spec)
        {
            if (spec.name.empty())
                return true;
            if (hasAnnotation(spec.annotations, "source"))
                return true;
            return false;
        }
    } // anonymous namespace

    std::vector<std::unique_ptr<EffectParam>> parseReshadeEffect(
        const std::string& effectName,
        const std::string& effectPath,
        Config* pConfig)
    {
        std::vector<std::unique_ptr<EffectParam>> params;

        // Setup preprocessor
        reshadefx::preprocessor preprocessor;
        setupPreprocessor(preprocessor);

        if (!preprocessor.append_file(effectPath))
        {
            Logger::err("reshade_parser: failed to load shader file: " + effectPath);
            return params;
        }

        std::string errors = preprocessor.errors();
        if (!errors.empty())
            Logger::err("reshade_parser preprocessor errors: " + errors);

        // Parse
        reshadefx::parser parser;
        auto codegen = std::unique_ptr<reshadefx::codegen>(
            reshadefx::create_codegen_spirv(true, true, true, true));

        if (!parser.parse(std::move(preprocessor.output()), codegen.get()))
        {
            errors = parser.errors();
            if (!errors.empty())
                Logger::err("reshade_parser parse errors: " + errors);
            return params;
        }

        errors = parser.errors();
        if (!errors.empty())
            Logger::err("reshade_parser parse errors: " + errors);

        // Extract module and convert uniforms to parameters
        reshadefx::module module;
        codegen->write_result(module);

        // Process spec_constants
        // Note: float2/float3/float4 are split into multiple scalar spec_constants with the same name
        // We need to detect and combine them
        for (size_t i = 0; i < module.spec_constants.size(); i++)
        {
            const auto& spec = module.spec_constants[i];

            if (shouldSkipSpecConstant(spec))
                continue;

            // Check if this is part of a vector (same name appears multiple times consecutively)
            size_t componentCount = 1;
            while (i + componentCount < module.spec_constants.size() &&
                   module.spec_constants[i + componentCount].name == spec.name)
            {
                componentCount++;
            }

            if (componentCount >= 2 && componentCount <= 4)
            {
                // Vector type - combine multiple scalar spec_constants with same name
                auto labelIt = findAnnotation(spec.annotations, "ui_label");
                std::string label = (labelIt != spec.annotations.end()) ? labelIt->value.string_data : spec.name;

                auto tooltipIt = findAnnotation(spec.annotations, "ui_tooltip");
                std::string tooltip = (tooltipIt != spec.annotations.end()) ? tooltipIt->value.string_data : "";

                auto typeIt = findAnnotation(spec.annotations, "ui_type");
                std::string uiType = (typeIt != spec.annotations.end()) ? typeIt->value.string_data : "";

                auto minIt = findAnnotation(spec.annotations, "ui_min");
                auto maxIt = findAnnotation(spec.annotations, "ui_max");
                auto stepIt = findAnnotation(spec.annotations, "ui_step");

                if (spec.type.is_floating_point())
                {
                    // float2/float3/float4
                    auto p = std::make_unique<FloatVecParam>();
                    p->effectName = effectName;
                    p->name = spec.name;
                    p->label = label;
                    p->tooltip = tooltip;
                    p->uiType = uiType;
                    p->componentCount = static_cast<uint32_t>(componentCount);

                    for (size_t c = 0; c < componentCount; c++)
                    {
                        std::string suffix = "[" + std::to_string(c) + "]";
                        p->defaultValue[c] = module.spec_constants[i + c].initializer_value.as_float[0];
                        p->value[c] = pConfig->getInstanceOption<float>(effectName, spec.name + suffix, p->defaultValue[c]);
                        if (minIt != spec.annotations.end())
                            p->minValue[c] = getAnnotationFloat(*minIt);
                        if (maxIt != spec.annotations.end())
                            p->maxValue[c] = getAnnotationFloat(*maxIt);
                    }
                    if (stepIt != spec.annotations.end())
                        p->step = getAnnotationFloat(*stepIt);

                    params.push_back(std::move(p));
                }
                else if (spec.type.is_integral() && spec.type.is_signed())
                {
                    // int2/int3/int4
                    auto p = std::make_unique<IntVecParam>();
                    p->effectName = effectName;
                    p->name = spec.name;
                    p->label = label;
                    p->tooltip = tooltip;
                    p->uiType = uiType;
                    p->componentCount = static_cast<uint32_t>(componentCount);

                    for (size_t c = 0; c < componentCount; c++)
                    {
                        std::string suffix = "[" + std::to_string(c) + "]";
                        p->defaultValue[c] = module.spec_constants[i + c].initializer_value.as_int[0];
                        p->value[c] = pConfig->getInstanceOption<int32_t>(effectName, spec.name + suffix, p->defaultValue[c]);
                        if (minIt != spec.annotations.end())
                            p->minValue[c] = getAnnotationInt(*minIt);
                        if (maxIt != spec.annotations.end())
                            p->maxValue[c] = getAnnotationInt(*maxIt);
                    }
                    if (stepIt != spec.annotations.end())
                        p->step = getAnnotationFloat(*stepIt);

                    params.push_back(std::move(p));
                }
                else if (spec.type.is_integral() && !spec.type.is_signed())
                {
                    // uint2/uint3/uint4
                    auto p = std::make_unique<UintVecParam>();
                    p->effectName = effectName;
                    p->name = spec.name;
                    p->label = label;
                    p->tooltip = tooltip;
                    p->uiType = uiType;
                    p->componentCount = static_cast<uint32_t>(componentCount);

                    for (size_t c = 0; c < componentCount; c++)
                    {
                        std::string suffix = "[" + std::to_string(c) + "]";
                        p->defaultValue[c] = module.spec_constants[i + c].initializer_value.as_uint[0];
                        p->value[c] = pConfig->getInstanceOption<uint32_t>(effectName, spec.name + suffix, p->defaultValue[c]);
                        if (minIt != spec.annotations.end())
                            p->minValue[c] = static_cast<uint32_t>(getAnnotationInt(*minIt));
                        if (maxIt != spec.annotations.end())
                            p->maxValue[c] = static_cast<uint32_t>(getAnnotationInt(*maxIt));
                    }
                    if (stepIt != spec.annotations.end())
                        p->step = getAnnotationFloat(*stepIt);

                    params.push_back(std::move(p));
                }

                // Skip the remaining components since we've already processed them
                i += componentCount - 1;
            }
            else
            {
                // Regular scalar parameter
                auto param = convertSpecConstant(spec, effectName, pConfig);
                if (param)
                    params.push_back(std::move(param));
            }
        }

        // Process uniforms (runtime-changeable values)
        for (const auto& uniform : module.uniforms)
        {
            if (shouldSkipSpecConstant(uniform))
                continue;

            auto param = convertSpecConstant(uniform, effectName, pConfig);
            if (param)
                params.push_back(std::move(param));
        }

        return params;
    }

    ShaderTestResult testShaderCompilation(
        const std::string& effectName,
        const std::string& effectPath)
    {
        ShaderTestResult result;
        result.effectName = effectName;
        result.filePath = effectPath;

        try
        {
            // Setup preprocessor with include paths
            reshadefx::preprocessor preprocessor;
            setupPreprocessor(preprocessor);

            // Try to load and preprocess the file
            if (!preprocessor.append_file(effectPath))
            {
                result.success = false;
                result.errorMessage = "Failed to load shader file";
                std::string ppErrors = preprocessor.errors();
                if (!ppErrors.empty())
                    result.errorMessage += ": " + ppErrors;
                return result;
            }

            // Check for preprocessor errors
            std::string ppErrors = preprocessor.errors();
            if (!ppErrors.empty())
            {
                result.success = false;
                result.errorMessage = "Preprocessor errors: " + ppErrors;
                return result;
            }

            // Try to parse the shader
            reshadefx::parser parser;
            auto codegen = std::unique_ptr<reshadefx::codegen>(
                reshadefx::create_codegen_spirv(true, true, true, true));

            if (!parser.parse(std::move(preprocessor.output()), codegen.get()))
            {
                result.success = false;
                result.errorMessage = "Parse errors: " + parser.errors();
                return result;
            }

            // Check for parse warnings/errors
            std::string parseErrors = parser.errors();
            if (!parseErrors.empty())
            {
                // Some shaders have warnings but still work
                result.success = true;
                result.errorMessage = "Warnings: " + parseErrors;
                return result;
            }

            // Try to generate code
            reshadefx::module module;
            codegen->write_result(module);

            result.success = true;
        }
        catch (const std::exception& e)
        {
            result.success = false;
            result.errorMessage = "Exception: " + std::string(e.what());
        }
        catch (...)
        {
            result.success = false;
            result.errorMessage = "Unknown exception during compilation";
        }

        return result;
    }

    // Built-in macros that should not be exposed to users
    static const std::set<std::string> builtInMacros = {
        "__RESHADE__",
        "__RESHADE_PERFORMANCE_MODE__",
        "__RENDERER__",
        "BUFFER_WIDTH",
        "BUFFER_HEIGHT",
        "BUFFER_RCP_WIDTH",
        "BUFFER_RCP_HEIGHT",
        "BUFFER_COLOR_DEPTH",
        "__FILE__",
        "__LINE__",
        "__DATE__",
        "__TIME__",
        "__VENDOR__",
        "__APPLICATION__",
        "RESHADE_DEPTH_INPUT_IS_UPSIDE_DOWN",
        "RESHADE_DEPTH_INPUT_IS_REVERSED",
        "RESHADE_DEPTH_INPUT_IS_LOGARITHMIC",
        "RESHADE_DEPTH_INPUT_X_SCALE",
        "RESHADE_DEPTH_INPUT_Y_SCALE",
        "RESHADE_DEPTH_INPUT_X_OFFSET",
        "RESHADE_DEPTH_INPUT_Y_OFFSET",
        "RESHADE_DEPTH_INPUT_X_PIXEL_OFFSET",
        "RESHADE_DEPTH_INPUT_Y_PIXEL_OFFSET",
        "RESHADE_DEPTH_LINEARIZATION_FAR_PLANE",
        "RESHADE_DEPTH_MULTIPLIER",
        "RESHADE_MIX_STAGE_DEPTH_MAP",
    };

    std::vector<PreprocessorDefinition> extractPreprocessorDefinitions(
        const std::string& effectName,
        const std::string& effectPath)
    {
        std::vector<PreprocessorDefinition> defs;

        // Setup preprocessor
        reshadefx::preprocessor preprocessor;
        setupPreprocessor(preprocessor);

        if (!preprocessor.append_file(effectPath))
        {
            Logger::err("extractPreprocessorDefinitions: failed to load shader: " + effectPath);
            return defs;
        }

        // Get all macros that were actually used in the shader
        auto usedMacros = preprocessor.used_macro_definitions();

        for (const auto& [name, value] : usedMacros)
        {
            // Skip built-in macros
            if (builtInMacros.count(name))
                continue;

            // Skip macros that start with underscore (internal/compiler)
            if (!name.empty() && name[0] == '_')
                continue;

            PreprocessorDefinition def;
            def.name = name;
            def.effectName = effectName;
            def.defaultValue = value.empty() ? "1" : value;
            def.value = def.defaultValue;
            defs.push_back(def);
        }

        if (!defs.empty())
        {
            Logger::debug("extractPreprocessorDefinitions: found " + std::to_string(defs.size()) +
                " user macros in " + effectName);
            for (const auto& def : defs)
                Logger::debug("  " + def.name + " = " + def.defaultValue);
        }

        return defs;
    }

} // namespace vkBasalt
