//-------------------------------------------------------------------------------------------------
// file:    Shader.cpp
// author:  Kenny Mecham, Andy Ellinger
// brief:   Shader functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "InternalTypes.h"
#include <d3d11.h>
#include <unordered_set>
#include <string>
#include <sstream>
#include <d3dcompiler.h>
#include <memory>
#include <format>
#include <cassert>

module Shader;
import Errors;
import GraphicsSystem;

namespace DGL
{

//----------------------------------------------------------------------------------- ShaderManager

//*************************************************************************************************
void ShaderManager::Initialize(ID3D11Device* device, const DGL_SysInitInfo& sysInitInfo)
{
    assert(device && "ShaderManager::Initialize called when already initialized.");

    mDevice = device;

    // Create default vertex shader
    string shaderPath{ sysInitInfo.defaultShaderPath };
    shaderPath += sysInitInfo.defaultVertexShader;
    mVertexShader = CreateVertexShader(shaderPath, true);

    // Create default pixel shader 
    shaderPath = sysInitInfo.defaultShaderPath;
    shaderPath += sysInitInfo.defaultPixelShader;
    mPixelShader = CreatePixelShader(shaderPath);
}

//*************************************************************************************************
void ShaderManager::ShutDown()
{
    if (mVertexShader)
    {
        mVertexShader->Release();
        mVertexShader = nullptr;
    }

    if (mPixelShader)
    {
        mPixelShader->Release();
        mPixelShader = nullptr;
    }
}

//*************************************************************************************************
DGL_PixelShader* ShaderManager::LoadPixelShader(std::string_view filename)
{
    assert(!filename.empty());

    DGL_PixelShader* shader = new DGL_PixelShader(filename);

    shader->shader = CreatePixelShader(filename);

    return shader;
}

//*************************************************************************************************
DGL_VertexShader* ShaderManager::LoadVertexShader(std::string_view filename)
{
    assert(!filename.empty());

    DGL_VertexShader* shader = new DGL_VertexShader(filename);

    shader->shader = CreateVertexShader(filename);

    return shader;
}

//*************************************************************************************************
void ShaderManager::SetCustomPixelShader(const DGL_PixelShader* shader)
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
void ShaderManager::SetCustomVertexShader(const DGL_VertexShader * shader)
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
ID3D11PixelShader* ShaderManager::GetCurrentPixelShader() const
{
    switch (mCurrentPixelShaderMode)
    {
        case DGL_PSM_DEFAULT: return mPixelShader;
        case DGL_PSM_CUSTOM: return mPixelCustomShader;
        default:
            gError->SetError("Invalid pixel shader mode in ShaderManager::GetCurrentPixelShader.");
            return nullptr;
    }
}

//*************************************************************************************************
ID3D11VertexShader* ShaderManager::GetCurrentVertexShader() const
{
    switch (mCurrentVertexShaderMode)
    {
        case DGL_VSM_DEFAULT: return mVertexShader;
        case DGL_VSM_CUSTOM: return mVertexCustomShader;
        default:
            gError->SetError("Invalid vertex shader mode in ShaderManager::GetCurrentVertexShader.");
            return nullptr;
    }
}

//*************************************************************************************************
void ShaderManager::Release(const DGL_PixelShader* shader)
{
    if (shader)
    {
        shader->shader->Release();
    }
}

//*************************************************************************************************
void ShaderManager::Release(const DGL_VertexShader* shader)
{
    if (shader)
    {
        shader->shader->Release();
    }
}

//*************************************************************************************************
ID3D11PixelShader* ShaderManager::CreatePixelShader(std::string_view filename)
{
    std::wstring wideString(filename.size(), '\0');

    for (std::size_t i = 0; i < filename.size(); ++i)
    {
        wideString[i] = filename[i];
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif
    // Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
    LPCSTR profile = (mDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "ps_5_0" : "ps_4_0";

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(
        wideString.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        profile,
        flags, 0,
        &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to compile pixel shader file \"" << filename << "\". ";

        if (errorBlob)
        {
            errorMsg << (char*)errorBlob->GetBufferPointer();
            gError->SetError(errorMsg.str());
            errorBlob->Release();
        }
        else
            DGL::gError->SetError(errorMsg.str(), hr);

        if (shaderBlob)
            shaderBlob->Release();

        return nullptr;
    }

    ID3D11PixelShader* shader = nullptr;

    hr = mDevice->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &shader);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to create pixel shader from file \"" << filename << "\". ";
        DGL::gError->SetError(errorMsg.str(), hr);
        return nullptr;
    }

    return shader;
}

//*************************************************************************************************
ID3D11VertexShader* ShaderManager::CreateVertexShader(std::string_view filename, bool createLayout)
{
    std::wstring wideString(filename.size(), '\0');

    for (std::size_t i = 0; i < filename.size(); ++i)
    {
        wideString[i] = filename[i];
    }

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3DCOMPILE_DEBUG;
#endif
    // Prefer higher CS shader profile when possible as CS 5.0 provides better performance on 11-class hardware.
    LPCSTR profile = (mDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "vs_5_0" : "vs_4_0";

    ID3DBlob* shaderBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;
    HRESULT hr = D3DCompileFromFile(
        wideString.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        profile,
        flags, 0,
        &shaderBlob, &errorBlob);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to compile vertex shader file \"" << filename << "\". ";

        if (errorBlob)
        {
            errorMsg << (char*)errorBlob->GetBufferPointer();
            gError->SetError(errorMsg.str());
            errorBlob->Release();
        }
        else
            gError->SetError(errorMsg.str(), hr);

        if (shaderBlob)
            shaderBlob->Release();
    }

    ID3D11VertexShader* shader = nullptr;

    hr = mDevice->CreateVertexShader(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to create vertex shader from file \"" << filename << "\". ";
        gError->SetError(errorMsg.str(), hr);
        return nullptr;
    }

    if (createLayout)
    {
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
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            &gGraphics->D3D.mInputLayout
        );
        if (FAILED(hr))
        {
            gError->SetError("Problem creating shader input layout. ", hr);
        }
    }

    return shader;
}

}   // namespace DGL
