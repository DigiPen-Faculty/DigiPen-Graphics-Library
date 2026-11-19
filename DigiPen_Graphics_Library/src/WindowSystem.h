//-------------------------------------------------------------------------------------------------
// file:    WindowSystem.h
// author:  Andy Ellinger
// brief:   Windows system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace DGL
{

//----------------------------------------------------------------------------------- WindowSystem

class WindowSystem
{
public:
    // Sets the global variable
    WindowSystem();

    // Initializes GLFW and creates the window. Returns false if there was an error.
    bool Initialize(const DGL_SysInitInfo& sysInitInfo);

    // Processes all of the Windows messages
    void Update();

    // Un-registers the window class
    void ShutDown();

    // Tells the window to close
    void Close();

    // Returns the GLFW window
    GLFWwindow* GetWindow() const;

    // Returns the native window handle (an HWND on Windows)
    void* GetNativeHandle() const;

    // Returns true if the window currently exists
    bool GetWindowExists() const;

    // Returns true if this window currently has focus
    bool GetWindowHasFocus() const;

    // Returns the size of the monitor the window is on
    DGL_Vec2 GetMonitorSize() const;

    // Returns the size of the drawable area of the window, in pixels
    DGL_Vec2 GetWindowSize() const;

    // Returns the position of the mouse cursor relative to the top left corner of the window, in pixels
    DGL_Vec2 GetCursorPosition() const;

    // Sets the window size and the position of the top left corner
    void SetWindowSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, 
        int windowTopPos) const;

    // Sets the window size, keeping it at its current position
    void SetWindowSize(int windowWidth, int windowHeight) const;

    // Switches between fullscreen and windowed mode
    void SetFullscreen(bool fullscreen);

    // Returns true if the window is currently fullscreen
    bool GetIsFullscreen() const;

    // Sets whether the user can resize the window
    void SetResizable(bool resizable) const;

    // Sets whether the window has a title bar and borders
    void SetDecorated(bool decorated) const;

    // Shows or hides the mouse cursor while it is over the window
    void SetCursorVisible(bool visible) const;

private:
    // Creates a console window for stdout and stderr, if there isn't one already
    void CreateConsole() const;

    // Loads the image file and sets it as the window icon
    void SetWindowIcon(const char* fileName) const;

    // Sets the GLFW callback functions for the window
    void SetCallbacks() const;

    // The GLFW window object
    GLFWwindow* mWindow{ nullptr };
    // Whether GLFW has been initialized
    bool mGlfwInitialized{ false };
    // The position and size of the window before it was made fullscreen
    int mWindowedLeft{ 0 };
    int mWindowedTop{ 0 };
    int mWindowedWidth{ 0 };
    int mWindowedHeight{ 0 };
};

// Global variable for accessing this system
extern WindowSystem* gWinSys;

} // namepspace DGL
