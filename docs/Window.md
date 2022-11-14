This file includes all the functions in the Window section.

# Table Of Contents

- [DGL_Window_GetMonitorSize](#dgl_window_getmonitorsize)
- [DGL_Window_GetSize](#dgl_window_getsize)
- [DGL_Window_HasFocus](#dgl_window_hasfocus)
- [DGL_Window_SetSize](#dgl_window_setsize)
- [DGL_Window_SetSizeAndPosition](#dgl_window_setsizeandposition)
- [DGL_Window_SetStyle](#dgl_window_setstyle)

# DGL_Window_GetMonitorSize

Returns the width and height of the user's current monitor.

## Function

```C
DGL_Vec2 DGL_Window_GetMonitorSize(void)
```

### Parameters

- This function has no parameters.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - This will contain the width of the user's monitor in the X value and the height in the Y value.

## Example

```C
DGL_Vec2 monitorSize = DGL_Window_GetMonitorSize();
DGL_Window_SetSizeAndPosition(monitorSize.x, monitorSize.y, 0, 0);
```

## Related

- [DGL_Window_GetSize](#dgl_window_getsize)

# DGL_Window_GetSize

Returns the size of the client area of the window (not including title bar or borders).

## Function

```C
DGL_Vec2 DGL_Window_GetSize(void)
```

### Parameters

- This function has no parameters.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - This will contain the width of the window in the X value and the height in the Y value.

## Example

```C
DGL_Vec2 windowSize = DGL_Window_GetSize();
float center = windowSize.x / 2.0f;
```

## Related

- [DGL_Window_GetMonitorSize](#dgl_window_getmonitorsize)

# DGL_Window_HasFocus

Returns TRUE if the application window currently has focus, and FALSE if it does not.

This can be used to do things such as automatically pause and resume when the application window is minimized or un-selected.

## Function

```C
BOOL DGL_Window_HasFocus(void)
```

### Parameters

- This function has no parameters.

### Return

- BOOL - This will be TRUE if the application window currently has focus and FALSE if it does not.

## Example

```C
if (DGL_Window_HasFocus() == FALSE)
    PauseGame();
```

## Related

- None

# DGL_Window_SetSize

Changes the width and height of the application window, leaving it in the same position.

## Function

```C
void DGL_Window_SetSize(int windowWidth, int windowHeight)
```

### Parameters

- windowWidth (int) - The width to use for the new window size.
- windowHeight (int) - The height to use for the new window size.

### Return

- This function does not return anything.

## Example

```C
DGL_Window_SetSize(1024, 768);
```

## Related

- [DGL_Window_SetSizeAndPosition](#dgl_window_setsizeandposition)

# DGL_Window_SetSizeAndPosition

Changes the width, height, and position of the application window.

## Function

```C
void DGL_Window_SetSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, int windowTopPos)
```

### Parameters

- windowWidth (int) - The width to use for the new window size.
- windowHeight (int) - The height to use for the new window size.
- windowLeftPos (int) - The monitor X position to put the top left corner at.
- windowTopPos (int) - The monitor Y position to put the top left corner at.

### Return

- This function does not return anything.

## Example

```C
DGL_Vec2 monitorSize = DGL_Window_GetMonitorSize();
DGL_Window_SetSizeAndPosition(monitorSize.x, monitorSize.y, 0, 0);
```

## Related

- [DGL_Window_SetSize](#dgl_window_setsize)

# DGL_Window_SetStyle

Changes the style of the current window. This can be used to switch between fullscreen and windowed modes.

## Function

```C
void DGL_Window_SetStyle(unsigned style)
```

### Parameters

- style (unsigned) - The desired style of the window.

### Return

- This function does not return anything.

## Example

```C
DGL_Window_SetStyle(WS_EX_TOPMOST | WS_POPUP);
DGL_Window_SetSizeAndPosition(monitorWidth, monitorHeight, 0, 0);
```

## Related

- None