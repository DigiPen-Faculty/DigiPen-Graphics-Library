This page contains all the functions in the System section.

# Table Of Contents

- [DGL_System_DoesWindowExist](#dgl_system_doeswindowexist)
- [DGL_System_Exit](#dgl_system_exit)
- [DGL_System_FrameControl](#dgl_system_framecontrol)
- [DGL_System_GetDeltaTime](#dgl_system_getdeltatime)
- [DGL_System_GetFrameCount](#dgl_system_getframecount)
- [DGL_System_GetLastError](#dgl_system_getlasterror)
- [DGL_System_GetTime](#dgl_system_gettime)
- [DGL_System_GetWindowHandle](#dgl_system_getwindowhandle)
- [DGL_System_HandleWindowsMessage](#dgl_system_handlewindowsmessage)
- [DGL_System_Init](#dgl_system_init)
- [DGL_System_Reset](#dgl_system_reset)
- [DGL_System_Update](#dgl_system_update)

# DGL_System_DoesWindowExist

Returns FALSE if there is no window, and TRUE if there is. This is used to check if you should break out of your game loop and exit.

## Function

```C
BOOL DGL_System_DoesWindowExist(void)
```

### Parameters

- This function has no parameters.

### Return

- BOOL - This will be FALSE if there is no window, and TRUE if there is.

## Example

```C
if (!DGL_System_DoesWindowExist())
    gameRunning = FALSE;
```

## Related

- None

# DGL_System_Exit

Shuts down the graphics system and de-registers the window. Will return 1 if there were any problems, such as unreleased meshes and textures, otherwise returns 0.

## Function

```C
int DGL_System_Exit(void)
```

### Parameters

- This function has no parameters.

### Return

- int - Will return 1 if there were any problems, such as unreleased meshes and textures, otherwise returns 0.

## Example

```C
DGL_System_Exit();
```

## Related

- None

# DGL_System_FrameControl

Tells the framerate controller a new frame is beginning, and delays if necessary to match max framerate.

This must be called at the beginning of every update loop.

## Function

```C
void DGL_System_FrameControl(void)
```

### Parameters

- This function has no parameters.

### Return

- This function does not return anything.

## Example

```C
int running = 1;
while (running)
{
    DGL_System_FrameControl();
    DGL_System_Update();

    // Update the game

    if (!DGL_System_DoesWindowExist() || DGL_Input_KeyTriggered(VK_ESCAPE))
        running = 0;
}
```

## Related

- None

# DGL_System_GetDeltaTime

Returns the time between the beginning of the last frame and the beginning of this frame, in seconds. To be more accurate, it is the time between when [DGL_System_FrameControl](#dgl_system_framecontrol) was called on the last frame and on this frame.

## Function

```C
double DGL_System_GetDeltaTime(void)
```

### Parameters

- This function has no parameters.

### Return

- double - The time between the beginning of the last frame and the beginning of this frame, in seconds.

## Example

```C
playerPosition += playerVelocity * DGL_System_GetDeltaTime();
```

## Related

- [DGL_System_FrameControl](#dgl_system_framecontrol)
- [DGL_System_GetTime](#dgl_system_gettime)

# DGL_System_GetFrameCount

Returns the total number of frames since the program began. This number will be reset to 0 after calling [DGL_System_Reset](#dgl_system_reset).

## Function

```C
unsigned DGL_System_GetFrameCount(void)
```

### Parameters

- This function has no parameters.

### Return

- unsigned - The total number of frames since the program began or since [DGL_System_Reset](#dgl_system_reset) was called.

## Example

```C
unsigned totalFrames = DGL_System_GetFrameCount();
```

## Related

- [DGL_System_Reset](#dgl_system_reset)

# DGL_System_GetLastError

Returns the text of the last error in the DGL systems.

## Function

```C
const char* DGL_System_GetLastError(void)
```

### Parameters

- This function has no parameters.

### Return

- const char* - This will contain text describing the last error that occurred in the DGL.

## Example

```C
HWND windowHandle = DGL_System_Init(&initInfo);
if (!windowHandle)
{
    printf(DGL_System_GetLastError());
    return 1;
}
```

## Related

- None

# DGL_System_GetTime

Returns the current system time according to the user's computer, in seconds.

## Function

```C
double DGL_System_GetTime(void)
```

### Parameters

- This function has no parameters.

### Return

- double - The current system time, in seconds.

## Example

```C
double currentTime = DGL_System_GetTime();
if (currentTime - lastSavedTime > 3.0)
{
    lastSavedTime = currentTime;
    // Do some timed logic
}
```

## Related

- None

# DGL_System_GetWindowHandle

Returns the Windows window handle, which can be used to manipulate the window or to get data from Windows.

## Function

```C
HWND DGL_System_GetWindowHandle(void)
```

### Parameters

- This function has no parameters.

### Return

- HWND - The handle of the application window.

## Example

```C
SetWindowPos(DGL_System_GetWindowHandle(), HWND_TOP, rect.left, rect.top, windowWidth, windowHeight, SWP_NOZORDER);
ShowWindow(DGL_System_GetWindowHandle(), SW_SHOWNORMAL);
```

## Related

- None

# DGL_System_HandleWindowsMessage

Handles some of the windows messages. Returns TRUE if the message was handled, otherwise returns FALSE. If the message was handled, use the `result` value to return from the Windows callback.

## Function

```C
BOOL DGL_System_HandleWindowsMessage(UINT message, WPARAM wParam, LPARAM lParam, int* result)
```

### Parameters

- message (UINT) - You should pass in the message parameter from the Windows callback here.
- wParam (WPARAM) - You should pass in the wParam parameter from the Windows callback here.
- lParam (LPARAM) - You should pass in the lParam parameter from the Windows callback here.
- result (int*) - The address of an `int` variable where the result will be stored. If the function returns TRUE, use this value to return from the callback.

### Return

- BOOL - This will be TRUE if the message was handled, otherwise it will be FALSE.

## Example

```C
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
```

## Related

- None

# DGL_System_Init

Initializes the DGL using the specified options. This will create the window and initialize graphics and the framerate controller. Returns the window handle, which will be NULL if there was a problem with initialization.

## Function

```C
HWND DGL_System_Init(const DGL_SysInitInfo* sysInitInfo)
```

### Parameters

- sysInitInfo (const DGL_SysInitInfo*) - The address of a [DGL_SysInitInfo](Types/#dgl_sysinitinfo) struct with the settings to use when initializing the DGL.

### Return

- HWND - The handle of the window that was created. If not successful, this will be NULL.

## Example

```C
HWND windowHandle = DGL_System_Init(&initInfo);
if (windowHandle == NULL)
{
    printf(DGL_System_GetLastError());
    return 1;
}
```

## Related

- [DGL_SysInitInfo](Types/#dgl_sysinitinfo)

# DGL_System_Reset

Resets the framerate controller back to the initial state.

## Function

```C
void DGL_System_Reset(void)
```

### Parameters

- This function has no parameters.

### Return

- This function does not return anything.

## Example

```C
DGL_System_Reset();
```

## Related

- [DGL_System_FrameControl](#dgl_system_framecontrol)
- [DGL_System_GetFrameCount](#dgl_system_getframecount)

# DGL_System_Update

Updates input and Windows messages. Should be called near the beginning of the update loop.

## Function

```C
void DGL_System_Update(void)
```

### Parameters

- This function has no parameters.

### Return

- This function does not return anything.

## Example

```C
int running = 1;
while (running)
{
    DGL_System_FrameControl();
    DGL_System_Update();

    // Update the game

    if (!DGL_System_DoesWindowExist() || DGL_Input_KeyTriggered(VK_ESCAPE))
        running = 0;
}
```

## Related

- None