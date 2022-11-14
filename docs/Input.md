This file includes all the functions in the Input section.

# Table Of Contents

- [DGL_Input_GetMousePosition](#dgl_input_getmouseposition)
- [DGL_Input_GetMousePositionDelta](#dgl_input_getmousepositiondelta)
- [DGL_Input_GetMouseScrollDelta](#dgl_input_getmousescrolldelta)
- [DGL_Input_KeyDown](#dgl_input_keydown)
- [DGL_Input_KeyReleased](#dgl_input_keyreleased)
- [DGL_Input_KeyTriggered](#dgl_input_keytriggered)
- [DGL_Input_ShowCursor](#dgl_input_showcursor)

# DGL_Input_GetMousePosition

Returns the current X and Y position of the mouse, in screen coordinates. The value (0, 0) will be returned when the mouse is in the top left corner of the winodw, and (window_width, window_height) will be returned when the mouse is in the bottom right corner. Values are not capped at the edge of the window. 

The function [DGL_Camera_ScreenCoordToWorld](Camera/#dgl_camera_screencoordtoworld) can be used to translate this screen position into world coordinates.

## Function

```C
DGL_Vec2 DGL_Input_GetMousePosition(void)
```

### Parameters

- This function has no parameters.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - This will contain the current X and Y position of the mouse cursor.

## Example

```C
DGL_Vec2 mouseScreen = DGL_Input_GetMousePosition();
DGL_Vec2 mouseWorld = DGL_Camera_ScreenCoordToWorld(&mouseScreen);
```

## Related

- [DGL_Camera_ScreenCoordToWorld](Camera/#dgl_camera_screencoordtoworld)

# DGL_Input_GetMousePositionDelta

Returns the change in mouse position from last frame to this frame.

## Function

```C
DGL_Vec2 DGL_Input_GetMousePositionDelta(void)
```

### Parameters

- This function has no parameters.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - This will contain the difference between the mouse's position last frame and its current position.

## Example

```C
DGL_Vec2 delta = DGL_Input_GetMousePositionDelta();
```

## Related

- None

# DGL_Input_GetMouseScrollDelta

Returns the change in the mouse scroll wheel from last frame to this frame. Positive values indicate scrolling up, negative values indicate scrolling down. A single "tick" on the mouse wheel will be 1 or -1.

## Function

```C
int DGL_Input_GetMouseScrollDelta(void)
```

### Parameters

- This function has no parameters.

### Return

- int - The change in the mouse scroll wheel from last frame to this frame.

## Example

```C
int delta = DGL_Input_GetMouseScrollDelta();
```

## Related

- None

# DGL_Input_KeyDown

Returns TRUE if the specified key is currently down, and FALSE if it is not.

## Function

```C
BOOL DGL_Input_KeyDown(char key)
```

### Parameters

- key (char) - The key to be checked. This could be a character such as `'A'` or it could be a virtual key code such as `VK_SPACE` or `VK_LBUTTON`. See [Microsoft's documentation](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) for a full list of key codes.

### Return

- BOOL - Will be TRUE if the key is down and FALSE if it is not.

## Example

```C
if (DGL_Input_KeyDown('W'))
    MoveUp();

if (DGL_Input_KeyDown(VK_LBUTTON))
    ButtonIsPressed();
```

## Related

- [DGL_Input_KeyTriggered](#dgl_input_keytriggered)
- [DGL_Input_KeyReleased](#dgl_input_keyreleased)

# DGL_Input_KeyReleased

Returns TRUE if the specified key is currently not down but was down last frame, and FALSE if not.

## Function

```C
BOOL DGL_Input_KeyReleased(char key)
```

### Parameters

- key (char) - The key to be checked. This could be a character such as `'A'` or it could be a virtual key code such as `VK_SPACE` or `VK_LBUTTON`. See [Microsoft's documentation](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) for a full list of key codes.

### Return

- BOOL - Will be TRUE if the key was released and FALSE if it was not.

## Example

```C
if (DGL_Input_KeyReleased(VK_LBUTTON))
    ButtonActivated();
```

## Related

- [DGL_Input_KeyTriggered](#dgl_input_keytriggered)
- [DGL_Input_KeyDown](#dgl_input_keydown)

# DGL_Input_KeyTriggered

Returns TRUE if the specified key is currently down but was not down last frame, and FALSE if not.

## Function

```C
BOOL DGL_Input_KeyTriggered(char key)
```

### Parameters

- key (char) - The key to be checked. This could be a character such as `'A'` or it could be a virtual key code such as `VK_SPACE` or `VK_LBUTTON`. See [Microsoft's documentation](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) for a full list of key codes.

### Return

- BOOL - Will be TRUE if the key was triggered and FALSE if it was not.

## Example

```C
if (DGL_Input_KeyTriggered(VK_SPACE))
    Jump();
```

## Related

- [DGL_Input_KeyReleased](#dgl_input_keyreleased)
- [DGL_Input_KeyDown](#dgl_input_keydown)

# DGL_Input_ShowCursor

Pass in FALSE to hide the mouse cursor and TRUE to display it.

## Function

```C
void DGL_Input_ShowCursor(BOOL show)
```

### Parameters

- show (BOOL) - Pass in FALSE to hide the mouse cursor and TRUE to display it.

### Return

- This function does not return anything.

## Example

```C
DGL_Input_ShowCursor(FALSE);
```

## Related

- None