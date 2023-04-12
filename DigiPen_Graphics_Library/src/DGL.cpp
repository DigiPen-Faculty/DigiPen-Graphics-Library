//-------------------------------------------------------------------------------------------------
// file:    DGL.cpp
// author:  Andy Ellinger
// brief:   Base DGL functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"

import Errors;
import FrameRateControl;
import Input;
import GraphicsSystem;
import WindowsSystem;

DGL::GraphicsSystem graphics;
DGL::FrameRateController frameController;
DGL::WindowsSystem winSys;
DGL::InputSystem input;
DGL::ErrorHandler error;


//*************************************************************************************************
HWND DGL_System_Init(const DGL_SysInitInfo* sysInitInfo)
{
    // Initialize the window and save the window handle
    HWND handle = winSys.Initialize(sysInitInfo);
    // Check for an error
    if (handle == NULL)
        return NULL;

    // Initialize graphics and check for an error
    if (graphics.Initialize(handle) != 0)
        return NULL;

    // Initialize the frame rate controller with the provided max frame rate
    frameController.Initialize(sysInitInfo->mMaxFrameRate);

    // Return the window handle
    return handle;
}

//*************************************************************************************************
void DGL_System_Reset(void)
{
    // Reset the frame rate controller
    frameController.Reset();
}

//*************************************************************************************************
void DGL_System_FrameControl(void)
{
    // Delay to match the desired frame rate (if necessary)
    frameController.StartFrame();
}

//*************************************************************************************************
void DGL_System_Update(void)
{
    // Update the input system
    // This should always happen before updating the window
    // (previous input data needs to be saved before handling Windows messages)
    input.Update();

    // Update the window system
    winSys.Update();
}

//*************************************************************************************************
BOOL DGL_System_HandleWindowsMessage(UINT message, WPARAM wParam, LPARAM lParam, int* result)
{
    return winSys.HandleWindowsMessage(message, wParam, lParam, result);
}

//*************************************************************************************************
int DGL_System_Exit(void)
{
    // Save the return value from shutting down graphics
    int returnValue = graphics.ShutDown();
    // Shut down the window
    winSys.ShutDown();
    // Return the value from graphics
    return returnValue;
}

//*************************************************************************************************
BOOL DGL_System_DoesWindowExist(void)
{
    return winSys.GetWindowExists();
}

//*************************************************************************************************
HWND DGL_System_GetWindowHandle(void)
{
    return winSys.GetWindowHandle();
}

//*************************************************************************************************
double DGL_System_GetDeltaTime(void)
{
    return frameController.GetDeltaTime();
}

//*************************************************************************************************
unsigned DGL_System_GetFrameCount(void)
{
    return frameController.GetFrameCount();
}

//*************************************************************************************************
double DGL_System_GetTime(void)
{
    signed long long countsPerSecond, t;
    double r, r0, r1;

    QueryPerformanceFrequency((LARGE_INTEGER*)(&countsPerSecond));
    QueryPerformanceCounter((LARGE_INTEGER*)(&t));

    r0 = (double)(t / countsPerSecond);
    r1 = (t - ((t / countsPerSecond) * countsPerSecond)) / (double)(countsPerSecond);
    r = r0 + r1;

    return r;
}
