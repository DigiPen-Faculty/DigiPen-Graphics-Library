This file contains information about all the functions in the Graphics section.

# Table Of Contents

Settings
- [DGL_Graphics_SetBackgroundColor](#dgl_graphics_setbackgroundcolor)
- [DGL_Graphics_SetBlendMode](#dgl_graphics_setblendmode)
- [DGL_Graphics_SetCustomPixelShader](#dgl_graphics_setcustompixelshader)
- [DGL_Graphics_SetCustomVertexShader](#dgl_graphics_setcustomvertexshader)
- [DGL_Graphics_SetShaderMode](#dgl_graphics_setpixelshadermode)
- [DGL_Graphics_SetTexture](#dgl_graphics_settexture)
- [DGL_Graphics_SetTextureSamplerData](#dgl_graphics_settexturesamplerdata)

Shaders
- [DGL_Graphics_FreePixelShader](#dgl_graphics_freepixelshader)
- [DGL_Graphics_FreeVertexShader](#dgl_graphics_freevertexshader)
- [DGL_Graphics_LoadPixelShader](#dgl_graphics_loadpixelshader)
- [DGL_Graphics_LoadVertexShader](#dgl_graphics_loadvertexshader)

Textures
- [DGL_Graphics_FreeTexture](#dgl_graphics_freetexture)
- [DGL_Graphics_GetTextureSize](#dgl_graphics_gettexturesize)
- [DGL_Graphics_LoadTexture](#dgl_graphics_loadtexture)
- [DGL_Graphics_LoadTextureFromMemory](#dgl_graphics_loadtexturefrommemory)

Meshes
- [DGL_Graphics_AddTriangle](#dgl_graphics_addtriangle)
- [DGL_Graphics_AddVertex](#dgl_graphics_addvertex)
- [DGL_Graphics_EndMesh](#dgl_graphics_endmesh)
- [DGL_Graphics_EndMeshIndexed](#dgl_graphics_endmeshindexed)
- [DGL_Graphics_FreeMesh](#dgl_graphics_freemesh)
- [DGL_Graphics_StartMesh](#dgl_graphics_startmesh)

Drawing
- [DGL_Graphics_DrawMesh](#dgl_graphics_drawmesh)
- [DGL_Graphics_FinishDrawing](#dgl_graphics_finishdrawing)
- [DGL_Graphics_StartDrawing](#dgl_graphics_startdrawing)

Constant buffer
- [DGL_Graphics_SetCB_Alpha](#dgl_graphics_setcb_alpha)
- [DGL_Graphics_SetCB_TextureOffset](#dgl_graphics_setcb_textureoffset)
- [DGL_Graphics_SetCB_TintColor](#dgl_graphics_setcb_tintcolor)
- [DGL_Graphics_SetCB_TransformData](#dgl_graphics_setcb_transformdata)
- [DGL_Graphics_SetCB_TransformMatrix](#dgl_graphics_setcb_transformmatrix)

# Settings

----------------

# DGL_Graphics_SetBackgroundColor

Sets the background color of the window. The alpha value of the color parameter will be ignored.

## Function

```C
void DGL_Graphics_SetBackgroundColor(const DGL_Color* color)
```

### Parameters

- color (const [DGL_Color](Types/#dgl_color)*) - Address of a [DGL_Color](Types/#dgl_color) variable containing the color to set the background to.

### Return

- This function does not return anything.

## Example

```C
DGL_Color color = { 0.05f, 0.1f, 0.08f, 1.0f };
DGL_Graphics_SetBackgroundColor(&color);
```

## Related

- [DGL_Color](Types/#dgl_color)

--------------------

# DGL_Graphics_SetBlendMode

Sets the blend mode to use for everything drawn after this call. The default setting is `DGL_BM_NONE`.

## Function

```C
void DGL_Graphics_SetBlendMode(DGL_BlendMode mode)
```

### Parameters

- mode ([DGL_BlendMode](Types/#dgl_blendmode)) - The blend mode to use.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_SetBlendMode(DGL_BM_BLEND);
```

## Related

- [DGL_BlendMode](Types/#dgl_blendmode)

---------------------

# DGL_Graphics_SetCustomPixelShader

Sets the custom pixel shader to use when using the DGL_PSM_CUSTOM pixel shader mode.

## Function

```C
void DGL_Graphics_SetCustomPixelShader(const DGL_PixelShader* shader)
```

### Parameters

- shader ([DGL_PixelShader](Types/#dgl_pixelshader)*) - Pointer to a [DGL_PixelShader](Types/#dgl_pixelshader) containing the loaded shader to use.

### Return

- This function does not return anything.

## Example

```C
DGL_PixelShader* shader = DGL_Graphics_LoadPixelShader("./Assets/Shaders/pixelshader.hlsl");

DGL_Graphics_SetPixelShaderMode(DGL_PSM_CUSTOM);
DGL_Graphics_SetTexture(texture);
DGL_Graphics_SetCB_TransformData(&position, &scale, rotation);
DGL_Graphics_DrawMesh(square, DGL_DM_TRIANGLELIST);
```

## Related

- [DGL_Graphics_LoadPixelShader](#dgl_graphics_loadpixelshader)
- [DGL_Graphics_FreePixelShader](#dgl_graphics_freepixelshader)
- [DGL_PixelShaderMode](Types/#dgl_pixelshadermode)

--------------------------

# DGL_Graphics_SetCustomVertexShader

Sets the custom vertex shader to use when using the DGL_VSM_CUSTOM vertex shader mode.

## Function

```C
void DGL_Graphics_SetCustomVertexShader(const DGL_VertexShader* shader)
```

### Parameters

- shader ([DGL_VertexShader](Types/#dgl_vertexshader)*) - Pointer to a [DGL_VertexShader](Types/#dgl_vertexshader) containing the loaded shader to use.

### Return

- This function does not return anything.

## Example

```C
DGL_VertexShader* shader = DGL_Graphics_LoadVertexShader("./Assets/Shaders/vertexshader.hlsl");

DGL_Graphics_SetVertexShaderMode(DGL_VSM_CUSTOM);
DGL_Graphics_SetTexture(texture);
DGL_Graphics_SetCB_TransformData(&position, &scale, rotation);
DGL_Graphics_DrawMesh(square, DGL_DM_TRIANGLELIST);
```

## Related

- [DGL_Graphics_LoadvertexShader](#dgl_graphics_loadvertexshader)
- [DGL_Graphics_FreeVertexShader](#dgl_graphics_freevertexshader)
- [DGL_VertexShaderMode](Types/#dgl_vertexshadermode)

-----------------------------

# DGL_Graphics_SetShaderMode

Sets which pixel and vertex shaders to use. See [DGL_PixelShaderMode](Types/#dgl_pixelshadermode) and [DGL_VertexShaderMode](Types/#dgl_vertexshadermode) for the available options.

## Function

```C
void DGL_Graphics_SetPixelShaderMode(DGL_PixelShaderMode pixelMode, DGL_VertexShaderMode vertexMode)
```

### Parameters

- pixelMode ([DGL_PixelShaderMode](Types/#dgl_pixelshadermode)) - The pixel shader mode to use when drawing a mesh.
- vertexMode ([DGL_VertexShaderMode](Types/#dgl_vertexshadermode)) - The vertex shader mode to use when drawing a mesh.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_SetShaderMode(DGL_PSM_COLOR, DGL_VSM_DEFAULT);
DGL_Graphics_SetCB_TransformData(&position, &scale, rotation);
DGL_Graphics_DrawMesh(line, DGL_DM_LINESTRIP);

DGL_Graphics_SetShaderMode(DGL_SM_TEXTURE, DGL_VSM_DEFAULT);
DGL_Graphics_SetTexture(texture);
DGL_Graphics_SetCB_TransformData(&position, &scale, rotation);
DGL_Graphics_DrawMesh(square, DGL_DM_TRIANGLELIST);
```

## Related

- [DGL_PixelShaderMode](Types/#dgl_pixelshadermode)
- [DGL_VertexShaderMode](Types/#dgl_vertexshadermode)

---------------------------------

# DGL_Graphics_SetTexture

Sets the texture to use when drawing with the texture-based pixel shader.

## Function

```C
void DGL_Graphics_SetTexture(const DGL_Texture* texture)
```

### Parameters

- texture ([DGL_Texture](Types/#dgl_texture)) - The texture to use when drawing a mesh.

### Return

- This function does not return anything.

## Example

```C
DGL_Texture* texture = DGL_Graphics_LoadTexture("./Assets/DigiPen_WHITE.png");
DGL_Graphics_SetTexture(texture);
```

## Related

- [DGL_Texture](Types/#dgl_texture)
- [DGL_Graphics_LoadTexture](#dgl_graphics_loadtexture)

-------------------------------

# DGL_Graphics_SetTextureSamplerData

Sets the texture sampling mode and address mode to use for all textures drawn after this call.

## Function

```C
void DGL_Graphics_SetTextureSamplerData(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode)
```

### Parameters

- sampleMode ([DGL_TextureSampleMode](Types/#dgl_texturesamplemode)) - The texture sample mode to use.
- addressMode ([DGL_TextureAddressMode](Types/#dgl_textureaddressmode)) - The texture address mode to use.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_SetTextureSamplerData(DGL_TSM_LINEAR, DGL_AM_WRAP);
```

## Related

- [DGL_TextureSampleMode](Types/#dgl_texturesamplemode)
- [DGL_TextureAddressMode](Types/#dgl_textureaddressmode)

--------------------------

# Shaders

-------------------------

# DGL_Graphics_FreePixelShader

Unloads the provided pixel shader from memory. The pointer passed in will be set to NULL.

## Function

```C
void DGL_Graphics_FreePixelShader(const DGL_PixelShader** shader)
```

### Parameters

- shader ([DGL_PixelShader](Types/#dgl_pixelshader)) - The loaded pixel shader that should be unloaded from memory.

### Return

- This function does not return anything.

## Example

```C
DGL_PixelShader* shader = DGL_Graphics_LoadPixelShader("./Assets/Shaders/pixelshader.hlsl");

DGL_Graphics_FreePixelShader(&shader);
```

## Related

- [DGL_Graphics_LoadPixelShader](dgl_graphics_loadpixelshader)
- [DGL_PixelShader](Types/#dgl_pixelshader)

-------------------------

# DGL_Graphics_FreeVertexShader

Unloads the provided vertex shader from memory. The pointer passed in will be set to NULL.

## Function

```C
void DGL_Graphics_FreeVertexShader(const DGL_VertexShader** shader)
```

### Parameters

- shader ([DGL_VertexShader](Types/#dgl_vertexshader)) - The loaded vertex shader that should be unloaded from memory.

### Return

- This function does not return anything.

## Example

```C
DGL_VertexShader* shader = DGL_Graphics_LoadVertexShader("./Assets/Shaders/vertexshader.hlsl");

DGL_Graphics_FreeVertexShader(&shader);
```

## Related

- [DGL_Graphics_LoadVertexShader](dgl_graphics_loadvertexshader)
- [DGL_VertexShader](Types/#dgl_vertexshader)

-------------------------

# DGL_Graphics_LoadPixelShader

Loads a pixel shader with the provided name and path into memory. The shader must be in HLSL format. Unless using a custom vertex shader, the pixel shader should use the following input data:

```
struct vs_out {
    float4 position_clip : SV_POSITION; 
    float4 color : COLOR0;
    float2 tex_coord : TEXCOORD0;
    float alpha : COLOR1;
};
```

## Function

```C
const DGL_PixelShader* DGL_Graphics_LoadPixelShader(const char* filename)
```

### Parameters

- filename (const char*) - The name of the file to load, including the path.

### Return

- [DGL_PixelShader](Types/#dgl_pixelshader)* - A pointer to the new pixel shader instance. If unsuccessful, this will be NULL.

## Example

```C
DGL_PixelShader* shader = DGL_Graphics_LoadPixelShader("./Assets/Shaders/pixelshader.hlsl");
DGL_Graphics_SetCustomPixelShader(shader);

DGL_Graphics_FreePixelShader(&shader);
```

## Related

- [DGL_Graphics_SetCustomPixelShader](dgl_graphics_setcustompixelshader)
- [DGL_Graphics_FreePixelShader](dgl_graphics_freepixelshader)
- [DGL_PixelShader](Types/#dgl_pixelshader)

-------------------------

# DGL_Graphics_LoadVertexShader

Loads a vertex shader with the provided name and path into memory. The shader must be in HLSL format. Vertex shaders will receive the following input data from DGL:

```
struct vs_in {
    float2 position_local : POSITION;
    float4 color : COLOR;
    float2 tex_coord : TEX;
};

cbuffer cbPerObject
{
    float4x4 worldViewProjection;
    float4x4 transform;
    float4 tintColor;
    float2 texOffset;
    float alpha;
    float padding;
};
```

## Function

```C
const DGL_VertexShader* DGL_Graphics_LoadVertexShader(const char* filename)
```

### Parameters

- filename (const char*) - The name of the file to load, including the path.

### Return

- [DGL_VertexShader](Types/#dgl_vertexshader)* - A pointer to the new vertex shader instance. If unsuccessful, this will be NULL.

## Example

```C
DGL_VertexShader* shader = DGL_Graphics_LoadVertexShader("./Assets/Shaders/vertexshader.hlsl");
DGL_Graphics_SetCustomVertexShader(shader);

DGL_Graphics_FreeVertexShader(&shader);
```

## Related

- [DGL_Graphics_SetCustomVertexShader](dgl_graphics_setcustomvertexshader)
- [DGL_Graphics_FreeVertexShader](dgl_graphics_freevertexshader)
- [DGL_VertexShader](Types/#dgl_vertexshader)

-------------------

# Textures

------------------

# DGL_Graphics_FreeTexture

Unloads the provided texture from memory. The pointer passed in will be set to NULL.

## Function

```C
void DGL_Graphics_FreeTexture(DGL_Texture** texture)
```

### Parameters

- texture ([DGL_Texture](Types/#dgl_texture)**) - The address of the texture pointer to be freed.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_FreeTexture(&texture);
```

## Related

- [DGL_Texture](Types/#dgl_texture)

-----------------------------

# DGL_Graphics_GetTextureSize

Returns the width and height of the provided texture, in pixels.

## Function

```C
DGL_Vec2 DGL_Graphics_GetTextureSize(DGL_Texture* texture)
```

### Parameters

- texture ([DGL_Texture](Types/#dgl_texture)*) - The texture to get the width and height of.

### Return

- [DGL_Vec2](Types/#dgl_vec2) - The x value will contain the width of the texture, in pixels, and the y value will contain the height.

## Example

```C
DGL_Texture* texture = DGL_Graphics_LoadTexture("./Assets/tree.png");

DGL_Vec2 textureSize = DGL_Graphics_GetTextureSize(texture);
```

## Related

- [DGL_Texture](Types/#dgl_texture)

-------------------------

# DGL_Graphics_LoadTexture

Loads a texture with the provided name and path into memory. Returns a pointer to the new texture instance.

## Function

```C
DGL_Texture* DGL_Graphics_LoadTexture(const char* fileName)
```

### Parameters

- fileName (const char*) - The name of the file to load, including the path.

### Return

- [DGL_Texture](Types/#dgl_texture)* - A pointer to the new texture. If unsuccessful, this will be NULL.

## Example

```C
DGL_Texture* texture = DGL_Graphics_LoadTexture("./Assets/DigiPen_WHITE.png");
```

## Related

- [DGL_Texture](Types/#dgl_texture)
- [DGL_Graphics_FreeTexture](#dgl_graphics_freetexture)

----------------------------

# DGL_Graphics_LoadTextureFromMemory

Loads a texture from the provided array of colors. Color data should include four char values for every pixel (R G B A) with values from 0 to 255. Returns a pointer to the new texture instance.

## Function

```C
DGL_Texture* DGL_Graphics_LoadTextureFromMemory(unsigned char* data, int width, int height)
```

### Parameters

- data (unsigned char*) - The array of pixel color data.
- width (int) - The width of the texture, in pixels. 
- height (int) - The height of the texture, in pixels.

### Return

- [DGL_Texture](Types/#dgl_texture)* - A pointer to the new texture. If unsuccessful, this will be NULL.

## Example

```C
#define textureWidth 2
#define textureHeight 2
unsigned char colors[(textureWidth * textureHeight) * 4] = {
    255, 0,   0,   255,     0,   255, 0,   255,
    0,   0,   255, 255,     255, 255, 255, 255
};

DGL_Texture* texture = DGL_Graphics_LoadTextureFromMemory(colors, textureWidth, textureHeight);
```

## Related

- [DGL_Texture](Types/#dgl_texture)
- [DGL_Graphics_FreeTexture](#dgl_graphics_freetexture)

-----------------------------

# Meshes

------------------------------

# DGL_Graphics_AddTriangle

Adds a triangle (three vertexes) to the list for the current mesh.

## Function

```C
void DGL_Graphics_AddTriangle(
    const DGL_Vec2* position1, const DGL_Color* color1, const DGL_Vec2* textureOffset1,
    const DGL_Vec2* position2, const DGL_Color* color2, const DGL_Vec2* textureOffset2,
    const DGL_Vec2* position3, const DGL_Color* color3, const DGL_Vec2* textureOffset3
)
```

### Parameters

- position1 (const [DGL_Vec2](Types/#dgl_vec2)*) - The position of the first vertex.
- color1 (const [DGL_Color](Types/#dgl_color)*) - The color of the first vertex.
- textureOffset1 (const [DGL_Vec2](Types/#dgl_vec2)*) - The texture coordinate of the first vertex.
- position2 (const [DGL_Vec2](Types/#dgl_vec2)*) - The position of the second vertex.
- color2 (const [DGL_Color](Types/#dgl_color)*) - The color of the second vertex.
- textureOffset2 (const [DGL_Vec2](Types/#dgl_vec2)*) - The texture coordinate of the second vertex.
- position3 (const [DGL_Vec2](Types/#dgl_vec2)*) - The position of the third vertex.
- color3 (const [DGL_Color](Types/#dgl_color)*) - The color of the third vertex.
- textureOffset3 (const [DGL_Vec2](Types/#dgl_vec2)*) - The texture coordinate of the third vertex.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_StartMesh();

DGL_Vec2 pos1 = { 0.0f, 0.5f };
DGL_Color color1 = { 1.0f, 0.0f, 0.0f, 1.0f };
DGL_Vec2 uv = { 0.0f, 0.0f };
DGL_Vec2 pos2 = { 0.5f, -0.5f };
DGL_Color color2 = { 0.0f, 1.0f, 0.0f, 1.0f };
DGL_Vec2 pos3 = { -0.5f, -0.5f };
DGL_Color color3 = { 0.0f, 0.0f, 1.0f, 1.0f };

DGL_Graphics_AddTriangle(
    &pos1, &color1, &uv, 
    &pos2, &color2, &uv, 
    &pos3, &color3, &uv);

DGL_Mesh* mesh = DGL_Graphics_EndMesh();
```

## Related

- [DGL_Graphics_StartMesh](#dgl_graphics_startmesh)
- [DGL_Graphics_EndMesh](#dgl_graphics_endmesh)
- [DGL_Graphics_AddVertex](#dgl_graphics_addvertex)
- [DGL_Vec2](Types/#dgl_vec2)
- [DGL_Color](Types/#dgl_color)

------------------------------

# DGL_Graphics_AddVertex

Adds a new vertex to the list for the current mesh.

## Function

```C
void DGL_Graphics_AddVertex(const DGL_Vec2* position, const DGL_Color* color, const DGL_Vec2* textureOffset)
```

### Parameters

- position (const [DGL_Vec2](Types/#dgl_vec2)*) - The position of the vertex.
- color (const [DGL_Color](Types/#dgl_color)*) - The color of the vertex.
- textureOffset (const [DGL_Vec2](Types/#dgl_vec2)*) - The texture coordinate of the vertex.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_StartMesh();

DGL_Vec2 pos = { 0.0f, 0.5f };
DGL_Color color = { 1.0f, 0.0f, 0.0f, 1.0f };
DGL_Vec2 uv = { 0.0f, 0.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

pos = (DGL_Vec2){ 0.5f, -0.5f };
color = (DGL_Color){ 0.0f, 1.0f, 0.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

pos = (DGL_Vec2){ -0.5f, -0.5f };
color = (DGL_Color){ 0.0f, 0.0f, 1.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

DGL_Mesh* mesh = DGL_Graphics_EndMesh();
```

## Related

- [DGL_Graphics_StartMesh](#dgl_graphics_startmesh)
- [DGL_Graphics_EndMesh](#dgl_graphics_endmesh)
- [DGL_Graphics_EndMeshIndexed](#dgl_graphics_endmeshindexed)
- [DGL_Graphics_AddTriangle](#dgl_graphics_addtriangle)
- [DGL_Vec2](Types/#dgl_vec2)
- [DGL_Color](Types/#dgl_color)

---------------------------

# DGL_Graphics_EndMesh

Tells the system to complete a mesh with the existing list of vertices. Returns a pointer to the new mesh instance.

## Function

```C
DGL_Mesh* DGL_Graphics_EndMesh(void)
```

### Parameters

- This function has no parameters.

### Return

- [DGL_Mesh](Types/#dgl_mesh)* - The pointer to the new mesh. If unsuccessful, the pointer will be NULL.

## Example

```C
DGL_Mesh* mesh = DGL_Graphics_EndMesh();
```

## Related

- [DGL_Graphics_EndMeshIndexed](#dgl_graphics_endmeshindexed)
- [DGL_Graphics_StartMesh](#dgl_graphics_startmesh)
- [DGL_Graphics_AddVertex](#dgl_graphics_addvertex)
- [DGL_Graphics_AddTriangle](#dgl_graphics_addtriangle)
- [DGL_Mesh](Types/#dgl_mesh)

-----------------------

# DGL_Graphics_EndMeshIndexed

Tells the system to complete a mesh with the existing list of vertices, and to treat it as an indexed mesh. Requires an array of indices and the size of the array. The indices control which vertices are drawn and in what order. Returns a pointer to the new mesh instance.

## Function

```C
DGL_Mesh* DGL_Graphics_EndMeshIndexed(unsigned* indices, unsigned indexCount)
```

### Parameters

- indices (unsigned*) - The address of an array of indices.
- indexCount (unsigned) - The number of elements in the array.

### Return

- [DGL_Mesh](Types/#dgl_mesh)* - The pointer to the new mesh. If unsuccessful, the pointer will be NULL.

## Example

```C
DGL_Graphics_StartMesh();

pos = (DGL_Vec2){ -0.5f, -0.5f };
color = (DGL_Color){ 0.0f, 0.0f, 0.0f, 1.0f };
uv = (DGL_Vec2){ 0.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

pos = (DGL_Vec2){ -0.5f, 0.5f };
uv = (DGL_Vec2){ 0.0f, 0.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

pos = (DGL_Vec2){ 0.5f, 0.5f };
uv = (DGL_Vec2){ 1.0f, 0.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

pos = (DGL_Vec2){ 0.5f, -0.5f };
uv = (DGL_Vec2){ 1.0f, 1.0f };
DGL_Graphics_AddVertex(&pos, &color, &uv);

unsigned indices[6] = { 0, 1, 2, 0, 2, 3 };
DGL_Mesh* square = DGL_Graphics_EndMeshIndexed(indices, 6);
```

## Related

- [DGL_Graphics_EndMesh](#dgl_graphics_endmesh)
- [DGL_Graphics_StartMesh](#dgl_graphics_startmesh)
- [DGL_Graphics_AddVertex](#dgl_graphics_addvertex)
- [DGL_Graphics_AddTriangle](#dgl_graphics_addtriangle)
- [DGL_Mesh](Types/#dgl_mesh)

--------------------------

# DGL_Graphics_FreeMesh

Releases the provided mesh from memory. The pointer passed in will be set to NULL.

## Function

```C
void DGL_Graphics_FreeMesh(DGL_Mesh** mesh)
```

### Parameters

- mesh ([DGL_Mesh](Types/#dgl_mesh)**) - The address of the mesh pointer to be freed.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_FreeMesh(&mesh);
```

## Related

- [DGL_Mesh](Types/#dgl_mesh)

--------------------------

# DGL_Graphics_StartMesh

Tells the graphics system to start building a new mesh. Any vertices added before this point will be discarded.

## Function

```C
void DGL_Graphics_StartMesh(void)
```

### Parameters

- This function has no parameters.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_StartMesh();
```

## Related

- [DGL_Graphics_EndMesh](#dgl_graphics_endmesh)
- [DGL_Graphics_EndMeshIndexed](#dgl_graphics_endmeshindexed)
- [DGL_Graphics_AddVertex](#dgl_graphics_addvertex)
- [DGL_Graphics_AddTriangle](#dgl_graphics_addtriangle)
- [DGL_Graphics_FreeMesh](#dgl_graphics_freemesh)

----------------------------

# Drawing

-----------------------------

# DGL_Graphics_DrawMesh

Draws the provided mesh with the provided mode, using the current settings.

## Function

```C
void DGL_Graphics_DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode)
```

### Parameters

- mesh (const [DGL_Mesh](Types/#dgl_mesh)*) - The mesh to use for drawing.
- mode ([DGL_DrawMode](Types/#dgl_drawmode)) - The drawing mode to use.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_DrawMesh(square, DGL_DM_TRIANGLELIST);
```

## Related

- [DGL_Mesh](Types/#dgl_mesh)
- [DGL_DrawMode](Types/#dgl_drawmode)

----------------------------

# DGL_Graphics_FinishDrawing

Ends the current graphics session and sends the data to be displayed. This must be called each frame when drawing is finished.

## Function

```C
void DGL_Graphics_FinishDrawing(void)
```

### Parameters

- This function has no parameters.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_StartDrawing();

// Draw all sprites

DGL_Graphics_FinishDrawing();
```

## Related

- [DGL_Graphics_StartDrawing](#dgl_graphics_startdrawing)

--------------------------

# DGL_Graphics_StartDrawing

Starts a new set of graphics rendering data. This must be called each frame before any drawing is done.

## Function

```C
void DGL_Graphics_StartDrawing(void)
```

### Parameters

- This function has no parameters.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_StartDrawing();

// Draw all sprites

DGL_Graphics_FinishDrawing();
```

## Related

- [DGL_Graphics_FinishDrawing](#dgl_graphics_finishdrawing)

-----------------------------

# Constant Buffer

---------------------------------

# DGL_Graphics_SetCB_Alpha

Sets the transparency value which will be multiplied with the alpha value of the mesh and texture.

## Function

```C
void DGL_Graphics_SetCB_Alpha(float alpha)
```

### Parameters

- alpha (float) - The transparency value to use.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_SetCB_Alpha(0.5f);
```

## Related

- None

------------------------------

# DGL_Graphics_SetCB_TextureOffset

Sets the texture offset to use when drawing meshes with textures.

## Function

```C
void DGL_Graphics_SetCB_TextureOffset(const DGL_Vec2* textureOffset)
```

### Parameters

- textureOffset (const [DGL_Vec2](Types/#dgl_vec2)*) - A pointer to a [DGL_Vec2](Types/#dgl_vec2) variable containing the texture offset coordinates to use.

### Return

- This function does not return anything.

## Example

```C
DGL_Vec2 texOffset = { 0, 0 };
DGL_Graphics_SetCB_TextureOffset(&texOffset);
```

## Related

- [DGL_Vec2](Types/#dgl_vec2)

---------------------------

# DGL_Graphics_SetCB_TintColor

Sets the tint color to be applied when drawing meshes. 

## Function

```C
void DGL_Graphics_SetCB_TintColor(const DGL_Color* color)
```

### Parameters

- color (const [DGL_Color](Types/#dgl_color)*) - Address of a [DGL_Color](Types/#dgl_color) variable containing the color to set the tint to.

### Return

- This function does not return anything.

## Example

```C
DGL_Color color = { 0.5f, 0.0f, 0.0f, 0.75f };
DGL_Graphics_SetCB_TintColor(&color);
```

## Related

- [DGL_Color](Types/#dgl_color)

----------------------------------

# DGL_Graphics_SetCB_TransformData

Sets the position, scale, and rotation (in radians) which will be used for drawing meshes.

## Function

```C
void DGL_Graphics_SetCB_TransformData(const DGL_Vec2* position, const DGL_Vec2* scale, float rotationRadians)
```

### Parameters

- position (const [DGL_Vec2](Types/#dgl_vec2)*) - The address of a [DGL_Vec2](Types/#dgl_vec2) variable containing the world position.
- scale (const [DGL_Vec2](Types/#dgl_vec2)*) - The address of a [DGL_Vec2](Types/#dgl_vec2) variable containing the scale in the X and Y directions.
- rotationRadians (float) - The rotation, in radians.

### Return

- This function does not return anything.

## Example

```C
DGL_Vec2 position = { 0.0f, 0.0f };
DGL_Vec2 scale = { 100.0f, 130.0f };
float rotationDegrees = 15.0f;
DGL_Graphics_SetCB_TransformData(&position, &scale, rotationDegrees * PI / 180.0f);
```

## Related

- [DGL_Vec2](Types/#dgl_vec2)

-------------------------------

# DGL_Graphics_SetCB_TransformMatrix

Sets the transformation matrix with position, scale, and rotation to use when drawing meshes.

## Function

```C
void DGL_Graphics_SetCB_TransformMatrix(const DGL_Mat4* transformationMatrix)
```

### Parameters

- transformationMatrix (const [DGL_Mat4](Types/#dgl_mat4)*) - A pointer to a [DGL_Mat4](Types/#dgl_mat4) variable which contains the transformation matrix to use.

### Return

- This function does not return anything.

## Example

```C
DGL_Graphics_SetCB_TransformMatrix(&matrix);
```

## Related

- [DGL_Mat4](Types/#dgl_mat4)

