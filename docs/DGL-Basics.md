# How to Use the DigiPen Graphics Library

This is currently a work in progress

---

## Initial Setup

Add this code to the Windows callback function (`WndProc`):

    int result;
    if (DGL_System_HandleWindowsMessage(message, wParam, lParam, &result))
        return result;


The first thing we need to do is set up the `DGL_SysInitInfo` struct used to pass data to DGL when it's initialized. You can use the following values as a starting point. See the [DGL_SysInitInfo](Types/#dgl_sysinitinfo) documentation for more explanation of each setting.

    DGL_SysInitInfo initInfo;
    initInfo.mAppInstance = hInstance;
    initInfo.mClassStyle = CS_HREDRAW | CS_VREDRAW;
    initInfo.mMaxFrameRate = 60;
    initInfo.mShow = nCmdShow;
    initInfo.mWindowStyle = WS_OVERLAPPEDWINDOW;
    initInfo.mWindowTitle = "Game Window";          // Change to your window title
    initInfo.mWindowHeight = 768;                   // Change to your window height
    initInfo.mWindowWidth = 1024;                   // Change to your window width
    initInfo.mCreateConsole = FALSE;
    initInfo.pWindowsCallback = WndProc;
    initInfo.mWindowIcon = IDI_AWESOMEGAME;         // Change AWESOMEGAME to the name of your project


Now that we have the struct set up we can initialize the DGL systems by calling [DGL_System_Init](System/#dgl_system_init). If the function returns NULL, something has gone wrong, and you should exit the program. You can check what the error was by calling [DGL_System_GetLastError](System/#dgl_system_getlasterror).

    HWND windowHandle = DGL_System_Init(&initInfo);
    if (windowHandle == NULL)
        return 1;


If you want a background color that is not black, you can set it using a [DGL_Color](Types/#dgl_color) struct.

    DGL_Color color = { 0.1f, 0.1f, 0.1f, 1.0f };
    DGL_Graphics_SetBackgroundColor(&color);


Now that the setup is finished, you can start your game loop. 

- The very first thing you do on each update should be calling [DGL_System_FrameControl](System/#dgl_system_framecontrol) and [DGL_System_Update](System/#dgl_system_update).

- Before drawing any objects you must call [DGL_Graphics_StartDrawing](Graphics/#dgl_graphics_startdrawing), and you must call [DGL_Graphics_FinishDrawing](Graphics/#dgl_graphics_finishdrawing) when you are done drawing on this frame.

- You should always call [DGL_System_DoesWindowExist](System/#dgl_system_doeswindowexist) to check if you should break out of the game loop, in addition to checking for any other ways you want the user to exit the program. This will let you stop the program if the user has closed the window manually.

A very simple version of the game update could look like this:

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


Once the game has finished you need to shut down the DGL systems.

    DGL_System_Exit();

    return 0;

---

## Drawing an Image

Blend mode

    DGL_Graphics_SetBlendMode(DGL_BM_BLEND);

Creating a mesh

    DGL_Mesh* CreateMesh(float width, float height)
    {
        float halfWidth = width / 2.0f;
        float halfHeight = width / 2.0f;

        DGL_Graphics_StartMesh();

        DGL_Vec2 pos = { -halfWidth, -halfHeight };
        DGL_Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
        DGL_Vec2 uv = { 0.0f, 1.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ -halfWidth, halfHeight };
        uv = (DGL_Vec2){ 0.0f, 0.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ halfWidth, halfHeight };
        uv = (DGL_Vec2){ 1.0f, 0.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ -halfWidth, -halfHeight };
        uv = (DGL_Vec2){ 0.0f, 1.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ halfWidth, halfHeight };
        uv = (DGL_Vec2){ 1.0f, 0.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ halfWidth, -halfHeight };
        uv = (DGL_Vec2){ 1.0f, 1.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        return DGL_Graphics_EndMesh();
    }

    
    DGL_Mesh* mesh = CreateMesh(1.0f, 1.0f);

Loading a texture

    DGL_Texture* ballTexture = DGL_Graphics_LoadTexture("./Assets/ball.png");

Drawing the object

    void Draw(DGL_Mesh* mesh, DGL_Texture* texture, DGL_Vec2 position, DGL_Vec2 scale, float rotation)
    {
        DGL_Graphics_SetShaderMode(DGL_SM_TEXTURE);

        DGL_Graphics_SetCB_TransformData(&position, &scale, rotation);

        DGL_Graphics_SetTexture(texture);

        DGL_Graphics_DrawMesh(mesh, DGL_DM_TRIANGLELIST);
    }

    Draw(mesh, ballTexture, (DGL_Vec2) { 0.0f, 0.0f }, (DGL_Vec2) { 50.0f, 50.0f }, 0.0f);

---

## Sprite Sheets

Add columns and rows to creating the mesh

    DGL_Mesh* CreateMesh(float width, float height, float columns, float rows)
    {
        float halfWidth = width / 2.0f;
        float halfHeight = width / 2.0f;
        float uWidth = 1.0f / columns;
        float vHeight = 1.0f / rows;

        DGL_Graphics_StartMesh();

        DGL_Vec2 pos = { -halfWidth, -halfHeight };
        DGL_Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
        DGL_Vec2 uv = { 0.0f, vHeight };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ -halfWidth, halfHeight };
        uv = (DGL_Vec2){ 0.0f, 0.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ halfWidth, halfHeight };
        uv = (DGL_Vec2){ uWidth, 0.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ -halfWidth, -halfHeight };
        uv = (DGL_Vec2){ 0.0f, vHeight };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ halfWidth, halfHeight };
        uv = (DGL_Vec2){ uWidth, 0.0f };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        pos = (DGL_Vec2){ halfWidth, -halfHeight };
        uv = (DGL_Vec2){ uWidth, vHeight };
        DGL_Graphics_AddVertex(&pos, &color, &uv);

        return DGL_Graphics_EndMesh();
    }

    
    DGL_Mesh* ssMesh = CreateMesh(1.0f, 1.0f, 5, 1);

Keep track of frames

    if (DGL_System_GetTime() - lastTime > 0.25f)
    {
        lastTime = DGL_System_GetTime();
        animFrame = (animFrame + 1) % maxFrames;
    }

Set the texture offset

    DGL_Vec2 texOffset = { 0,0 };
    texOffset.x = (1.0f / maxFrames) * animFrame;
    DGL_Graphics_SetCB_TextureOffset(&texOffset);