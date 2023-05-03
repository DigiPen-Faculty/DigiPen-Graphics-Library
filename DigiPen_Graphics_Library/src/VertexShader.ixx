//-------------------------------------------------------------------------------------------------
// file:    VertexShader.ixx
// author:  Kenny Mecham
// brief:   Header for custom vertex shader functionality
//
// Copyright © 2023 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include <d3d11.h>
#include <functional>
#include <string>

export module VertexShader;

export struct DGL_VertexShader
{
    DGL_VertexShader(std::string_view name, ID3D11Device* device);
    DGL_VertexShader(const DGL_VertexShader& other) = delete;

    ~DGL_VertexShader();

    bool IsValid() const noexcept;

    ID3D11VertexShader* shader;
    std::string filename;
};
