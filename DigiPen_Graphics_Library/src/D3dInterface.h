//-------------------------------------------------------------------------------------------------
// file:    D3dInterface.cpp
// author:  Andy Ellinger
// brief:   Wrapper for D3D functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include <d3d11.h>
#include <unordered_map>
using std::unordered_map;

namespace DGL
{

struct cbPerObject
{
    // The world view matrix, taking into account the window size, camera position, and scale
    DGL_Mat4 mWorldMatrix{ 
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    // The transformation matrix, with the position, rotation, and scale of the object being drawn
    DGL_Mat4 mTransformMatrix{ 0 };
    // The color that will be added to the object's color
    DGL_Color mTintColor{ 0 };
    // The texture offset coordinates to use when getting texture data for the object
    DGL_Vec2 mTexOffset{ 0 };
    // The alpha value to multiply with the color
    float mAlpha{ 1.0f };
    // This is not used - it is required to make the constant buffer a valid size
    float padding{ 0 };
};

//------------------------------------------------------------------------------------ D3DInterface

class D3DInterface
{
public:

    // Start a new drawing session
    void StartUpdate();

    // End the current drawing session and present the buffer
    void EndUpdate();

    // Set the blend mode to use on the next draw
    void SetBlendMode(DGL_BlendMode mode);

    // Set the sampler state to use on the next draw
    void SetSamplerState(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode);

    // Returns the current shader mode setting
    DGL_ShaderMode GetShaderMode();

    // Set the shader mode to use on the next draw
    void SetShaderMode(DGL_ShaderMode mode);

    // Get the current pixel shader, according to the shader mode
    ID3D11PixelShader* GetCurrentShader() const;

    // Update the D3D constant buffer with the current stored data
    void UpdateConstantBuffer();

    // Set the transformation matrix for the constant buffer
    void SetTransform(const DGL_Mat4* matrix);

    // Set the texture offset for the constant buffer
    void SetTextureOffset(const DGL_Vec2& offset);

    // Set the alpha value for the constant buffer
    void SetAlpha(float alpha);

    // Set the tint color for the constant buffer
    void SetTintColor(const DGL_Color* color);

    // Set the world matrix for the constant buffer
    void SetWorldMatrix(const DGL_Mat4& matrix);

    // Adjust to a change in window size
    void ResetOnSizeChange();

    // The color that will be used to clear the render target view
    float mBackgroundColor[4]{ 0.0f, 0.0f, 0.0f, 1.0f };

private:
    // Releases all D3D objects
    void Release();

    // Calls a set of functions to initialize D3D, and gets the current world matrix
    int InitializeD3D();

    // Creates the vertex shader and the two pixel shaders
    int InitializeShaders();

    // Creates the D3D device and the swap chain
    int CreateDevice();

    // Creates the render target view from the swap chain buffer
    int CreateRenderTarget();

    // Creates the rasterizer state
    int CreateRasterizerState();

    // Creates the blend states for all possible blend settings
    int CreateBlendStates();

    // Creates the D3D constant buffer
    int CreateConstantBuffer();

    // Sets the viewport data on the device context
    void SetViewport();

    // Creates the sampler states for all combinations of sample modes and texture address modes
    int CreateSamplers();

    // Stores the constant buffer data that will be applied
    cbPerObject mConstantBuffer;
    // The D3D device object
    ID3D11Device* mDevice{ nullptr };
    // The D3D device context object
    ID3D11DeviceContext* mDeviceContext{ nullptr };
    // The D3D swap chain object
    IDXGISwapChain* mSwapChain{ nullptr };
    // The D3D render target view object
    ID3D11RenderTargetView* mRenderTargetView{ nullptr };
    // The D3D vertex shader object
    ID3D11VertexShader* mVertexShader{ nullptr };
    // The D3D pixel shader object for color only
    ID3D11PixelShader* mPixelShader{ nullptr };
    // The D3D pixel shader object that uses textures
    ID3D11PixelShader* mPixelTextureShader{ nullptr };
    // The D3D input layout object
    ID3D11InputLayout* mInputLayout{ nullptr };
    // The D3D constant buffer object
    ID3D11Buffer* mPerObjectBuffer{ nullptr };
    // The current shader mode
    DGL_ShaderMode mCurrentShaderMode{ DGL_SM_COLOR };

    // The values for blend states
    enum class BlendStates { None, Transparent, Add, Multiply };
    // A map of blend state values to D3D blend state objects
    unordered_map<BlendStates, ID3D11BlendState*> mBlendStates;

    // The values for sample modes
    enum class SampleModes { Linear, Point };
    // The values for texture address modes
    enum class TextureAddressModes { Wrap, Mirror, Clamp, Mirror_Once };
    // A map of sample mode values to maps of texture address modes to D3D sampler state objects
    unordered_map <SampleModes, unordered_map<TextureAddressModes, ID3D11SamplerState*>> mSamplerStates;

    // The graphics system needs to be able to call private functions
    friend class GraphicsSystem;
};

} // namespace DGL
