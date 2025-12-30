#pragma once

#include <cstdint>
#include <string>

namespace vkBasalt
{
    struct KeyboardState
    {
        std::string typedChars;     // Characters typed since last call
        std::string lastKeyName;    // X11 name of last key pressed (for keybind capture)
        bool backspace = false;
        bool del = false;
        bool enter = false;
        bool left = false;
        bool right = false;
        bool home = false;
        bool end = false;
    };

    uint32_t convertToKeySym(std::string key);
    bool     isKeyPressed(uint32_t ks);
    KeyboardState getKeyboardState();
} // namespace vkBasalt
