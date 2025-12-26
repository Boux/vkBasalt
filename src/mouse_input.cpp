#include "mouse_input.hpp"
#include "logger.hpp"

#include <X11/Xlib.h>
#include <memory>
#include <functional>
#include <cstring>

namespace vkBasalt
{
    MouseState getMouseState()
    {
        static int usesX11 = -1;
        static std::unique_ptr<Display, std::function<void(Display*)>> display;

        MouseState state;

        if (usesX11 < 0)
        {
            const char* disVar = getenv("DISPLAY");
            if (!disVar || !std::strcmp(disVar, ""))
            {
                usesX11 = 0;
            }
            else
            {
                display = std::unique_ptr<Display, std::function<void(Display*)>>(
                    XOpenDisplay(disVar), [](Display* d) { XCloseDisplay(d); });
                usesX11 = display ? 1 : 0;
            }
        }

        if (!usesX11 || !display)
            return state;

        // Get the focused window to get window-relative coordinates
        Window focusedWindow;
        int revertTo;
        XGetInputFocus(display.get(), &focusedWindow, &revertTo);

        if (focusedWindow == None || focusedWindow == PointerRoot)
            return state;

        Window root, child;
        int rootX, rootY;
        unsigned int mask;

        if (XQueryPointer(display.get(), focusedWindow, &root, &child, &rootX, &rootY, &state.x, &state.y, &mask))
        {
            state.leftButton = (mask & Button1Mask) != 0;
            state.middleButton = (mask & Button2Mask) != 0;
            state.rightButton = (mask & Button3Mask) != 0;
        }

        return state;
    }

} // namespace vkBasalt
