//-------------------------------------------------------------------------------------------------
// file:    Texture.h
// author:  Andy Ellinger
// brief:   Header for texture functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include <d3d11.h>

typedef struct DGL_Texture
{
    // The D3D 2D texture object
    ID3D11Texture2D* texture{ nullptr };
    // The D3D shader resource view object
    ID3D11ShaderResourceView* texResourceView{ nullptr };
} DGL_Texture;

namespace DGL
{

//---------------------------------------------------------------------------------- TextureManager

class TextureManager
{
public:

    // Creates a new texture from the provided file name
    static DGL_Texture* LoadTexture(const char* pFileName, ID3D11Device* device);

    // Creates a new texture from the provided pixel data
    static DGL_Texture* LoadTextureFromMemory(const unsigned char* data, int width, int height, 
        ID3D11Device* device);

    // Releases the D3D objects and deletes the texture
    static void ReleaseTexture(DGL_Texture* texture);

};

} // namespace DGL
