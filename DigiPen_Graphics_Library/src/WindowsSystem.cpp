//-------------------------------------------------------------------------------------------------
// file:    WindowsSystem.cpp
// author:  Andy Ellinger
// brief:   Windows system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <stdio.h>

module WindowsSystem;

import Errors;
import Input;
import GraphicsSystem;

namespace DGL
{

WindowsSystem* gWinSys = nullptr;

//----------------------------------------------------------------------------------- WindowsSystem

//*************************************************************************************************
WindowsSystem::WindowsSystem()
{
    gWinSys = this;
}

//*************************************************************************************************
HWND WindowsSystem::Initialize(const DGL_SysInitInfo* pSysInitInfo)
{
    // Check if we should create a console
    if (pSysInitInfo->mCreateConsole)
    {
        // Create the console
        AllocConsole();
        // Set it up to show stdout and stderr
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
    }

    // Save the instance
    mInstance = pSysInitInfo->mAppInstance;

    // Set up the struct used for registering the window class
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = pSysInitInfo->mClassStyle;
    wcex.lpfnWndProc = pSysInitInfo->pWindowsCallback;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = pSysInitInfo->mAppInstance;
    wcex.hIcon = LoadIcon(pSysInitInfo->mAppInstance, MAKEINTRESOURCE(pSysInitInfo->mWindowIcon));
    wcex.hCursor = LoadCursor(0, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = cWindowClassName;
    wcex.hIconSm = LoadIcon(pSysInitInfo->mAppInstance, MAKEINTRESOURCE(pSysInitInfo->mWindowIcon));

    // Register the window class and check the result
    if (!RegisterClassEx(&wcex))
    {
        gError->SetError("Failed to register window class. ", GetLastError());
        return NULL;
    }

    // Create the window
    mWindowHandle = CreateWindow(
        cWindowClassName,
        pSysInitInfo->mWindowTitle,
        pSysInitInfo->mWindowStyle,
        CW_USEDEFAULT, 
        CW_USEDEFAULT,
        pSysInitInfo->mWindowWidth,
        pSysInitInfo->mWindowHeight,
        NULL,
        NULL,
        pSysInitInfo->mAppInstance,
        NULL
    );
    // Check if the window was created successfully
    if (!mWindowHandle)
    {
        gError->SetError("Failed to create window. ", GetLastError());
        return NULL;
    }

    // Show and update the window
    ShowWindow(mWindowHandle, pSysInitInfo->mShow);
    UpdateWindow(mWindowHandle);

    // Return the window handle
    return mWindowHandle;
}

//*************************************************************************************************
void WindowsSystem::Update()
{
    // Process all of the Windows messages
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//*************************************************************************************************
void WindowsSystem::ShutDown()
{
    // Un-register the window class
    UnregisterClass(cWindowClassName, mInstance);
}

//*************************************************************************************************
HWND WindowsSystem::GetWindowHandle() const
{
    return mWindowHandle;
}

//*************************************************************************************************
bool WindowsSystem::GetWindowExists() const
{
    return mWindowHandle != nullptr;
}

//*************************************************************************************************
bool WindowsSystem::GetWindowHasFocus() const
{
    // Check if the handle of the active window is our window handle
    return GetActiveWindow() == mWindowHandle;
}

//*************************************************************************************************
DGL_Vec2 WindowsSystem::GetMonitorSize() const
{
    DGL_Vec2 size{ 0 };

    // Get an HMONITOR struct with data about the user's primary monitor. 
    // Check the MonitorFromWindow Microsoft documentation for options.
    HMONITOR monitor = MonitorFromWindow(mWindowHandle, MONITOR_DEFAULTTOPRIMARY);

    // Handle an error case where we don't get information.
    if (!monitor)
        return size;

    // Create a MONITORINFO struct and set its size (could be a MONITORINFOEX struct instead).
    MONITORINFO info;
    info.cbSize = sizeof(MONITORINFO);

    // Fill out the struct with the monitor info.
    GetMonitorInfo(monitor, &info);

    // Get the width and height based on the left/right and top/bottom edges.
    size.x = (float)(info.rcMonitor.right - info.rcMonitor.left);
    size.y = (float)(info.rcMonitor.bottom - info.rcMonitor.top);

    return size;
}

//*************************************************************************************************
DGL_Vec2 WindowsSystem::GetWindowSize() const
{
    if (!mWindowHandle)
        return DGL_Vec2{ 0 };

    // Get the window size from the window itself (this will account for title bar, borders, etc.)
    RECT rect;
    GetClientRect(mWindowHandle, &rect);

    // Return the width and height of the window
    return { (float)(rect.right - rect.left), (float)(rect.bottom - rect.top) };
}

//*************************************************************************************************
void WindowsSystem::SetWindowSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, 
    int windowTopPos) const
{
    if (!mWindowHandle)
        return;

    if (windowWidth <= 0 || windowHeight <= 0 || windowLeftPos < 0 || windowTopPos < 0)
    {
        gError->SetError("Passed invalid parameter for window size or position.");
        return;
    }

    // Set the size and position of the application window.
    // See the Microsoft documentation for SetWindowPos for information about the flag options.
    SetWindowPos(
        mWindowHandle,
        HWND_TOP,
        windowLeftPos,
        windowTopPos,
        windowWidth,
        windowHeight,
        SWP_NOZORDER);

    // Resets the window.
    ShowWindow(DGL_System_GetWindowHandle(), SW_SHOWNORMAL);
}

//*************************************************************************************************
void WindowsSystem::SetWindowSize(int windowWidth, int windowHeight) const
{
    if (!mWindowHandle)
        return;

    // Save the current positions of all four sides of the application window.
    RECT rect;
    GetWindowRect(mWindowHandle, &rect);

    // Set the size and position using the current left and top position
    SetWindowSizeAndPosition(windowWidth, windowHeight, rect.left, rect.top);
}

//*************************************************************************************************
void WindowsSystem::SetWindowStyle(unsigned style) const
{
    // Sets the window style.
    SetWindowLongPtr(mWindowHandle, GWL_STYLE, style);

    // Resets the window.
    ShowWindow(mWindowHandle, SW_SHOWNORMAL);
}

//*************************************************************************************************
BOOL WindowsSystem::HandleWindowsMessage(UINT message, WPARAM wParam, LPARAM lParam, int* result)
{
    UNREFERENCED_PARAMETER(lParam);

    *result = 0;

    switch (message)
    {
    case WM_DESTROY:
        // If the window has been destroyed, set that no window exists
        mWindowHandle = nullptr;
        break;
    case WM_QUIT:
        // If the application is supposed to quit, set that no window exists
        mWindowHandle = nullptr;
        break;
    case WM_MOVE:
        // Invalidate the rect to force a redraw
        InvalidateRect(mWindowHandle, NULL, FALSE);
        break;
    case WM_SIZE:
        // If the window has been maximized, set the window size 
        if (wParam == SIZE_MAXIMIZED)
            gGraphics->Camera.ResetWindowSize();
        // If the window has been restored and we are not resizing, set the window size
        if (wParam == SIZE_RESTORED && !mResizing)
            gGraphics->Camera.ResetWindowSize();
        break;
    case WM_SIZING:
        // Set that we are currently resizing
        mResizing = true;
        break;
    case WM_EXITSIZEMOVE:
        // If we have been resizing, set the window size with the saved values
        if (mResizing)
            gGraphics->Camera.ResetWindowSize();
        mResizing = false;
        break;
    case WM_KEYDOWN:
        // Set the key state to down
        gInput->SetKeyState((unsigned char)wParam, true);
        break;
    case WM_KEYUP:
        // Set the key state to up
        gInput->SetKeyState((unsigned char)wParam, false);
        break;
    case WM_SYSKEYDOWN:
        // Catch the F10 key
        if (wParam == VK_F10)
            gInput->SetKeyState(VK_F10, true);
        break;
    case WM_SYSKEYUP:
        // Catch the F10 key
        if (wParam == VK_F10)
            gInput->SetKeyState(VK_F10, false);
        break;
    case WM_MOUSEWHEEL:
        // Calculate the mouse scroll delta value
        gInput->SetMouseScrollDelta(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
        break;
    case WM_LBUTTONDOWN:
        // Check for the left mouse button being down
        gInput->SetKeyState(VK_LBUTTON, true);
        break;
    case WM_LBUTTONUP:
        // Check for the left mouse button being released
        gInput->SetKeyState(VK_LBUTTON, false);
        break;
    case WM_RBUTTONDOWN:
        // Check for the right mouse button being down
        gInput->SetKeyState(VK_RBUTTON, true);
        break;
    case WM_RBUTTONUP:
        // Check for the right mouse button being released
        gInput->SetKeyState(VK_RBUTTON, false);
        break;
    case WM_MBUTTONDOWN:
        // Check for the middle mouse button being down
        gInput->SetKeyState(VK_MBUTTON, true);
        break;
    case WM_MBUTTONUP:
        // Check for the middle mouse button being released
        gInput->SetKeyState(VK_MBUTTON, false);
        break;
    default:
        return FALSE;
        break;
    }

    return TRUE;
}

} // namepspace DGL

using namespace DGL;

//*************************************************************************************************
BOOL DGL_Window_HasFocus(void)
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
void DGL_Window_SetStyle(unsigned style)
{
    gWinSys->SetWindowStyle(style);
}
