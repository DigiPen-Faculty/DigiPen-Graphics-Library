//-------------------------------------------------------------------------------------------------
// file:    GraphicsSystem.cpp
// author:  Andy Ellinger
// brief:   Graphics system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "DGL.h"
#include "GraphicsSystem.h"
#include "Error.h"
#include "Math.h"

namespace DGL
{
GraphicsSystem* gGraphics = nullptr;

//---------------------------------------------------------------------------------- GraphicsSystem

//*************************************************************************************************
GraphicsSystem::GraphicsSystem() 
{
    gGraphics = this;
}

//*************************************************************************************************
GraphicsSystem::~GraphicsSystem()
{
    // Check if graphics is currently initialized before shutting down
    if (mInitialized)
        ShutDown();
}

//*************************************************************************************************
int GraphicsSystem::Initialize(HWND window)
{
    Camera.Initialize(window);

    // Initialize D3D and check for any errors
    if (D3D.InitializeD3D() != 0)
    {
        // Release anything that was created
        D3D.Release();
        return 1;
    }

    // Initialize the shaders and check for any errors
    if (D3D.InitializeShaders() != 0)
    {
        // Release anything that was created
        D3D.Release();
        return 1;
    }

    // Initializes the COM library for use by this thread
    (void)CoInitialize(NULL);

    mInitialized = true;

    return 0;
}

//*************************************************************************************************
int GraphicsSystem::ShutDown()
{
    if (!mInitialized)
        return 0;

    int returnValue = 0;

    // Set an error if there are meshes or textures that weren't released
    if (mMeshes || mTextures)
    {
        returnValue = 1;
        gError->SetError("Not all meshes and textures were released");
    }

    // Release all D3D objects
    D3D.Release();

    // Uninitialize the COM library 
    CoUninitialize();

    mInitialized = false;

    return returnValue;
}

//*************************************************************************************************
DGL_Texture* GraphicsSystem::LoadTexture(const char* pFileName)
{
    // Create the texture through the texture manager
    DGL_Texture* texture = TextureManager::LoadTexture(pFileName, D3D.mDevice); 

    // If it loaded successfuly, increase the texture counter
    if (texture)
        ++mTextures;

    // Return the new texture
    return texture;
}

//*************************************************************************************************
DGL_Texture* GraphicsSystem::LoadTextureFromMemory(const unsigned char* data, int width, int height)
{
    // Create the texture through the texture manager
    DGL_Texture* texture = TextureManager::LoadTextureFromMemory(data, width, height, D3D.mDevice);

    // If it loaded successfuly, increase the texture counter
    if (texture)
        ++mTextures;

    // Return the new texture
    return texture;
}

//*************************************************************************************************
void GraphicsSystem::ReleaseTexture(DGL_Texture* texture)
{
    // Make sure the pointer is not null
    if (!texture)
        return;

    // Release the texture through the texture manager
    TextureManager::ReleaseTexture(texture);

    // Decrease the texture counter
    --mTextures;
}

//*************************************************************************************************
void GraphicsSystem::SetCurrentTexture(const DGL_Texture* texture)
{
    // Save the texture
    mCurrentTexture = texture;
}

//*************************************************************************************************
void GraphicsSystem::StartMesh()
{
    // Clear any existing vertices in the list
    Meshes.mVertexList.clear();
}

//*************************************************************************************************
DGL_Mesh* GraphicsSystem::EndMesh()
{
    // Create the new mesh using the mesh manager
    DGL_Mesh* newMesh = Meshes.CreateMesh(D3D.mDevice);

    // If it was successful, increase the mesh counter
    if (newMesh)
        ++mMeshes;

    // Return the new mesh
    return newMesh;
}

//*************************************************************************************************
DGL_Mesh* GraphicsSystem::EndMeshIndexed(unsigned* indices, unsigned indexCount)
{
    // Create the new indexed mesh using the mesh manager
    DGL_Mesh* newMesh = Meshes.CreateMeshIndexed(indices, indexCount, D3D.mDevice);

    // If it was successful, increase the mesh counter
    if (newMesh)
        ++mMeshes;

    // Return the new mesh
    return newMesh;
}

//*************************************************************************************************
void GraphicsSystem::AddVertex(const DGL_Vec2& position, const DGL_Color& color, const DGL_Vec2& texCoord)
{
    // Add the vertex to the list on the mesh manager
    Meshes.mVertexList.push_back({position, color, texCoord});
}

//*************************************************************************************************
void GraphicsSystem::ReleaseMesh(DGL_Mesh* mesh)
{
    // Make sure the pointer is valid
    if (!mesh)
        return;

    // Delete the mesh
    MeshManager::ReleaseMesh(mesh);

    // Reduce the mesh counter
    --mMeshes;
}

//*************************************************************************************************
void GraphicsSystem::DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode)
{
    // Draw the mesh using the mesh manager
    MeshManager::Draw(mesh, mode, mCurrentTexture, D3D.GetCurrentShader(), D3D.mDeviceContext);
}

} // namepspace DGL


using namespace DGL;

//*************************************************************************************************
DGL_Vec2 DGL_Camera_ScreenCoordToWorld(const DGL_Vec2* position)
{
    return gGraphics->Camera.ScreenToWorld(*position);
}

//*************************************************************************************************
DGL_Vec2 DGL_Camera_GetPosition(void)
{
    return gGraphics->Camera.GetCameraPosition();
}

//*************************************************************************************************
void DGL_Camera_SetPosition(const DGL_Vec2* position)
{
    gGraphics->Camera.SetCameraPosition(*position);
}

//*************************************************************************************************
float DGL_Camera_GetZoom(void)
{
    return gGraphics->Camera.GetCameraZoom();
}

//*************************************************************************************************
void DGL_Camera_SetZoom(float zoom)
{
    gGraphics->Camera.SetCameraZoom(zoom);
}

//*************************************************************************************************
void DGL_Graphics_StartDrawing(void)
{
    gGraphics->D3D.StartUpdate();
}

//*************************************************************************************************
void DGL_Graphics_FinishDrawing(void)
{
    gGraphics->D3D.EndUpdate();
}

//*************************************************************************************************
void DGL_Graphics_SetBackgroundColor(const DGL_Color* color)
{
    gGraphics->D3D.mBackgroundColor[0] = color->r;
    gGraphics->D3D.mBackgroundColor[1] = color->g;
    gGraphics->D3D.mBackgroundColor[2] = color->b;
}

//*************************************************************************************************
void DGL_Graphics_SetTextureSamplerData(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode)
{
    gGraphics->D3D.SetSamplerState(sampleMode, addressMode);
}

//*************************************************************************************************
void DGL_Graphics_SetBlendMode(DGL_BlendMode mode)
{
    gGraphics->D3D.SetBlendMode(mode);
}

//*************************************************************************************************
void DGL_Graphics_SetShaderMode(DGL_ShaderMode mode)
{
    gGraphics->D3D.SetShaderMode(mode);
}

//*************************************************************************************************
void DGL_Graphics_SetTexture(const DGL_Texture* texture)
{
    gGraphics->SetCurrentTexture(texture);
}

//*************************************************************************************************
DGL_Texture* DGL_Graphics_LoadTexture(const char* fileName)
{
    return gGraphics->LoadTexture(fileName);
}

//*************************************************************************************************
DGL_Texture* DGL_Graphics_LoadTextureFromMemory(const unsigned char* data, int width, int height)
{
    return gGraphics->LoadTextureFromMemory(data, width, height);
}

//*************************************************************************************************
void DGL_Graphics_FreeTexture(DGL_Texture** texture)
{
    gGraphics->ReleaseTexture(*texture);
    *texture = nullptr;
}

//*************************************************************************************************
void DGL_Graphics_StartMesh(void)
{
    gGraphics->StartMesh();
}

//*************************************************************************************************
DGL_Mesh* DGL_Graphics_EndMesh(void)
{
    return gGraphics->EndMesh();
}

//*************************************************************************************************
DGL_Mesh* DGL_Graphics_EndMeshIndexed(unsigned* indices, unsigned indexCount)
{
    return gGraphics->EndMeshIndexed(indices, indexCount);
}

//*************************************************************************************************
void DGL_Graphics_AddVertex(const DGL_Vec2* position, const DGL_Color* color, const DGL_Vec2* textureOffset)
{
    gGraphics->AddVertex(*position, *color, *textureOffset);
}

//*************************************************************************************************
void DGL_Graphics_AddTriangle(
    const DGL_Vec2* position1, const DGL_Color* color1, const DGL_Vec2* textureOffset1,
    const DGL_Vec2* position2, const DGL_Color* color2, const DGL_Vec2* textureOffset2,
    const DGL_Vec2* position3, const DGL_Color* color3, const DGL_Vec2* textureOffset3
)
{
    DGL_Graphics_AddVertex(position1, color1, textureOffset1);
    DGL_Graphics_AddVertex(position2, color2, textureOffset2);
    DGL_Graphics_AddVertex(position3, color3, textureOffset3);
}

//*************************************************************************************************
void DGL_Graphics_FreeMesh(DGL_Mesh** mesh)
{
    gGraphics->ReleaseMesh(*mesh);
    *mesh = nullptr;
}

//*************************************************************************************************
void DGL_Graphics_DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode)
{
    gGraphics->DrawMesh(mesh, mode);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TransformData(const DGL_Vec2* position, const DGL_Vec2* scale,
    float rotationRadians)
{
    // Create the scale matrix
    DGL_Mat4 scaleMatrix;
    Matrix_SetToIdentity(scaleMatrix);
    scaleMatrix.m[0][0] = scale->x;
    scaleMatrix.m[1][1] = scale->y;

    // Create the rotation matrix
    DGL_Mat4 rotationMatrix = Matrix_RotateZ(rotationRadians);

    // Create the translation matrix
    DGL_Mat4 txMatrix;
    Matrix_SetToIdentity(txMatrix);
    txMatrix.m[0][3] = position->x;
    txMatrix.m[1][3] = position->y;

    // Multiply all matrices together
    DGL_Mat4 result = Matrix_Multiply(Matrix_Multiply(txMatrix, rotationMatrix), scaleMatrix);

    // Set the transform matrix on the constant buffer
    gGraphics->D3D.SetTransform(&result);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TransformMatrix(const DGL_Mat4* transformationMatrix)
{
    gGraphics->D3D.SetTransform(transformationMatrix);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TextureOffset(const DGL_Vec2* textureOffset)
{
    gGraphics->D3D.SetTextureOffset(*textureOffset);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_Alpha(float alpha)
{
    gGraphics->D3D.SetAlpha(alpha);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TintColor(const DGL_Color* color)
{
    gGraphics->D3D.SetTintColor(color);
}
