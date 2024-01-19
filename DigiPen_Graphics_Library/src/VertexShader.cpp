//-------------------------------------------------------------------------------------------------
// file:    VertexShader.cpp
// author:  Kenny Mecham
// brief:   Vertex Shader functionality
//
// Copyright © 2023 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include <d3d11.h>
#include <unordered_set>
#include <string>
#include <sstream>>
#include <d3dcompiler.h>
#include <format>
#include <cassert>

module VertexShader;
import Errors;

namespace DGL
{
//---------------------------------------------------------------------------------- DGL_VertexShader

//*************************************************************************************************
DGL_VertexShader::DGL_VertexShader(std::string_view filename, ID3D11Device* device) :
    shader(nullptr),
    filename(filename)
{
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
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);

    if (FAILED(hr))
    {
        std::stringstream errorMsg;
        errorMsg << "Failed to create vertex shader from file \"" << filename << "\". ";
        gError->SetError(errorMsg.str(), hr);
    }
}

//*************************************************************************************************
DGL_VertexShader::~DGL_VertexShader()
{
    if (shader)
    {
        shader->Release();
    }
}

//*************************************************************************************************
bool DGL_VertexShader::IsValid() const noexcept
{
    return shader;
}

}