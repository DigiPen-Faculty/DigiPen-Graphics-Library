//-------------------------------------------------------------------------------------------------
// file:    D3dInterface.ixx
// author:  Andy Ellinger
// brief:   Wrapper for D3D functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <d3d11.h>
#include <unordered_map>

export module D3DInterface;

namespace DGL
{

using std::unordered_map;

export struct cbPerObject
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
    // Extra data which can be used by custom shaders
    float mShaderData{ 0 };

    // Note: if adding any additional variables, you must account
    // for the valid constant buffer sizes
};

//------------------------------------------------------------------------------------ D3DInterface

export class D3DInterface
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

    // Returns the current pixel shader mode setting
    DGL_PixelShaderMode GetPixelShaderMode() const;

    // Set the pixel shader mode to use on the next draw
    void SetPixelShaderMode(DGL_PixelShaderMode mode);

    // Returns the current vertex shader mode setting
    DGL_VertexShaderMode GetVertexShaderMode() const;

    // Set the vertex shader mode to use on the next draw
    void SetVertexShaderMode(DGL_VertexShaderMode mode);

    // Set the custom pixel shader to use with DGL_SM_CUSTOM
    void SetCustomPixelShader(const DGL_PixelShader* shader);

    // Set the custom vertex shader to use with DGL_SM_CUSTOM
    void SetCustomVertexShader(const DGL_VertexShader* shader);

    // Get the current pixel shader, according to the shader mode
    ID3D11PixelShader* GetCurrentPixelShader() const;

    // Get the current pixel shader, according to the shader mode
    ID3D11VertexShader* GetCurrentVertexShader() const;

    // Update the D3D constant buffer with the current stored data
    void UpdateConstantBuffer();

    // Adjust to a change in window size
    void ResetOnSizeChange();

    // Stores the constant buffer data that will be applied 
    cbPerObject mConstantBuffer;

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

    // Creates the depth-stencil resources
    int CreateDepthStencil();

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

    // The D3D device object
    ID3D11Device* mDevice{ nullptr };
    // The D3D device context object
    ID3D11DeviceContext* mDeviceContext{ nullptr };
    // The D3D swap chain object
    IDXGISwapChain* mSwapChain{ nullptr };
    // The D3D render target view object
    ID3D11RenderTargetView* mRenderTargetView{ nullptr };
    // The D3D depth stencil texture
    ID3D11Texture2D* mDepthStencil{ nullptr };
    // The D3D depth stencil view object
    ID3D11DepthStencilView* mDepthStencilView{ nullptr };
    // The D3D vertex shader object
    ID3D11VertexShader* mVertexShader{ nullptr };
    // The D3D pixel shader object for color only
    ID3D11PixelShader* mPixelShader{ nullptr };
    // The D3D pixel shader object that uses textures
    ID3D11PixelShader* mPixelTextureShader{ nullptr };
    // The custom D3D pixel shader that a user gives us
    ID3D11PixelShader* mPixelCustomShader{ nullptr };
    // The custom D3D vertex shader that a user gives us
    ID3D11VertexShader* mVertexCustomShader{ nullptr };
    // The D3D input layout object
    ID3D11InputLayout* mInputLayout{ nullptr };
    // The D3D constant buffer object
    ID3D11Buffer* mPerObjectBuffer{ nullptr };
    // The current pixel shader mode
    DGL_PixelShaderMode mCurrentPixelShaderMode{ DGL_PSM_COLOR };
    // The current vertex shader mode
    DGL_VertexShaderMode mCurrentVertexShaderMode{ DGL_VSM_DEFAULT };
    // Used to make sure StartUpdate is called before EndUpdate
    bool mUpdateStarted{ false };

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
