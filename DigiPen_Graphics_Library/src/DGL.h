//-------------------------------------------------------------------------------------------------
// file:    DGL.h
// author:  Andy Ellinger
// brief:   DGL API interface 
//          v2.0.0
//          Documentation: https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/wiki 
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#if defined(DGL_EXPORTS)
#define DGL_API __declspec(dllexport)
#else
#define DGL_API __declspec(dllimport)
#endif

//*************************************************************************************************
// Struct type definitions
//*************************************************************************************************

// This struct is used to pass sets of floats to functions and to return data from functions.
typedef struct DGL_Vec2
{
    float x{ 0 };
    float y{ 0 };
} DGL_Vec2;

// This struct is used to pass matrix data to functions.
typedef struct DGL_Mat4
{
    float m[4][4]{ 0 };
} DGL_Mat4;

// This struct is used to pass color data to functions. 
// Color values should be between 0.0 and 1.0.
typedef struct DGL_Color
{
    float r{ 0 }; // red
    float g{ 0 }; // green
    float b{ 0 }; // blue
    float a{ 0 }; // alpha (transparency)
} DGL_Color;

// This struct is used to tell DGL information it needs to create the window.
// It is passed as a parameter to the DGL_System_Init() function.
// Make sure that all variables in the struct are set correctly.
typedef struct DGL_SysInitInfo
{
    // This specifies the width and height of the window to be created, using X for width.
    // This is the size of the drawable area of the window (not including the title bar and borders).
    // When "fullscreen" is true this is ignored and the monitor's resolution is used.
    DGL_Vec2 windowSize{ 1024, 768 };

    // This is used to set the framerate controller's maximum framerate. 
    unsigned int maxFrameRate{ 0 };

    // Sets the window title text that will be shown on the top bar of a windowed application.
    const char* windowTitle{ "DGL" };

    // Specifies whether a debug console should be created. 
    // Use TRUE to show the console and FALSE to hide it.
    // This console will display output from stdout and stderr, including printf output.
    bool createConsole{ false };

    // Use true to create a fullscreen window on the primary monitor, false for a normal window.
    bool fullscreen{ false };

    // Use true to allow the user to resize the window, false to keep it at a fixed size.
    bool resizable{ true };

    // Use true to show the title bar and borders, false for a borderless window.
    bool decorated{ true };

    // The relative path to an image file (such as a .png) to use for the window title bar icon.
    // Leave as NULL to use the default icon.
    const char* windowIconFile{ nullptr };

    // This is the name of the default pixel shader file to use. It must exist in the folder
    // specified in "defaultShaderPath"
    const char* defaultPixelShader{ "PixelShader.hlsl" };

    // This is the name of the default vertex shader file to use. It must exist in the folder
    // specified in "defaultShaderPath"
    const char* defaultVertexShader{ "VertexShader.hlsl" };

    // This is the relative path to the folder where the default shader files are located.
    const char* defaultShaderPath{ "shaders/" };

} DGL_SysInitInfo;

// This is the type used for texture data. You will only be working with pointers to this type.
typedef struct DGL_Texture DGL_Texture;

// This is the type used for mesh data. You will only be working with pointers to this type.
typedef struct DGL_Mesh DGL_Mesh;

// This is the type used for custom pixel shaders. You will only be working with pointers to this type.
typedef struct DGL_PixelShader DGL_PixelShader;

// This is the type used for custom vertex shaders. You will only be working with pointers to this type.
typedef struct DGL_VertexShader DGL_VertexShader;

// This struct defines the data that is sent to the shaders for every mesh that is drawn.
// Do not modify this struct unless you are sure you know what you're doing!
struct DGL_ConstantBuffer
{
    // The world view matrix, taking into account the window size, camera position, and scale.
    DGL_Mat4 worldMatrix{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // The transformation matrix, with the position, rotation, and scale of the object being drawn.
    DGL_Mat4 transformMatrix{ 0 };
    // The color that will be multiplied with the object's vertex and texture color.
    DGL_Color tintColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    // The texture offset coordinates to use when getting texture data for the object.
    DGL_Vec2 texOffset{ 0, 0 };
    // The alpha value to multiply with the color.
    float alpha{ 1.0f };
    // Extra data which can be used by custom shaders.
    float shaderFloat{ 0 };
    // Extra data which can be used by custom shaders.
    DGL_Color shaderVec4{ 0, 0, 0, 0 };

    // Note: if adding any additional variables, the size of the struct 
    // must be a multiple of 16, so you may need to add padding.
};


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
    DGL_PSM_DEFAULT,     // Draw using the default pixel shader
    DGL_PSM_CUSTOM,      // Draw using the last set custom pixel shader
} DGL_PixelShaderMode;

// These values are used to specify which vertex shader to use when drawing.
typedef enum
{
    DGL_VSM_DEFAULT,    // Draw using the default vertex shader 
    DGL_VSM_CUSTOM,     // Draw using the last set custom vertex shader
} DGL_VertexShaderMode;

// These values are used with the DGL_Input key functions to specify which key or mouse button to check.
// Letter and number keys can also be specified with their uppercase character, such as 'A' or '1'.
typedef enum
{
    DGL_MOUSE_LEFT = 0x01,
    DGL_MOUSE_RIGHT = 0x02,
    DGL_MOUSE_MIDDLE = 0x04,
    DGL_MOUSE_BUTTON_4 = 0x05,
    DGL_MOUSE_BUTTON_5 = 0x06,

    DGL_KEY_BACKSPACE = 0x08,
    DGL_KEY_TAB = 0x09,
    DGL_KEY_ENTER = 0x0D,
    DGL_KEY_SHIFT = 0x10, // Either shift key
    DGL_KEY_CONTROL = 0x11, // Either control key
    DGL_KEY_ALT = 0x12, // Either alt key
    DGL_KEY_PAUSE = 0x13,
    DGL_KEY_CAPS_LOCK = 0x14,
    DGL_KEY_ESCAPE = 0x1B,
    DGL_KEY_SPACE = 0x20,
    DGL_KEY_PAGE_UP = 0x21,
    DGL_KEY_PAGE_DOWN = 0x22,
    DGL_KEY_END = 0x23,
    DGL_KEY_HOME = 0x24,
    DGL_KEY_LEFT = 0x25,
    DGL_KEY_UP = 0x26,
    DGL_KEY_RIGHT = 0x27,
    DGL_KEY_DOWN = 0x28,
    DGL_KEY_PRINT_SCREEN = 0x2C,
    DGL_KEY_INSERT = 0x2D,
    DGL_KEY_DELETE = 0x2E,

    DGL_KEY_0 = '0', DGL_KEY_1, DGL_KEY_2, DGL_KEY_3, DGL_KEY_4,
    DGL_KEY_5, DGL_KEY_6, DGL_KEY_7, DGL_KEY_8, DGL_KEY_9,

    DGL_KEY_A = 'A', DGL_KEY_B, DGL_KEY_C, DGL_KEY_D, DGL_KEY_E, DGL_KEY_F, DGL_KEY_G,
    DGL_KEY_H, DGL_KEY_I, DGL_KEY_J, DGL_KEY_K, DGL_KEY_L, DGL_KEY_M, DGL_KEY_N, DGL_KEY_O,
    DGL_KEY_P, DGL_KEY_Q, DGL_KEY_R, DGL_KEY_S, DGL_KEY_T, DGL_KEY_U, DGL_KEY_V, DGL_KEY_W,
    DGL_KEY_X, DGL_KEY_Y, DGL_KEY_Z,

    DGL_KEY_LEFT_SUPER = 0x5B, // Left Windows key
    DGL_KEY_RIGHT_SUPER = 0x5C, // Right Windows key
    DGL_KEY_MENU = 0x5D,

    DGL_KEY_NUMPAD_0 = 0x60,
    DGL_KEY_NUMPAD_1, DGL_KEY_NUMPAD_2, DGL_KEY_NUMPAD_3, DGL_KEY_NUMPAD_4, DGL_KEY_NUMPAD_5,
    DGL_KEY_NUMPAD_6, DGL_KEY_NUMPAD_7, DGL_KEY_NUMPAD_8, DGL_KEY_NUMPAD_9,
    DGL_KEY_NUMPAD_MULTIPLY = 0x6A,
    DGL_KEY_NUMPAD_ADD = 0x6B,
    DGL_KEY_NUMPAD_SUBTRACT = 0x6D,
    DGL_KEY_NUMPAD_DECIMAL = 0x6E,
    DGL_KEY_NUMPAD_DIVIDE = 0x6F,

    DGL_KEY_F1 = 0x70,
    DGL_KEY_F2, DGL_KEY_F3, DGL_KEY_F4, DGL_KEY_F5, DGL_KEY_F6, DGL_KEY_F7, DGL_KEY_F8,
    DGL_KEY_F9, DGL_KEY_F10, DGL_KEY_F11, DGL_KEY_F12, DGL_KEY_F13, DGL_KEY_F14, DGL_KEY_F15,
    DGL_KEY_F16, DGL_KEY_F17, DGL_KEY_F18, DGL_KEY_F19, DGL_KEY_F20, DGL_KEY_F21, DGL_KEY_F22,
    DGL_KEY_F23, DGL_KEY_F24,

    DGL_KEY_NUM_LOCK = 0x90,
    DGL_KEY_SCROLL_LOCK = 0x91,
    DGL_KEY_LEFT_SHIFT = 0xA0,
    DGL_KEY_RIGHT_SHIFT = 0xA1,
    DGL_KEY_LEFT_CONTROL = 0xA2,
    DGL_KEY_RIGHT_CONTROL = 0xA3,
    DGL_KEY_LEFT_ALT = 0xA4,
    DGL_KEY_RIGHT_ALT = 0xA5,

    DGL_KEY_SEMICOLON = 0xBA, // ;
    DGL_KEY_EQUAL = 0xBB, // =
    DGL_KEY_COMMA = 0xBC, // ,
    DGL_KEY_MINUS = 0xBD, // -
    DGL_KEY_PERIOD = 0xBE, // .
    DGL_KEY_SLASH = 0xBF, // /
    DGL_KEY_GRAVE_ACCENT = 0xC0, // `
    DGL_KEY_LEFT_BRACKET = 0xDB, // [
    DGL_KEY_BACKSLASH = 0xDC, // \ (backslash)
    DGL_KEY_RIGHT_BRACKET = 0xDD, // ]
    DGL_KEY_APOSTROPHE = 0xDE, // '
} DGL_KeyCode;


//*************************************************************************************************
// System functions
//*************************************************************************************************

// Initializes the DGL using the specified options.
// This will create the window and initialize graphics and the framerate controller.
// Returns true if successful, or false if there was a problem with initialization.
DGL_API bool DGL_System_Init(const DGL_SysInitInfo& sysInitInfo);

// Resets the framerate controller back to the initial state.
DGL_API void DGL_System_Reset(void);

// Tells the framerate controller a new frame is beginning, and delays if necessary to match max framerate.
// This must be called at the beginning of every update loop.
DGL_API void DGL_System_FrameControl(void);

// Updates input and Windows messages. Should be called near the beginning of the update loop.
DGL_API void DGL_System_Update(void);

// Shuts down the graphics system and de-registers the window.
DGL_API void DGL_System_Exit(void);

// Returns FALSE if there is no window, and TRUE if there is.
// This is used to check if you should break out of your game loop and exit.
DGL_API bool DGL_System_DoesWindowExist(void);

// Tells the window to close. After this, DGL_System_DoesWindowExist() will return false.
DGL_API void DGL_System_CloseWindow(void);

// Returns the native operating system window handle (an HWND on Windows), or NULL if there is no window.
// This is only needed for advanced uses, such as passing the window to another library.
DGL_API void* DGL_System_GetNativeWindowHandle(void);

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
DGL_API DGL_Vec2 DGL_Camera_ScreenCoordToWorld(const DGL_Vec2& position);

// Returns the current X and Y position of the camera, in world coordinates.
DGL_API DGL_Vec2 DGL_Camera_GetPosition(void);

// Sets the position of the camera, in world coordinates.
DGL_API void DGL_Camera_SetPosition(const DGL_Vec2& position);

// Returns the current zoom level of the camera.
DGL_API float DGL_Camera_GetZoom(void);

// Sets the zoom level of the camera. Default is 1.0. Smaller values will move the camera in 
// (objects look larger) and larger values will move it out (objects look smaller).
DGL_API void DGL_Camera_SetZoom(float zoom);

// Returns the current rotation of the camera, in radians.
DGL_API float DGL_Camera_GetRotation(void);

// Sets the rotation of the camera, in radians.
DGL_API void DGL_Camera_SetRotation(float radians);


//*************************************************************************************************
// Graphics functions
//*************************************************************************************************

//-------------------------------------------------------------------------------------------------
// *** Settings ***********************************************************************************

// Sets the background color of the window. The alpha value of the color parameter will be ignored.
DGL_API void DGL_Graphics_SetBackgroundColor(const DGL_Color& color);

// Sets the texture sampling mode and address mode to use for all textures drawn after this call.
DGL_API void DGL_Graphics_SetTextureSamplerData(DGL_TextureSampleMode sampleMode, 
    DGL_TextureAddressMode addressMode);

// Sets the blend mode to use for everything drawn after this call.
DGL_API void DGL_Graphics_SetBlendMode(DGL_BlendMode mode);

// Sets which pixel and vertex shaders to use. See the enum declarations for available options.
DGL_API void DGL_Graphics_SetShaderMode(DGL_PixelShaderMode pixelMode, DGL_VertexShaderMode vertexMode);

// Sets the custom pixel shader to use when using the DGL_PSM_CUSTOM shader mode.
DGL_API void DGL_Graphics_SetCustomPixelShader(const DGL_PixelShader* shader);

// Sets the custom vertex shader to use when using DGL_VSM_CUSTOM shader mode.
DGL_API void DGL_Graphics_SetCustomVertexShader(const DGL_VertexShader* shader);

// Sets the texture to use when drawing with the texture-based pixel shader.
DGL_API void DGL_Graphics_SetTexture(const DGL_Texture* texture);

//-------------------------------------------------------------------------------------------------
// *** Shaders ************************************************************************************

// Loads a pixel shader with the provided name and path into memory.
// Returns a pointer to the new pixel shader instance
DGL_API DGL_PixelShader* DGL_Graphics_LoadPixelShader(const char* filename);

// Loads a vertex shader with the provided name and path into memory.
// Returns a pointer to the new vertex shader instance
DGL_API DGL_VertexShader* DGL_Graphics_LoadVertexShader(const char* filename);

// Unloads the provided pixel shader from memory.
// The pointer passed in will be set to NULL.
DGL_API void DGL_Graphics_FreePixelShader(DGL_PixelShader** shader);

// Unloads the provided vertex shader from memory.
// The pointer passed in will be set to NULL.
DGL_API void DGL_Graphics_FreeVertexShader(DGL_VertexShader** shader);

//-------------------------------------------------------------------------------------------------
// *** Textures ***********************************************************************************

// Loads a texture with the provided name and path into memory.
// Returns a pointer to the new texture instance.
DGL_API DGL_Texture* DGL_Graphics_LoadTexture(const char* fileName);

// Loads a texture from the provided array of colors.
// Color data should include four char values for every pixel (R G B A).
// Returns a pointer to the new texture instance.
DGL_API DGL_Texture* DGL_Graphics_LoadTextureFromMemory(const unsigned char* data, int width, int height);

// Creates a texture that will be used for drawing to.
// It can also be drawn to the screen like a normal texture, once it has data.
DGL_API DGL_Texture* DGL_Graphics_CreateRenderTexture(int width, int height);

// Clears the specified render texture and fills it with the specified color.
// Note: if the alpha of this color is not zero, drawing transparent textures onto it may not work well.
DGL_API void DGL_Graphics_ClearRenderTexture(const DGL_Texture* renderTexture, const DGL_Color& color);

// Unloads the provided texture from memory.
// The pointer passed in will be set to NULL.
DGL_API void DGL_Graphics_FreeTexture(DGL_Texture** texture);

// Returns the width and height of the texture.
DGL_API DGL_Vec2 DGL_Graphics_GetTextureSize(DGL_Texture* texture);

//-------------------------------------------------------------------------------------------------
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
DGL_API void DGL_Graphics_AddVertex(const DGL_Vec2& position, const DGL_Color& color, 
    const DGL_Vec2& textureOffset);

// Adds a triangle (three vertexes) to the list for the current mesh.
DGL_API void DGL_Graphics_AddTriangle(
    const DGL_Vec2& position1, const DGL_Color& color1, const DGL_Vec2& textureOffset1,
    const DGL_Vec2& position2, const DGL_Color& color2, const DGL_Vec2& textureOffset2,
    const DGL_Vec2& position3, const DGL_Color& color3, const DGL_Vec2& textureOffset3
);

// Releases the provided mesh from memory.
// The pointer passed in will be set to NULL.
DGL_API void DGL_Graphics_FreeMesh(DGL_Mesh** mesh);

//-------------------------------------------------------------------------------------------------
// *** Drawing ************************************************************************************
    
// Starts a new set of graphics rendering data. 
// This must be called each frame before any drawing is done.
DGL_API void DGL_Graphics_StartDrawing(void);

// Ends the current graphics session and sends the data to be displayed. 
// This must be called each frame when drawing is finished.
DGL_API void DGL_Graphics_FinishDrawing(void);

// Draws the provided mesh with the provided mode.
DGL_API void DGL_Graphics_DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode);

// Draws the provided mesh to the provided render texture, with the provided mode.
DGL_API void DGL_Graphics_DrawMeshToTexture(const DGL_Mesh* mesh, DGL_DrawMode mode, 
    const DGL_Texture* renderTexture);

//-------------------------------------------------------------------------------------------------
// *** Constant buffer ****************************************************************************

// Sets the position, scale, and rotation (in radians) which will be used for drawing meshes.
DGL_API void DGL_Graphics_SetCB_TransformData(const DGL_Vec2& position, const DGL_Vec2& scale, 
    float rotationRadians);

// Sets the transformation matrix with position, scale, and rotation to use when drawing meshes.
DGL_API void DGL_Graphics_SetCB_TransformMatrix(const DGL_Mat4& transformationMatrix);

// Sets the texture offset to use when drawing meshes with textures.
DGL_API void DGL_Graphics_SetCB_TextureOffset(const DGL_Vec2& textureOffset);

// Sets the transparency value which will be multiplied with the alpha value of the mesh and texture.
DGL_API void DGL_Graphics_SetCB_Alpha(float alpha);

// Sets the tint color to be applied when drawing meshes. 
// This color will be multiplied with the vertex color and texture color.
DGL_API void DGL_Graphics_SetCB_TintColor(const DGL_Color& color);

// Sets the float data which is available for custom shaders.
DGL_API void DGL_Graphics_SetCB_ShaderFloat(float data);

// Sets the vector of four floats which is available for custom shaders.
DGL_API void DGL_Graphics_SetCB_ShaderVec4(const DGL_Color& vector);

// Sets the constant buffer data directly. It is your responsibility to make sure
// all information is filled out correctly.
// Note: this will be overwritten when calling any of the other SetCB functions.
DGL_API void DGL_Graphics_SetConstantBuffer(const DGL_ConstantBuffer& data);


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
// Use a DGL_KeyCode value (such as DGL_KEY_SPACE or DGL_MOUSE_LEFT) or an uppercase character (such as 'A').
DGL_API bool DGL_Input_KeyDown(unsigned char key);

// Returns TRUE if the specified key is currently down but was not down last frame, and FALSE if not.
// Use a DGL_KeyCode value (such as DGL_KEY_SPACE or DGL_MOUSE_LEFT) or an uppercase character (such as 'A').
DGL_API bool DGL_Input_KeyTriggered(unsigned char key);

// Returns TRUE if the specified key is currently not down but was down last frame, and FALSE if not.
// Use a DGL_KeyCode value (such as DGL_KEY_SPACE or DGL_MOUSE_LEFT) or an uppercase character (such as 'A').
DGL_API bool DGL_Input_KeyReleased(unsigned char key);

// Returns the last key that was triggered (down this frame and not down last frame).
// If no key no key was triggered, will return 0.
DGL_API unsigned char DGL_Input_LastKeyTriggered(void);

// Pass in false to hide the mouse cursor and true to display it.
DGL_API void DGL_Input_ShowCursor(bool show);


//*************************************************************************************************
// Window functions
//*************************************************************************************************

// Returns true if the application window currently has focus, and false if it does not.
// This can be used to do things such as automatically pause and resume when 
// the application window is minimized or un-selected.
DGL_API bool DGL_Window_HasFocus(void);

// Returns the width and height of the user's current monitor.
DGL_API DGL_Vec2 DGL_Window_GetMonitorSize(void);

// Returns the size of the client area of the window (not including title bar or borders).
DGL_API DGL_Vec2 DGL_Window_GetSize(void);

// Changes the width, height, and position of the application window.
// The width and height are the size of the drawable area (not including the title bar and borders).
// The position is the top left corner of the drawable area, in screen coordinates.
// Has no effect while the window is fullscreen.
DGL_API void DGL_Window_SetSizeAndPosition(int windowWidth, int windowHeight, int windowLeftPos, 
    int windowTopPos);

// Changes the width and height of the application window, leaving it in the same position.
// Has no effect while the window is fullscreen.
DGL_API void DGL_Window_SetSize(int windowWidth, int windowHeight);

// Pass in true to make the window fullscreen on its current monitor, or false to return to a normal window.
// When returning to a normal window, the previous size and position are restored.
DGL_API void DGL_Window_SetFullscreen(bool fullscreen);

// Returns true if the window is currently fullscreen, and false if it is not.
DGL_API bool DGL_Window_IsFullscreen(void);

// Pass in true to allow the user to resize the window, or false to prevent it.
DGL_API void DGL_Window_SetResizable(bool resizable);

// Pass in true to show the title bar and borders, or false for a borderless window.
DGL_API void DGL_Window_SetDecorated(bool decorated);

