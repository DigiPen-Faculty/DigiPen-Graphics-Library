This file contains information about the types defined by the DGL.

# Table Of Contents

- [DGL_BlendMode](#dgl_blendmode)
- [DGL_Color](#dgl_color)
- [DGL_DrawMode](#dgl_drawmode)
- [DGL_Mat4](#dgl_mat4)
- [DGL_Mesh](#dgl_mesh)
- [DGL_ShaderMode](#dgl_shadermode)
- [DGL_SysInitInfo](#dgl_sysinitinfo)
- [DGL_Texture](#dgl_texture)
- [DGL_TextureAddressMode](#dgl_textureaddressmode)
- [DGL_TextureSampleMode](#dgl_texturesamplemode)
- [DGL_Vec2](#dgl_vec2)

## DGL_BlendMode

These values are used to specify the type of blending used when drawing overlapping meshes.

### Enum Values

- DGL_BM_NONE - No blending.
- DGL_BM_BLEND - Color values will be blended using alpha values (normal transparency).
- DGL_BM_ADD - Color values will be added together.
- DGL_BM_MULTIPLY - Color values will be multiplied together.

### Related

- [DGL_Graphics_SetBlendMode](Graphics.md/#dgl_graphics_setblendmode)

## DGL_Color

This struct is used to pass color data to functions. Color values should be between 0.0 and 1.0.

### Struct Members

- r (float) - The value to use for the red channel.
- g (float) - The value to use for the green channel.
- b (float) - The value to use for the blue channel.
- a (float) - The value to use for the alpha channel.

### Related

- [DGL_Graphics_SetBackgroundColor](Graphics.md/#dgl_graphics_setbackgroundcolor)
- [DGL_Graphics_SetCB_TintColor](Graphics.md/#dgl_graphics_setcb_tintcolor)
- [DGL_Graphics_AddVertex](Graphics.md/#dgl_graphics_addvertex)

## DGL_DrawMode

These values are used to specify the draw mode to use when interpreting a mesh's vertices.

### Enum Values

- DGL_DM_TRIANGLELIST - Vertex data is a list of triangles (three vertices per triangle).
- DGL_DM_TRIANGLESTRIP - Vertex data is a triangle strip (three vertices for first triangle, one vertex for each additional triangle).
- DGL_DM_LINELIST - Vertex data is a list of line segments (two vertices per line).
- DGL_DM_LINESTRIP - Vertex data is a series of contiguous lines (starting vertex plus one vertex per line segment).
- DGL_DM_POINTLIST - Vertex data is a list of points.

### Related

- [DGL_Graphics_DrawMesh](Graphics.md/#dgl_graphics_drawmesh)

## DGL_Mat4

This struct is used to pass matrix data to functions.

### Struct Members

- m[4][4] (float)

### Related

- [DGL_Graphics_SetCB_TransformMatrix](Graphics.md/#dgl_graphics_setcb_transformmatrix)

## DGL_Mesh

This is the type used for mesh data. You will only be working with pointers to this type.

### Related

- [DGL_Graphics_EndMesh](Graphics.md/#dgl_graphics_endmesh)
- [DGL_Graphics_FreeMesh](Graphics.md/#dgl_graphics_freemesh)
- [DGL_Graphics_DrawMesh](Graphics.md/#dgl_graphics_drawmesh)

## DGL_ShaderMode

These values are used to specify which pixel shader to use when drawing.

### Enum Values

- DGL_SM_COLOR - Draw with color data from the mesh.
- DGL_SM_TEXTURE - Draw using data from the current texture.

### Related

- [DGL_Graphics_SetShaderMode](Graphics.md/#dgl_graphics_setshadermode)

## DGL_SysInitInfo

This struct is used to tell DGL information it needs to create the window. It is passed as a parameter to the DGL_System_Init() function. Make sure that all variables in the struct are set correctly.

### Struct Members

- mAppInstance (HINSTANCE) - This should be set to the first parameter from WinMain. It is used to identify the application instance when creating and closing the window.
- mShow (int) - This should be set to the fourth parameter from WinMain. It is used when creating the window to control whether it is shown or not.
- mWindowWidth (unsigned) - The width of the window to be created, including the size of the border (if using).
- mWindowHeight (unsigned) - The height of the window to be created, including the size of the title bar and border (if using).
- mMaxFrameRate (unsigned) - The maximum number of frames per second that the framerate controller should apply.
- mClassStyle (unsigned) - The Window Class Style to be used. This should usually be set to `"CS_HREDRAW | CS_VREDRAW"`.
- mWindowStyle (unsigned) - The Window Style to be used. If you are unsure, use `"WS_OVERLAPPEDWINDOW"` for a windowed application or `"WS_EX_TOPMOST | WS_POPUP"` for a fullscreen application. When using `"WS_OVERLAPPEDWINDOW"`, adding `"^ WS_THICKFRAME"` will disable resizing.
- mWindowTitle (const char*) - The text to be shown in the top bar of a windowed application.
- mCreateConsole (BOOL) - Specifies whether a debug console should be created. Use TRUE to show the console and FALSE to hide it. This console will display output from `stdout` and `stderr`, including `printf` output.
- mWindowIcon (int) - The icon resource definition to use for the window title bar icon. Visual Studio will automatically create a file named Resource.h and a .ico file with the same name as your project. You can edit the .ico file with your own custom icon. Usually what you will set this to is `IDI_YOURPROJECTNAME`.
- pWindowsCallback (WNDPROC) - This should be set to the name of your Windows callback function. When created automatically by Visual Studio this is usually called `WndProc`. 

### Related

- [DGL_System_Init](System.md/#dgl_system_init)

## DGL_Texture

This is the type used for texture data. You will only be working with pointers to this type.

### Related

- [DGL_Graphics_LoadTexture](Graphics.md/#dgl_graphics_loadtexture)
- [DGL_Graphics_FreeTexture](Graphics.md/#dgl_graphics_freetexture)
- [DGL_Graphics_SetTexture](Graphics.md/#dgl_graphics_settexture)

## DGL_TextureAddressMode

These values are used to specify the behavior when a texture is sampled outside its boundaries.

### Enum Values

- DGL_AM_WRAP - Wrap to the opposite edge of the texture and continue sampling (repeats the texture).
- DGL_AM_MIRROR - Sample backwards across the texture to the opposite edge.
- DGL_AM_CLAMP - Use the pixel color at the edge of the texture.
- DGL_AM_MIRROR_ONCE - Mirror once and then clamp.

### Related

- [DGL_Graphics_SetTextureSamplerData](Graphics.md/#dgl_graphics_settexturesamplerdata)

## DGL_TextureSampleMode

These values are used to specify the type of sampling to use when drawing textures.

### Enum Values

- DGL_TSM_LINEAR - Use linear interpolation between nearest pixels.
- DGL_TSM_POINT - Use exact pixel, no interpolation.

### Related

- [DGL_Graphics_SetTextureSamplerData](Graphics.md/#dgl_graphics_settexturesamplerdata)

## DGL_Vec2

This struct is used to pass sets of floats to functions and to return data from functions.

### Struct Members

- x (float)
- y (float)

### Related

- None