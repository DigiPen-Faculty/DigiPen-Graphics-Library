//-------------------------------------------------------------------------------------------------
// file:    GraphicsSystem.cpp
// author:  Andy Ellinger, Kenny Mecham
// brief:   Graphics system
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "GraphicsSystem.h"

#include "DGL.h"
#include "InternalTypes.h"
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "DGLMath.h"
#include "Error.h"
#include "Shader.h"
#include "WindowSystem.h"

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
bool GraphicsSystem::Initialize(const DGL_SysInitInfo& sysInitInfo)
{
    Camera.Initialize();

    // Initialize and check for any errors
    if (Renderer.Initialize(sysInitInfo) != 0)
    {
        // Release anything that was created
        Renderer.Release();
        return false;
    }

    mInitialized = true;

    // Initialize the shaders and check for any errors
    if (!mShaderManager.Initialize(sysInitInfo) != 0)
    {
        // Release anything that was created
        mShaderManager.ShutDown();
        Renderer.Release();
        mInitialized = false;
        return false;
    }

    return true;
}

//*************************************************************************************************
void GraphicsSystem::ShutDown()
{
    if (!mInitialized)
        return;

    // Set an error if there are meshes or textures that weren't released
    bool error{ false };
    std::stringstream msg;
    msg << "The following resources were not released: ";
    if (!mMeshes.empty())
    {
        msg << mMeshes.size() << " meshes";
        error = true;
    }
    if (!mTextures.empty())
    {
        if (error)
            msg << ", ";
        else
            error = true;

        msg << mTextures.size() << " textures";
    }
    if (!mPixelShaders.empty())
    {
        if (error)
            msg << ", ";
        else
            error = true;

        msg << mPixelShaders.size() << " pixel shaders";
    }
    if (!mVertexShaders.empty())
    {
        if (error)
            msg << ", ";
        else
            error = true;

        msg << mVertexShaders.size() << " vertex shaders";
    }
    if (error)
        gError->SetError(msg.str());

    // Delete anything that wasn't released, since Vulkan objects must be destroyed before the device
    mMeshes.clear();
    mTextures.clear();
    mPixelShaders.clear();
    mVertexShaders.clear();
    mCurrentTexture = nullptr;
    mMeshBuilder.Cancel();

    // Release the default shaders, then all Vulkan objects
    mShaderManager.ShutDown();
    Renderer.Release();

    mInitialized = false;
}

//*************************************************************************************************
DGL_PixelShader* GraphicsSystem::LoadPixelShader(const char* filename)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_LoadPixelShader when Graphics is not initialized.");
        return nullptr;
    }

    if (!filename)
    {
        gError->SetError("Passed a null filename to DGL_Graphics_LoadPixelShader.");
        return nullptr;
    }

    std::unique_ptr<DGL_PixelShader> shader = mShaderManager.LoadPixelShader(filename);
    DGL_PixelShader* shaderPtr = shader.get();
    if (shaderPtr)
        mPixelShaders.emplace(shaderPtr, std::move(shader));

    return shaderPtr;
}

//*************************************************************************************************
void GraphicsSystem::ReleasePixelShader(const DGL_PixelShader* shader)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_FreePixelShader when Graphics is not initialized.");
        return;
    }
    
    if (!shader)
        return;

    auto found = mPixelShaders.find(shader);
    if (found == mPixelShaders.end())
    {
        gError->SetError("Called DGL_Graphics_FreePixelShader with a shader that was already freed.");
        return;
    }

    mShaderManager.StopUsing(shader);
    mPixelShaders.erase(found);
}

//*************************************************************************************************
DGL_VertexShader* GraphicsSystem::LoadVertexShader(const char* filename)
{
    if (!mInitialized)
    {
        gError->SetError("Graphics is not initialized");
        return nullptr;
    }

    if (!filename)
    {
        gError->SetError("Passed a null filename to DGL_Graphics_LoadVertexShader.");
        return nullptr;
    }

    std::unique_ptr<DGL_VertexShader> shader = mShaderManager.LoadVertexShader(filename);
    DGL_VertexShader* shaderPtr = shader.get();
    if (shaderPtr)
        mVertexShaders.emplace(shaderPtr, std::move(shader));

    return shaderPtr;
}

//*************************************************************************************************
void GraphicsSystem::ReleaseVertexShader(const DGL_VertexShader* shader)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_FreeVertexShader when Graphics is not initialized.");
        return;
    }
    
    if (!shader)
        return;

    auto found = mVertexShaders.find(shader);
    if (found == mVertexShaders.end())
    {
        gError->SetError("Called DGL_Graphics_FreeVertexShader with a shader that was already freed.");
        return;
    }

    mShaderManager.StopUsing(shader);
    mVertexShaders.erase(found);
}

//*************************************************************************************************
DGL_Texture* GraphicsSystem::LoadTexture(const char* pFileName)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_LoadTexture when Graphics is not initialized.");
        return nullptr;
    }

    if (!pFileName)
    {
        gError->SetError("Passed a null filename to DGL_Graphics_LoadTexture.");
        return nullptr;
    }

    std::unique_ptr<DGL_Texture> texture = Texture::LoadTextureFile(pFileName);
    DGL_Texture* texturePtr = texture.get();
    if (texturePtr)
        mTextures.emplace(texturePtr, std::move(texture));

    return texturePtr;
}

//*************************************************************************************************
DGL_Texture* GraphicsSystem::LoadTextureFromMemory(const unsigned char* data, int width, int height)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_LoadTextureFromMemory when Graphics is not initialized.");
        return nullptr;
    }

    if (!data)
    {
        gError->SetError("Passed null data to DGL_Graphics_LoadTextureFromMemory.");
        return nullptr;
    }

    if (width <= 0 || height <= 0)
    {
        gError->SetError("Passed invalid size to DGL_Graphics_LoadTextureFromMemory.");
        return nullptr;
    }

    std::unique_ptr<DGL_Texture> texture = Texture::CreateTexture(data, width, height);
    DGL_Texture* texturePtr = texture.get();
    if (texturePtr)
        mTextures.emplace(texturePtr, std::move(texture));

    return texturePtr;
}

//*************************************************************************************************
DGL_Texture* GraphicsSystem::CreateRenderTexture(int width, int height)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_LoadTextureFromMemory when Graphics is not initialized.");
        return nullptr;
    }

    if (width <= 0 || height <= 0)
    {
        gError->SetError("Passed invalid size to DGL_Graphics_LoadTextureFromMemory.");
        return nullptr;
    }

    std::unique_ptr<DGL_Texture> texture = Texture::CreateRenderTexture(width, height);
    DGL_Texture* texturePtr = texture.get();
    if (texturePtr)
        mTextures.emplace(texturePtr, std::move(texture));

    return texturePtr;
}

//*************************************************************************************************
void GraphicsSystem::ReleaseTexture(DGL_Texture* texture)
{
    // Make sure the pointer is not null
    if (!texture)
        return;

    auto found = mTextures.find(texture);
    if (found == mTextures.end())
    {
        gError->SetError("Called DGL_Graphics_FreeTexture with a texture that was already freed.");
        return;
    }

    // Stop using the texture if it is the current one
    if (mCurrentTexture == texture)
        mCurrentTexture = nullptr;

    mTextures.erase(found);
}

//*************************************************************************************************
void GraphicsSystem::SetCurrentTexture(const DGL_Texture* texture)
{
    // Save the texture
    mCurrentTexture = texture;
}

//*************************************************************************************************
void GraphicsSystem::ClearRenderTexture(const DGL_Texture* renderTexture, const DGL_Color& color)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_ClearRenderTexture when Graphics is not initialized.");
        return;
    }

    if (!renderTexture)
    {
        gError->SetError("Called DGL_Graphics_ClearRenderTexture with a NULL texture pointer.");
        return;
    }

    if (!renderTexture->IsRenderTexture())
    {
        gError->SetError("Called DGL_Graphics_ClearRenderTexture with a texture that was not created for rendering.");
        return;
    }

    Renderer.ClearImage(renderTexture->image, color);
}

//*************************************************************************************************
void GraphicsSystem::StartMesh()
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_StartMesh when Graphics is not initialized.");
        return;
    }

    if (mMeshBuilder.IsStarted())
    {
        gError->SetError("Called DGL_Graphics_StartMesh again without calling DGL_Graphics_EndMesh.");
        return;
    }

    mMeshBuilder.Start();
}

//*************************************************************************************************
DGL_Mesh* GraphicsSystem::EndMesh()
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_EndMesh when Graphics is not initialized.");
        return nullptr;
    }

    if (!mMeshBuilder.IsStarted())
    {
        gError->SetError("Called DGL_Graphics_EndMesh without calling DGL_Graphics_StartMesh.");
        return nullptr;
    }

    if (mMeshBuilder.GetVertexCount() == 0)
    {
        gError->SetError("Couldn't create mesh, no vertices added.");
        mMeshBuilder.Cancel();
        return nullptr;
    }

    std::unique_ptr<DGL_Mesh> mesh = mMeshBuilder.Finish();
    DGL_Mesh* meshPtr = mesh.get();
    if (meshPtr)
        mMeshes.emplace(meshPtr, std::move(mesh));

    return meshPtr;
}

//*************************************************************************************************
DGL_Mesh* GraphicsSystem::EndMeshIndexed(unsigned* indices, unsigned indexCount)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_EndMeshIndexed when Graphics is not initialized.");
        return nullptr;
    }

    if (!mMeshBuilder.IsStarted())
    {
        gError->SetError("Called DGL_Graphics_EndMeshIndexed without calling DGL_Graphics_StartMesh.");
        return nullptr;
    }

    if (!indices || indexCount == 0)
    {
        gError->SetError("Couldn't create indexed mesh, no indexes.");
        mMeshBuilder.Cancel();
        return nullptr;
    }

    if (mMeshBuilder.GetVertexCount() == 0)
    {
        gError->SetError("Couldn't create mesh, no vertices added.");
        mMeshBuilder.Cancel();
        return nullptr;
    }

    std::unique_ptr<DGL_Mesh> mesh = mMeshBuilder.Finish(indices, indexCount);
    DGL_Mesh* meshPtr = mesh.get();
    if (meshPtr)
        mMeshes.emplace(meshPtr, std::move(mesh));

    return meshPtr;
}

//*************************************************************************************************
void GraphicsSystem::AddVertex(const DGL_Vec2& position, const DGL_Color& color, const DGL_Vec2& texCoord)
{
    if (!mMeshBuilder.IsStarted())
    {
        gError->SetError("Called DGL_Graphics_AddVertex or DGL_Graphics_AddTriangle without calling DGL_Graphics_StartMesh.");
        return;
    }

    mMeshBuilder.AddVertex({ position, color, texCoord });
}

//*************************************************************************************************
void GraphicsSystem::ReleaseMesh(DGL_Mesh* mesh)
{
    // Make sure the pointer is valid
    if (!mesh)
        return;

    auto found = mMeshes.find(mesh);
    if (found == mMeshes.end())
    {
        gError->SetError("Called DGL_Graphics_FreeMesh with a mesh that was already freed.");
        return;
    }

    mMeshes.erase(found);
}

//*************************************************************************************************
void GraphicsSystem::DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_DrawMeshToTexture when Graphics is not initialized.");
        return;
    }

    Draw(mesh, mode, nullptr, "DGL_Graphics_DrawMesh");
}

//*************************************************************************************************
void GraphicsSystem::DrawMeshToTexture(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* renderTexture)
{
    if (!mInitialized)
    {
        gError->SetError("Called DGL_Graphics_DrawMeshToTexture when Graphics is not initialized.");
        return;
    }

    if (!renderTexture)
    {
        gError->SetError("Called DGL_Graphics_DrawMeshToTexture with a NULL pointer.");
        return;
    }

    if (!renderTexture->IsRenderTexture())
    {
        gError->SetError("Called DGL_Graphics_DrawMeshToTexture with a texture that is not set up for rendering.");
        return;
    }

    Draw(mesh, mode, renderTexture, "DGL_Graphics_DrawMeshToTexture");
}

//*************************************************************************************************
void GraphicsSystem::SetTransformData(const DGL_Vec2& position, const DGL_Vec2& scale, float rotation)
{
    mDrawPosition = position;
    mDrawScale = scale;
    mDrawRotation = rotation;

    mCreateMatrix = true;
}

//*************************************************************************************************
void GraphicsSystem::SetTransformMatrix(const DGL_Mat4& transformationMatrix)
{
    Renderer.mConstantBuffer.transformMatrix = transformationMatrix;

    mCreateMatrix = false;
}

//*************************************************************************************************
void GraphicsSystem::CreateTransformMatrix()
{
    if (!mCreateMatrix)
        return;

    // Create the scale matrix
    DGL_Mat4 scaleMatrix;
    Matrix_SetToIdentity(scaleMatrix);
    scaleMatrix.m[0][0] = mDrawScale.x;
    scaleMatrix.m[1][1] = mDrawScale.y;

    // Create the rotation matrix
    DGL_Mat4 rotationMatrix = Matrix_RotateZ(mDrawRotation);

    // Create the translation matrix
    DGL_Mat4 txMatrix;
    Matrix_SetToIdentity(txMatrix);
    txMatrix.m[0][3] = mDrawPosition.x;
    txMatrix.m[1][3] = mDrawPosition.y;

    // Multiply all matrices together
    DGL_Mat4 result = Matrix_Multiply(Matrix_Multiply(txMatrix, rotationMatrix), scaleMatrix);

    // Set the transform matrix on the constant buffer
    Renderer.mConstantBuffer.transformMatrix = result;

    mCreateMatrix = false;
}

//*************************************************************************************************
void GraphicsSystem::Draw(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* renderTexture, const char* functionName)
{
    if (!mesh)
    {
        gError->SetError(std::string("Called ") + functionName + " with a NULL mesh pointer.");
        return;
    }

    if (!Renderer.IsUpdateStarted())
    {
        gError->SetError("Tried to draw a mesh either before DGL_Graphics_StartDrawing was called or after DGL_Graphics_FinishDrawing was called.");
        return;
    }

    if (!mCurrentTexture)
    {
        gError->SetError("Trying to draw a mesh with a NULL texture pointer.");
        return;
    }

    if (mCurrentTexture == renderTexture)
    {
        gError->SetError("Tried to draw a render texture onto itself.");
        return;
    }

    const DGL_VertexShader* vertexShader = mShaderManager.GetCurrentVertexShader();
    const DGL_PixelShader* pixelShader = mShaderManager.GetCurrentPixelShader();
    if (!vertexShader || !pixelShader)
    {
        gError->SetError("Tried to draw using a custom shader mode without setting a custom shader.");
        return;
    }

    CreateTransformMatrix();

    // Render textures use their own world matrix, showing the whole texture
    if (renderTexture)
        Renderer.mConstantBuffer.worldMatrix = *renderTexture->renderWorldMatrix;

    Renderer.DrawMesh(*mesh, mode, mCurrentTexture->image, *vertexShader, *pixelShader,
        renderTexture ? &renderTexture->image : nullptr);

    if (renderTexture)
        Renderer.mConstantBuffer.worldMatrix = Camera.GetWorldMatrix();
}

} // namepspace DGL


using namespace DGL;

//*************************************************************************************************
DGL_Vec2 DGL_Camera_ScreenCoordToWorld(const DGL_Vec2& position)
{
    return gGraphics->Camera.ScreenToWorld(position);
}

//*************************************************************************************************
DGL_Vec2 DGL_Camera_GetPosition(void)
{
    return gGraphics->Camera.GetCameraPosition();
}

//*************************************************************************************************
void DGL_Camera_SetPosition(const DGL_Vec2& position)
{
    gGraphics->Camera.SetCameraPosition(position);
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
float DGL_Camera_GetRotation(void)
{
    return gGraphics->Camera.GetCameraRotation();
}

//*************************************************************************************************
void DGL_Camera_SetRotation(float radians)
{
    gGraphics->Camera.SetCameraRotation(radians);
}

//*************************************************************************************************
void DGL_Graphics_StartDrawing(void)
{
    gGraphics->Renderer.StartUpdate();
}

//*************************************************************************************************
void DGL_Graphics_FinishDrawing(void)
{
    gGraphics->Renderer.EndUpdate();
}

//*************************************************************************************************
void DGL_Graphics_SetBackgroundColor(const DGL_Color& color)
{
    gGraphics->Renderer.mBackgroundColor[0] = color.r;
    gGraphics->Renderer.mBackgroundColor[1] = color.g;
    gGraphics->Renderer.mBackgroundColor[2] = color.b;
}

//*************************************************************************************************
void DGL_Graphics_SetTextureSamplerData(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode)
{
    gGraphics->Renderer.SetSamplerState(sampleMode, addressMode);
}

//*************************************************************************************************
void DGL_Graphics_SetBlendMode(DGL_BlendMode mode)
{
    gGraphics->Renderer.SetBlendMode(mode);
}

//*************************************************************************************************
void DGL_Graphics_SetShaderMode(DGL_PixelShaderMode pixelMode, DGL_VertexShaderMode vertexMode)
{
    gGraphics->mShaderManager.mCurrentPixelShaderMode = pixelMode;
    gGraphics->mShaderManager.mCurrentVertexShaderMode = vertexMode;
}

//*************************************************************************************************
void DGL_Graphics_SetCustomPixelShader(const DGL_PixelShader* shader)
{
    gGraphics->mShaderManager.SetCustomPixelShader(shader);
}

//*************************************************************************************************
void DGL_Graphics_SetCustomVertexShader(const DGL_VertexShader* shader)
{
    gGraphics->mShaderManager.SetCustomVertexShader(shader);
}

//*************************************************************************************************
void DGL_Graphics_SetTexture(const DGL_Texture* texture)
{
    gGraphics->SetCurrentTexture(texture);
}

//*************************************************************************************************
DGL_PixelShader* DGL_Graphics_LoadPixelShader(const char* filename)
{
    return gGraphics->LoadPixelShader(filename);
}

//*************************************************************************************************
DGL_VertexShader* DGL_Graphics_LoadVertexShader(const char* filename)
{
    return gGraphics->LoadVertexShader(filename);
}

//*************************************************************************************************
void DGL_Graphics_FreePixelShader(DGL_PixelShader** shader)
{
    if (!shader)
        return;

    gGraphics->ReleasePixelShader(*shader);
    *shader = nullptr;
}

//*************************************************************************************************
void DGL_Graphics_FreeVertexShader(DGL_VertexShader** shader)
{
    if (!shader)
        return;

    gGraphics->ReleaseVertexShader(*shader);
    *shader = nullptr;
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
DGL_Texture* DGL_Graphics_CreateRenderTexture(int width, int height)
{
    return gGraphics->CreateRenderTexture(width, height);
}

//*************************************************************************************************
void DGL_Graphics_ClearRenderTexture(const DGL_Texture* renderTexture, const DGL_Color& color)
{
    gGraphics->ClearRenderTexture(renderTexture, color);
}

//*************************************************************************************************
void DGL_Graphics_FreeTexture(DGL_Texture** texture)
{
    if (!texture)
        return;

    gGraphics->ReleaseTexture(*texture);
    *texture = nullptr;
}

//*************************************************************************************************
DGL_Vec2 DGL_Graphics_GetTextureSize(DGL_Texture* texture)
{
    if (!texture)
        return DGL_Vec2{ 0 };

    return DGL_Vec2{ (float)texture->image.extent.width, (float)texture->image.extent.height };
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
void DGL_Graphics_AddVertex(const DGL_Vec2& position, const DGL_Color& color, const DGL_Vec2& textureOffset)
{
    gGraphics->AddVertex(position, color, textureOffset);
}

//*************************************************************************************************
void DGL_Graphics_AddTriangle(
    const DGL_Vec2& position1, const DGL_Color& color1, const DGL_Vec2& textureOffset1,
    const DGL_Vec2& position2, const DGL_Color& color2, const DGL_Vec2& textureOffset2,
    const DGL_Vec2& position3, const DGL_Color& color3, const DGL_Vec2& textureOffset3
)
{
    DGL_Graphics_AddVertex(position1, color1, textureOffset1);
    DGL_Graphics_AddVertex(position2, color2, textureOffset2);
    DGL_Graphics_AddVertex(position3, color3, textureOffset3);
}

//*************************************************************************************************
void DGL_Graphics_FreeMesh(DGL_Mesh** mesh)
{
    if (!mesh)
        return;

    gGraphics->ReleaseMesh(*mesh);
    *mesh = nullptr;
}

//*************************************************************************************************
void DGL_Graphics_DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode)
{
    gGraphics->DrawMesh(mesh, mode);
}

//*************************************************************************************************
void DGL_Graphics_DrawMeshToTexture(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* renderTexture)
{
    gGraphics->DrawMeshToTexture(mesh, mode, renderTexture);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TransformData(const DGL_Vec2& position, const DGL_Vec2& scale,
    float rotationRadians)
{
    gGraphics->SetTransformData(position, scale, rotationRadians);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TransformMatrix(const DGL_Mat4& transformationMatrix)
{
    gGraphics->SetTransformMatrix(transformationMatrix);
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TextureOffset(const DGL_Vec2& textureOffset)
{
    gGraphics->Renderer.mConstantBuffer.texOffset = textureOffset;
}

//*************************************************************************************************
void DGL_Graphics_SetCB_Alpha(float alpha)
{
    gGraphics->Renderer.mConstantBuffer.alpha = alpha;
}

//*************************************************************************************************
void DGL_Graphics_SetCB_TintColor(const DGL_Color& color)
{
    gGraphics->Renderer.mConstantBuffer.tintColor = color;
}

//*************************************************************************************************
void DGL_Graphics_SetCB_ShaderFloat(float data)
{
    gGraphics->Renderer.mConstantBuffer.shaderFloat = data;
}

//*************************************************************************************************
void DGL_Graphics_SetCB_ShaderVec4(const DGL_Color& vector)
{
    gGraphics->Renderer.mConstantBuffer.shaderVec4 = vector;
}

//*************************************************************************************************
void DGL_Graphics_SetConstantBuffer(const DGL_ConstantBuffer& data)
{
    memcpy(&gGraphics->Renderer.mConstantBuffer, &data, sizeof(DGL_ConstantBuffer));
}
