//-------------------------------------------------------------------------------------------------
// file:    GraphicsSystem.h
// author:  Andy Ellinger
// brief:   Graphics system
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include <vector>
#include "VulkanRenderer.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"

namespace DGL
{

//---------------------------------------------------------------------------------- GraphicsSystem

class GraphicsSystem
{
public:
    // Sets the global pointer
    GraphicsSystem();
    // Calls ShutDown if necessary
    ~GraphicsSystem();

    // Initializes the graphics system
    bool Initialize(const DGL_SysInitInfo& sysInitInfo);

    // Shuts down the graphics system
    void ShutDown();

    // Loads a pixel shader from the provided file
    DGL_PixelShader* LoadPixelShader(const char* fileName);

    // Releases the pixel shader and deletes the struct
    void ReleasePixelShader(const DGL_PixelShader* shader);

    // Loads a vertex shader from the provided file
    DGL_VertexShader* LoadVertexShader(const char* filename);

    // Releases the vertex shader and deletes the struct
    void ReleaseVertexShader(const DGL_VertexShader* shader);

    // Loads a texture from the provided file
    DGL_Texture* LoadTexture(const char* fileName);

    // Loads a texture from the provided pixel data
    DGL_Texture* LoadTextureFromMemory(const unsigned char* data, int width, int height);

    // Creates a texture for rendering
    DGL_Texture* CreateRenderTexture(int width, int height);

    // Releases the texture and deletes the struct
    void ReleaseTexture(DGL_Texture* texture);

    // Sets the texture to use when drawing a mesh
    void SetCurrentTexture(const DGL_Texture* texture);

    // Clears a render texture with the specified color
    void ClearRenderTexture(const DGL_Texture* renderTexture, const DGL_Color& color);

    // Starts creating a new mesh by clearing the list of vertices
    void StartMesh();

    // Creates a mesh from the existing list of vertices
    DGL_Mesh* EndMesh();

    // Creates an indexed mesh from the existing list of vertices, with the provided indices
    DGL_Mesh* EndMeshIndexed(unsigned* indices, unsigned indexCount);

    // Adds a new vertex to the list for creating a new mesh
    void AddVertex(const DGL_Vec2& position, const DGL_Color& color, const DGL_Vec2& texCoord);

    // Releases the mesh and deletes the struct
    void ReleaseMesh(DGL_Mesh* mesh);

    // Draws the mesh with the specified mode
    void DrawMesh(const DGL_Mesh* mesh, DGL_DrawMode mode);

    // Draws the mesh to the specified texture, with the specified mode
    // The texture must have been created as a render texture
    void DrawMeshToTexture(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* renderTexture);

    // Sets the transform data to be used when drawing the next mesh
    void SetTransformData(const DGL_Vec2& position, const DGL_Vec2& scale, float rotation);

    // Sets the transform matrix directly 
    void SetTransformMatrix(const DGL_Mat4& transformationMatrix);

    VulkanRenderer Renderer;
    CameraObject Camera;
    ShaderManager mShaderManager;

private:
    void CreateTransformMatrix();

    // Checks the draw call and draws the mesh (to the render texture, if it isn't null).
    // functionName is the API function, for error messages.
    void Draw(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* renderTexture, const char* functionName);

    std::unordered_map<const DGL_Texture*, std::unique_ptr<DGL_Texture>> mTextures;
    std::unordered_map<const DGL_Mesh*, std::unique_ptr<DGL_Mesh>> mMeshes;
    std::unordered_map<const DGL_PixelShader*, std::unique_ptr<DGL_PixelShader>> mPixelShaders;
    std::unordered_map<const DGL_VertexShader*, std::unique_ptr<DGL_VertexShader>> mVertexShaders;

    // The texture to use when drawing the next mesh
    const DGL_Texture* mCurrentTexture{ nullptr };
    // Tracks whether or not the graphics system has been initialized
    bool mInitialized{ false };
    // Tracks whether we need to recreate the transform matrix
    bool mCreateMatrix{ true };

    DGL_Vec2 mDrawPosition{ 0, 0 };
    DGL_Vec2 mDrawScale{ 0,0 };
    float mDrawRotation{ 0 };

    // Collects the vertices for the mesh being created
    MeshBuilder mMeshBuilder;
};

// Global pointer for accessing the graphics system
extern GraphicsSystem* gGraphics;

} // namepspace DGL
