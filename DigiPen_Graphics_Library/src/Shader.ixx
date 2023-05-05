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
export import VertexShader;

export struct DGL_PixelShader
{
    DGL_PixelShader(std::string_view name) :
        shader(nullptr),
        name(name)
    {}

    bool operator==(const DGL_PixelShader&) const noexcept = default;

    ID3D11PixelShader* shader;
    std::string name;
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