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
#include <d3dcompiler.h>
#include <format>
#include <cassert>

module VertexShader;
import Errors;

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
        DGL::gError->SetError(std::format("Failed to compile shader file [%s] with error [%08x]\n", filename.data(), hr));
        if (errorBlob)
        {
            DGL::gError->SetError(std::format("%.*s\n", (int)errorBlob->GetBufferSize(), (char*)errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();
    }

    hr = device->CreateVertexShader(
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);

    if (FAILED(hr))
    {
        DGL::gError->SetError(std::format("Failed to create Vertex Shader [%s] with error [%08x]\n", filename.data(), hr));
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
