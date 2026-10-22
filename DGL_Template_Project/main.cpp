//-------------------------------------------------------------------------------------------------
// file:    main.c
// author:  
// brief:   
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"

int main(void)
{
    DGL_SysInitInfo initInfo;
    initInfo.maxFrameRate = 60;
    initInfo.windowTitle = "Awesome Game";
    initInfo.windowSize.x = 1024;
    initInfo.windowSize.y = 768;
    initInfo.createConsole = true;
    // To use a custom window icon, set this to the path of an image file:
    //initInfo.windowIconFile = "icon.png";

    if (!DGL_System_Init(initInfo))
        return 1;

    DGL_Graphics_SetBackgroundColor({ 0.1f, 0.1f, 0.1f, 1.0f });

    int running = true;
    while (running)
    {
        DGL_System_FrameControl();
        DGL_System_Update();

        DGL_Graphics_StartDrawing();

        DGL_Graphics_FinishDrawing();

        if (!DGL_System_DoesWindowExist())
            running = false;
    }

    DGL_System_Exit();

    return 0;
}
