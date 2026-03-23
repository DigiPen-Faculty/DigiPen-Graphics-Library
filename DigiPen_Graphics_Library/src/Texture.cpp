//-------------------------------------------------------------------------------------------------
// file:    Texture.cpp
// author:  Andy Ellinger
// brief:   Texture functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "InternalTypes.h"
#include "WICTextureLoader11.h"
#include <DirectXMath.h>
#include <sstream>

module Texture;

import Errors;
import Math;

namespace DGL
{
//---------------------------------------------------------------------------------- TextureManager

//*************************************************************************************************
DGL_Texture* TextureManager::LoadTexture(const char* pFileName, ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Trying to load texture when Graphics is not initialized.");
        return nullptr;
    }

    // Create the new texture object
    DGL_Texture* newTexture = new DGL_Texture;

    // Translate the file name to wide char
    std::wstring wideFileName;
    size_t fileNameSize = strlen(pFileName);
    for (unsigned i = 0; i < fileNameSize; ++i)
        wideFileName += (wchar_t)pFileName[i];

    // Temporary variable for creating the texture
    ID3D11Resource* temp;

    // Create the texture using the WIC texture loader
    HRESULT hr = DirectX::CreateWICTextureFromFileEx(
        device,
        wideFileName.c_str(),
        0,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE,
        0,
        0,
        DirectX::WIC_LOADER_IGNORE_SRGB,
        &temp,
        &newTexture->texResourceView
    );
    if (FAILED(hr))
    {
        // If it didn't work, set the error message and delete the texture
        std::stringstream stream;
        stream << "Failed to load texture from file \"" << pFileName << "\". ";
        gError->SetError(stream.str(), hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Save the texture as a Texture2D
    newTexture->texture = (ID3D11Texture2D*)temp;

    // Save the size of the texture
    D3D11_TEXTURE2D_DESC texInfo{ 0 };
    newTexture->texture->GetDesc(&texInfo);
    newTexture->textureSize.x = (float)texInfo.Width;
    newTexture->textureSize.y = (float)texInfo.Height;

    // Return the new texture object
    return newTexture;
}

//*************************************************************************************************
DGL_Texture* TextureManager::LoadTextureFromMemory(const unsigned char* data, int width, int height, 
    ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Trying to load texture when Graphics is not initialized.");
        return nullptr;
    }

    // Check for invalid data
    if (!data || width == 0 || height == 0)
        return nullptr;

    // Create the new texture object
    DGL_Texture* newTexture = new DGL_Texture;

    // Set up the subresource data struct
    D3D11_SUBRESOURCE_DATA subrecData = { 0 };
    subrecData.pSysMem = data;
    subrecData.SysMemPitch = width * sizeof(uint32_t);

    // Set up the texture description struct
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // Create the texture using the subresource and texture structs
    HRESULT hr = device->CreateTexture2D(&texDesc, &subrecData, &newTexture->texture);
    if (FAILED(hr))
    {
        // If it didn't work, set the error message and delete the texture
        gError->SetError("Failed to load texture from memory. ", hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Set up the shader resource view description
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    ZeroMemory(&srDesc, sizeof(srDesc));
    srDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MipLevels = 1;
    srDesc.Texture2D.MostDetailedMip = 0;

    // Create the shader resource view using the texture and shader resource struct
    hr = device->CreateShaderResourceView(newTexture->texture, &srDesc,
        &newTexture->texResourceView);
    if (FAILED(hr))
    {
        // If it didn't work, set the error message and delete the texture
        gError->SetError("Problem creating shader resource for texture from memory. ", hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Save the size of the texture
    newTexture->textureSize.x = (float)width;
    newTexture->textureSize.y = (float)height;

    // Return the new texture object
    return newTexture;
}

//*************************************************************************************************
void TextureManager::ReleaseTexture(DGL_Texture* texture)
{
    if (!texture)
        return;

    // Release the D3D objects
    if (texture->renderInfo)
    {
        if (texture->renderInfo->renderTargetView)
            texture->renderInfo->renderTargetView->Release();
        delete texture->renderInfo;
    }
    if (texture->texResourceView)
        texture->texResourceView->Release();
    if (texture->texture)
        texture->texture->Release();

    // Delete the DGL struct
    delete texture;
}

//*************************************************************************************************
DGL_Texture* TextureManager::CreateRenderTexture(int width, int height, ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Trying to create texture when Graphics is not initialized.");
        return nullptr;
    }

    // Check for invalid data
    if (width == 0 || height == 0)
        return nullptr;

    // Set up the texture description struct
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // Create the new texture object
    DGL_Texture* newTexture = new DGL_Texture;

    // Create the texture 
    HRESULT hr = device->CreateTexture2D(&texDesc, 0, &newTexture->texture);
    if (FAILED(hr))
    {
        // If it didn't work, set the error message and delete the texture
        gError->SetError("Failed to create texture for rendering. ", hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Create the render info struct
    newTexture->renderInfo = new RenderTextureInfo();

    // Set up the render target view description struct
    D3D11_RENDER_TARGET_VIEW_DESC renderDesc;
    ZeroMemory(&renderDesc, sizeof(renderDesc));
    renderDesc.Format = texDesc.Format;
    renderDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderDesc.Texture2D.MipSlice = 0;

    // Create the render target view
    hr = device->CreateRenderTargetView(newTexture->texture, &renderDesc, &newTexture->renderInfo->renderTargetView);
    if (FAILED(hr))
    {
        // If it didn't work, set the error message and delete the texture
        gError->SetError("Problem creating render target view for texture for rendering. ", hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Set up the shader resource view description
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    ZeroMemory(&srDesc, sizeof(srDesc));
    srDesc.Format = texDesc.Format;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MipLevels = 1;
    srDesc.Texture2D.MostDetailedMip = 0;

    // Create the shader resource view 
    hr = device->CreateShaderResourceView(newTexture->texture, &srDesc,
        &newTexture->texResourceView);
    if (FAILED(hr))
    {
        // If it didn't work, set the error message and delete the texture
        gError->SetError("Problem creating shader resource for texture for rendering. ", hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Set up the viewport
    newTexture->renderInfo->viewport.Width = (float)width;
    newTexture->renderInfo->viewport.Height = (float)height;
    newTexture->renderInfo->viewport.MaxDepth = 1.0f;
    newTexture->renderInfo->viewport.MinDepth = 0.0f;
    newTexture->renderInfo->viewport.TopLeftX = 0;
    newTexture->renderInfo->viewport.TopLeftY = 0;

    // Create the world matrix

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(
        DirectX::XMVectorSet(0.0f, 0.0f, -10.0f, 0.0f),   // eye position
        DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),   // eye looking at
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)    // up direction
    );

    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixOrthographicLH(
        (float)width, (float)height, 1.0f, 1000.0f );

    newTexture->renderInfo->worldMatrix = DxToMat4(DirectX::XMMatrixTranspose(viewMatrix * projectionMatrix));

    // Save the size of the texture
    newTexture->textureSize.x = (float)width;
    newTexture->textureSize.y = (float)height;

    // Return the new texture object
    return newTexture;
}

//*************************************************************************************************
void TextureManager::ClearRenderTexture(const DGL_Texture* renderTexture, const DGL_Color& color,
    ID3D11DeviceContext* context)
{
    if (!renderTexture || !renderTexture->renderInfo || !renderTexture->renderInfo->renderTargetView)
        return;

    context->ClearRenderTargetView(renderTexture->renderInfo->renderTargetView, &(color.r));
}

} // namespace DGL
