//-------------------------------------------------------------------------------------------------
// file:    Texture.cpp
// author:  Andy Ellinger
// brief:   Texture functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Texture.h"
#include "Error.h"
#include "WICTextureLoader11.h"

namespace DGL
{
//---------------------------------------------------------------------------------- TextureManager

//*************************************************************************************************
DGL_Texture* TextureManager::LoadTexture(const char* pFileName, ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Graphics is not initialized");
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
        stream << "Problem loading texture " << pFileName << " from file: error " << hr;
        gError->SetError(stream.str());
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Save the texture as a Texture2D
    newTexture->texture = (ID3D11Texture2D*)temp;

    // Return the new texture object
    return newTexture;
}

//*************************************************************************************************
DGL_Texture* TextureManager::LoadTextureFromMemory(const unsigned char* data, int width, int height, 
    ID3D11Device* device)
{
    if (!device)
    {
        gError->SetError("Graphics is not initialized");
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
        gError->SetError("Problem creating texture from memory: error ", hr);
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
        gError->SetError("Problem creating shader resource for texture from memory: error ", hr);
        ReleaseTexture(newTexture);
        return nullptr;
    }

    // Return the new texture object
    return newTexture;
}

//*************************************************************************************************
void TextureManager::ReleaseTexture(DGL_Texture* texture)
{
    if (!texture)
        return;

    // Release the texture and shader resource view
    if (texture->texResourceView) 
        texture->texResourceView->Release();
    if (texture->texture) 
        texture->texture->Release();

    // Delete the DGL struct
    delete texture;
}

} // namespace DGL
