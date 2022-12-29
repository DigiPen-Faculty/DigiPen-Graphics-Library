# How to Use the DigiPen Graphics Library

This page covers the basic things you need to do in order to display a colored mesh or texture.

## Contents
- [Windows Callback Function](#windows-callback-function)
- [Initializing DGL](#initializing-dgl)
- [Game Loop](#game-loop)
- [Shutting Down](#shutting-down)
- [Settings](#settings)
- [Creating a Mesh](#creating-a-mesh)
- [Drawing a Mesh Without a Texture](#drawing-a-mesh-without-a-texture)
- [Loading a Texture](#loading-a-texture)
- [Drawing a Texture](#drawing-a-texture)

---

## Windows Callback Function

The Windows callback function (Visual Studio will name it `WndProc` when creating a new project) is used to respond to messages from Windows about all the things your program needs to know about. See the [Windows Messages](Windows-Messages) page for more information about this.

Some of the messages need to be handled by the DGL systems, so you need to add some code to the Windows callback in your program. The following code will pass the message on to the DGL, and return if the message was handled. If DGL doesn't handle the message, and you don't either, you should return from the function with `return DefWindowProc(hWnd, message, wParam, lParam)`.

Add this code to your Windows callback function (probably called `WndProc`):

```c
int result;
if (DGL_System_HandleWindowsMessage(message, wParam, lParam, &result))
    return result;
```

---

## Initializing DGL

The first thing we need to do is set up the `DGL_SysInitInfo` struct used to pass data to DGL when it's initialized. You can use the following values as a starting point. See the [DGL_SysInitInfo](Types/#dgl_sysinitinfo) documentation for more explanation of each setting.

```c
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
```

Now that we have the struct set up we can initialize the DGL systems by calling [DGL_System_Init](System/#dgl_system_init). If the function returns NULL, something has gone wrong, and you should exit the program. You can check what the error was by calling [DGL_System_GetLastError](System/#dgl_system_getlasterror).

```c
HWND windowHandle = DGL_System_Init(&initInfo);
if (windowHandle == NULL)
    return 1;
```

---

## Game Loop

Now that the setup is finished, you can start your game loop. This can be done in different ways, but you will need to keep the following things in mind:

- The very first thing you do on each update should be calling [DGL_System_FrameControl](System/#dgl_system_framecontrol) and [DGL_System_Update](System/#dgl_system_update). This will keep your program running at the specified max frame rate and run the necessary DGL system updates.

- Before drawing any objects you must call [DGL_Graphics_StartDrawing](Graphics/#dgl_graphics_startdrawing), and you must call [DGL_Graphics_FinishDrawing](Graphics/#dgl_graphics_finishdrawing) when you are done drawing on this frame.

- You should always call [DGL_System_DoesWindowExist](System/#dgl_system_doeswindowexist) to check if you should break out of the game loop, in addition to checking for any other ways you want the user to exit the program. This will let you stop the program if the user has closed the window manually.

A very simple version of the game update could look like this:

```c
int running = TRUE;
while (running)
{
    DGL_System_FrameControl();
    DGL_System_Update();

    DGL_Graphics_StartDrawing();

    // Draw some objects

    DGL_Graphics_FinishDrawing();

    if (!DGL_System_DoesWindowExist())
        running = FALSE;
}
```

You will of course have additional code to update your game engine's systems.

---

## Shutting Down

Once the game has finished you need to shut down the DGL systems. In addition, you must make sure that you release any meshes and textures that you have created. This should happen before calling [DGL_System_Exit](System/#dgl_system_exit).

```c
DGL_System_Exit();

return 0;
```

---

## Settings

If you want to display textures with transparency, you will need to set the [blend mode](Types/#dgl_blendmode) to `DGL_BM_BLEND`. Usually you would only need to do this once, unless you want to use different modes for different objects.

```c
DGL_Graphics_SetBlendMode(DGL_BM_BLEND);
```

You may also want to set the background color using [DGL_Graphics_SetBackgroundColor](Graphics/#dgl_graphics_setbackgroundcolor) if you want it to be a color other than black. Again, this is usually done once when initializing the program.

```c
DGL_Graphics_SetBackgroundColor(&(DGL_Color){ 0.05f, 0.1f, 0.08f, 1.0f });
```

---

## Creating a Mesh

To draw a texture, we first need to create a mesh. Remember that a single mesh can be used for multiple textures. Usually you will want to create one mesh that is a single pixel high and wide, then scale the mesh to the size of the object when it is drawn.

To create a mesh, you first call [DGL_Graphics_StartMesh](Graphics/#dgl_graphics_startmesh) to tell DGL you are creating a new mesh, then call [DGL_Graphics_AddVertex](Graphics/#dgl_graphics_addvertex) for every vertex you want to add to the mesh. Alternatively, you could call [DGL_Graphics_AddTriangle](Graphics/#dgl_graphics_addtriangle) to add three vertices at once. When you are done, call [DGL_Graphics_EndMesh](Graphics/#dgl_graphics_endmesh), which returns a pointer to the new mesh object. (See the [Indexed Meshes](Indexed-Meshes) page for an explanation of using indexed meshes to avoid duplicating vertices.)

> For every mesh that you create, make sure that you call [DGL_Graphics_FreeMesh](Graphics/#dgl_graphics_freemesh) at some point before your program closes.

The following code shows how to create a square mesh (made up of two triangles) at a specified width and height.

```c
float width = 1.0f;
float height = 1.0f;

// Save the half width and height values
float halfWidth = width / 2.0f;
float halfHeight = width / 2.0f;

// Tell DGL that we are starting a new mesh
DGL_Graphics_StartMesh();

// All vertices will use this color
// You could change it between adding vertices to interpolate colors across the mesh
DGL_Color color = { 0.0f, 0.0f, 0.0f, 1.0f };

// First triangle

// Add a vertex at the bottom-left corner
DGL_Vec2 pos = { -halfWidth, -halfHeight };
DGL_Vec2 uv = { 0.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

// Add a vertex at the top-left corner
pos = (DGL_Vec2){ -halfWidth, halfHeight };
uv = (DGL_Vec2){ 0.0f, 0.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

// Add a vertex at the top-right corner
pos = (DGL_Vec2){ halfWidth, halfHeight };
uv = (DGL_Vec2){ 1.0f, 0.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

// Second triangle

// Add a vertex at the bottom-left corner
pos = (DGL_Vec2){ -halfWidth, -halfHeight };
uv = (DGL_Vec2){ 0.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

// Add a vertex at the top-right corner
pos = (DGL_Vec2){ halfWidth, halfHeight };
uv = (DGL_Vec2){ 1.0f, 0.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

// Add a vertex at the bottom-right corner
pos = (DGL_Vec2){ halfWidth, -halfHeight };
uv = (DGL_Vec2){ 1.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

// Tell DGL we are done adding vertices and save the mesh object
DGL_Mesh* mesh = DGL_Graphics_EndMesh();
```

---

## Drawing a Mesh Without a Texture

The DGL has two shader modes, one for displaying a mesh with no texture data, and one for displaying a mesh with a texture. To draw a mesh without a texture you need to call [DGL_Graphics_SetShaderMode](Graphics/#dgl_graphics_setshadermode) and pass in the `DGL_SM_COLOR` [shader mode](Types/#dgl_shadermode).

```c
DGL_Graphics_SetShaderMode(DGL_SM_COLOR);
```

Before drawing the mesh you must set the transform data (the object's position, scale, and rotation). You can do this by calling [DGL_Graphics_SetCB_TransformData](Graphics/#dgl_graphics_setcb_transformdata) and passing in variables containing the data, or, if you want to create the transformation matrix yourself, you can call [DGL_Graphics_SetCB_TransformMatrix](Graphics/#dgl_graphics_setcb_transformmatrix).

```c
// Display the mesh at the world coordinates (0, 0)
DGL_Vec2 position = { 0.0f, 0.0f };
// Scale the mesh to be 50px by 20px (assuming a 1x1 mesh)
DGL_Vec2 scale = { 50.0f, 20.0f };

// Set the transform data with the position and scale variables and no rotation
DGL_Graphics_SetCB_TransformData(&position, &scale, 0.0f);
```

Once you've set the shader mode and the transform data, you can call [DGL_Graphics_DrawMesh](Graphics/#dgl_graphics_drawmesh).

```c
DGL_Graphics_DrawMesh(mesh, DGL_DM_TRIANGLELIST);
```

---

## Loading a Texture

To load a texture, you call [DGL_Graphics_LoadTexture](Graphics/#dgl_graphics_loadtexture) and pass in the path to the file you want to load.

> For every texture that you load, make sure that you call [DGL_Graphics_FreeTexture](Graphics/#dgl_graphics_freetexture) at some point before your program closes.

```c
DGL_Texture* ballTexture = DGL_Graphics_LoadTexture("./Assets/ball.png");
```

---

## Drawing a Texture

To draw a mesh with a texture you need to call [DGL_Graphics_SetShaderMode](Graphics/#dgl_graphics_setshadermode) with the `DGL_SM_TEXTURE` [shader mode](Types/#dgl_shadermode). 

```c
DGL_Graphics_SetShaderMode(DGL_SM_TEXTURE);
```

The transform data is set in the same way whether or not there is a texture.

```c
// Display the texture at the world coordinates (100, -230)
DGL_Vec2 position = { 100.0f, -230.0f };
// Scale the mesh and texture to be 80px by 80px (assuming a 1x1 mesh)
DGL_Vec2 scale = { 80.0f, 80.0f };

// Set the transform data with the position and scale variables and a rotation of 45 degrees
DGL_Graphics_SetCB_TransformData(&position, &scale, PI / 4.0f);
```

Before drawing you need to tell DGL which texture to use by calling [DGL_Graphics_SetTexture](Graphics/#dgl_graphics_settexture) and passing in the pointer to the loaded texture.

```c
DGL_Graphics_SetTexture(ballTexture);
```

Now that everything is set, you can draw the mesh.

```c
DGL_Graphics_DrawMesh(mesh, DGL_DM_TRIANGLELIST);
```

---

