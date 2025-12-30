#include "mouse_input.hpp"

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <cstdlib>

namespace vkBasalt
{
    static Display* display = nullptr;
    static int xiOpcode = 0;
    static float scrollAccumulator = 0.0f;

    MouseState getMouseState()
    {
        MouseState state;

        // Initialize X11 and XInput2 once
        if (!display)
        {
            const char* disVar = getenv("DISPLAY");
            if (!disVar || !*disVar)
                return state;

            display = XOpenDisplay(disVar);
            if (!display)
                return state;

            int event, error;
            if (XQueryExtension(display, "XInputExtension", &xiOpcode, &event, &error))
            {
                int major = 2, minor = 0;
                if (XIQueryVersion(display, &major, &minor) == Success)
                {
                    unsigned char mask[XIMaskLen(XI_RawButtonPress)] = {0};
                    XISetMask(mask, XI_RawButtonPress);

                    XIEventMask eventMask = {XIAllMasterDevices, sizeof(mask), mask};
                    XISelectEvents(display, DefaultRootWindow(display), &eventMask, 1);
                }
            }
        }

        // Process scroll events
        while (XPending(display) > 0)
        {
            XEvent ev;
            XNextEvent(display, &ev);
            if (ev.xcookie.type == GenericEvent && ev.xcookie.extension == xiOpcode &&
                XGetEventData(display, &ev.xcookie))
            {
                if (ev.xcookie.evtype == XI_RawButtonPress)
                {
                    int button = ((XIRawEvent*)ev.xcookie.data)->detail;
                    if (button == 4) scrollAccumulator += 1.0f;
                    else if (button == 5) scrollAccumulator -= 1.0f;
                }
                XFreeEventData(display, &ev.xcookie);
            }
        }

        // Get pointer state
        Window focused, root, child;
        int revertTo, rootX, rootY;
        unsigned int mask;

        XGetInputFocus(display, &focused, &revertTo);
        if (focused == None || focused == PointerRoot)
            focused = DefaultRootWindow(display);

        if (XQueryPointer(display, focused, &root, &child, &rootX, &rootY, &state.x, &state.y, &mask))
        {
            state.leftButton = mask & Button1Mask;
            state.middleButton = mask & Button2Mask;
            state.rightButton = mask & Button3Mask;
        }

        state.scrollDelta = scrollAccumulator;
        scrollAccumulator = 0.0f;
        return state;
    }

} // namespace vkBasalt
