//-------------------------------------------------------------------------------------------------
// file:    main.c
// author:  
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Resource.h"

#include "DGL.h"

LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    int result;
    if (DGL_System_HandleWindowsMessage(message, wParam, lParam, &result))
        return result;

    //switch (message)
    //{
    //default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    //}
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    lpCmdLine;
    hPrevInstance;
    DGL_SysInitInfo initInfo;
    initInfo.appInstance = hInstance;
    initInfo.classStyle = CS_HREDRAW | CS_VREDRAW;
    initInfo.maxFrameRate = 60;
    initInfo.show = nCmdShow;
    initInfo.windowStyle = WS_OVERLAPPEDWINDOW;
    initInfo.windowTitle = "Awesome Game";
    initInfo.windowSize.y = 768;
    initInfo.windowSize.x = 1024;
    initInfo.createConsole = TRUE;
    initInfo.windowsCallback = WndProc;
    initInfo.windowIcon = IDI_DGLTEMPLATE;

    HWND windowHandle = DGL_System_Init(initInfo);
    if (windowHandle == NULL)
        return 1;

    DGL_Graphics_SetBackgroundColor({ 0.1f, 0.1f, 0.1f, 1.0f });

    int running = TRUE;
    while (running)
    {
        DGL_System_FrameControl();
        DGL_System_Update();

        DGL_Graphics_StartDrawing();

        DGL_Graphics_FinishDrawing();

        if (!DGL_System_DoesWindowExist())
            running = FALSE;
    }

    DGL_System_Exit();

    return 0;
}
