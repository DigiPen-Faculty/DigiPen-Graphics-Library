//-------------------------------------------------------------------------------------------------
// file:    InternalTypes.h
// author:  Andy Ellinger
// brief:   Definitions of exposed types that are not defined in the public header file
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <string_view>
#include <string>

// Data used for a render texture, stored in DGL_Texture if the texture is a render texture.
typedef struct RenderTextureInfo
{
    // The D3D render target view object
    ID3D11RenderTargetView* renderTargetView{ nullptr };
    // The D3D viewport information
    D3D11_VIEWPORT viewport{ 0 };
    // The calculated world matrix for this texture
    DGL_Mat4 worldMatrix;

} RenderTextureInfo;

// Stores information about a texture.
typedef struct DGL_Texture
{
    // The D3D 2D texture object
    ID3D11Texture2D* texture{ nullptr };
    // The D3D shader resource view object
    ID3D11ShaderResourceView* texResourceView{ nullptr };
    // The render texture info (if applicable)
    RenderTextureInfo* renderInfo{ nullptr };
    // Width and height of the texture
    DGL_Vec2 textureSize{ 0 };
} DGL_Texture;

// Stores information about a pixel shader, automatically releases the shader when destroyed.
typedef struct DGL_PixelShader
{
    DGL_PixelShader(std::string_view name) :
        name(name)
    {}
    DGL_PixelShader(const DGL_PixelShader& other) = delete;

    ~DGL_PixelShader() { if (shader) shader->Release(); }

    ID3D11PixelShader* shader{ nullptr };
    std::string name;
}DGL_PixelShader;

// Stores information about a vertex shader, automatically releases the shader when destroyed.
typedef struct DGL_VertexShader
{
    DGL_VertexShader(std::string_view name) :
        filename(name)
    {}
    DGL_VertexShader(const DGL_VertexShader& other) = delete;

    ~DGL_VertexShader() { if (shader) shader->Release(); }

    ID3D11VertexShader* shader{ nullptr };
    std::string filename;
}DGL_VertexShader;

// Stores the data for a single vertex, including position, color, and texture coordinates.
// Meshes use a list of these to store their vertex data.
typedef struct VertexData
{
    // The position of this vertex
    DGL_Vec2 mPosition;
    // The color associated with this vertex
    DGL_Color mColor;
    // The texture coordinates associated with this vertex
    DGL_Vec2 mTexCoord;
} VertexData;

// Stores information about a mesh, including vertex data, index data (if applicable), and D3D buffer objects.
typedef struct DGL_Mesh
{
    // The list of vertex data for this mesh
    VertexData* mVertexList{ nullptr };
    // The number of vertices in this mesh
    unsigned mVertexCount{ 0 };
    // The array of indices for an indexed mesh (will be null for a non-indexed mesh)
    unsigned* mIndices{ nullptr };
    // The number of indices in the index array
    unsigned mIndexCount{ 0 };
    // The D3D vertex buffer object
    ID3D11Buffer* mVertexBuffer{ nullptr };
    // The D3D index buffer object
    ID3D11Buffer* mIndexBuffer{ nullptr };
} DGL_Mesh;