//-------------------------------------------------------------------------------------------------
// file:    Shader.cpp
// author:  Kenny Mecham
// brief:   Shader functionality
//
// Copyright © 2023 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

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

namespace DGL
{

//----------------------------------------------------------------------------------- ShaderManager

//*************************************************************************************************
const DGL_PixelShader* ShaderManager::LoadPixelShader(std::string_view filename, ID3D11Device* device)
{
    assert(!filename.empty());
    assert(device);

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
    LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "ps_5_0" : "ps_4_0";

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

    DGL_PixelShader shader(filename);

    hr = device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &shader.shader);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to create pixel shader from file \"" << filename << "\". ";
        DGL::gError->SetError(errorMsg.str(), hr);
        return nullptr;
    }

    auto shaderIter = mPixelShaders.insert(shader);
    return &(*shaderIter.first);
}

//*************************************************************************************************
const DGL_VertexShader* ShaderManager::LoadVertexShader(std::string_view filename, ID3D11Device* device)
{
    assert(!filename.empty());
    assert(device);

    auto shader = std::make_unique<DGL_VertexShader>(filename);

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
    LPCSTR profile = (device->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0) ? "vs_5_0" : "vs_4_0";

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

    hr = device->CreateVertexShader(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader->shader);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to create vertex shader from file \"" << filename << "\". ";
        gError->SetError(errorMsg.str(), hr);
    }

    auto shaderIter = mVertexShaders.insert_or_assign(
        filename.data(),
        std::move(shader));

    return shaderIter.first->second.get();
}

//*************************************************************************************************
std::size_t ShaderManager::PixelShaderCount() const noexcept
{
    return mPixelShaders.size();
}

//*************************************************************************************************
std::size_t ShaderManager::VertexShaderCount() const noexcept
{
    return mVertexShaders.size();
}

//*************************************************************************************************
void ShaderManager::Release(const DGL_PixelShader* shader)
{
    if (shader)
    {
        mPixelShaders.erase(*shader);
    }
}

//*************************************************************************************************
void ShaderManager::Release(const DGL_VertexShader* shader)
{
    if (shader)
    {
        mVertexShaders.erase(shader->filename);
    }
}
}   // namespace DGL
