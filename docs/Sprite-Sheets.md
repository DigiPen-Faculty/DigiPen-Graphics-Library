# Using Sprite Sheets for Animations

One way we can easily display an animation using a single image file is by placing each frame of the animation into equally spaced sections of an image. This is called a **sprite sheet**.

For example, say we have a 64x64 circle:

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/sprite-sheets/circle.png" alt="Red circle" vspace=20>

If we want to animate the circle by cycling through different colors, we can add those animation frames to the image file, with each frame in its own 64x64 section:

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/sprite-sheets/animation.png" alt="Sequence of colored circles" vspace=20>

Now we can use a mesh that will only show one section at a time, and move through the different sections to animate the circle. To do this, we need to change the texture coordinates that are assigned to each vertex of the mesh.

---

## Displaying a Texture Subsection

In a normal mesh, the UV texture coordinates go from (0, 0) to (1, 1), because we want to show the entire texture. For our animated circle, we only want to show 1/5 of the width of the texture at any one time. 

So, instead of going to (1, 1), our UV texture coordinates would go to (0.2, 1). This means that the mesh would show the entire height of the texture (since the Y coordinate is 1) but only 20% of the width of the texture.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/sprite-sheets/coordinates.png" alt="Illustration of texture coordinates" vspace=20>

We can easily write a function that will create a mesh for any number of animation frames by passing in a row and column value. Our circle animation, for instance, would have one row and five columns. 

We use these row and column values to set the UV coordinates for the vertices.

```c
DGL_Mesh* CreateMesh(float width, float height, float columns, float rows)
{
    float halfWidth = width / 2.0f;
    float halfHeight = width / 2.0f;

    // Adjust the texture coordinates based on the animation frames
    float uWidth = 1.0f / columns;
    float vHeight = 1.0f / rows;

    DGL_Graphics_StartMesh();

    DGL_Vec2 pos = { -halfWidth, -halfHeight };
    DGL_Color color = { 0.0f, 0.0f, 0.0f, 1.0f };
    DGL_Vec2 uv = { 0.0f, vHeight };                // Bottom left coordinate
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ -halfWidth, halfHeight };
    uv = (DGL_Vec2){ 0.0f, 0.0f };                  // Top left coordinate
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ halfWidth, halfHeight };
    uv = (DGL_Vec2){ uWidth, 0.0f };                // Top right coordinate
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ -halfWidth, -halfHeight };
    uv = (DGL_Vec2){ 0.0f, vHeight };               // Bottom left coordinate
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ halfWidth, halfHeight };
    uv = (DGL_Vec2){ uWidth, 0.0f };                // Top right coordinate
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    pos = (DGL_Vec2){ halfWidth, -halfHeight };
    uv = (DGL_Vec2){ uWidth, vHeight };             // Bottom right coordinate
    DGL_Graphics_AddVertex(&pos, &color, &uv);

    return DGL_Graphics_EndMesh();
}

// Create a 1x1 sized mesh with five columns and one row
// for an animation with five frames that are all next to each other
DGL_Mesh* animatedMesh = CreateMesh(1.0f, 1.0f, 5, 1);
```

If we use this mesh to display our animated circle texture, it will show the red circle in the first section. 

---

## Displaying Different Subsections

Now that we have the ability to show a subsection of our texture, we need to be able to change which subsection is being shown, so that we can cycle through the frames of our animation.

To do this we can use the [DGL_Graphics_SetCB_TextureOffset](Graphics/#dgl_graphics_setcb_textureoffset) function to set an offset value that is added to the texture coordinates of the mesh.

For example, say we have a mesh with vertices that have texture coordinates from (0, 0) to (1, 1). If we set a texture offset of (0.25, 0.5) the mesh will read texture data from (0.25, 0.5) to (1.25, 1.5). 

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/sprite-sheets/offset.png" alt="Illustration of texture offset" vspace=20>

> Since this ends up reading the texture at a value above 1.0, the way this displays will depend on the [wrap mode](Types/#dgl_textureaddressmode) that is set.

For our animated circle, we have a mesh with texture coordinates from (0, 0) to (0.2, 1), showing 20% of the width of the texture. If we set a texture offset of (0.2, 0) that will shift what it displays to be (0.2, 0) to (0.4, 1), which means it will show the second frame of the animation.

<img src="https://github.com/DigiPen-Faculty/DigiPen-Graphics-Library/blob/main/docs/images/sprite-sheets/coordinates2.png" alt="Illustration of texture offset" vspace=20>

We can abstract this by keeping track of the maximum animation frames we have and the current animation frame that we are displaying. `1.0f / maxAnimationFrames` will tell us the width of each frame (assuming all frames are next to each other in a line), and we can multiply this by the current animation frame (counting from zero) to get the offset.

```c
DGL_Vec2 texOffset = { 0,0 };
texOffset.x = (1.0f / maxAnimationFrames) * currentAnimationFrame;
DGL_Graphics_SetCB_TextureOffset(&texOffset);
```