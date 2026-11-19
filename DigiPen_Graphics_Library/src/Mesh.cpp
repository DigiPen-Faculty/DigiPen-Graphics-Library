//-------------------------------------------------------------------------------------------------
// file:    Mesh.cpp
// author:  Andy Ellinger
// brief:   Mesh functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Mesh.h"
#include "Error.h"
#include "GraphicsSystem.h"


//*************************************************************************************************
DGL_Mesh::~DGL_Mesh()
{
    // The renderer destroys the buffers once the GPU is no longer using them
    DGL::gGraphics->Renderer.DestroyBuffer(vertexBuffer);
    DGL::gGraphics->Renderer.DestroyBuffer(indexBuffer);
}

namespace DGL
{

//------------------------------------------------------------------------------------- MeshBuilder

//*************************************************************************************************
void MeshBuilder::Start()
{
    mVertices.clear();
    mStarted = true;
}

//*************************************************************************************************
bool MeshBuilder::IsStarted() const
{
    return mStarted;
}

//*************************************************************************************************
void MeshBuilder::AddVertex(const VertexData& vertex)
{
    mVertices.push_back(vertex);
}

//*************************************************************************************************
size_t MeshBuilder::GetVertexCount() const
{
    return mVertices.size();
}

//*************************************************************************************************
std::unique_ptr<DGL_Mesh> MeshBuilder::Finish(const unsigned* indices, unsigned indexCount)
{
    mStarted = false;

    // The buffers are filled straight from the lists, so the mesh doesn't need its own copy.
    // If anything fails, deleting the mesh releases whatever was created.
    auto mesh = std::make_unique<DGL_Mesh>();
    mesh->vertexCount = (unsigned)mVertices.size();
    if (!gGraphics->Renderer.CreateBuffer(mVertices.data(), sizeof(VertexData) * mVertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mesh->vertexBuffer))
        return nullptr;

    if (indices)
    {
        mesh->indexCount = indexCount;
        if (!gGraphics->Renderer.CreateBuffer(indices, sizeof(unsigned) * indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            mesh->indexBuffer))
            return nullptr;
    }

    return mesh;
}

//*************************************************************************************************
void MeshBuilder::Cancel()
{
    mStarted = false;
}

} // namespace DGL
