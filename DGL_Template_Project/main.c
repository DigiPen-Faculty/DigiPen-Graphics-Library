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

    switch (message)
    {
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    DGL_SysInitInfo initInfo;
    initInfo.mAppInstance = hInstance;
    initInfo.mClassStyle = CS_HREDRAW | CS_VREDRAW;
    initInfo.mMaxFrameRate = 60;
    initInfo.mShow = nCmdShow;
    initInfo.mWindowStyle = WS_OVERLAPPEDWINDOW;
    initInfo.mWindowTitle = "Game Window";
    initInfo.mWindowHeight = 768;
    initInfo.mWindowWidth = 1024;
    initInfo.mCreateConsole = FALSE;
    initInfo.pWindowsCallback = WndProc;
    initInfo.mWindowIcon = IDI_DGLTEMPLATE;

    HWND windowHandle = DGL_System_Init(&initInfo);
    if (windowHandle == NULL)
        return 1;

    DGL_Color color = { 0.1f, 0.1f, 0.1f, 1.0f };
    DGL_Graphics_SetBackgroundColor(&color);

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
