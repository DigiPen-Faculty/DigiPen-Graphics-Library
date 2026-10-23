//-------------------------------------------------------------------------------------------------
// file:    TestScene.h
// brief:   A scene that uses most DGL graphics features, drawn the same way every frame so that
//          screenshots can be compared between DGL versions.
//          Only uses graphics and camera functions, so it works with any DGL version.
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"

struct TestScene
{
    DGL_Mesh* colorSquare{ nullptr };   // Square with a different color on each corner
    DGL_Mesh* whiteSquare{ nullptr };   // White square
    DGL_Mesh* indexedSquare{ nullptr }; // White square made with an index list
    DGL_Mesh* lineStrip{ nullptr };     // Zig-zag line
    DGL_Texture* whiteTexture{ nullptr };
    DGL_Texture* checkerTexture{ nullptr };
    DGL_Texture* renderTexture{ nullptr };
};

inline DGL_Mesh* TestScene_CreateSquare(const DGL_Color& c0, const DGL_Color& c1, const DGL_Color& c2,
    const DGL_Color& c3)
{
    DGL_Graphics_StartMesh();
    DGL_Graphics_AddTriangle(
        { -0.5f, -0.5f }, c0, { 0.0f, 1.0f },
        { 0.5f, -0.5f }, c1, { 1.0f, 1.0f },
        { 0.5f, 0.5f }, c2, { 1.0f, 0.0f });
    DGL_Graphics_AddTriangle(
        { -0.5f, -0.5f }, c0, { 0.0f, 1.0f },
        { 0.5f, 0.5f }, c2, { 1.0f, 0.0f },
        { -0.5f, 0.5f }, c3, { 0.0f, 0.0f });
    return DGL_Graphics_EndMesh();
}

inline bool TestScene_Create(TestScene& scene)
{
    const DGL_Color white{ 1.0f, 1.0f, 1.0f, 1.0f };

    scene.colorSquare = TestScene_CreateSquare(
        { 1.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, white);
    scene.whiteSquare = TestScene_CreateSquare(white, white, white, white);

    DGL_Graphics_StartMesh();
    DGL_Graphics_AddVertex({ -0.5f, -0.5f }, white, { 0.0f, 2.0f });
    DGL_Graphics_AddVertex({ 0.5f, -0.5f }, white, { 2.0f, 2.0f });
    DGL_Graphics_AddVertex({ 0.5f, 0.5f }, white, { 2.0f, 0.0f });
    DGL_Graphics_AddVertex({ -0.5f, 0.5f }, white, { 0.0f, 0.0f });
    unsigned indices[] = { 0, 1, 2, 0, 2, 3 };
    scene.indexedSquare = DGL_Graphics_EndMeshIndexed(indices, 6);

    DGL_Graphics_StartMesh();
    for (int i = 0; i < 9; ++i)
    {
        float t = i / 8.0f;
        DGL_Graphics_AddVertex({ t - 0.5f, (i % 2) ? 0.5f : -0.5f }, { t, 1.0f, 1.0f - t, 1.0f }, { 0, 0 });
    }
    scene.lineStrip = DGL_Graphics_EndMesh();

    const unsigned char whitePixel[] = { 255, 255, 255, 255 };
    scene.whiteTexture = DGL_Graphics_LoadTextureFromMemory(whitePixel, 1, 1);

    // 8x8 checkerboard of orange and dark blue pixels
    unsigned char checker[8 * 8 * 4];
    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 8; ++x)
        {
            unsigned char* p = &checker[(y * 8 + x) * 4];
            bool odd = (x + y) % 2 != 0;
            p[0] = odd ? 255 : 20;
            p[1] = odd ? 140 : 40;
            p[2] = odd ? 0 : 120;
            p[3] = 255;
        }
    }
    scene.checkerTexture = DGL_Graphics_LoadTextureFromMemory(checker, 8, 8);

    scene.renderTexture = DGL_Graphics_CreateRenderTexture(256, 256);

    return scene.colorSquare && scene.whiteSquare && scene.indexedSquare && scene.lineStrip &&
        scene.whiteTexture && scene.checkerTexture && scene.renderTexture;
}

inline void TestScene_Free(TestScene& scene)
{
    DGL_Graphics_FreeMesh(&scene.colorSquare);
    DGL_Graphics_FreeMesh(&scene.whiteSquare);
    DGL_Graphics_FreeMesh(&scene.indexedSquare);
    DGL_Graphics_FreeMesh(&scene.lineStrip);
    DGL_Graphics_FreeTexture(&scene.whiteTexture);
    DGL_Graphics_FreeTexture(&scene.checkerTexture);
    DGL_Graphics_FreeTexture(&scene.renderTexture);
}

// Draws the scene. Must be called between DGL_Graphics_StartDrawing and DGL_Graphics_FinishDrawing.
inline void TestScene_Draw(const TestScene& scene)
{
    const DGL_Color white{ 1.0f, 1.0f, 1.0f, 1.0f };

    DGL_Graphics_SetShaderMode(DGL_PSM_DEFAULT, DGL_VSM_DEFAULT);
    DGL_Graphics_SetBlendMode(DGL_BM_BLEND);
    DGL_Graphics_SetTextureSamplerData(DGL_TSM_LINEAR, DGL_AM_CLAMP);
    DGL_Graphics_SetCB_Alpha(1.0f);
    DGL_Graphics_SetCB_TintColor(white);
    DGL_Graphics_SetCB_TextureOffset({ 0.0f, 0.0f });

    // Draw a rotated color square into the render texture
    DGL_Graphics_ClearRenderTexture(scene.renderTexture, { 0.15f, 0.15f, 0.4f, 1.0f });
    DGL_Graphics_SetTexture(scene.whiteTexture);
    DGL_Graphics_SetCB_TransformData({ 0.0f, 0.0f }, { 150.0f, 150.0f }, 0.785f);
    DGL_Graphics_DrawMeshToTexture(scene.colorSquare, DGL_DM_TRIANGLELIST, scene.renderTexture);

    // Top row: vertex colors, point-sampled wrapped checker with offset, indexed linear-sampled
    // mirrored checker, and the render texture
    DGL_Graphics_SetTexture(scene.whiteTexture);
    DGL_Graphics_SetCB_TransformData({ -380.0f, 200.0f }, { 160.0f, 160.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.colorSquare, DGL_DM_TRIANGLELIST);

    DGL_Graphics_SetTextureSamplerData(DGL_TSM_POINT, DGL_AM_WRAP);
    DGL_Graphics_SetTexture(scene.checkerTexture);
    DGL_Graphics_SetCB_TextureOffset({ 0.3f, 0.1f });
    DGL_Graphics_SetCB_TransformData({ -190.0f, 200.0f }, { 160.0f, 160.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.whiteSquare, DGL_DM_TRIANGLELIST);
    DGL_Graphics_SetCB_TextureOffset({ 0.0f, 0.0f });

    DGL_Graphics_SetTextureSamplerData(DGL_TSM_LINEAR, DGL_AM_MIRROR);
    DGL_Graphics_SetCB_TransformData({ 0.0f, 200.0f }, { 160.0f, 160.0f }, 0.3f);
    DGL_Graphics_DrawMesh(scene.indexedSquare, DGL_DM_TRIANGLELIST);
    DGL_Graphics_SetTextureSamplerData(DGL_TSM_LINEAR, DGL_AM_CLAMP);

    DGL_Graphics_SetTexture(scene.renderTexture);
    DGL_Graphics_SetCB_TransformData({ 250.0f, 200.0f }, { 200.0f, 200.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.whiteSquare, DGL_DM_TRIANGLELIST);

    // Middle row: a gray bar with one half-transparent orange square for each blend mode
    DGL_Graphics_SetTexture(scene.whiteTexture);
    DGL_Graphics_SetCB_TintColor({ 0.5f, 0.5f, 0.5f, 1.0f });
    DGL_Graphics_SetCB_TransformData({ 0.0f, -60.0f }, { 800.0f, 60.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.whiteSquare, DGL_DM_TRIANGLELIST);

    const DGL_BlendMode modes[] = { DGL_BM_NONE, DGL_BM_BLEND, DGL_BM_ADD, DGL_BM_MULTIPLY };
    for (int i = 0; i < 4; ++i)
    {
        DGL_Graphics_SetBlendMode(modes[i]);
        DGL_Graphics_SetCB_TintColor({ 1.0f, 0.5f, 0.2f, 0.5f });
        DGL_Graphics_SetCB_TransformData({ -300.0f + i * 200.0f, -60.0f }, { 120.0f, 120.0f }, 0.0f);
        DGL_Graphics_DrawMesh(scene.whiteSquare, DGL_DM_TRIANGLELIST);
    }
    DGL_Graphics_SetBlendMode(DGL_BM_BLEND);

    // Alpha fade
    DGL_Graphics_SetCB_TintColor(white);
    for (int i = 0; i < 5; ++i)
    {
        DGL_Graphics_SetCB_Alpha(0.2f + i * 0.2f);
        DGL_Graphics_SetCB_TransformData({ -400.0f + i * 70.0f, -220.0f }, { 60.0f, 60.0f }, 0.0f);
        DGL_Graphics_DrawMesh(scene.colorSquare, DGL_DM_TRIANGLELIST);
    }
    DGL_Graphics_SetCB_Alpha(1.0f);

    // Bottom row: the other draw modes
    DGL_Graphics_SetCB_TransformData({ 120.0f, -220.0f }, { 200.0f, 80.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.lineStrip, DGL_DM_LINESTRIP);
    DGL_Graphics_SetCB_TransformData({ 120.0f, -300.0f }, { 200.0f, 40.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.lineStrip, DGL_DM_LINELIST);
    DGL_Graphics_SetCB_TransformData({ 380.0f, -240.0f }, { 120.0f, 120.0f }, 0.0f);
    DGL_Graphics_DrawMesh(scene.lineStrip, DGL_DM_POINTLIST);
}
