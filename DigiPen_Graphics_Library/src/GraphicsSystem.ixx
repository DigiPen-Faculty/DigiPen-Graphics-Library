//-------------------------------------------------------------------------------------------------
// file:    GraphicsSystem.ixx
// author:  Andy Ellinger
// brief:   Graphics system
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <vector>

export module GraphicsSystem;

import Camera;
import D3DInterface;
import Mesh;
import Shader;

namespace DGL
{

//---------------------------------------------------------------------------------- GraphicsSystem

export class GraphicsSystem
{
public:
    // Sets the global pointer
    GraphicsSystem();
    // Calls ShutDown if necessary
    ~GraphicsSystem();

    // Initializes the graphics system
    int Initialize(HWND window);

    // Shuts down the graphics system
    int ShutDown();

    // Loads a pixel shader from the provided file
    const DGL_PixelShader* LoadPixelShader(const char* fileName);

    // Releases the pixel shader and deletes the struct
    void ReleasePixelShader(const DGL_PixelShader* shader);

    // Loads a vertex shader from the provided file
    const DGL_VertexShader* LoadVertexShader(const char* filename);

    // Releases the vertex shader and deletes the struct
    void ReleaseVertexShader(const DGL_VertexShader* shader);

    // Loads a texture from the provided file
    DGL_Texture* LoadTexture(const char* fileName);

    // Loads a texture from the provided pixel data
    DGL_Texture* LoadTextureFromMemory(const unsigned char* data, int width, int height);

    // Releases the texture and deletes the struct
    void ReleaseTexture(DGL_Texture* texture);

    // Sets the texture to use when drawing a mesh
    void SetCurrentTexture(const DGL_Texture* texture);

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

    D3DInterface D3D;
    CameraObject Camera;

private:
    // The number of textures that have been loaded and not released
    int mTextures{ 0 };
    // The number of meshes that have been loaded and not released
    int mMeshes{ 0 };
    // The texture to use when drawing the next mesh
    const DGL_Texture* mCurrentTexture{ nullptr };
    // Tracks whether or not the graphics system has been initialized
    bool mInitialized{ false };
    // Tracks mesh creation status
    bool mCreatingMesh{ false };

    MeshManager Meshes;
    ShaderManager mShaderManager;
};

// Global pointer for accessing the graphics system
export extern GraphicsSystem* gGraphics;

} // namepspace DGL
