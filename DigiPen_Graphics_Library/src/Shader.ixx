//-------------------------------------------------------------------------------------------------
// file:    Shader.h
// author:  Kenny Mecham
// brief:   Header for additional shader functionality
//
// Copyright � 2023 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include "DGL.h"
#include <d3d11.h>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <memory>

export module Shader;

namespace DGL
{

//------------------------------------------------------------------------------- ShaderManager

export class ShaderManager
{
public:

    DGL_PixelShader* LoadPixelShader(std::string_view filename, ID3D11Device* device);
    DGL_VertexShader* LoadVertexShader(std::string_view filename, ID3D11Device* device);

    std::size_t PixelShaderCount() const noexcept;
    std::size_t VertexShaderCount() const noexcept;

    void Release(const DGL_PixelShader* shader);
    void Release(const DGL_VertexShader* shader);

private:
    std::unordered_map<std::string, std::unique_ptr<DGL_PixelShader>> mPixelShaders;
    std::unordered_map<std::string, std::unique_ptr<DGL_VertexShader>> mVertexShaders;
};
}