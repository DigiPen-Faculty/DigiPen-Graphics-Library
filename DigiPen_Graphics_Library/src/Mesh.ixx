//-------------------------------------------------------------------------------------------------
// file:    Mesh.ixx
// author:  Andy Ellinger
// brief:   Header for mesh functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include "InternalTypes.h"
#include <d3d11.h>
#include <vector>

export module Mesh;

namespace DGL
{

//------------------------------------------------------------------------------------- MeshManager

export class MeshManager
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
        ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader, ID3D11DeviceContext* deviceContext);

    // List of vertex data to use when creating a new mesh
    std::vector<VertexData> mVertexList;

    static constexpr UINT vertex_stride{ sizeof(VertexData) };
    static constexpr UINT vertex_offset{ 0 };
};

} // namespace DGL