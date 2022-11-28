//-------------------------------------------------------------------------------------------------
// file:    WindowsSystem.h
// author:  Andy Ellinger
// brief:   Windows system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include "Math.h"

namespace DGL
{

//----------------------------------------------------------------------------------- WindowsSystem

class WindowsSystem
{
public:
    // Sets the global variable
    WindowsSystem();

    // Creates the window
    HWND Initialize(const DGL_SysInitInfo* pSysInitInfo);

    // Processes all of the Windows messages
    void Update();

    // Un-registers the window class
    void ShutDown();

    // Returns the handle of the window
    HWND GetWindowHandle() const;

    // Returns true if the window currently exists
    bool GetWindowExists() const;

    // Returns true if this window currently has focus
    bool GetWindowHasFocus() const;

    // Returns the size of the user's monitor
    DGL_Vec2 GetMonitorSize() const;

    // Returns the size of the window
    DGL_Vec2 GetWindowSize() const;

    // Sets the window size and the position of the top left corner
    void SetWindowSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, 
        int windowTopPos) const;

    // Sets the window size, keeping it at its current position
    void SetWindowSize(int windowWidth, int windowHeight) const;

    // Sets the style on the current window
    void SetWindowStyle(unsigned style) const;

    // Handles specific Windows messages that DGL cares about
    BOOL HandleWindowsMessage(UINT message, WPARAM wParam, LPARAM lParam, int* result);

private:
    // The name used for the window class
    const char* cWindowClassName{ "WindowClass" };
    // The handle of the current window
    HWND mWindowHandle{ nullptr };
    // The instance for this application
    HINSTANCE mInstance{ nullptr };
    // Keeps track of whether the window is currently being resized
    bool mResizing{ false };
};

// Global variable for accessing this system
extern WindowsSystem* gWinSys;

} // namepspace DGL
