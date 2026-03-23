//-------------------------------------------------------------------------------------------------
// file:    D3dInterface.ixx
// author:  Andy Ellinger
// brief:   Wrapper for D3D functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
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
    // Extra data which can be used by custom shaders
    DGL_Color mShaderVector{ 0 };

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

    // Returns true if StartUpdate has been called and EndUpdate has not been called
    bool IsUpdateStarted() const;

    // Set the blend mode to use on the next draw
    void SetBlendMode(DGL_BlendMode mode);

    // Set the sampler state to use on the next draw
    void SetSamplerState(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode);

    // Update the D3D constant buffer with the current stored data
    void UpdateConstantBuffer();

    // Adjust to a change in window size
    void ResetOnSizeChange();

    // Stores the constant buffer data that will be applied 
    cbPerObject mConstantBuffer;

    // The color that will be used to clear the render target view
    float mBackgroundColor[4]{ 0.0f, 0.0f, 0.0f, 1.0f };

    // The D3D input layout object
    ID3D11InputLayout* mInputLayout{ nullptr };

private:
    // Releases all D3D objects
    void Release();

    // Calls a set of functions to initialize D3D, and gets the current world matrix
    int InitializeD3D();

    // Creates the vertex shader and the two pixel shaders
    int InitializeShaders(const DGL_SysInitInfo& sysInitInfo);

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

    // Sets the current render target to a texture
    void SetRenderTargetToTexture(const DGL_Texture* renderTexture);

    // Resets the current render target back to the default
    void ResetRenderTarget();

    // The D3D device object
    ID3D11Device* mDevice{ nullptr };
    // The D3D device context object
    ID3D11DeviceContext* mDeviceContext{ nullptr };
    // The D3D swap chain object
    IDXGISwapChain* mSwapChain{ nullptr };
    // The D3D render target view object
    ID3D11RenderTargetView* mRenderTargetView{ nullptr };
    // The D3D constant buffer object
    ID3D11Buffer* mPerObjectBuffer{ nullptr };
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
