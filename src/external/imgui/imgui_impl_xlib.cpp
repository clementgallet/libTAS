// dear imgui: Platform Backend for Xlib
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

// Implemented features:
//  [ ] Platform: Clipboard support.
//  [X] Platform: Mouse support.
//  [ ] Platform: TouchScreen support.
//  [X] Platform: Keyboard support.
//  [ ] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [ ] Platform: Basic IME support.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_xlib.h"

// Clang warnings with -Weverything
// #if defined(__clang__)
// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
// #endif

// Xlib
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/XKBlib.h> // XkbKeycodeToKeysym
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2.h>
#include <X11/cursorfont.h>

#include <time.h> // clock_gettime()
#include <iostream>

#ifdef X_HAVE_UTF8_STRING
#include <locale.h>
#endif

// #include <SDL2/SDL.h>
// #include <SDL2/SDL_syswm.h>
// #if defined(__APPLE__)
// #include <TargetConditionals.h>
// #endif
// #include "hook.h"

// SDL Data
struct ImGui_ImplXlib_Data
{
    Display*        Dpy;
    Window          Win;
    int             Xi2Opcode;
    XIM             IM;
    XIC             IC;
    
    timespec        Time;
    // Uint32          MouseWindowID;
    int             MouseButtonsDown;
    Cursor          MouseCursors[ImGuiMouseCursor_COUNT];
    Cursor          LastMouseCursor;
    // int             PendingMouseLeaveFrame;
    // char*           ClipboardTextData;

    ImGui_ImplXlib_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static ImGui_ImplXlib_Data* ImGui_ImplXlib_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplXlib_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

// Functions
// static const char* ImGui_ImplXlib_GetClipboardText(void*)
// {
//     ImGui_ImplXlib_Data* bd = ImGui_ImplXlib_GetBackendData();
//     if (bd->ClipboardTextData)
//         SDL_free(bd->ClipboardTextData);
//     bd->ClipboardTextData = SDL_GetClipboardText();
//     return bd->ClipboardTextData;
// }
// 
// static void ImGui_ImplXlib_SetClipboardText(void*, const char* text)
// {
//     SDL_SetClipboardText(text);
// }

// Note: native IME will only display if user calls SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1") _before_ SDL_CreateWindow().
// static void ImGui_ImplXlib_SetPlatformImeData(ImGuiViewport*, ImGuiPlatformImeData* data)
// {
//     if (data->WantVisible)
//     {
//         SDL_Rect r;
//         r.x = (int)data->InputPos.x;
//         r.y = (int)data->InputPos.y;
//         r.w = 1;
//         r.h = (int)data->InputLineHeight;
//         SDL_SetTextInputRect(&r);
//     }
// }

static ImGuiKey ImGui_ImplXlib_KeySymToImGuiKey(KeySym keysym)
{
    switch (keysym)
    {
        case XK_Tab: return ImGuiKey_Tab;
        case XK_Left: return ImGuiKey_LeftArrow;
        case XK_Right: return ImGuiKey_RightArrow;
        case XK_Up: return ImGuiKey_UpArrow;
        case XK_Down: return ImGuiKey_DownArrow;
        case XK_Prior: return ImGuiKey_PageUp;
        case XK_Next: return ImGuiKey_PageDown;
        case XK_Home: return ImGuiKey_Home;
        case XK_End: return ImGuiKey_End;
        case XK_Insert: return ImGuiKey_Insert;
        case XK_Delete: return ImGuiKey_Delete;
        case XK_BackSpace: return ImGuiKey_Backspace;
        case XK_space: return ImGuiKey_Space;
        case XK_Return: return ImGuiKey_Enter;
        case XK_Escape: return ImGuiKey_Escape;
        case XK_quoteright: return ImGuiKey_Apostrophe;
        case XK_comma: return ImGuiKey_Comma;
        case XK_minus: return ImGuiKey_Minus;
        case XK_period: return ImGuiKey_Period;
        case XK_slash: return ImGuiKey_Slash;
        case XK_semicolon: return ImGuiKey_Semicolon;
        case XK_equal: return ImGuiKey_Equal;
        case XK_bracketleft: return ImGuiKey_LeftBracket;
        case XK_backslash: return ImGuiKey_Backslash;
        case XK_bracketright: return ImGuiKey_RightBracket;
        case XK_quoteleft: return ImGuiKey_GraveAccent;
        case XK_Caps_Lock: return ImGuiKey_CapsLock;
        case XK_Scroll_Lock: return ImGuiKey_ScrollLock;
        case XK_Num_Lock: return ImGuiKey_NumLock;
        case XK_Print: return ImGuiKey_PrintScreen;
        case XK_Pause: return ImGuiKey_Pause;
        case XK_KP_0: return ImGuiKey_Keypad0;
        case XK_KP_1: return ImGuiKey_Keypad1;
        case XK_KP_2: return ImGuiKey_Keypad2;
        case XK_KP_3: return ImGuiKey_Keypad3;
        case XK_KP_4: return ImGuiKey_Keypad4;
        case XK_KP_5: return ImGuiKey_Keypad5;
        case XK_KP_6: return ImGuiKey_Keypad6;
        case XK_KP_7: return ImGuiKey_Keypad7;
        case XK_KP_8: return ImGuiKey_Keypad8;
        case XK_KP_9: return ImGuiKey_Keypad9;
        case XK_KP_Decimal: return ImGuiKey_KeypadDecimal;
        case XK_KP_Divide: return ImGuiKey_KeypadDivide;
        case XK_KP_Multiply: return ImGuiKey_KeypadMultiply;
        case XK_KP_Subtract: return ImGuiKey_KeypadSubtract;
        case XK_KP_Add: return ImGuiKey_KeypadAdd;
        case XK_KP_Enter: return ImGuiKey_KeypadEnter;
        case XK_KP_Equal: return ImGuiKey_KeypadEqual;
        case XK_Control_L: return ImGuiKey_LeftCtrl;
        case XK_Shift_L: return ImGuiKey_LeftShift;
        case XK_Alt_L: return ImGuiKey_LeftAlt;
        case XK_Super_L: return ImGuiKey_LeftSuper;
        case XK_Control_R: return ImGuiKey_RightCtrl;
        case XK_Shift_R: return ImGuiKey_RightShift;
        case XK_Alt_R: return ImGuiKey_RightAlt;
        case XK_Super_R: return ImGuiKey_RightSuper;
        case XK_Menu: return ImGuiKey_Menu;
        case XK_0: return ImGuiKey_0;
        case XK_1: return ImGuiKey_1;
        case XK_2: return ImGuiKey_2;
        case XK_3: return ImGuiKey_3;
        case XK_4: return ImGuiKey_4;
        case XK_5: return ImGuiKey_5;
        case XK_6: return ImGuiKey_6;
        case XK_7: return ImGuiKey_7;
        case XK_8: return ImGuiKey_8;
        case XK_9: return ImGuiKey_9;
        case XK_a: return ImGuiKey_A;
        case XK_b: return ImGuiKey_B;
        case XK_c: return ImGuiKey_C;
        case XK_d: return ImGuiKey_D;
        case XK_e: return ImGuiKey_E;
        case XK_f: return ImGuiKey_F;
        case XK_g: return ImGuiKey_G;
        case XK_h: return ImGuiKey_H;
        case XK_i: return ImGuiKey_I;
        case XK_j: return ImGuiKey_J;
        case XK_k: return ImGuiKey_K;
        case XK_l: return ImGuiKey_L;
        case XK_m: return ImGuiKey_M;
        case XK_n: return ImGuiKey_N;
        case XK_o: return ImGuiKey_O;
        case XK_p: return ImGuiKey_P;
        case XK_q: return ImGuiKey_Q;
        case XK_r: return ImGuiKey_R;
        case XK_s: return ImGuiKey_S;
        case XK_t: return ImGuiKey_T;
        case XK_u: return ImGuiKey_U;
        case XK_v: return ImGuiKey_V;
        case XK_w: return ImGuiKey_W;
        case XK_x: return ImGuiKey_X;
        case XK_y: return ImGuiKey_Y;
        case XK_z: return ImGuiKey_Z;
        case XK_F1: return ImGuiKey_F1;
        case XK_F2: return ImGuiKey_F2;
        case XK_F3: return ImGuiKey_F3;
        case XK_F4: return ImGuiKey_F4;
        case XK_F5: return ImGuiKey_F5;
        case XK_F6: return ImGuiKey_F6;
        case XK_F7: return ImGuiKey_F7;
        case XK_F8: return ImGuiKey_F8;
        case XK_F9: return ImGuiKey_F9;
        case XK_F10: return ImGuiKey_F10;
        case XK_F11: return ImGuiKey_F11;
        case XK_F12: return ImGuiKey_F12;
        case XK_F13: return ImGuiKey_F13;
        case XK_F14: return ImGuiKey_F14;
        case XK_F15: return ImGuiKey_F15;
        case XK_F16: return ImGuiKey_F16;
        case XK_F17: return ImGuiKey_F17;
        case XK_F18: return ImGuiKey_F18;
        case XK_F19: return ImGuiKey_F19;
        case XK_F20: return ImGuiKey_F20;
        case XK_F21: return ImGuiKey_F21;
        case XK_F22: return ImGuiKey_F22;
        case XK_F23: return ImGuiKey_F23;
        case XK_F24: return ImGuiKey_F24;
        case XF86XK_Back: return ImGuiKey_AppBack;
        case XF86XK_Forward: return ImGuiKey_AppForward;
    }
    return ImGuiKey_None;
}

static void ImGui_ImplXlib_UpdateKeyModifiers(unsigned int xlib_key_mods)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, (xlib_key_mods & ControlMask) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, (xlib_key_mods & ShiftMask) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, (xlib_key_mods & Mod1Mask) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (xlib_key_mods & Mod4Mask) != 0);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
// If you have multiple SDL events and some of them are not meant to be used by dear imgui, you may need to filter events based on their windowID field.
bool ImGui_ImplXlib_ProcessEvent(XEvent* event)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplXlib_Data* bd = ImGui_ImplXlib_GetBackendData();

    // Needed for Xim events
    if (XFilterEvent(event, None) == True)
        return true;
    
    switch (event->type)
    {
        case GenericEvent:
        {
            XGenericEventCookie *cookie = (XGenericEventCookie*)&event->xcookie;
            if (cookie->extension == bd->Xi2Opcode && XGetEventData(event->xcookie.display, cookie))
            {
                XIDeviceEvent *dev = (XIDeviceEvent*)cookie->data;                
                switch (cookie->evtype)
                {
                    case XI_Motion:
                    {
                        ImVec2 mouse_pos((float)dev->event_x, (float)dev->event_y);
                        io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
                        io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
                        return true;
                    }
                    case XI_ButtonPress:
                    case XI_ButtonRelease:
                    {
                        if (dev->detail >= Button1 && dev->detail <= Button3)
                        {
                            int mouse_button = -1;
                            if (dev->detail == Button1) { mouse_button = 0; }
                            if (dev->detail == Button2) { mouse_button = 1; }
                            if (dev->detail == Button3) { mouse_button = 2; }
                            
                            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
                            io.AddMouseButtonEvent(mouse_button, (cookie->evtype == XI_ButtonPress));
                            bd->MouseButtonsDown = (cookie->evtype == XI_ButtonPress) ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
                            return true;
                        }
                        else if (dev->detail == Button4 || dev->detail == Button5)
                        {
                            float wheel_y = (dev->detail == Button4) ? 1.0f : -1.0f;
                            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
                            io.AddMouseWheelEvent(0, wheel_y);
                            return true;
                        }
                    }
                }
            }
            XFreeEventData(event->xcookie.display, cookie);
            return true;
        }
        case KeyPress:
        case KeyRelease:
        {
            ImGui_ImplXlib_UpdateKeyModifiers(event->xkey.state);
            KeySym ks = XkbKeycodeToKeysym(event->xkey.display, event->xkey.keycode, 0, 0);
            ImGuiKey key = ImGui_ImplXlib_KeySymToImGuiKey(ks);
            io.AddKeyEvent(key, (event->type == KeyPress));

            char text[64];
            Status status = 0;
#ifdef X_HAVE_UTF8_STRING
            if (bd->IC && event->type == KeyPress) {
                int size = Xutf8LookupString(bd->IC, &event->xkey, text, sizeof(text), NULL, &status);
                // Don't post text for unprintable characters
                unsigned char c = text[0];
                if ((size > 0) && (c > '\x20') && (c != '\x7f'))
                    io.AddInputCharactersUTF8(text);
            }
            else
#endif
            {
                // The following function must be called even for key release events */
                int size = XLookupString(&event->xkey, text, sizeof(text), NULL, NULL);
                if (event->type == KeyPress && text[0]) {
                    // Don't post text for unprintable characters
                    unsigned char c = text[0];
                    if ((size > 0) && (c > '\x20') && (c != '\x7f'))
                        io.AddInputCharacter(c);
                }
            }
            return true;
        }
        case FocusIn:
        case FocusOut:
        {
            io.AddFocusEvent(event->type == FocusIn);
            
#ifdef X_HAVE_UTF8_STRING
            if (bd->IC) {
                if (event->type == FocusIn)
                    XSetICFocus(bd->IC);
                else
                    XUnsetICFocus(bd->IC);
            }
#endif
            return true;
        }
    }
    return false;
}

bool ImGui_ImplXlib_Init(Display* display, Window window)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Check and store if we are on a SDL backend that supports global mouse position
    // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
//     bool mouse_can_use_global_state = false;
// #if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
//     const char* sdl_backend = SDL_GetCurrentVideoDriver();
//     const char* global_mouse_whitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
//     for (int n = 0; n < IM_ARRAYSIZE(global_mouse_whitelist); n++)
//         if (strncmp(sdl_backend, global_mouse_whitelist[n], strlen(global_mouse_whitelist[n])) == 0)
//             mouse_can_use_global_state = true;
// #endif

    // Setup backend capabilities flags
    ImGui_ImplXlib_Data* bd = IM_NEW(ImGui_ImplXlib_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "imgui_impl_xlib";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    // io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)

    bd->Dpy = display;
    bd->Win = window;

    // io.SetClipboardTextFn = ImGui_ImplXlib_SetClipboardText;
    // io.GetClipboardTextFn = ImGui_ImplXlib_GetClipboardText;
    // io.ClipboardUserData = nullptr;
    // io.SetPlatformImeDataFn = ImGui_ImplXlib_SetPlatformImeData;

    // Select keyboard/focus events
    // XSelectInput(display, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask | FocusChangeMask);
    XSelectInput(display, window, KeyPressMask | KeyReleaseMask | FocusChangeMask);

    // Setup XInput for keyboard/mouse inputs
    int xi2_opcode, xi2_event, xi2_error;
    Bool ret = XQueryExtension(display, "XInputExtension", &xi2_opcode, &xi2_event, &xi2_error);

    IM_ASSERT(ret == True && "Could not load XInputExtension!");

    bd->Xi2Opcode = xi2_opcode;

    XIEventMask xi2_eventmask = {};
    static unsigned char xi2_mask[XIMaskLen(XI_LASTEVENT)] = {};

    XISetMask(xi2_mask, XI_Motion);
    XISetMask(xi2_mask, XI_ButtonPress);
    XISetMask(xi2_mask, XI_ButtonRelease);

    xi2_eventmask.deviceid = XIAllMasterDevices;
    xi2_eventmask.mask_len = sizeof(xi2_mask);
    xi2_eventmask.mask = xi2_mask;

    XISelectEvents(display, window, &xi2_eventmask, 1);

    // Setup XIM
#ifdef X_HAVE_UTF8_STRING

    XSetLocaleModifiers("");
    bd->IM = XOpenIM(display, NULL, NULL, NULL);

    bd->IC = XCreateIC(bd->IM, XNClientWindow, window, XNFocusWindow, window, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);

#endif

    // Load mouse cursors
    bd->MouseCursors[ImGuiMouseCursor_Arrow] = XCreateFontCursor(display, XC_left_ptr);
    bd->MouseCursors[ImGuiMouseCursor_TextInput] = XCreateFontCursor(display, XC_xterm);
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = XCreateFontCursor(display, XC_fleur);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = XCreateFontCursor(display, XC_sb_v_double_arrow);
    bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = XCreateFontCursor(display, XC_sb_h_double_arrow);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = XCreateFontCursor(display, XC_fleur);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = XCreateFontCursor(display, XC_fleur);
    bd->MouseCursors[ImGuiMouseCursor_Hand] = XCreateFontCursor(display, XC_hand2);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = XCreateFontCursor(display, XC_pirate);

    // Set platform dependent data in viewport
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
//     ImGuiViewport* main_viewport = ImGui::GetMainViewport();
//     main_viewport->PlatformHandleRaw = nullptr;
//     SDL_SysWMinfo info;
//     SDL_VERSION(&info.version);
//     if (SDL_GetWindowWMInfo(window, &info))
//     {
// #if defined(SDL_VIDEO_DRIVER_WINDOWS)
//         main_viewport->PlatformHandleRaw = (void*)info.info.win.window;
// #elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
//         main_viewport->PlatformHandleRaw = (void*)info.info.cocoa.window;
// #endif
//     }

    // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the event.
    // Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed as hovered.
    // (This is unfortunately a global SDL setting, so enabling it might have a side-effect on your application.
    // It is unlikely to make a difference, but if your app absolutely needs to ignore the initial on-focus click:
    // you can ignore SDL_MOUSEBUTTONDOWN events coming right after a SDL_WINDOWEVENT_FOCUS_GAINED)
// #ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
//     SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
// #endif

    // From 2.0.18: Enable native IME.
    // IMPORTANT: This is used at the time of SDL_CreateWindow() so this will only affects secondary windows, if any.
    // For the main window to be affected, your application needs to call this manually before calling SDL_CreateWindow().
// #ifdef SDL_HINT_IME_SHOW_UI
//     SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
// #endif

    // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
// #ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
//     SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
// #endif

    return true;
}

void ImGui_ImplXlib_Shutdown()
{
    ImGui_ImplXlib_Data* bd = ImGui_ImplXlib_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    if (bd->IC)
        XDestroyIC(bd->IC); 
    // if (bd->ClipboardTextData)
    //     SDL_free(bd->ClipboardTextData);
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        XFreeCursor(bd->Dpy, bd->MouseCursors[cursor_n]);
    bd->LastMouseCursor = 0;

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos);
    IM_DELETE(bd);
}

// static void ImGui_ImplXlib_UpdateMouseData()
// {
//     ImGui_ImplXlib_Data* bd = ImGui_ImplXlib_GetBackendData();
//     ImGuiIO& io = ImGui::GetIO();
// 
//     // We forward mouse input when hovered or captured (via SDL_MOUSEMOTION) or when focused (below)
// #if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
//     // SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger other operations outside
//     SDL_CaptureMouse((bd->MouseButtonsDown != 0) ? SDL_TRUE : SDL_FALSE);
//     SDL_Window* focused_window = SDL_GetKeyboardFocus();
//     const bool is_app_focused = (bd->Window == focused_window);
// #else
//     const bool is_app_focused = (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0; // SDL 2.0.3 and non-windowed systems: single-viewport only
// #endif
//     if (is_app_focused)
//     {
//         // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
//         if (io.WantSetMousePos)
//             SDL_WarpMouseInWindow(bd->Window, (int)io.MousePos.x, (int)io.MousePos.y);
// 
//         // (Optional) Fallback to provide mouse position when focused (SDL_MOUSEMOTION already provides this when hovered or captured)
//         if (bd->MouseButtonsDown == 0)
//         {
//             int window_x, window_y, mouse_x_global, mouse_y_global;
//             SDL_GetGlobalMouseState(&mouse_x_global, &mouse_y_global);
//             SDL_GetWindowPosition(bd->Window, &window_x, &window_y);
//             io.AddMousePosEvent((float)(mouse_x_global - window_x), (float)(mouse_y_global - window_y));
//         }
//     }
// }

static void ImGui_ImplXlib_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;
    ImGui_ImplXlib_Data* bd = ImGui_ImplXlib_GetBackendData();

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        XUndefineCursor(bd->Dpy, bd->Win);
    }
    else
    {
        // Show OS mouse cursor
        Cursor expected_cursor = bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow];
        if (bd->LastMouseCursor != expected_cursor)
        {
            XDefineCursor(bd->Dpy, bd->Win, expected_cursor);
            // XFlush(bd->Dpy);
            bd->LastMouseCursor = expected_cursor;
        }
    }
}

void ImGui_ImplXlib_NewFrame()
{
    ImGui_ImplXlib_Data* bd = ImGui_ImplXlib_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplXlib_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int x, y;
    unsigned int w = 0, h = 0, border_width, depth;
    Window root;
    XGetGeometry(bd->Dpy, bd->Win, &root, &x, &y, &w, &h, &border_width, &depth);

    io.DisplaySize = ImVec2((float)w, (float)h);

    // Setup time step
    timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    if ((current_time.tv_sec < bd->Time.tv_sec) ||
        ((current_time.tv_sec == bd->Time.tv_sec) && (current_time.tv_nsec < bd->Time.tv_nsec)))
    {
        current_time.tv_sec = bd->Time.tv_sec;
        current_time.tv_nsec = bd->Time.tv_nsec + 1;
    }
    
    if (bd->Time.tv_sec > 0 || bd->Time.tv_nsec > 0)
        io.DeltaTime = (float)(current_time.tv_sec - bd->Time.tv_sec) + (float)((double)(current_time.tv_nsec - bd->Time.tv_nsec) / 1000000000.0f);
    else
        io.DeltaTime = (float)(1.0f / 60.0f);
    bd->Time = current_time;
    
    // if (bd->PendingMouseLeaveFrame && bd->PendingMouseLeaveFrame >= ImGui::GetFrameCount() && bd->MouseButtonsDown == 0)
    // {
    //     bd->MouseWindowID = 0;
    //     bd->PendingMouseLeaveFrame = 0;
    //     io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    // }

    // ImGui_ImplXlib_UpdateMouseData();
    ImGui_ImplXlib_UpdateMouseCursor();
}

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef IMGUI_DISABLE
