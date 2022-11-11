//-------------------------------------------------------------------------------------------------
// file:    Mesh.h
// author:  Andy Ellinger
// brief:   Header for mesh functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include <d3d11.h>
#include <vector>

typedef struct
{
    // The position of this vertex
    DGL_Vec2 mPosition;
    // The color associated with this vertex
    DGL_Color mColor;
    // The texture coordinates associated with this vertex
    DGL_Vec2 mTexCoord;
} VertexData;

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


namespace DGL
{

//------------------------------------------------------------------------------------- MeshManager

class MeshManager
{
public:

    // Creates a new mesh based on the current vertices in the list
    DGL_Mesh* CreateMesh(ID3D11Device* device);

    // Creates a new indexed mesh based on the current vertices in the list 
    // and the provided index list
    DGL_Mesh* CreateMeshIndexed(unsigned* indices, unsigned indexCount, ID3D11Device* device);

    // Releases the data in the provided mesh and deletes the mesh object
    static void ReleaseMesh(DGL_Mesh* mesh);

    // Draws the mesh with the provided mode, texture, and shader
    static void Draw(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* texture,
        ID3D11PixelShader* shader, ID3D11DeviceContext* deviceContext);

    // List of vertex data to use when creating a new mesh
    std::vector<VertexData> mVertexList;

    static constexpr UINT vertex_stride{ sizeof(VertexData) };
    static constexpr UINT vertex_offset{ 0 };
};

} // namespace DGL