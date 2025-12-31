#ifndef FIELD_EDITOR_HPP_INCLUDED
#define FIELD_EDITOR_HPP_INCLUDED

#include "../imgui_overlay.hpp"
#include <memory>
#include <map>
#include <functional>

namespace vkBasalt
{
    // Base class for field editors
    // Each field type (float, int, bool, float2, etc.) has its own editor
    class FieldEditor
    {
    public:
        virtual ~FieldEditor() = default;

        // Render the field UI, returns true if value changed
        virtual bool render(EffectParameter& param) = 0;

        // Reset parameter to default value
        virtual void resetToDefault(EffectParameter& param) = 0;
    };

    // Factory for creating field editors
    class FieldEditorFactory
    {
    public:
        using CreatorFunc = std::function<std::unique_ptr<FieldEditor>()>;

        static FieldEditorFactory& instance();

        // Register a field editor for a param type
        void registerEditor(ParamType type, CreatorFunc creator);

        // Get editor for a param type (returns nullptr if not found)
        FieldEditor* getEditor(ParamType type);

    private:
        FieldEditorFactory() = default;
        std::map<ParamType, std::unique_ptr<FieldEditor>> editors;
        std::map<ParamType, CreatorFunc> creators;
    };

    // Helper macro for auto-registration
    #define REGISTER_FIELD_EDITOR(ParamTypeValue, EditorClass) \
        namespace { \
            static bool _registered_##EditorClass = []() { \
                FieldEditorFactory::instance().registerEditor( \
                    ParamTypeValue, \
                    []() { return std::make_unique<EditorClass>(); }); \
                return true; \
            }(); \
        }

    // Main entry point - renders the appropriate editor for a parameter
    bool renderFieldEditor(EffectParameter& param);

} // namespace vkBasalt

#endif // FIELD_EDITOR_HPP_INCLUDED
