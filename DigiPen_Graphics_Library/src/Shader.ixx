//-------------------------------------------------------------------------------------------------
// file:    Shader.h
// author:  Kenny Mecham
// brief:   Header for additional shader functionality
//
// Copyright � 2023 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

module;

#include <d3d11.h>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <memory>

export module Shader;

export struct DGL_PixelShader
{
    DGL_PixelShader(std::string_view name) :
        name(name)
    {}

    ~DGL_PixelShader() { if (shader) shader->Release(); }

    bool operator==(const DGL_PixelShader&) const noexcept = default;

    ID3D11PixelShader* shader{ nullptr };
    std::string name;
};

export struct DGL_VertexShader
{
    DGL_VertexShader(std::string_view name) :
        filename(name)
    {}
    DGL_VertexShader(const DGL_VertexShader& other) = delete;

    ~DGL_VertexShader() { if (shader) shader->Release(); }

    ID3D11VertexShader* shader{ nullptr };
    std::string filename;
};

template <>
struct std::hash<DGL_PixelShader>
{
    std::size_t operator()(const DGL_PixelShader& shader) const
    {
        return std::hash<std::string>{}(shader.name);
    }
};

namespace DGL
{

//------------------------------------------------------------------------------- ShaderManager

export class ShaderManager
{
public:
    const DGL_PixelShader* LoadPixelShader(std::string_view filename, ID3D11Device* device);
    const DGL_VertexShader* LoadVertexShader(std::string_view filename, ID3D11Device* device);

    std::size_t PixelShaderCount() const noexcept;
    std::size_t VertexShaderCount() const noexcept;

    void Release(const DGL_PixelShader* shader);
    void Release(const DGL_VertexShader* shader);

private:
    std::unordered_set<DGL_PixelShader> mPixelShaders;
    std::unordered_map<std::string, std::unique_ptr<DGL_VertexShader>> mVertexShaders;
};
}