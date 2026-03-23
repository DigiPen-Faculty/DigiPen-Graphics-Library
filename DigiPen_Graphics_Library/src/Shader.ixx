//-------------------------------------------------------------------------------------------------
// file:    Shader.ixx
// author:  Kenny Mecham, Andy Ellinger
// brief:   Header for additional shader functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
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
    ShaderManager() = default;
    ~ShaderManager() = default;

    void Initialize(ID3D11Device* device, const DGL_SysInitInfo& sysInitInfo);
    void ShutDown();

    DGL_PixelShader* LoadPixelShader(std::string_view filename);
    DGL_VertexShader* LoadVertexShader(std::string_view filename);

    void SetCustomPixelShader(const DGL_PixelShader* shader);
    void SetCustomVertexShader(const DGL_VertexShader* shader);

    ID3D11PixelShader* GetCurrentPixelShader() const;
    ID3D11VertexShader* GetCurrentVertexShader() const;

    void Release(const DGL_PixelShader* shader);
    void Release(const DGL_VertexShader* shader);

    // The current pixel shader mode
    DGL_PixelShaderMode mCurrentPixelShaderMode{ DGL_PSM_DEFAULT };
    // The current vertex shader mode
    DGL_VertexShaderMode mCurrentVertexShaderMode{ DGL_VSM_DEFAULT };

private:
    // The D3D device object
    ID3D11Device* mDevice{ nullptr };
    // The default D3D vertex shader object
    ID3D11VertexShader* mVertexShader{ nullptr };
    // The default D3D pixel shader object 
    ID3D11PixelShader* mPixelShader{ nullptr };
    // The current custom D3D pixel shader that a user gives us
    ID3D11PixelShader* mPixelCustomShader{ nullptr };
    // The current custom D3D vertex shader that a user gives us
    ID3D11VertexShader* mVertexCustomShader{ nullptr };

    ID3D11PixelShader* CreatePixelShader(std::string_view filename);
    ID3D11VertexShader* CreateVertexShader(std::string_view filename, bool createLayout = false);
};
}
