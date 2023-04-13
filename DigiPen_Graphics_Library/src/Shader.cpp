//-------------------------------------------------------------------------------------------------
// file:    Shader.cpp
// author:  Kenny Mecham
// brief:   Shader functionality
//
// Copyright © 2023 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include <d3dcompiler.h>
#include "Shader.h"

namespace DGL
{
const DGL_PixelShader* ShaderManager::LoadPixelShader(std::string_view filename, ID3D11Device* device)
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
        printf("Failed to compile shader file [%s] with error [%08x]\n", filename.data(), hr);
        if (errorBlob)
        {
            std::printf("%.*s\n", (int)errorBlob->GetBufferSize(), (char*)errorBlob->GetBufferPointer());
            errorBlob->Release();
        }

        if (shaderBlob)
            shaderBlob->Release();

        return nullptr;
    }

    DGL_PixelShader shader(filename);

    hr = device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &shader.shader);

    if (FAILED(hr))
    {
        printf("Failed to create Pixel Shader from file [%s]. Error [%08x]\n", filename.data(), hr);
        return nullptr;
    }

    auto shaderIter = mPixelShaders.insert(shader);
    return &(*shaderIter.first);
}

void ShaderManager::Release(const DGL_PixelShader* shader)
{
    if (!shader)
    {
        return;
    }

    if (shader->shader)
    {
        shader->shader->Release();
    }

    mPixelShaders.erase(*shader);
}
}
