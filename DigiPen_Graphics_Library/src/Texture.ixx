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

export typedef struct RenderTextureInfo
{
    // The D3D render target view object
    ID3D11RenderTargetView* renderTargetView{ nullptr };
    // The D3D viewport information
    D3D11_VIEWPORT viewport{ 0 };
    // The calculated world matrix for this texture
    DGL_Mat4 worldMatrix;

} RenderTextureInfo;

export typedef struct DGL_Texture
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
