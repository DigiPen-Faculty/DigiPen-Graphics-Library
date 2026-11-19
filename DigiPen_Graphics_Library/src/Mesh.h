//-------------------------------------------------------------------------------------------------
// file:    Mesh.h
// author:  Andy Ellinger
// brief:   Header for mesh functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include "InternalTypes.h"
#include <memory>
#include <vector>

namespace DGL
{

//------------------------------------------------------------------------------------- MeshBuilder

// Collects vertices between DGL_Graphics_StartMesh and DGL_Graphics_EndMesh, then creates the mesh.
// The calls are checked by the graphics system before they get here.
class MeshBuilder
{
public:
    // Starts a new mesh, discarding any vertices from a previous one
    void Start();

    // Returns true if a mesh has been started and not finished
    bool IsStarted() const;

    // Adds a vertex to the mesh being built
    void AddVertex(const VertexData& vertex);

    // Returns the number of vertices added so far
    size_t GetVertexCount() const;

    // Creates a mesh from the vertices (indexed, if indices is not null) and stops building.
    // Returns null if there was an error.
    std::unique_ptr<DGL_Mesh> Finish(const unsigned* indices = nullptr, unsigned indexCount = 0);

    // Stops building without creating a mesh
    void Cancel();

private:
    std::vector<VertexData> mVertices;
    bool mStarted{ false };
};


} // namespace DGL
