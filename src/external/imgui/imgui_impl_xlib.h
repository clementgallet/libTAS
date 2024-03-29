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

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

#include <X11/Xlib.h>

// struct SDL_Window;
// struct SDL_Renderer;
// typedef union SDL_Event SDL_Event;

IMGUI_IMPL_API bool     ImGui_ImplXlib_Init(Display* d, Window w);
IMGUI_IMPL_API void     ImGui_ImplXlib_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplXlib_NewFrame();
IMGUI_IMPL_API bool     ImGui_ImplXlib_ProcessEvent(XEvent* event);

#endif // #ifndef IMGUI_DISABLE
