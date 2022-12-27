//-------------------------------------------------------------------------------------------------
// file:    DGL.h
// author:  Andy Ellinger
// brief:   DGL API interface 
//          Documentation: https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/wiki 
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#define DGL_API __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


//*************************************************************************************************
// Struct type definitions
//*************************************************************************************************

// This struct is used to pass sets of floats to functions and to return data from functions.
typedef struct DGL_Vec2
{
    float x;
    float y;
} DGL_Vec2;

// This struct is used to pass matrix data to functions.
typedef struct DGL_Mat4
{
    float m[4][4];
} DGL_Mat4;

// This struct is used to pass color data to functions. 
// Color values should be between 0.0 and 1.0.
typedef struct DGL_Color
{
    float r; // red
    float g; // green
    float b; // blue
    float a; // alpha (transparency)
} DGL_Color;

// This struct is used to tell DGL information it needs to create the window.
// It is passed as a parameter to the DGL_System_Init() function.
// Make sure that all variables in the struct are set correctly.
typedef struct DGL_SysInitInfo
{
    // This should be set to the first parameter from WinMain.
    // It is used to identify the application instance when creating and closing the window.
    HINSTANCE mAppInstance;

    // This should be set to the fourth parameter from WinMain.
    // It is used when creating the window to control whether it is shown or not.
    int mShow;

    // These two variables specify the width and height of the window to be created.
    // This will be the total window size including title bar and borders (if using).
    // Use the DGL_System_GetWindowSize() function to get the client size of a bordered window.
    unsigned int mWindowWidth;
    unsigned int mWindowHeight;

    // This is used to set the framerate controller's maximum framerate. 
    unsigned int mMaxFrameRate;

    // Sets the Window Class Style to be used. 
    // This should usually be set to "CS_HREDRAW | CS_VREDRAW".
    unsigned int mClassStyle;

    // Sets the Window Style to be used.
    // If you are unsure, use "WS_OVERLAPPEDWINDOW" for a windowed application
    // or "WS_EX_TOPMOST | WS_POPUP" for a fullscreen application.
    // When using "WS_OVERLAPPEDWINDOW", adding "^ WS_THICKFRAME" will disable resizing.
    unsigned int mWindowStyle;

    // Sets the window title text that will be shown on the top bar of a windowed application.
    const char* mWindowTitle;

    // Specifies whether a debug console should be created. 
    // Use TRUE to show the console and FALSE to hide it.
    // This console will display output from stdout and stderr, including printf output.
    BOOL mCreateConsole;

    // The icon resource definition to use for the window title bar icon. 
    // Visual Studio will automatically create a file named Resource.h and a .ico file 
    // with the same name as your project. You can edit the .ico file with your own custom icon. 
    // Usually what you will set this to is IDI_YOURPROJECTNAME.
    int mWindowIcon;

    // This should be set to the name of your Windows callback function.
    // When created automatically by Visual Studio this is usually called WndProc. 
    WNDPROC pWindowsCallback;

} DGL_SysInitInfo;

// This is the type used for texture data. You will only be working with pointers to this type.
typedef struct DGL_Texture DGL_Texture;

// This is the type used for mesh data. You will only be working with pointers to this type.
typedef struct DGL_Mesh DGL_Mesh;


//*************************************************************************************************
// Enum definitions
//*************************************************************************************************

// These values are used to specify the draw mode to use when interpreting a mesh's vertices.
typedef enum 
{
    DGL_DM_TRIANGLELIST,    // Vertex data is a list of triangles (three vertices per triangle)
    DGL_DM_TRIANGLESTRIP,   // Vertex data is a triangle strip (three vertices for first triangle, 
                            // one vertex for each additional triangle)
    DGL_DM_LINELIST,        // Vertex data is a list of line segments (two vertices per line)
    DGL_DM_LINESTRIP,       // Vertex data is a series of contiguous lines (starting vertex plus 
                            // one vertex per line segment)
    DGL_DM_POINTLIST,       // Vertex data is a list of points
} DGL_DrawMode;

// These values are used to specify the type of blending used when drawing overlapping meshes.
typedef enum
{
    DGL_BM_NONE,        // No blending 
    DGL_BM_BLEND,       // Color values will be blended using alpha values (normal transparency)
    DGL_BM_ADD,         // Color values will be added together
    DGL_BM_MULTIPLY,    // Color values will be multiplied together
} DGL_BlendMode;

// These values are used to specify the type of sampling to use when drawing textures.
typedef enum
{
    DGL_TSM_LINEAR,      // Use linear interpolation between nearest pixels.
    DGL_TSM_POINT,       // Use exact pixel, no interpolation.
} DGL_TextureSampleMode;

// These values are used to specify the behavior when a texture is sampled outside its boundaries.
typedef enum
{
    DGL_AM_WRAP,        // Wrap to the opposite edge of the texture and continue sampling (repeats the texture)
    DGL_AM_MIRROR,      // Sample backwards across the texture to the opposite edge
    DGL_AM_CLAMP,       // Use the pixel color at the edge of the texture
    DGL_AM_MIRROR_ONCE, // Mirror once and then clamp
} DGL_TextureAddressMode;

// These values are used to specify which pixel shader to use when drawing.
typedef enum
{
    DGL_SM_COLOR,       // Draw with color data from the mesh
    DGL_SM_TEXTURE,     // Draw using data from the current texture
} DGL_ShaderMode;


#ifdef __cplusplus
extern "C"
{
#endif

//*************************************************************************************************
// System functions
//*************************************************************************************************

// Initializes the DGL using the specified options.
// This will create the window and initialize graphics and the framerate controller.
// Returns the window handle, which will be NULL if there was a problem with initialization.
DGL_API HWND DGL_System_Init(const DGL_SysInitInfo* sysInitInfo);

// Resets the framerate controller back to the initial state.
DGL_API void DGL_System_Reset(void);

// Tells the framerate controller a new frame is beginning, and delays if necessary to match max framerate.
// This must be called at the beginning of every update loop.
DGL_API void DGL_System_FrameControl(void);

// Updates input and Windows messages. Should be called near the beginning of the update loop.
DGL_API void DGL_System_Update(void);

// Handles some of the windows messages. Returns TRUE if the message was handled, otherwise returns FALSE.
// If the message was handled, use the "result" value to return from the Windows callback.
DGL_API BOOL DGL_System_HandleWindowsMessage(UINT message, WPARAM wParam, LPARAM lParam, int* result);

// Shuts down the graphics system and de-registers the window.
// Will return 1 if there were any problems, such as unreleased meshes and textures, otherwise returns 0.
DGL_API int DGL_System_Exit(void);

// Returns FALSE if there is no window, and TRUE if there is.
// This is used to check if you should break out of your game loop and exit.
DGL_API BOOL DGL_System_DoesWindowExist(void);

// Returns the Windows window handle, which can be used to manipulate the window or to get data from Windows.
DGL_API HWND DGL_System_GetWindowHandle(void);

// Returns the time between the beginning of the last frame and the beginning of this frame, in seconds.
DGL_API double DGL_System_GetDeltaTime(void);

// Returns the total number of frames since the program began.
// This number will be reset to 0 after calling DGL_System_Reset().
DGL_API unsigned DGL_System_GetFrameCount(void);

// Returns the current system time, in seconds.
DGL_API double DGL_System_GetTime(void);

// Returns the text of the last error in the DGL systems.
DGL_API const char* DGL_System_GetLastError(void);


//*************************************************************************************************
// Camera functions
//*************************************************************************************************

// Takes a position in screen coordinates and returns the equivalent in world coordinates.
DGL_API DGL_Vec2 DGL_Camera_ScreenCoordToWorld(const DGL_Vec2* position);

// Returns the current X and Y position of the camera, in world coordinates.
DGL_API DGL_Vec2 DGL_Camera_GetPosition(void);

// Sets the position of the camera, in world coordinates.
DGL_API void DGL_Camera_SetPosition(const DGL_Vec2* position);

// Returns the current zoom level of the camera.
DGL_API float DGL_Camera_GetZoom(void);

// Sets the zoom level of the camera. Default is 1.0. Smaller values will move the camera in 
// (objects look larger) and larger values will move it out (objects look smaller).
DGL_API void DGL_Camera_SetZoom(float zoom);


//*************************************************************************************************
// Graphics functions
//*************************************************************************************************
    
// Starts a new set of graphics rendering data. 
// This must be called each frame before any drawing is done.
DGL_API void DGL_Graphics_StartDrawing(void);

// Ends the current graphics session and sends the data to be displayed. 
// This must be called each frame when drawing is finished.
DGL_API void DGL_Graphics_FinishDrawing(void);

// *** Settings ***********************************************************************************

// Sets the background color of the window. The alpha value of the color parameter will be ignored.
DGL_API void DGL_Graphics_SetBackgroundColor(const DGL_Color* color);

// Sets the texture sampling mode and address mode to use for all textures drawn after this call.
DGL_API void DGL_Graphics_SetTextureSamplerData(DGL_TextureSampleMode sampleMode, 
    DGL_TextureAddressMode addressMode);

// Sets the blend mode to use for everything drawn after this call.
DGL_API void DGL_Graphics_SetBlendMode(DGL_BlendMode mode);

// Sets which pixel shader to use, color-only or texture.
DGL_API void DGL_Graphics_SetShaderMode(DGL_ShaderMode mode);

// Sets the texture to use when drawing with the texture-based pixel shader.
DGL_API void DGL_Graphics_SetTexture(const DGL_Texture* texture);

// *** Textures ***********************************************************************************

// Loads a texture with the provided name and path into memory.
// Returns a pointer to the new texture instance.
DGL_API DGL_Texture* DGL_Graphics_LoadTexture(const char* fileName);

// Loads a texture from the provided array of colors.
// Color data should include four char values for every pixel (R G B A).
// Returns a pointer to the new texture instance.
DGL_API DGL_Texture* DGL_Graphics_LoadTextureFromMemory(const unsigned char* data, int width, int height);

// Unloads the provided texture from memory.
// The pointer passed in will be set to NULL.
DGL_API void DGL_Graphics_FreeTexture(DGL_Texture** texture);

// *** Meshes *************************************************************************************

// Tells the graphics system to start building a new mesh.
// Any vertices added before this point will be discarded.
DGL_API void DGL_Graphics_StartMesh(void);

// Tells the system to complete a mesh with the existing list of vertices.
// Returns a pointer to the new mesh instance.
DGL_API DGL_Mesh* DGL_Graphics_EndMesh(void);

// Tells the system to complete a mesh with the existing list of vertices, and to treat it as an indexed mesh.
// Requires an array of indices and the size of the array.
// The indices control which vertices are drawn and in what order.
// Returns a pointer to the new mesh instance.
DGL_API DGL_Mesh* DGL_Graphics_EndMeshIndexed(unsigned* indices, unsigned indexCount);

// Adds a new vertex to the list for the current mesh.
DGL_API void DGL_Graphics_AddVertex(const DGL_Vec2* position, const DGL_Color* color, 
    const DGL_Vec2* textureOffset);

// Adds a triangle (three vertexes) to the list for the current mesh.
DGL_API void DGL_Graphics_AddTriangle(
    const DGL_Vec2* position1, const DGL_Color* color1, const DGL_Vec2* textureOffset1,
    const DGL_Vec2* position2, const DGL_Color* color2, const DGL_Vec2* textureOffset2,
    const DGL_Vec2* position3, const DGL_Color* color3, const DGL_Vec2* textureOffset3
);

// Releases the provided mesh from memory.
// The pointer passed in will be set to NULL.
DGL_API void DGL_Graphics_FreeMesh(DGL_Mesh** mesh);

// *** Drawing ************************************************************************************

// Draws the provided mesh with the provided mode.
DGL_API void DGL_Graphics_DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode);

// *** Constant buffer ****************************************************************************

// Sets the position, scale, and rotation (in radians) which will be used for drawing meshes.
DGL_API void DGL_Graphics_SetCB_TransformData(const DGL_Vec2* position, const DGL_Vec2* scale, 
    float rotationRadians);

// Sets the transformation matrix with position, scale, and rotation to use when drawing meshes.
DGL_API void DGL_Graphics_SetCB_TransformMatrix(const DGL_Mat4* transformationMatrix);

// Sets the texture offset to use when drawing meshes with textures.
DGL_API void DGL_Graphics_SetCB_TextureOffset(const DGL_Vec2* textureOffset);

// Sets the transparency value which will be multiplied with the alpha value of the mesh and texture.
DGL_API void DGL_Graphics_SetCB_Alpha(float alpha);

// Sets the tint color to be applied when drawing meshes. 
DGL_API void DGL_Graphics_SetCB_TintColor(const DGL_Color* color);


//*************************************************************************************************
// Input functions
//*************************************************************************************************

// Returns the change in the mouse scroll wheel from last frame to this frame.
// Positive values indicate scrolling up, negative values indicate scrolling down 
DGL_API int DGL_Input_GetMouseScrollDelta(void);

// Returns the current X and Y position of the mouse, in screen coordinates.
DGL_API DGL_Vec2 DGL_Input_GetMousePosition(void);

// Returns the change in mouse position from last frame to this frame.
DGL_API DGL_Vec2 DGL_Input_GetMousePositionDelta(void);

// Returns TRUE if the specified key is currently down, and FALSE if it is not.
DGL_API BOOL DGL_Input_KeyDown(char key);

// Returns TRUE if the specified key is currently down but was not down last frame, and FALSE if not.
DGL_API BOOL DGL_Input_KeyTriggered(char key);

// Returns TRUE if the specified key is currently not down but was down last frame, and FALSE if not.
DGL_API BOOL DGL_Input_KeyReleased(char key);

// Pass in FALSE to hide the mouse cursor and TRUE to display it.
DGL_API void DGL_Input_ShowCursor(BOOL show);


//*************************************************************************************************
// Window functions
//*************************************************************************************************

// Returns TRUE if the application window currently has focus, and FALSE if it does not.
// This can be used to do things such as automatically pause and resume when 
// the application window is minimized or un-selected.
DGL_API BOOL DGL_Window_HasFocus(void);

// Returns the width and height of the user's current monitor.
DGL_API DGL_Vec2 DGL_Window_GetMonitorSize(void);

// Returns the size of the client area of the window (not including title bar or borders).
DGL_API DGL_Vec2 DGL_Window_GetSize(void);

// Changes the width, height, and position of the application window.
DGL_API void DGL_Window_SetSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, 
    int windowTopPos);

// Changes the width and height of the application window, leaving it in the same position.
DGL_API void DGL_Window_SetSize(int windowWidth, int windowHeight);

// Changes the style of the current window. This can be used to switch between fullscreen and windowed modes.
DGL_API void DGL_Window_SetStyle(unsigned style);




#ifdef __cplusplus
}
#endif
