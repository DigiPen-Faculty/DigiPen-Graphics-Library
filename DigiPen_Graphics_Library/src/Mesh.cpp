//-------------------------------------------------------------------------------------------------
// file:    Mesh.cpp
// author:  Andy Ellinger
// brief:   Mesh functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <d3d11.h>
#include <vector>

module Mesh;

import Errors;
import Texture;
import GraphicsSystem;

namespace DGL
{
//------------------------------------------------------------------------------------- MeshManager

//*************************************************************************************************
DGL_Mesh* MeshManager::CreateMesh(ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Trying to create mesh when Graphics is not initialized.");
        return nullptr;
    }

    // Check to make sure there are vertices in the list
    if (mVertexList.size() == 0)
    {
        gError->SetError("Couldn't create mesh, no vertices added.");
        return nullptr;
    }

    // Create the new mesh object
    DGL_Mesh* newMesh = new DGL_Mesh;

    // Save the number of vertices
    newMesh->mVertexCount = (unsigned)mVertexList.size();
    // Copy the vertices into the mesh's list
    newMesh->mVertexList = new VertexData[newMesh->mVertexCount];
    memcpy(newMesh->mVertexList, mVertexList.data(), sizeof(VertexData) * newMesh->mVertexCount);

    // Set up the vertex buffer description struct
    D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
    vertexBufferDesc.ByteWidth = sizeof(VertexData) * newMesh->mVertexCount;
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // Set up the subresource data struct
    D3D11_SUBRESOURCE_DATA srData = { 0 };
    srData.pSysMem = newMesh->mVertexList;
    // Create the vertex buffer
    HRESULT hr = device->CreateBuffer(
        &vertexBufferDesc,
        &srData,
        &newMesh->mVertexBuffer
    );
    if (FAILED(hr))
    {
        // If there was a problem, set the error message and delete the mesh
        gError->SetError("Problem creating vertex buffer for mesh. ", hr);
        ReleaseMesh(newMesh);
        return nullptr;
    }

    return newMesh;
}

//*************************************************************************************************
DGL_Mesh* MeshManager::CreateMeshIndexed(unsigned* indices, unsigned indexCount, ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Trying to create mesh when Graphics is not initialized.");
        return nullptr;
    }

    // Make sure there are indices to use
    if (indexCount == 0)
    {
        gError->SetError("Couldn't create indexed mesh, no indexes.");
        return nullptr;
    }

    // Create the basic mesh
    DGL_Mesh* newMesh = MeshManager::CreateMesh(device);
    if (!newMesh)
        return nullptr;

    // Save the indices
    newMesh->mIndexCount = indexCount;
    newMesh->mIndices = new unsigned[indexCount];
    memcpy(newMesh->mIndices, indices, sizeof(unsigned) * indexCount);

    // Set up the index buffer description struct
    D3D11_BUFFER_DESC indexBufferDesc = { 0 };
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    // Set up the subresource data struct
    D3D11_SUBRESOURCE_DATA srData = { 0 };
    srData.pSysMem = indices;
    // Create the index buffer
    HRESULT hr = device->CreateBuffer(&indexBufferDesc, &srData, &newMesh->mIndexBuffer);
    if (FAILED(hr))
    {
        // If there was a problem, set the error message and delete the mesh
        gError->SetError("Problem creating index buffer for mesh. ", hr);
        ReleaseMesh(newMesh);
        return nullptr;
    }

    return newMesh;
}

//*************************************************************************************************
void MeshManager::ReleaseMesh(DGL_Mesh* mesh)
{
    if (!mesh)
        return;

    // Release the vertex and index buffers (if they exist)
    if (mesh->mVertexBuffer) 
        mesh->mVertexBuffer->Release();
    if (mesh->mIndexBuffer) 
        mesh->mIndexBuffer->Release();

    // Delete the lists, if necessary
    if (mesh->mVertexList)
        delete mesh->mVertexList;
    if (mesh->mIndices)
        delete mesh->mIndices;

    // Delete the DGL mesh
    delete mesh;
}

//*************************************************************************************************
void MeshManager::Draw(const DGL_Mesh* mesh, DGL_DrawMode mode, const DGL_Texture* texture, 
    ID3D11VertexShader* vertexShader, ID3D11PixelShader* pixelShader, ID3D11DeviceContext* deviceContext)
{
    if (!deviceContext)
    {
        gError->SetError("Trying to draw mesh when Graphics is not initialized.");
        return;
    }

    // Set the primitive topology setting as specified
    switch (mode)
    {
    case DGL_DM_TRIANGLELIST:
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        break;
    case DGL_DM_LINELIST:
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        break;
    case DGL_DM_LINESTRIP:
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        break;
    case DGL_DM_POINTLIST:
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        break;
    case DGL_DM_TRIANGLESTRIP:
        deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        break;
    default:
        gError->SetError("Passed in an invalid DGL_DrawMode value to DGL_Graphics_DrawMesh.");
        break;
    }

    // Set the shader
    deviceContext->VSSetShader(vertexShader, NULL, 0);
    deviceContext->PSSetShader(pixelShader, NULL, 0);

    // If there is a texture, set the shader resource
    if (gGraphics->D3D.GetPixelShaderMode() != DGL_PSM_COLOR && texture)
        deviceContext->PSSetShaderResources(0, 1, &(texture->texResourceView));
    else
    {
        ID3D11ShaderResourceView* nullSRV = { nullptr };
        deviceContext->PSSetShaderResources(0, 1, &nullSRV);
    }


    // Set the vertex buffer
    deviceContext->IASetVertexBuffers(
        0,
        1,
        &mesh->mVertexBuffer,
        &vertex_stride,
        &vertex_offset
    );

    // Update the constant buffer data
    gGraphics->D3D.UpdateConstantBuffer();

    // If the mesh is not indexed, draw it normally
    if (mesh->mIndexCount == 0)
        deviceContext->Draw(mesh->mVertexCount, 0);
    else
    {
        // Set the index buffer
        deviceContext->IASetIndexBuffer(mesh->mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        // Draw the indexed mesh
        deviceContext->DrawIndexed(mesh->mIndexCount, 0, 0);
    }
}

} // namespace DGL
