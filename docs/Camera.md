This file contains information about all the functions in the Camera section.

# Table Of Contents

- [DGL_Camera_GetPosition](#dgl_camera_getposition)
- [DGL_Camera_GetZoom](#dgl_camera_getzoom)
- [DGL_Camera_ScreenCoordToWorld](#dgl_camera_screencoordtoworld)
- [DGL_Camera_SetPosition](#dgl_camera_setposition)
- [DGL_Camera_SetZoom](#dgl_camera_setzoom)


# DGL_Camera_GetPosition

Returns the current X and Y position of the camera, in world coordinates.

## Function

```C
DGL_Vec2 DGL_Camera_GetPosition(void)
```

### Parameters

- This function has no parameters.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - This will contain the X and Y world coordinates of the camera.

## Example

```C
if (DGL_Input_KeyDown('W'))
{
    DGL_Vec2 pos = DGL_Camera_GetPosition();
    pos.y += 2.0f;
    DGL_Camera_SetPosition(&pos);
}
```

## Related

- [DGL_Camera_SetPosition](#dgl_camera_setposition)
- [DGL_Vec2](Types/#dgl_vec2)

# DGL_Camera_GetZoom

Returns the current zoom level of the camera.

## Function

```C
float DGL_Camera_GetZoom(void)
```

### Parameters

- This function has no parameters.

### Return

- float - The current zoom level of the DGL camera.

## Example

```C
if (DGL_Input_KeyDown(VK_UP))
{
    float zoom = DGL_Camera_GetZoom();
    zoom += 0.01f;
    DGL_Camera_SetZoom(zoom);
}
```

## Related

- [DGL_Camera_SetZoom](#dgl_camera_setzoom)

# DGL_Camera_ScreenCoordToWorld

Takes a position in screen coordinates and returns the equivalent in world coordinates.

## Function

```C
DGL_Vec2 DGL_Camera_ScreenCoordToWorld(const DGL_Vec2* position)
```

### Parameters

- position (const [DGL_Vec2](Types/#dgl_vec2)*) - The address of a [DGL_Vec2](Types/#dgl_vec2) variable containing the screen coordinates that will be translated into world coordinates.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - This will contain the equivalent world coordinates.

## Example

```C
DGL_Vec2 mouseScreen = DGL_Input_GetMousePosition();
DGL_Vec2 mouseWorld = DGL_Camera_ScreenCoordToWorld(&mouseScreen);
```

## Related

- [DGL_Input_GetMousePosition](Input/#dgl_input_getmouseposition)
- [DGL_Vec2](Types/#dgl_vec2)

# DGL_Camera_SetPosition

Sets the position of the camera, in world coordinates.

## Function

```C
void DGL_Camera_SetPosition(const DGL_Vec2* position)
```

### Parameters

- position (const [DGL_Vec2](Types/#dgl_vec2)*) - The address of a [DGL_Vec2](Types/#dgl_vec2) variable containing the world position to move the camera to.

### Return

- This function does not return anything.

## Example

```C
if (DGL_Input_KeyDown('W'))
{
    DGL_Vec2 pos = DGL_Camera_GetPosition();
    pos.y += 2.0f;
    DGL_Camera_SetPosition(&pos);
}
```

## Related

- [DGL_Camera_GetPosition](#dgl_camera_getposition)
- [DGL_Vec2](Types/#dgl_vec2)

# DGL_Camera_SetZoom

Sets the zoom level of the camera. Default is 1.0. Smaller values will move the camera in (objects look larger) and larger values will move it out (objects look smaller).

## Function

```C
void DGL_Camera_SetZoom(float zoom)
```

### Parameters

- zoom (float) - The value to set the camera zoom to.

### Return

- This function does not return anything.

## Example

```C
if (DGL_Input_KeyDown(VK_UP))
{
    float zoom = DGL_Camera_GetZoom();
    zoom += 0.01f;
    DGL_Camera_SetZoom(zoom);
}
```

## Related

- [DGL_Camera_GetZoom](#dgl_camera_getzoom)