//-------------------------------------------------------------------------------------------------
// file:    Shader.h
// author:  Kenny Mecham, Andy Ellinger
// brief:   Header for additional shader functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include "InternalTypes.h"
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace DGL
{

// Compiles HLSL to SPIR-V with DXC (defined in Shader.cpp, since it uses the DXC headers)
class ShaderCompiler;

//----------------------------------------------------------------------------------- ShaderManager

// Loads HLSL shader files, compiling them to SPIR-V with the DXC compiler.
// Files ending in ".spv" are loaded as already-compiled SPIR-V.
class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    // Loads the default shaders. Returns false if there was an error.
    bool Initialize(const DGL_SysInitInfo& sysInitInfo);

    // Releases the default shaders and the compiler
    void ShutDown();

    // Loads a shader from the file. Returns null if there was an error (and sets the error message).
    std::unique_ptr<DGL_PixelShader> LoadPixelShader(std::string_view filename);
    std::unique_ptr<DGL_VertexShader> LoadVertexShader(std::string_view filename);

    // Sets the shaders to use with the custom shader modes
    void SetCustomPixelShader(const DGL_PixelShader* shader);
    void SetCustomVertexShader(const DGL_VertexShader* shader);

    // Stops using the shader as the custom shader (if it is), since it is about to be deleted
    void StopUsing(const DGL_PixelShader* shader);
    void StopUsing(const DGL_VertexShader* shader);

    // Returns the shaders to use with the current shader modes (null if there is no custom shader set)
    const DGL_PixelShader* GetCurrentPixelShader() const;
    const DGL_VertexShader* GetCurrentVertexShader() const;

    // The current shader modes (checked when they are set)
    DGL_PixelShaderMode mCurrentPixelShaderMode{ DGL_PSM_DEFAULT };
    DGL_VertexShaderMode mCurrentVertexShaderMode{ DGL_VSM_DEFAULT };

private:
    // Compiles an HLSL file, creating the compiler the first time
    bool CompileHlsl(std::string_view filename, const wchar_t* profile, bool reflect, std::vector<uint32_t>& code);

    // The default shaders
    std::unique_ptr<DGL_PixelShader> mPixelShader;
    std::unique_ptr<DGL_VertexShader> mVertexShader;
    // The current custom shaders that a user gives us
    const DGL_PixelShader* mPixelCustomShader{ nullptr };
    const DGL_VertexShader* mVertexCustomShader{ nullptr };
    // Created the first time an HLSL file is compiled, so programs that only use .spv files
    // don't need dxcompiler.dll
    std::unique_ptr<ShaderCompiler> mCompiler;
};

}
