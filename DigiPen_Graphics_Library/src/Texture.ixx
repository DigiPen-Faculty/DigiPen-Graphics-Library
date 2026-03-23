//-------------------------------------------------------------------------------------------------
// file:    Texture.ixx
// author:  Andy Ellinger
// brief:   Header for texture functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include <d3d11.h>
#include <DirectXMath.h>
#include "DGL.h"

export module Texture;

namespace DGL
{

//---------------------------------------------------------------------------------- TextureManager

export class TextureManager
{
public:

    // Creates a new texture from the provided file name
    static DGL_Texture* LoadTexture(const char* pFileName, ID3D11Device* device);

    // Creates a new texture from the provided pixel data
    static DGL_Texture* LoadTextureFromMemory(const unsigned char* data, int width, int height, 
        ID3D11Device* device);

    // Releases the D3D objects and deletes the texture
    static void ReleaseTexture(DGL_Texture* texture);

    // Creates a new texture to be used for rendering
    static DGL_Texture* CreateRenderTexture(int width, int height, ID3D11Device* device);

    // Clears the specified render texture with the specified color
    static void ClearRenderTexture(const DGL_Texture* renderTexture, const DGL_Color& color, 
        ID3D11DeviceContext* context);

};

} // namespace DGL
