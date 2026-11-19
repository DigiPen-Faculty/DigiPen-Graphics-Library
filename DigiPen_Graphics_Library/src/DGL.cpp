//-------------------------------------------------------------------------------------------------
// file:    DGL.cpp
// author:  Andy Ellinger
// brief:   Base DGL functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"
#include <chrono>

#include "Error.h"
#include "FrameRateController.h"
#include "InputSystem.h"
#include "GraphicsSystem.h"
#include "WindowSystem.h"

DGL::GraphicsSystem graphics;
DGL::FrameRateController frameController;
DGL::WindowSystem winSys;
DGL::InputSystem input;
DGL::ErrorHandler error;

//*************************************************************************************************
bool DGL_System_Init(const DGL_SysInitInfo& sysInitInfo)
{
    // Initialize the window and check for an error
    if (!winSys.Initialize(sysInitInfo))
    {
        winSys.ShutDown();
        return false;
    }

    // Initialize graphics and check for an error
    if (!graphics.Initialize(sysInitInfo))
    {
        winSys.ShutDown();
        return false;
    }

    // Initialize the frame rate controller with the provided max frame rate
    frameController.Initialize(sysInitInfo.maxFrameRate);

    return true;
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
void DGL_System_Exit(void)
{
    // Save the return value from shutting down graphics
    graphics.ShutDown();
    // Shut down the window
    winSys.ShutDown();
}

//*************************************************************************************************
bool DGL_System_DoesWindowExist(void)
{
    return winSys.GetWindowExists();
}

//*************************************************************************************************
void DGL_System_CloseWindow(void)
{
    winSys.Close();
}

//*************************************************************************************************
void* DGL_System_GetNativeWindowHandle(void)
{
    return winSys.GetNativeHandle();
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
    // Use a steady (monotonic) high-resolution clock, converted to seconds
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration<double>(now).count();
}
