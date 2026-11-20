//-------------------------------------------------------------------------------------------------
// file:    main.cpp
// brief:   DGL feature test.
//
//          Run with no arguments for an interactive test (the controls are printed to the console).
//          Run with "--selftest" to run automated checks of the window and input systems. Input is
//          sent directly to the test window, so it does not affect other programs. Returns the
//          number of failed checks.
//          Run with "--capture <seconds>" to show the test scene without changes for that long,
//          so a screenshot can be taken and compared between DGL versions.
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"
#include "TestScene.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Windows is only used by the self-test to send input to the window and check window state
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace
{

const int cWindowWidth = 1024;
const int cWindowHeight = 768;

TestScene gScene;

//*************************************************************************************************
// Runs one frame, drawing the test scene plus an optional extra square
void RunFrame(const DGL_Vec2* extraSquarePos = nullptr, const DGL_Color* extraSquareColor = nullptr)
{
    DGL_System_FrameControl();
    DGL_System_Update();

    DGL_Graphics_StartDrawing();
    TestScene_Draw(gScene);
    if (extraSquarePos && extraSquareColor)
    {
        DGL_Graphics_SetTexture(gScene.whiteTexture);
        DGL_Graphics_SetCB_TintColor(*extraSquareColor);
        DGL_Graphics_SetCB_TransformData(*extraSquarePos, { 30.0f, 30.0f }, 0.0f);
        DGL_Graphics_DrawMesh(gScene.whiteSquare, DGL_DM_TRIANGLELIST);
        DGL_Graphics_SetCB_TintColor({ 1.0f, 1.0f, 1.0f, 1.0f });
    }
    DGL_Graphics_FinishDrawing();
}

//-------------------------------------------------------------------------------------- Self test

int gFailures = 0;

//*************************************************************************************************
void Check(bool condition, const char* description)
{
    printf("[%s] %s\n", condition ? "PASS" : "FAIL", description);
    if (!condition)
        ++gFailures;
}

//*************************************************************************************************
bool Near(float a, float b, float tolerance = 0.5f)
{
    return fabsf(a - b) <= tolerance;
}

//*************************************************************************************************
bool SizeIs(DGL_Vec2 size, int width, int height)
{
    return Near(size.x, (float)width) && Near(size.y, (float)height);
}

//*************************************************************************************************
// Runs frames until the window reaches the expected size (window changes can take a few frames)
bool WaitForSize(int width, int height)
{
    for (int i = 0; i < 60 && !SizeIs(DGL_Window_GetSize(), width, height); ++i)
        RunFrame();
    return SizeIs(DGL_Window_GetSize(), width, height);
}

//*************************************************************************************************
// Sends a key message directly to the window, the same way Windows does for a real key press
void PostKey(HWND window, UINT virtualKey, bool down, bool extended = false)
{
    UINT scanCode = MapVirtualKeyA(virtualKey, MAPVK_VK_TO_VSC);
    LPARAM lParam = 1 | (scanCode << 16) | (extended ? (1 << 24) : 0);
    if (!down)
        lParam |= (1u << 30) | (1u << 31);
    PostMessageA(window, down ? WM_KEYDOWN : WM_KEYUP, virtualKey, lParam);
}

//*************************************************************************************************
// Presses and releases a key, checking that DGL reports the expected key values
void CheckKey(HWND window, UINT virtualKey, unsigned char dglKey, const char* name, bool extended = false,
    unsigned char dglCombinedKey = 0)
{
    char description[128];

    PostKey(window, virtualKey, true, extended);
    RunFrame();
    bool down = DGL_Input_KeyDown(dglKey) && DGL_Input_KeyTriggered(dglKey);
    if (dglCombinedKey)
        down = down && DGL_Input_KeyDown(dglCombinedKey);
    snprintf(description, sizeof(description), "%s press is reported as down and triggered", name);
    Check(down, description);

    PostKey(window, virtualKey, false, extended);
    RunFrame();
    bool released = !DGL_Input_KeyDown(dglKey) && DGL_Input_KeyReleased(dglKey);
    if (dglCombinedKey)
        released = released && !DGL_Input_KeyDown(dglCombinedKey);
    snprintf(description, sizeof(description), "%s release is reported as released", name);
    Check(released, description);
}

//*************************************************************************************************
void CheckMouseButton(HWND window, UINT downMessage, UINT upMessage, unsigned char dglKey, const char* name)
{
    char description[128];

    PostMessageA(window, downMessage, 0, MAKELPARAM(10, 10));
    RunFrame();
    snprintf(description, sizeof(description), "%s mouse button press is reported", name);
    Check(DGL_Input_KeyDown(dglKey) && DGL_Input_KeyTriggered(dglKey), description);

    PostMessageA(window, upMessage, 0, MAKELPARAM(10, 10));
    RunFrame();
    snprintf(description, sizeof(description), "%s mouse button release is reported", name);
    Check(!DGL_Input_KeyDown(dglKey) && DGL_Input_KeyReleased(dglKey), description);
}

//*************************************************************************************************
bool NoErrors()
{
    const char* error = DGL_System_GetLastError();
    if (error && error[0])
    {
        printf("       Last DGL error: %s\n", error);
        return false;
    }
    return true;
}

//*************************************************************************************************
void RunSelfTest()
{
    HWND window = (HWND)DGL_System_GetNativeWindowHandle();
    Check(window != nullptr && IsWindow(window), "Native window handle is a valid window");
    Check(DGL_System_DoesWindowExist(), "Window exists after init");
    Check(SizeIs(DGL_Window_GetSize(), cWindowWidth, cWindowHeight), "Window drawable size matches the requested size");

    RECT clientRect;
    GetClientRect(window, &clientRect);
    Check(clientRect.right == cWindowWidth && clientRect.bottom == cWindowHeight,
        "Win32 client rect matches the requested size");

    DGL_Vec2 monitor = DGL_Window_GetMonitorSize();
    Check(monitor.x > 0 && monitor.y > 0, "Monitor size is reported");

    // Camera: screen (0,0) is the top left corner of the world
    DGL_Vec2 topLeft = DGL_Camera_ScreenCoordToWorld({ 0.0f, 0.0f });
    Check(Near(topLeft.x, -cWindowWidth / 2.0f) && Near(topLeft.y, cWindowHeight / 2.0f),
        "Camera screen-to-world uses the window size");

    // Frame timing
    for (int i = 0; i < 5; ++i)
        RunFrame();
    double start = DGL_System_GetTime();
    unsigned startFrame = DGL_System_GetFrameCount();
    for (int i = 0; i < 30; ++i)
        RunFrame();
    double averageFrame = (DGL_System_GetTime() - start) / (DGL_System_GetFrameCount() - startFrame);
    printf("       Average frame time: %.2f ms\n", averageFrame * 1000.0);
    Check(averageFrame > 0.012 && averageFrame < 0.030, "Frame rate controller limits to about 60 FPS");
    Check(DGL_System_GetDeltaTime() > 0.0 && DGL_System_GetDeltaTime() < 0.1, "Delta time is reasonable");

    // Keyboard
    CheckKey(window, 'A', 'A', "Letter A");
    CheckKey(window, '7', DGL_KEY_7, "Number 7");
    CheckKey(window, VK_SPACE, DGL_KEY_SPACE, "Space");
    CheckKey(window, VK_ESCAPE, DGL_KEY_ESCAPE, "Escape");
    CheckKey(window, VK_RETURN, DGL_KEY_ENTER, "Enter");
    CheckKey(window, VK_LEFT, DGL_KEY_LEFT, "Left arrow", true);
    CheckKey(window, VK_UP, DGL_KEY_UP, "Up arrow", true);
    CheckKey(window, VK_DELETE, DGL_KEY_DELETE, "Delete", true);
    CheckKey(window, VK_F5, DGL_KEY_F5, "F5");
    CheckKey(window, VK_NUMPAD3, DGL_KEY_NUMPAD_3, "Numpad 3");
    CheckKey(window, VK_OEM_MINUS, DGL_KEY_MINUS, "Minus");
    // Shift can't be tested this way: GLFW checks the physical key state after processing events and
    // releases any shift key that isn't physically down. (Alt is skipped too, since a posted Alt
    // release opens the window menu.) Shift and Alt use the same code as Control, so test with the
    // interactive mode.
    CheckKey(window, VK_CONTROL, DGL_KEY_LEFT_CONTROL, "Left control", false, DGL_KEY_CONTROL);
    CheckKey(window, VK_CONTROL, DGL_KEY_RIGHT_CONTROL, "Right control", true, DGL_KEY_CONTROL);

    PostKey(window, 'Q', true);
    RunFrame();
    Check(DGL_Input_LastKeyTriggered() == 'Q', "Last key triggered is reported");
    RunFrame();
    Check(DGL_Input_LastKeyTriggered() == 0, "Last key triggered resets on the next frame");
    PostKey(window, 'Q', false);
    RunFrame();

    // Mouse buttons
    CheckMouseButton(window, WM_LBUTTONDOWN, WM_LBUTTONUP, DGL_MOUSE_LEFT, "Left");
    CheckMouseButton(window, WM_RBUTTONDOWN, WM_RBUTTONUP, DGL_MOUSE_RIGHT, "Right");
    CheckMouseButton(window, WM_MBUTTONDOWN, WM_MBUTTONUP, DGL_MOUSE_MIDDLE, "Middle");

    // Mouse wheel
    PostMessageA(window, WM_MOUSEWHEEL, MAKEWPARAM(0, 2 * WHEEL_DELTA), 0);
    RunFrame();
    Check(DGL_Input_GetMouseScrollDelta() == 2, "Scrolling up two steps gives a scroll delta of 2");
    RunFrame();
    Check(DGL_Input_GetMouseScrollDelta() == 0, "Scroll delta resets on the next frame");
    PostMessageA(window, WM_MOUSEWHEEL, MAKEWPARAM(0, (WORD)(short)-WHEEL_DELTA), 0);
    RunFrame();
    Check(DGL_Input_GetMouseScrollDelta() == -1, "Scrolling down one step gives a scroll delta of -1");
    PostMessageA(window, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA / 2), 0);
    RunFrame();
    bool halfStepIgnored = DGL_Input_GetMouseScrollDelta() == 0;
    PostMessageA(window, WM_MOUSEWHEEL, MAKEWPARAM(0, WHEEL_DELTA / 2), 0);
    RunFrame();
    Check(halfStepIgnored && DGL_Input_GetMouseScrollDelta() == 1, "Two half scroll steps add up to one step");

    // Mouse position (compared with Windows, without moving the cursor)
    RunFrame();
    POINT cursor;
    GetCursorPos(&cursor);
    ScreenToClient(window, &cursor);
    DGL_Vec2 mousePos = DGL_Input_GetMousePosition();
    Check(Near(mousePos.x, (float)cursor.x, 1.0f) && Near(mousePos.y, (float)cursor.y, 1.0f),
        "Mouse position matches the cursor position relative to the window");

    // Cursor visibility (can't be checked automatically, but must not cause errors)
    DGL_Input_ShowCursor(false);
    RunFrame();
    DGL_Input_ShowCursor(true);
    RunFrame();

    // Window size and position
    DGL_Window_SetSize(800, 600);
    Check(WaitForSize(800, 600), "SetSize changes the drawable size");
    DGL_Vec2 center = DGL_Camera_ScreenCoordToWorld({ 400.0f, 300.0f });
    topLeft = DGL_Camera_ScreenCoordToWorld({ 0.0f, 0.0f });
    Check(Near(center.x, 0.0f) && Near(center.y, 0.0f) && Near(topLeft.x, -400.0f) && Near(topLeft.y, 300.0f),
        "Camera updates for the new window size");

    DGL_Window_SetSizeAndPosition(640, 480, 120, 140);
    Check(WaitForSize(640, 480), "SetSizeAndPosition changes the drawable size");
    POINT origin{ 0, 0 };
    ClientToScreen(window, &origin);
    Check(origin.x == 120 && origin.y == 140, "SetSizeAndPosition moves the drawable area to the position");

    // Resizable and decorated
    DGL_Window_SetResizable(false);
    RunFrame();
    Check((GetWindowLongA(window, GWL_STYLE) & WS_THICKFRAME) == 0, "SetResizable(false) removes the resize border");
    DGL_Window_SetResizable(true);
    RunFrame();
    Check((GetWindowLongA(window, GWL_STYLE) & WS_THICKFRAME) != 0, "SetResizable(true) restores the resize border");
    DGL_Window_SetDecorated(false);
    RunFrame();
    Check((GetWindowLongA(window, GWL_STYLE) & WS_CAPTION) != WS_CAPTION, "SetDecorated(false) removes the title bar");
    DGL_Window_SetDecorated(true);
    RunFrame();
    Check((GetWindowLongA(window, GWL_STYLE) & WS_CAPTION) == WS_CAPTION, "SetDecorated(true) restores the title bar");
    Check(WaitForSize(640, 480), "Drawable size is unchanged after changing decorations");

    // Fullscreen
    Check(!DGL_Window_IsFullscreen(), "Window starts out not fullscreen");
    DGL_Window_SetFullscreen(true);
    monitor = DGL_Window_GetMonitorSize();
    Check(DGL_Window_IsFullscreen(), "SetFullscreen(true) makes the window fullscreen");
    Check(WaitForSize((int)monitor.x, (int)monitor.y), "Fullscreen drawable size matches the monitor size");
    for (int i = 0; i < 10; ++i)
        RunFrame();
    DGL_Window_SetFullscreen(false);
    Check(!DGL_Window_IsFullscreen(), "SetFullscreen(false) returns to a normal window");
    Check(WaitForSize(640, 480), "Returning from fullscreen restores the previous size");
    origin = { 0, 0 };
    ClientToScreen(window, &origin);
    Check(origin.x == 120 && origin.y == 140, "Returning from fullscreen restores the previous position");

    // Minimize and restore (drawing while minimized must not cause errors)
    ShowWindow(window, SW_MINIMIZE);
    for (int i = 0; i < 10; ++i)
        RunFrame();
    ShowWindow(window, SW_RESTORE);
    Check(WaitForSize(640, 480), "Restoring from minimized restores the size");

    // Back to the original size for the end of the test
    DGL_Window_SetSize(cWindowWidth, cWindowHeight);
    Check(WaitForSize(cWindowWidth, cWindowHeight), "Window returns to the original size");
    for (int i = 0; i < 10; ++i)
        RunFrame();

    Check(NoErrors(), "No DGL errors were reported");

    // Closing
    DGL_System_CloseWindow();
    Check(!DGL_System_DoesWindowExist(), "CloseWindow makes DoesWindowExist return false");
}

//*************************************************************************************************
void RunCapture(double seconds)
{
    double end = DGL_System_GetTime() + seconds;
    while (DGL_System_DoesWindowExist() && DGL_System_GetTime() < end)
        RunFrame();
}

//*************************************************************************************************
void RunInteractive()
{
    printf("Controls:\n"
        "  Arrow keys / WASD   move the yellow square\n"
        "  Mouse               the cyan square follows the mouse (left click turns it red)\n"
        "  Mouse wheel         zoom the camera\n"
        "  Q / E               rotate the camera\n"
        "  F11                 toggle fullscreen\n"
        "  C                   toggle cursor visibility\n"
        "  R                   toggle resizable\n"
        "  B                   toggle window borders\n"
        "  1 / 2               set window size to 800x600 / 1024x768\n"
        "  Escape              quit\n");

    DGL_Vec2 playerPos{ 0.0f, 80.0f };
    bool cursorVisible = true, resizable = true, decorated = true;
    DGL_Vec2 lastSize = DGL_Window_GetSize();

    while (DGL_System_DoesWindowExist())
    {
        DGL_System_FrameControl();
        DGL_System_Update();

        float speed = 300.0f * (float)DGL_System_GetDeltaTime();
        if (DGL_Input_KeyDown(DGL_KEY_LEFT) || DGL_Input_KeyDown('A')) playerPos.x -= speed;
        if (DGL_Input_KeyDown(DGL_KEY_RIGHT) || DGL_Input_KeyDown('D')) playerPos.x += speed;
        if (DGL_Input_KeyDown(DGL_KEY_UP) || DGL_Input_KeyDown('W')) playerPos.y += speed;
        if (DGL_Input_KeyDown(DGL_KEY_DOWN) || DGL_Input_KeyDown('S')) playerPos.y -= speed;

        if (int scroll = DGL_Input_GetMouseScrollDelta())
            DGL_Camera_SetZoom(DGL_Camera_GetZoom() * (scroll > 0 ? 0.9f : 1.1f));
        if (DGL_Input_KeyDown('Q')) DGL_Camera_SetRotation(DGL_Camera_GetRotation() + 0.02f);
        if (DGL_Input_KeyDown('E')) DGL_Camera_SetRotation(DGL_Camera_GetRotation() - 0.02f);

        if (DGL_Input_KeyTriggered(DGL_KEY_F11)) DGL_Window_SetFullscreen(!DGL_Window_IsFullscreen());
        if (DGL_Input_KeyTriggered('C')) DGL_Input_ShowCursor(cursorVisible = !cursorVisible);
        if (DGL_Input_KeyTriggered('R')) DGL_Window_SetResizable(resizable = !resizable);
        if (DGL_Input_KeyTriggered('B')) DGL_Window_SetDecorated(decorated = !decorated);
        if (DGL_Input_KeyTriggered('1')) DGL_Window_SetSize(800, 600);
        if (DGL_Input_KeyTriggered('2')) DGL_Window_SetSize(1024, 768);
        if (DGL_Input_KeyTriggered(DGL_KEY_ESCAPE)) DGL_System_CloseWindow();

        if (unsigned char key = DGL_Input_LastKeyTriggered())
            printf("Key triggered: 0x%02X\n", key);

        DGL_Vec2 size = DGL_Window_GetSize();
        if (!Near(size.x, lastSize.x) || !Near(size.y, lastSize.y))
        {
            printf("Window size: %.0f x %.0f\n", size.x, size.y);
            lastSize = size;
        }

        // Draw the scene with the player square, then the cursor square
        DGL_Color yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
        DGL_Graphics_StartDrawing();
        TestScene_Draw(gScene);
        DGL_Graphics_SetTexture(gScene.whiteTexture);
        DGL_Graphics_SetCB_TintColor(yellow);
        DGL_Graphics_SetCB_TransformData(playerPos, { 40.0f, 40.0f }, 0.0f);
        DGL_Graphics_DrawMesh(gScene.whiteSquare, DGL_DM_TRIANGLELIST);
        DGL_Color cursorColor = DGL_Input_KeyDown(DGL_MOUSE_LEFT) ? DGL_Color{ 1.0f, 0.0f, 0.0f, 1.0f }
            : DGL_Color{ 0.0f, 1.0f, 1.0f, 1.0f };
        DGL_Graphics_SetCB_TintColor(cursorColor);
        DGL_Graphics_SetCB_TransformData(DGL_Camera_ScreenCoordToWorld(DGL_Input_GetMousePosition()),
            { 20.0f, 20.0f }, 0.0f);
        DGL_Graphics_DrawMesh(gScene.whiteSquare, DGL_DM_TRIANGLELIST);
        DGL_Graphics_SetCB_TintColor({ 1.0f, 1.0f, 1.0f, 1.0f });
        DGL_Graphics_FinishDrawing();
    }
}

} // namespace

//*************************************************************************************************
int main(int argc, char** argv)
{
    bool selfTest = argc > 1 && strcmp(argv[1], "--selftest") == 0;
    bool capture = argc > 2 && strcmp(argv[1], "--capture") == 0;

    DGL_SysInitInfo initInfo;
    initInfo.maxFrameRate = 60;
    initInfo.windowTitle = "DGL Test";
    initInfo.windowSize = { (float)cWindowWidth, (float)cWindowHeight };
    initInfo.createConsole = !selfTest && !capture;

    if (!DGL_System_Init(initInfo))
    {
        printf("DGL_System_Init failed: %s\n", DGL_System_GetLastError());
        return 100;
    }

    DGL_Graphics_SetBackgroundColor({ 0.1f, 0.1f, 0.15f, 1.0f });

    if (!TestScene_Create(gScene))
    {
        printf("Failed to create the test scene: %s\n", DGL_System_GetLastError());
        DGL_System_Exit();
        return 101;
    }

    if (selfTest)
        RunSelfTest();
    else if (capture)
        RunCapture(atof(argv[2]));
    else
        RunInteractive();

    TestScene_Free(gScene);

    DGL_System_Exit();
    if (selfTest)
    {
        //Check(exitResult == 0, "DGL_System_Exit reports no unreleased resources");
        printf("%d check(s) failed\n", gFailures);
        return gFailures;
    }

    return 0;
}
