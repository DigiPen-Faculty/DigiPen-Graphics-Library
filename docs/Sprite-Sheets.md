# How to Use Sprite Sheets in the DGL

**This currently a work in progress**

Add columns and rows to creating the mesh

```c
DGL_Mesh* CreateMesh(float width, float height, float columns, float rows)
{
    float halfWidth = width / 2.0f;
    float halfHeight = width / 2.0f;
    float uWidth = 1.0f / columns;
    float vHeight = 1.0f / rows;

    DGL_Graphics_StartMesh();

    DGL_Vec2 pos = { -halfWidth, -halfHeight };
    DGL_Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
    DGL_Vec2 uv = { 0.0f, vHeight };
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ -halfWidth, halfHeight };
    uv = (DGL_Vec2){ 0.0f, 0.0f };
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ halfWidth, halfHeight };
    uv = (DGL_Vec2){ uWidth, 0.0f };
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ -halfWidth, -halfHeight };
    uv = (DGL_Vec2){ 0.0f, vHeight };
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ halfWidth, halfHeight };
    uv = (DGL_Vec2){ uWidth, 0.0f };
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ halfWidth, -halfHeight };
    uv = (DGL_Vec2){ uWidth, vHeight };
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    return DGL_Graphics_EndMesh();
}

DGL_Mesh* ssMesh = CreateMesh(1.0f, 1.0f, 5, 1);
```

Keep track of frames

```c
if (DGL_System_GetTime() - lastTime > 0.25f)
{
    lastTime = DGL_System_GetTime();
    animFrame = (animFrame + 1) % maxFrames;
}
```

Set the texture offset

```c
DGL_Vec2 texOffset = { 0,0 };
texOffset.x = (1.0f / maxFrames) * animFrame;
DGL_Graphics_SetCB_TextureOffset(&texOffset);
```