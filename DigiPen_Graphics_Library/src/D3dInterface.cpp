//-------------------------------------------------------------------------------------------------
// file:    D3dInterface.cpp
// author:  Andy Ellinger, Kenny Mecham
// brief:   Wrapper for D3D functionality
//
// Copyright © 2022 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <d3d11.h>
#include "VShader.h"
#include "PShader.h"
#include "PTexShader.h"

module D3DInterface;

import Errors;
import GraphicsSystem;
import WindowsSystem;
import Shader;

namespace DGL
{

//*************************************************************************************************
template<typename RefType>
void SafeRelease(RefType& pInterface)
{
    if (pInterface) pInterface->Release();
    pInterface = nullptr;
}

//------------------------------------------------------------------------------------ D3DInterface

//*************************************************************************************************
void D3DInterface::StartUpdate()
{
    if (!mDeviceContext)
    {
        gError->SetError("Started Graphics update when not initialized.");
        return;
    }

    // Clear the render target view with the current background color
    mDeviceContext->ClearRenderTargetView(mRenderTargetView, mBackgroundColor);
    // Clear the depth stencil view
    if (mDepthStencilView)
        mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    // Set the input layout
    mDeviceContext->IASetInputLayout(mInputLayout);

    // Set the render target and depth stencil view
    mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

    // Set the tracking flag
    mUpdateStarted = true;
}

//*************************************************************************************************
void D3DInterface::EndUpdate()
{
    if (!mSwapChain)
    {
        gError->SetError("Ended Graphics update when not initialized.");
        return;
    }
    else if (!mUpdateStarted)
    {
        gError->SetError("Called DGL_Graphics_FinishDrawing without calling DGL_Graphics_StartDrawing.");
        return;
    }

    // Send the current buffer of data to be displayed
    mSwapChain->Present(1, 0);

    // Reset the tracking flag
    mUpdateStarted = false;
}

//*************************************************************************************************
void D3DInterface::SetBlendMode(DGL_BlendMode mode)
{
    if (!mDeviceContext)
    {
        gError->SetError("Tried to set blend mode when Graphics not initialized.");
        return;
    }

    // Choose the appropriate blend state 
    ID3D11BlendState* blendState{ nullptr };
    switch (mode)
    {
    case DGL_BM_NONE:
        blendState = mBlendStates[BlendStates::None];
        break;
    case DGL_BM_ADD:
        blendState = mBlendStates[BlendStates::Add];
        break;
    case DGL_BM_BLEND:
        blendState = mBlendStates[BlendStates::Transparent];
        break;
    case DGL_BM_MULTIPLY:
        blendState = mBlendStates[BlendStates::Multiply];
        break;
    default:
        gError->SetError("Passed in an invalid DGL_BlendMode value to DGL_Graphics_SetBlendMode.");
        break;
    }

    // Set the blend state on the D3D device context
    mDeviceContext->OMSetBlendState(blendState, NULL, 0xffffffff);
}

//*************************************************************************************************
void D3DInterface::SetSamplerState(DGL_TextureSampleMode newSampleMode, DGL_TextureAddressMode addressMode)
{
    if (!mDeviceContext)
    {
        gError->SetError("Tried to set sampler state when Graphics not initialized.");
        return;
    }

    // Choose the appropriate sample mode
    SampleModes sampleMode;
    if (newSampleMode == DGL_TSM_LINEAR)
        sampleMode = SampleModes::Linear;
    else if (newSampleMode == DGL_TSM_POINT)
        sampleMode = SampleModes::Point;
    else
    {
        gError->SetError("Passed in an invalid DGL_TextureSampleMode value to DGL_Graphics_SetTextureSamplerData.");
        return;
    }

    // Use the sample mode and address mode to index into the arrays to get the sampler state
    ID3D11SamplerState** samplerState{ nullptr };
    switch (addressMode)
    {
    case DGL_AM_CLAMP:
        samplerState = &(mSamplerStates[sampleMode][TextureAddressModes::Clamp]);
        break;
    case DGL_AM_MIRROR:
        samplerState = &(mSamplerStates[sampleMode][TextureAddressModes::Mirror]);
        break;
    case DGL_AM_MIRROR_ONCE:
        samplerState = &(mSamplerStates[sampleMode][TextureAddressModes::Mirror_Once]);
        break;
    case DGL_AM_WRAP:
        samplerState = &(mSamplerStates[sampleMode][TextureAddressModes::Wrap]);
        break;
    default:
        gError->SetError("Passed in an invalid DGL_TextureAddressMode value to DGL_Graphics_SetTextureSamplerData.");
        break;
    }

    // Set the sampler for the pixel shader on the D3D device context
    mDeviceContext->PSSetSamplers(0, 1, samplerState);
}

//*************************************************************************************************
DGL_PixelShaderMode D3DInterface::GetPixelShaderMode() const
{
    return mCurrentPixelShaderMode;
}

//*************************************************************************************************
void D3DInterface::SetPixelShaderMode(DGL_PixelShaderMode mode)
{
    if (mode < 0 || mode > DGL_PSM_CUSTOM)
    {
        gError->SetError("Passed in an invalid DGL_PixelShaderMode value to DGL_Graphics_SetShaderMode.");
        return;
    }

    // Set the shader mode to use on the next draw call
    mCurrentPixelShaderMode = mode;
}

//*************************************************************************************************
DGL_VertexShaderMode D3DInterface::GetVertexShaderMode() const
{
    return mCurrentVertexShaderMode;
}

//*************************************************************************************************
void D3DInterface::SetVertexShaderMode(DGL_VertexShaderMode mode)
{
    if (mode < 0 || mode > DGL_VSM_CUSTOM)
    {
        gError->SetError("Passed in an invalid DGL_VertexShaderMode value to DGL_Graphics_SetShaderMode.");
        return;
    }

    // Set the shader mode to use on the next draw call
    mCurrentVertexShaderMode = mode;
}

//*************************************************************************************************
void D3DInterface::SetCustomPixelShader(const DGL_PixelShader* shader)
{
    if (shader)
    {
        mPixelCustomShader = shader->shader;
    }
    else
    {
        mPixelCustomShader = nullptr;
    }
}

//*************************************************************************************************
void D3DInterface::SetCustomVertexShader(const DGL_VertexShader* shader)
{
    if (shader)
    {
        mVertexCustomShader = shader->shader;
    }
    else
    {
        mVertexCustomShader = nullptr;
    }
}

//*************************************************************************************************
ID3D11PixelShader* D3DInterface::GetCurrentPixelShader() const
{
    // Return the appropriate pixel shader for the current shader mode
    switch (GetPixelShaderMode())
    {
        case DGL_PSM_TEXTURE: return mPixelTextureShader;
        case DGL_PSM_COLOR:   return mPixelShader;
        case DGL_PSM_CUSTOM:  return mPixelCustomShader;
    }

    return nullptr;
}

//*************************************************************************************************
ID3D11VertexShader* D3DInterface::GetCurrentVertexShader() const
{
    switch (GetVertexShaderMode())
    {
        case DGL_VSM_DEFAULT: return mVertexShader;
        case DGL_VSM_CUSTOM:  return mVertexCustomShader;
    }

    return nullptr;
}

//*************************************************************************************************
void D3DInterface::UpdateConstantBuffer()
{
    if (!mDeviceContext)
    {
        gError->SetError("Tried to set constant buffer when Graphics is not initialized.");
        return;
    }

    // Update the constant buffer resource
    mDeviceContext->UpdateSubresource(mPerObjectBuffer, 0, NULL, &mConstantBuffer, 0, 0);
    // Set the constant buffer
    mDeviceContext->VSSetConstantBuffers(0, 1, &mPerObjectBuffer);
}

//*************************************************************************************************
void D3DInterface::ResetOnSizeChange()
{
    // If we have no current device context, do nothing
    if (!mDeviceContext)
    {
        return;
    }

    // Reset the render target
    mDeviceContext->OMSetRenderTargets(0, 0, 0);

    // Release all references to swap chain buffer
    mRenderTargetView->Release();

    // Preserve existing buffer count and format, automatically choose width & height to match client rect
    HRESULT hr = mSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr))
    {
        // If this failed, set the error and return
        gError->SetError("Problem resizing swap chain buffer. ", hr);
        return;
    }

    CreateRenderTarget();

    SetViewport();

    // Get the updated world matrix for the constant buffer
    mConstantBuffer.mWorldMatrix = gGraphics->Camera.GetWorldMatrix();
}

//*************************************************************************************************
void D3DInterface::Release()
{
#ifdef DEBUG_REPORT_DEVICES
    ID3D11Debug* DebugDevice = nullptr;
    HRESULT hr = mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&DebugDevice));
#endif

    // Release all the blend states
    for (auto [key, value] : mBlendStates)
        SafeRelease(value);

    // Release all the sampler states
    for (auto [key, value] : mSamplerStates)
    {
        for (auto [key2, value2] : value)
            SafeRelease(value2);
    }

    // Release all other D3D objects
    SafeRelease(mPerObjectBuffer);
    SafeRelease(mInputLayout);
    SafeRelease(mPixelShader);
    SafeRelease(mPixelTextureShader);
    SafeRelease(mVertexShader);
    SafeRelease(mDepthStencilView);
    SafeRelease(mDepthStencil);
    SafeRelease(mRenderTargetView);
    SafeRelease(mSwapChain);
    SafeRelease(mDeviceContext);
    SafeRelease(mDevice);

#ifdef DEBUG_REPORT_DEVICES
    if (DebugDevice)
    {
        hr = DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
        DebugDevice->Release();
    }
#endif
}

//*************************************************************************************************
int D3DInterface::InitializeD3D()
{
    if (CreateDevice() == 1)
        return 1;

    if (CreateRenderTarget() == 1)
        return 1;

    if (CreateRasterizerState() == 1)
        return 1;

    if (CreateBlendStates() == 1)
        return 1;

    if (CreateConstantBuffer() == 1)
        return 1;

    if (CreateSamplers() == 1)
        return 1;

    if (CreateDepthStencil() == 1)
        return 1;

    SetViewport();

    mConstantBuffer.mWorldMatrix = gGraphics->Camera.GetWorldMatrix();

    return 0;
}

//*************************************************************************************************
int D3DInterface::InitializeShaders()
{
    // Create vertex shader from compiled header
    HRESULT hr = mDevice->CreateVertexShader(
        gVShader,
        sizeof(gVShader),
        nullptr,
        &mVertexShader
    );
    if (FAILED(hr))
    {
        gError->SetError("Problem creating default vertex shader. ", hr);
        return 1;
    }

    // Create pixel shader from compiled header
    hr = mDevice->CreatePixelShader(
        gPShader,
        sizeof(gPShader),
        nullptr,
        &mPixelShader
    );
    if (FAILED(hr))
    {
        gError->SetError("Problem creating default pixel color shader. ", hr);
        return 1;
    }

    // Create pixel shader for textures from compiled header
    hr = mDevice->CreatePixelShader(
        gPTexShader,
        sizeof(gPTexShader),
        nullptr,
        &mPixelTextureShader
    );
    if (FAILED(hr))
    {
        gError->SetError("Problem creating default pixel texture shader. ", hr);
        return 1;
    }

    // Create input description struct
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    // Create and save input layout
    hr = mDevice->CreateInputLayout(
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        gVShader,
        sizeof(gVShader),
        &mInputLayout
    );
    if (FAILED(hr))
    {
        gError->SetError("Problem creating shader input layout. ", hr);
        return 1;
    }

    // Set the shaders and sampler to defaults
    mDeviceContext->VSSetShader(mVertexShader, NULL, 0);
    mDeviceContext->PSSetShader(mPixelTextureShader, NULL, 0);
    mDeviceContext->PSSetSamplers(0, 1, &(mSamplerStates[SampleModes::Linear][TextureAddressModes::Wrap]));

    return 0;
}

//*************************************************************************************************
int D3DInterface::CreateDevice()
{
    // Set flags
    D3D_FEATURE_LEVEL d3dFeatureLevel;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined(DEBUG) || defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create swap chain description struct
    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = gWinSys->GetWindowHandle();
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // Create D3D device and swap chain
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &mSwapChain,
        &mDevice,
        &d3dFeatureLevel,
        &mDeviceContext
    );
    if (FAILED(hr) || !mSwapChain || !mDevice || !mDeviceContext)
    {
        // If in debug mode, try again without the debug device flag
#if defined(DEBUG) || defined(_DEBUG)
        flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
        hr = D3D11CreateDeviceAndSwapChain(
            NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            flags,
            NULL,
            0,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &mSwapChain,
            &mDevice,
            &d3dFeatureLevel,
            &mDeviceContext
        );

        // If it worked this time, return succesfully
        if (!FAILED(hr) && mSwapChain && mDevice && mDeviceContext)
            return 0;
#endif

        gError->SetError("Problem creating D3D device. ", hr);
        return 1;
    }

    return 0;
}

//*************************************************************************************************
int D3DInterface::CreateRenderTarget()
{
    // Create frame buffer
    ID3D11Texture2D* frameBuffer;
    HRESULT hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frameBuffer);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating frame buffer. ", hr);
        return 1;
    }

    // Create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
    ZeroMemory(&rtDesc, sizeof(rtDesc));
    rtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = mDevice->CreateRenderTargetView(frameBuffer, &rtDesc, &mRenderTargetView);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating render target. ", hr);
        return 1;
    }

    // Release frame buffer
    frameBuffer->Release();

    return 0;
}

//*************************************************************************************************
int D3DInterface::CreateDepthStencil()
{
    // Get the current client size
    RECT winRect;
    GetClientRect(gWinSys->GetWindowHandle(), &winRect);

    // Create the depth-stencil buffer
    D3D11_TEXTURE2D_DESC descDepth = { 0 };
    descDepth.Width = winRect.right - winRect.left;
    descDepth.Height = winRect.bottom - winRect.top;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    HRESULT hr = mDevice->CreateTexture2D(&descDepth, NULL, &mDepthStencil);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating depth-stencil buffer. ", hr);
        return 1;
    }

    // Create the depth-stencil state
    D3D11_DEPTH_STENCIL_DESC dsDesc = { 0 };

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    dsDesc.StencilEnable = true;
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Create depth stencil state
    ID3D11DepthStencilState* dsState;
    hr = mDevice->CreateDepthStencilState(&dsDesc, &dsState);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating depth-stencil state. ", hr);
        return 1;
    }

    // Bind depth stencil state
    mDeviceContext->OMSetDepthStencilState(dsState, 1);

    // Release the depth stencil state
    dsState->Release();

    // Create the depth stencil view description
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;

    // Create the depth stencil view
    hr = mDevice->CreateDepthStencilView(mDepthStencil, &descDSV, &mDepthStencilView);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating depth-stencil view. ", hr);
        return 1;
    }

    return 0;
}

//*************************************************************************************************
int D3DInterface::CreateRasterizerState()
{
    // Create the rasterizer description struct
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.FrontCounterClockwise = FALSE;

    // Create the rasterizer state
    ID3D11RasterizerState* rasterizer;
    HRESULT hr = mDevice->CreateRasterizerState(&rasterizerDesc, &rasterizer);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating rasterizer state. ", hr);
        return 1;
    }

    // Set the rasterizer on the device context
    mDeviceContext->RSSetState(rasterizer);

    // Release the rasterizer object
    rasterizer->Release();

    return 0;
}

//*************************************************************************************************
int D3DInterface::CreateBlendStates()
{
    // Create render target blend descriptor struct for no blend
    D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
    rtbd.BlendEnable = false;
    rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
    // Create blend descriptor struct
    D3D11_BLEND_DESC blendDesc = { 0 };
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;
    // Create and save blend state
    HRESULT hr = mDevice->CreateBlendState(&blendDesc, &mBlendStates[BlendStates::None]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating blend state. ", hr);
        return 1;
    }

    // Set render target blend descriptor struct for transparency
    rtbd = { 0 };
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    // Set blend descriptor struct
    blendDesc = { 0 };
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;
    // Create and save blend state
    hr = mDevice->CreateBlendState(&blendDesc, &mBlendStates[BlendStates::Transparent]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating blend state. ", hr);
        return 1;
    }

    // Set render target blend descriptor struct for additive
    rtbd = { 0 };
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_ONE;
    rtbd.DestBlend = D3D11_BLEND_ONE;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    // Set blend descriptor struct
    blendDesc = { 0 };
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;
    // Create and save blend state
    hr = mDevice->CreateBlendState(&blendDesc, &mBlendStates[BlendStates::Add]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating blend state. ", hr);
        return 1;
    }

    // Set render target blend descriptor struct for multiplicative
    rtbd = { 0 };
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_ZERO;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    // Set blend descriptor struct
    blendDesc = { 0 };
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;
    // Create and save blend state
    hr = mDevice->CreateBlendState(&blendDesc, &mBlendStates[BlendStates::Multiply]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating blend state. ", hr);
        return 1;
    }

    // Set the current blend state
    mDeviceContext->OMSetBlendState(mBlendStates[BlendStates::None], NULL, 0xffffffff);

    return 0;
}

//*************************************************************************************************
int D3DInterface::CreateConstantBuffer()
{
    // Create constant buffer descriptor struct
    D3D11_BUFFER_DESC cbBufferDesc = { 0 };
    cbBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    cbBufferDesc.ByteWidth = sizeof(cbPerObject);
    cbBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    // Create and save constant buffer
    HRESULT hr = mDevice->CreateBuffer(&cbBufferDesc, NULL, &mPerObjectBuffer);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating constant buffer. ", hr);
        return 1;
    }

    return 0;
}

//*************************************************************************************************
void D3DInterface::SetViewport()
{
    if (!mDeviceContext)
    {
        return;
    }

    // Get the current client size
    RECT winRect;
    GetClientRect(gWinSys->GetWindowHandle(), &winRect);
    // Create the viewport data
    D3D11_VIEWPORT viewport = {
        0.0f,
        0.0f,
        (float)(winRect.right - winRect.left),
        (float)(winRect.bottom - winRect.top),
        0.0f,
        1.0f
    };
    // Set the viewport on the device context
    mDeviceContext->RSSetViewports(1, &viewport);
}

//*************************************************************************************************
int D3DInterface::CreateSamplers()
{
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Samplers with point filter *************************************
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;

    // Wrap address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    HRESULT hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Point][TextureAddressModes::Wrap]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Clamp address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Point][TextureAddressModes::Clamp]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Mirror address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Point][TextureAddressModes::Mirror]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Mirror once address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Point][TextureAddressModes::Mirror_Once]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Samplers with linear filter **************************************
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    // Wrap address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Linear][TextureAddressModes::Wrap]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Clamp address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Linear][TextureAddressModes::Clamp]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Mirror address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Linear][TextureAddressModes::Mirror]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    // Mirror once address mode
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    hr = mDevice->CreateSamplerState(&sampDesc, 
        &mSamplerStates[SampleModes::Linear][TextureAddressModes::Mirror_Once]);
    if (FAILED(hr))
    {
        gError->SetError("Problem creating sampler. ", hr);
        return 1;
    }

    return 0;
}

} // namespace DGL
