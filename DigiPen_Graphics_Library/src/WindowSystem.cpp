//-------------------------------------------------------------------------------------------------
// file:    WindowSystem.cpp
// author:  Andy Ellinger
// brief:   Windows system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <stdio.h>
#include <string>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define NOMINMAX
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

// The stb_image implementation is compiled here (it is used to load the window icon)
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

module WindowSystem;

import Errors;
import Input;
import GraphicsSystem;

namespace DGL
{

WindowSystem* gWinSys = nullptr;

//------------------------------------------------------------------ Callbacks and helper functions

namespace
{

//*************************************************************************************************
// Converts a GLFW key value to the matching DGL_KeyCode value. Returns 0 if there is no match.
unsigned char TranslateKey(int key)
{
    // Letters and numbers have the same values in GLFW and DGL
    if ((key >= GLFW_KEY_A && key <= GLFW_KEY_Z) || (key >= GLFW_KEY_0 && key <= GLFW_KEY_9))
        return (unsigned char)key;

    // The function keys and number pad digits are in the same order in GLFW and DGL
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F24)
        return (unsigned char)(DGL_KEY_F1 + (key - GLFW_KEY_F1));
    if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9)
        return (unsigned char)(DGL_KEY_NUMPAD_0 + (key - GLFW_KEY_KP_0));

    switch (key)
    {
    case GLFW_KEY_SPACE:            return DGL_KEY_SPACE;
    case GLFW_KEY_APOSTROPHE:       return DGL_KEY_APOSTROPHE;
    case GLFW_KEY_COMMA:            return DGL_KEY_COMMA;
    case GLFW_KEY_MINUS:            return DGL_KEY_MINUS;
    case GLFW_KEY_PERIOD:           return DGL_KEY_PERIOD;
    case GLFW_KEY_SLASH:            return DGL_KEY_SLASH;
    case GLFW_KEY_SEMICOLON:        return DGL_KEY_SEMICOLON;
    case GLFW_KEY_EQUAL:            return DGL_KEY_EQUAL;
    case GLFW_KEY_LEFT_BRACKET:     return DGL_KEY_LEFT_BRACKET;
    case GLFW_KEY_BACKSLASH:        return DGL_KEY_BACKSLASH;
    case GLFW_KEY_RIGHT_BRACKET:    return DGL_KEY_RIGHT_BRACKET;
    case GLFW_KEY_GRAVE_ACCENT:     return DGL_KEY_GRAVE_ACCENT;
    case GLFW_KEY_ESCAPE:           return DGL_KEY_ESCAPE;
    case GLFW_KEY_ENTER:            return DGL_KEY_ENTER;
    case GLFW_KEY_TAB:              return DGL_KEY_TAB;
    case GLFW_KEY_BACKSPACE:        return DGL_KEY_BACKSPACE;
    case GLFW_KEY_INSERT:           return DGL_KEY_INSERT;
    case GLFW_KEY_DELETE:           return DGL_KEY_DELETE;
    case GLFW_KEY_RIGHT:            return DGL_KEY_RIGHT;
    case GLFW_KEY_LEFT:             return DGL_KEY_LEFT;
    case GLFW_KEY_DOWN:             return DGL_KEY_DOWN;
    case GLFW_KEY_UP:               return DGL_KEY_UP;
    case GLFW_KEY_PAGE_UP:          return DGL_KEY_PAGE_UP;
    case GLFW_KEY_PAGE_DOWN:        return DGL_KEY_PAGE_DOWN;
    case GLFW_KEY_HOME:             return DGL_KEY_HOME;
    case GLFW_KEY_END:              return DGL_KEY_END;
    case GLFW_KEY_CAPS_LOCK:        return DGL_KEY_CAPS_LOCK;
    case GLFW_KEY_SCROLL_LOCK:      return DGL_KEY_SCROLL_LOCK;
    case GLFW_KEY_NUM_LOCK:         return DGL_KEY_NUM_LOCK;
    case GLFW_KEY_PRINT_SCREEN:     return DGL_KEY_PRINT_SCREEN;
    case GLFW_KEY_PAUSE:            return DGL_KEY_PAUSE;
    case GLFW_KEY_KP_DECIMAL:       return DGL_KEY_NUMPAD_DECIMAL;
    case GLFW_KEY_KP_DIVIDE:        return DGL_KEY_NUMPAD_DIVIDE;
    case GLFW_KEY_KP_MULTIPLY:      return DGL_KEY_NUMPAD_MULTIPLY;
    case GLFW_KEY_KP_SUBTRACT:      return DGL_KEY_NUMPAD_SUBTRACT;
    case GLFW_KEY_KP_ADD:           return DGL_KEY_NUMPAD_ADD;
    case GLFW_KEY_KP_ENTER:         return DGL_KEY_ENTER;
    case GLFW_KEY_LEFT_SHIFT:       return DGL_KEY_LEFT_SHIFT;
    case GLFW_KEY_LEFT_CONTROL:     return DGL_KEY_LEFT_CONTROL;
    case GLFW_KEY_LEFT_ALT:         return DGL_KEY_LEFT_ALT;
    case GLFW_KEY_LEFT_SUPER:       return DGL_KEY_LEFT_SUPER;
    case GLFW_KEY_RIGHT_SHIFT:      return DGL_KEY_RIGHT_SHIFT;
    case GLFW_KEY_RIGHT_CONTROL:    return DGL_KEY_RIGHT_CONTROL;
    case GLFW_KEY_RIGHT_ALT:        return DGL_KEY_RIGHT_ALT;
    case GLFW_KEY_RIGHT_SUPER:      return DGL_KEY_RIGHT_SUPER;
    case GLFW_KEY_MENU:             return DGL_KEY_MENU;
    default:                        return 0;
    }
}

//*************************************************************************************************
// Sets the state of a key such as DGL_KEY_SHIFT, which is down if either the left or right key is down
void UpdateCombinedKey(unsigned char combinedKey, unsigned char leftKey, unsigned char rightKey)
{
    bool down = gInput->KeyDown(leftKey) || gInput->KeyDown(rightKey);
    if (down != gInput->KeyDown(combinedKey))
        gInput->SetKeyState(combinedKey, down);
}

//*************************************************************************************************
// Returns the monitor that the window is fullscreen on, or the monitor that contains most of the window
GLFWmonitor* GetCurrentMonitor(GLFWwindow* window)
{
    // If fullscreen, return the monitor
    if (GLFWmonitor* fullscreenMonitor = glfwGetWindowMonitor(window))
        return fullscreenMonitor;

    // Store the window size and top left position
    int winLeft, winTop, winWidth, winHeight;
    glfwGetWindowPos(window, &winLeft, &winTop);
    glfwGetWindowSize(window, &winWidth, &winHeight);

    // Get what GLFW considers to be the primary monitor
    GLFWmonitor* bestMonitor = glfwGetPrimaryMonitor();
    int bestOverlap = 0;

    // Look at all monitors that this window is on
    int monitorCount;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
    for (int i = 0; i < monitorCount; ++i)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitors[i]);
        int monLeft, monTop;
        glfwGetMonitorPos(monitors[i], &monLeft, &monTop);

        // Calculate how much of the window is on this monitor
        int overlapWidth = std::min(winLeft + winWidth, monLeft + mode->width) - std::max(winLeft, monLeft);
        int overlapHeight = std::min(winTop + winHeight, monTop + mode->height) - std::max(winTop, monTop);
        if (overlapWidth > 0 && overlapHeight > 0 && overlapWidth * overlapHeight > bestOverlap)
        {
            bestOverlap = overlapWidth * overlapHeight;
            bestMonitor = monitors[i];
        }
    }

    return bestMonitor;
}

//*************************************************************************************************
void ErrorCallback(int errorCode, const char* description)
{
    std::string msg = "GLFW error " + std::to_string(errorCode) + ": " + description;
    gError->SetError(msg);
}

//*************************************************************************************************
void KeyCallback(GLFWwindow*, int key, int, int action, int)
{
    // Key repeats don't change the key state
    if (action == GLFW_REPEAT)
        return;

    // Get the matching DGL key
    unsigned char dglKey = TranslateKey(key);
    if (!dglKey)
        return;

    gInput->SetKeyState(dglKey, action == GLFW_PRESS);

    // Keep the combined modifier keys up to date
    switch (dglKey)
    {
    case DGL_KEY_LEFT_SHIFT:
    case DGL_KEY_RIGHT_SHIFT:
        UpdateCombinedKey(DGL_KEY_SHIFT, DGL_KEY_LEFT_SHIFT, DGL_KEY_RIGHT_SHIFT);
        break;
    case DGL_KEY_LEFT_CONTROL:
    case DGL_KEY_RIGHT_CONTROL:
        UpdateCombinedKey(DGL_KEY_CONTROL, DGL_KEY_LEFT_CONTROL, DGL_KEY_RIGHT_CONTROL);
        break;
    case DGL_KEY_LEFT_ALT:
    case DGL_KEY_RIGHT_ALT:
        UpdateCombinedKey(DGL_KEY_ALT, DGL_KEY_LEFT_ALT, DGL_KEY_RIGHT_ALT);
        break;
    default:
        break;
    }
}

//*************************************************************************************************
void MouseButtonCallback(GLFWwindow*, int button, int action, int)
{
    unsigned char dglKey = 0;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:     dglKey = DGL_MOUSE_LEFT; break;
    case GLFW_MOUSE_BUTTON_RIGHT:    dglKey = DGL_MOUSE_RIGHT; break;
    case GLFW_MOUSE_BUTTON_MIDDLE:   dglKey = DGL_MOUSE_MIDDLE; break;
    case GLFW_MOUSE_BUTTON_4:        dglKey = DGL_MOUSE_BUTTON_4; break;
    case GLFW_MOUSE_BUTTON_5:        dglKey = DGL_MOUSE_BUTTON_5; break;
    default:                         return;
    }

    gInput->SetKeyState(dglKey, action == GLFW_PRESS);
}

//*************************************************************************************************
void ScrollCallback(GLFWwindow*, double, double yOffset)
{
    gInput->AddMouseScrollDelta(yOffset);
}

//*************************************************************************************************
void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    // Nothing to do while the window is minimized
    if (width <= 0 || height <= 0)
        return;

    // Update the camera and graphics for the new size
    gGraphics->Camera.ResetWindowSize();
}

} // namespace

//------------------------------------------------------------------------------------ WindowSystem

//*************************************************************************************************
WindowSystem::WindowSystem()
{
    gWinSys = this;
}

//*************************************************************************************************
bool WindowSystem::Initialize(const DGL_SysInitInfo& sysInitInfo)
{
    // Check if we should create a console
    if (sysInitInfo.createConsole)
        CreateConsole();

    int width = (int)sysInitInfo.windowSize.x;
    int height = (int)sysInitInfo.windowSize.y;
    if (width <= 0 || height <= 0)
    {
        gError->SetError("Passed invalid window size to DGL_System_Init.");
        return false;
    }
    
    // Set the error callback for GLFW
    glfwSetErrorCallback(ErrorCallback);

    // Initialize GLFW (any errors will be reported through the error callback)
    if (!glfwInit())
        return false;
    mGlfwInitialized = true;

    // The graphics API is set up separately, so GLFW should not create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, sysInitInfo.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, sysInitInfo.decorated ? GLFW_TRUE : GLFW_FALSE);
    // Don't minimize a fullscreen window when it loses focus
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    // Save the windowed size, which will be used if switching from fullscreen to windowed
    mWindowedWidth = width;
    mWindowedHeight = height;

    GLFWmonitor* monitor = nullptr;
    if (sysInitInfo.fullscreen)
    {
        // Use the primary monitor at its current resolution
        monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Center the saved windowed position on the monitor
        glfwGetMonitorPos(monitor, &mWindowedLeft, &mWindowedTop);
        mWindowedLeft += (mode->width - width) / 2;
        mWindowedTop += (mode->height - height) / 2;

        width = mode->width;
        height = mode->height;
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    // Create the window
    mWindow = glfwCreateWindow(width, height, sysInitInfo.windowTitle, monitor, nullptr);
    if (!mWindow)
        return false;

    // Set the icon if one was provided
    if (sysInitInfo.windowIconFile)
        SetWindowIcon(sysInitInfo.windowIconFile);

    SetCallbacks();

    return true;
}

//*************************************************************************************************
void WindowSystem::Update()
{
    // Process all of the window events (this will call the callback functions)
    glfwPollEvents();
}

//*************************************************************************************************
void WindowSystem::ShutDown()
{
    if (mWindow)
    {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }

    if (mGlfwInitialized)
    {
        glfwTerminate();
        mGlfwInitialized = false;
    }
}

//*************************************************************************************************
void WindowSystem::Close()
{
    if (mWindow)
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
}

//*************************************************************************************************
GLFWwindow* WindowSystem::GetWindow() const
{
    return mWindow;
}

//*************************************************************************************************
void* WindowSystem::GetNativeHandle() const
{
    if (!mWindow)
        return nullptr;

#ifdef _WIN32
    return glfwGetWin32Window(mWindow);
#else
    return nullptr;
#endif
}

//*************************************************************************************************
bool WindowSystem::GetWindowExists() const
{
    return mWindow != nullptr && !glfwWindowShouldClose(mWindow);
}

//*************************************************************************************************
bool WindowSystem::GetWindowHasFocus() const
{
    return mWindow != nullptr && glfwGetWindowAttrib(mWindow, GLFW_FOCUSED) == GLFW_TRUE;
}

//*************************************************************************************************
DGL_Vec2 WindowSystem::GetMonitorSize() const
{
    if (!mWindow)
        return DGL_Vec2{ 0 };

    const GLFWvidmode* mode = glfwGetVideoMode(GetCurrentMonitor(mWindow));
    if (!mode)
        return DGL_Vec2{ 0 };

    return { (float)mode->width, (float)mode->height };
}

//*************************************************************************************************
DGL_Vec2 WindowSystem::GetWindowSize() const
{
    if (!mWindow)
        return DGL_Vec2{ 0 };

    // Use the framebuffer size, which is the drawable area in pixels
    int width, height;
    glfwGetFramebufferSize(mWindow, &width, &height);

    return { (float)width, (float)height };
}

//*************************************************************************************************
DGL_Vec2 WindowSystem::GetCursorPosition() const
{
    if (!mWindow)
        return DGL_Vec2{ 0 };

    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);

    // The cursor position is in screen coordinates, which might not match pixels on some platforms,
    // so scale it to match the framebuffer size
    int windowWidth, windowHeight, bufferWidth, bufferHeight;
    glfwGetWindowSize(mWindow, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(mWindow, &bufferWidth, &bufferHeight);
    if (windowWidth > 0 && windowHeight > 0)
    {
        x *= (double)bufferWidth / windowWidth;
        y *= (double)bufferHeight / windowHeight;
    }

    return { (float)x, (float)y };
}

//*************************************************************************************************
void WindowSystem::SetWindowSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, 
    int windowTopPos) const
{
    if (!mWindow)
        return;

    if (windowWidth <= 0 || windowHeight <= 0)
    {
        gError->SetError("Passed invalid parameter for window size.");
        return;
    }

    // Size and position don't apply to a fullscreen window
    if (GetIsFullscreen())
        return;

    // Restore the window if it is minimized or maximized
    if (glfwGetWindowAttrib(mWindow, GLFW_ICONIFIED) || glfwGetWindowAttrib(mWindow, GLFW_MAXIMIZED))
        glfwRestoreWindow(mWindow);

    glfwSetWindowSize(mWindow, windowWidth, windowHeight);
    glfwSetWindowPos(mWindow, windowLeftPos, windowTopPos);
}

//*************************************************************************************************
void WindowSystem::SetWindowSize(int windowWidth, int windowHeight) const
{
    if (!mWindow)
        return;

    // Get the current position of the window
    int left, top;
    glfwGetWindowPos(mWindow, &left, &top);

    // Set the size and position using the current left and top position
    SetWindowSizeAndPosition(windowWidth, windowHeight, left, top);
}

//*************************************************************************************************
void WindowSystem::SetFullscreen(bool fullscreen)
{
    if (!mWindow || fullscreen == GetIsFullscreen())
        return;

    if (fullscreen)
    {
        // Save the current position and size so they can be restored later
        glfwGetWindowPos(mWindow, &mWindowedLeft, &mWindowedTop);
        glfwGetWindowSize(mWindow, &mWindowedWidth, &mWindowedHeight);

        // Make the window fullscreen on the monitor it is currently on, at the current resolution
        GLFWmonitor* monitor = GetCurrentMonitor(mWindow);
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(mWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    }
    else
    {
        // Restore the previous position and size
        glfwSetWindowMonitor(mWindow, nullptr, mWindowedLeft, mWindowedTop, mWindowedWidth,
            mWindowedHeight, GLFW_DONT_CARE);
    }
}

//*************************************************************************************************
bool WindowSystem::GetIsFullscreen() const
{
    return mWindow != nullptr && glfwGetWindowMonitor(mWindow) != nullptr;
}

//*************************************************************************************************
void WindowSystem::SetResizable(bool resizable) const
{
    int resizeValue;
    if (resizable)
        resizeValue = GLFW_TRUE;
    else
        resizeValue = GLFW_FALSE;

    if (mWindow)
        glfwSetWindowAttrib(mWindow, GLFW_RESIZABLE, resizeValue);
}

//*************************************************************************************************
void WindowSystem::SetDecorated(bool decorated) const
{
    int decoratedValue;
    if (decorated)
        decoratedValue = GLFW_TRUE;
    else
        decoratedValue = GLFW_FALSE;

    if (mWindow)
        glfwSetWindowAttrib(mWindow, GLFW_DECORATED, decoratedValue);
}

//*************************************************************************************************
void WindowSystem::SetCursorVisible(bool visible) const
{
    int visibleValue;
    if (visible)
        visibleValue = GLFW_CURSOR_NORMAL;
    else
        visibleValue = GLFW_CURSOR_HIDDEN;

    if (mWindow)
        glfwSetInputMode(mWindow, GLFW_CURSOR, visibleValue);
}

//*************************************************************************************************
void WindowSystem::CreateConsole() const
{
#ifdef _WIN32
    // Only create a console if the program doesn't already have one
    if (GetConsoleWindow() != nullptr || !AllocConsole())
        return;

    // Set it up to show stdout and stderr
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
#endif
}

//*************************************************************************************************
void WindowSystem::SetWindowIcon(const char* fileName) const
{
    // Load the image as 8-bit RGBA pixels
    int width, height, channels;
    unsigned char* pixels = stbi_load(fileName, &width, &height, &channels, 4);
    if (!pixels)
    {
        std::string msg = "Failed to load window icon file \"";
        msg += fileName;
        msg += "\": ";
        msg += stbi_failure_reason();
        gError->SetError(msg);
        return;
    }

    GLFWimage image{ width, height, pixels };
    glfwSetWindowIcon(mWindow, 1, &image);

    stbi_image_free(pixels);
}

//*************************************************************************************************
void WindowSystem::SetCallbacks() const
{
    glfwSetKeyCallback(mWindow, KeyCallback);
    glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
    glfwSetScrollCallback(mWindow, ScrollCallback);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferSizeCallback);
}

} // namepspace DGL

using namespace DGL;

//*************************************************************************************************
bool DGL_Window_HasFocus(void)
{
    return gWinSys->GetWindowHasFocus();
}

//*************************************************************************************************
DGL_Vec2 DGL_Window_GetMonitorSize(void)
{
    return gWinSys->GetMonitorSize();
}

//*************************************************************************************************
DGL_Vec2 DGL_Window_GetSize(void)
{
    return gWinSys->GetWindowSize();
}

//*************************************************************************************************
void DGL_Window_SetSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, int windowTopPos)
{
    gWinSys->SetWindowSizeAndPosition(windowWidth, windowHeight, windowLeftPos, windowTopPos);
}

//*************************************************************************************************
void DGL_Window_SetSize(int windowWidth, int windowHeight)
{
    gWinSys->SetWindowSize(windowWidth, windowHeight);
}

//*************************************************************************************************
void DGL_Window_SetFullscreen(bool fullscreen)
{
    gWinSys->SetFullscreen(fullscreen);
}

//*************************************************************************************************
bool DGL_Window_IsFullscreen(void)
{
    return gWinSys->GetIsFullscreen();
}

//*************************************************************************************************
void DGL_Window_SetResizable(bool resizable)
{
    gWinSys->SetResizable(resizable);
}

//*************************************************************************************************
void DGL_Window_SetDecorated(bool decorated)
{
    gWinSys->SetDecorated(decorated);
}
