//-------------------------------------------------------------------------------------------------
// file:    Shader.cpp
// author:  Kenny Mecham, Andy Ellinger
// brief:   Shader functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "Shader.h"

#if defined(_WIN32)
#include <Windows.h>
#include <objbase.h>
#else
#include <dlfcn.h>
#endif
// On Linux and macOS, dxcapi.h includes DXC's WinAdapter.h, which defines the COM types it uses
#include <dxc/dxcapi.h>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "Error.h"
#include "GraphicsSystem.h"

namespace DGL
{

namespace
{

    //------------------------------------------------------------------------------------------- SPIR-V

    // The SPIR-V values used below (from the SPIR-V specification)
    namespace spv
    {
        constexpr uint32_t MagicNumber = 0x07230203;
        constexpr uint32_t OpVariable = 59;
        constexpr uint32_t OpDecorate = 71;
        constexpr uint32_t OpDecorateString = 5632;
        constexpr uint32_t DecorationLocation = 30;
        constexpr uint32_t DecorationUserSemantic = 5635;
        constexpr uint32_t StorageClassInput = 1;
    }

    // An instruction in SPIR-V code
    struct SpirvInstruction
    {
        size_t start;
        uint32_t opcode;
        uint32_t wordCount;
    };

    //*************************************************************************************************
    // Splits SPIR-V code into instructions. Returns false if the code is not valid.
    bool GetInstructions(const std::vector<uint32_t>& code, std::vector<SpirvInstruction>& instructions)
    {
        if (code.size() < 5 || code[0] != spv::MagicNumber)
            return false;

        for (size_t i = 5; i < code.size();)
        {
            uint32_t wordCount = code[i] >> 16;
            if (wordCount == 0 || i + wordCount > code.size())
                return false;
            instructions.push_back({ i, code[i] & 0xFFFF, wordCount });
            i += wordCount;
        }
        return true;
    }

    //*************************************************************************************************
    // Reads a string stored in SPIR-V words
    std::string ReadSpirvString(const uint32_t* words, size_t wordCount)
    {
        const char* chars = (const char*)words;
        return std::string(chars, strnlen(chars, wordCount * sizeof(uint32_t)));
    }

    //*************************************************************************************************
    // Finds the input locations the vertex shader uses for each part of the vertex data, using the
    // semantics (such as POSITION) from the HLSL code. The code must be compiled with -fspv-reflect.
    bool FindVertexInputs(const std::vector<uint32_t>& code, VertexInputLocations& inputs, std::string& error)
    {
        std::vector<SpirvInstruction> instructions;
        if (!GetInstructions(code, instructions))
        {
            error = "The compiled shader code is not valid.";
            return false;
        }

        std::vector<uint32_t> inputVariables;
        std::vector<std::pair<uint32_t, uint32_t>> locations;
        std::vector<std::pair<uint32_t, std::string>> semantics;
        for (const SpirvInstruction& instruction : instructions)
        {
            const uint32_t* words = &code[instruction.start];
            if (instruction.opcode == spv::OpDecorate && instruction.wordCount >= 4 && words[2] == spv::DecorationLocation)
                locations.push_back({ words[1], words[3] });
            else if (instruction.opcode == spv::OpDecorateString && instruction.wordCount >= 4 &&
                words[2] == spv::DecorationUserSemantic)
                semantics.push_back({ words[1], ReadSpirvString(&words[3], instruction.wordCount - 3) });
            else if (instruction.opcode == spv::OpVariable && instruction.wordCount >= 4 && words[3] == spv::StorageClassInput)
                inputVariables.push_back(words[2]);
        }

        inputs = { -1, -1, -1 };
        for (uint32_t variable : inputVariables)
        {
            int location = -1;
            for (auto& [id, value] : locations)
            {
                if (id == variable)
                    location = (int)value;
            }
            // Built-in inputs (like SV_VertexID) don't have a location
            if (location < 0)
                continue;

            std::string semantic;
            for (auto& [id, value] : semantics)
            {
                if (id == variable)
                    semantic = value;
            }

            // Compare without case or index numbers (COLOR0 is the same as COLOR)
            std::string name = semantic;
            for (char& c : name)
                c = (char)toupper((unsigned char)c);
            while (!name.empty() && isdigit((unsigned char)name.back()))
                name.pop_back();

            if (name == "POSITION")
                inputs.position = location;
            else if (name == "COLOR")
                inputs.color = location;
            else if (name == "TEX" || name == "TEXCOORD")
                inputs.texCoord = location;
            else
            {
                error = "The vertex shader has an input with semantic \"" + semantic +
                    "\". The vertex shader inputs must use the semantics POSITION, COLOR, and TEX.";
                return false;
            }
        }

        return true;
    }

    //*************************************************************************************************
    // Loads a file of compiled SPIR-V code
    bool LoadSpirvFile(std::string_view filename, std::vector<uint32_t>& code)
    {
        std::ifstream file(std::string(filename), std::ios::binary | std::ios::ate);
        if (!file)
        {
            gError->SetError("Failed to open shader file \"" + std::string(filename) + "\".");
            return false;
        }

        size_t size = (size_t)file.tellg();
        code.resize(size / sizeof(uint32_t));
        file.seekg(0);
        file.read((char*)code.data(), code.size() * sizeof(uint32_t));

        if (size % sizeof(uint32_t) != 0 || code.empty() || code[0] != spv::MagicNumber)
        {
            gError->SetError("The shader file \"" + std::string(filename) + "\" does not contain SPIR-V code.");
            return false;
        }
        return true;
    }

    //*************************************************************************************************
    bool IsSpirvFile(std::string_view filename)
    {
        if (filename.size() <= 4)
            return false;
        std::string extension(filename.substr(filename.size() - 4));
        for (char& c : extension)
            c = (char)tolower((unsigned char)c);
        return extension == ".spv";
    }

    //*************************************************************************************************
    template <typename T>
    void SafeRelease(T*& object)
    {
        if (object)
            object->Release();
        object = nullptr;
    }

    //*************************************************************************************************
    // Converts UTF-8 text to a wide string, which is what the DXC functions use
    std::wstring Widen(std::string_view text)
    {
#if defined(_WIN32)
        int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), nullptr, 0);
        std::wstring wide(length, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), (int)text.size(), wide.data(), length);
        return wide;
#else
        // wchar_t is 32 bits on Linux and macOS, so each character is one code point
        std::wstring wide;
        for (size_t i = 0; i < text.size();)
        {
            unsigned char c = (unsigned char)text[i];
            size_t extra = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : 0;
            uint32_t codePoint = extra == 3 ? (c & 0x07) : extra == 2 ? (c & 0x0F) : extra == 1 ? (c & 0x1F) : c;
            for (size_t j = 1; j <= extra && i + j < text.size(); ++j)
                codePoint = (codePoint << 6) | ((unsigned char)text[i + j] & 0x3F);
            wide.push_back((wchar_t)codePoint);
            i += extra + 1;
        }
        return wide;
#endif
    }

    //*************************************************************************************************
    // The name of the DXC library on each platform
#if defined(_WIN32)
    constexpr const char* DxcLibraryName = "dxcompiler.dll";
#elif defined(__APPLE__)
    constexpr const char* DxcLibraryName = "libdxcompiler.dylib";
#else
    constexpr const char* DxcLibraryName = "libdxcompiler.so";
#endif

    //*************************************************************************************************
    // Loads the DXC library, returning null if it couldn't be found
    void* LoadDxcLibrary()
    {
#if defined(_WIN32)
        return LoadLibraryA(DxcLibraryName);
#else
        // First look in the same folder as the DGL library (like Windows does with the program's folder)
        Dl_info info{};
        if (dladdr((void*)&LoadDxcLibrary, &info) && info.dli_fname)
        {
            std::string path(info.dli_fname);
            size_t slash = path.find_last_of('/');
            path = (slash == std::string::npos ? std::string() : path.substr(0, slash + 1)) + DxcLibraryName;
            if (void* library = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL))
                return library;
        }
        // Then use the normal library search paths (such as the Vulkan SDK's lib folder)
        return dlopen(DxcLibraryName, RTLD_NOW | RTLD_LOCAL);
#endif
    }

    //*************************************************************************************************
    void* GetLibraryFunction(void* library, const char* name)
    {
#if defined(_WIN32)
        return (void*)GetProcAddress((HMODULE)library, name);
#else
        return dlsym(library, name);
#endif
    }

    //*************************************************************************************************
    void FreeDxcLibrary(void* library)
    {
#if defined(_WIN32)
        FreeLibrary((HMODULE)library);
#else
        dlclose(library);
#endif
    }

} // namespace

//---------------------------------------------------------------------------------- ShaderCompiler

// The DXC shader compiler
class ShaderCompiler
{
public:
    ShaderCompiler() = default;
    ShaderCompiler(const ShaderCompiler&) = delete;
    ShaderCompiler& operator=(const ShaderCompiler&) = delete;
    ~ShaderCompiler();

    // Loads dxcompiler.dll and creates the compiler. Returns false if there was an error.
    bool Initialize();

    // Compiles an HLSL file to SPIR-V. The entry function must be called "main".
    bool Compile(std::string_view filename, const wchar_t* profile, bool reflect, std::vector<uint32_t>& code);

private:
    void* mLibrary{ nullptr };
    IDxcUtils* mUtils{ nullptr };
    IDxcCompiler3* mCompiler{ nullptr };
    IDxcIncludeHandler* mIncludeHandler{ nullptr };
};

//*************************************************************************************************
ShaderCompiler::~ShaderCompiler()
{
    SafeRelease(mIncludeHandler);
    SafeRelease(mCompiler);
    SafeRelease(mUtils);
    if (mLibrary)
        FreeDxcLibrary(mLibrary);
}

//*************************************************************************************************
bool ShaderCompiler::Initialize()
{
    mLibrary = LoadDxcLibrary();
    if (!mLibrary)
    {
        gError->SetError(std::string("Couldn't load ") + DxcLibraryName + ", which is needed to compile "
            "HLSL shaders. Make sure it is in the same folder as the DGL library.");
        return false;
    }

    auto createInstance = (DxcCreateInstanceProc)GetLibraryFunction(mLibrary, "DxcCreateInstance");
    if (!createInstance ||
        FAILED(createInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), (void**)&mUtils)) ||
        FAILED(createInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler3), (void**)&mCompiler)) ||
        FAILED(mUtils->CreateDefaultIncludeHandler(&mIncludeHandler)))
    {
        gError->SetError("Problem creating the DXC shader compiler.");
        return false;
    }

    return true;
}

//*************************************************************************************************
bool ShaderCompiler::Compile(std::string_view filename, const wchar_t* profile, bool reflect,
    std::vector<uint32_t>& code)
{
    std::string name(filename);
    std::wstring wideName = Widen(filename);

    IDxcBlobEncoding* source = nullptr;
    HRESULT hr = mUtils->LoadFile(wideName.c_str(), nullptr, &source);
    if (FAILED(hr))
    {
        gError->SetError("Failed to open shader file \"" + name + "\". ", hr);
        return false;
    }
    DxcBuffer buffer{ source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_ACP };

    // #include files are found relative to the shader's folder
    std::wstring folder = L".";
    size_t slash = wideName.find_last_of(L"/\\");
    if (slash != std::wstring::npos)
        folder = wideName.substr(0, slash);

    // The bindings match DGL's descriptor layout: the constant buffer (b0) is binding 0, the texture (t0)
    // is binding 1, and the sampler (s0) is binding 2. The constant buffer uses the same memory layout
    // as D3D, and HLSL 2018 rules are used for compatibility with shaders written for D3D11.
    std::vector<const wchar_t*> args = {
        wideName.c_str(),
        L"-E", L"main",
        L"-T", profile,
        L"-spirv",
        L"-fspv-target-env=vulkan1.3",
        L"-HV", L"2018",
        L"-fvk-use-dx-layout",
        L"-fvk-auto-shift-bindings",
        L"-fvk-b-shift", L"0", L"0",
        L"-fvk-t-shift", L"1", L"0",
        L"-fvk-s-shift", L"2", L"0",
        L"-I", folder.c_str()
    };
    // Adds the HLSL semantics to the code, so the vertex inputs can be matched to the vertex data
    if (reflect)
        args.push_back(L"-fspv-reflect");

    IDxcResult* result = nullptr;
    hr = mCompiler->Compile(&buffer, args.data(), (UINT32)args.size(), mIncludeHandler,
        __uuidof(IDxcResult), (void**)&result);
    SafeRelease(source);

    HRESULT status = hr;
    if (SUCCEEDED(hr))
        result->GetStatus(&status);

    std::string messages;
    IDxcBlobUtf8* errors = nullptr;
    if (result && SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, __uuidof(IDxcBlobUtf8), (void**)&errors, nullptr)) &&
        errors && errors->GetStringLength() > 0)
        messages = errors->GetStringPointer();
    SafeRelease(errors);

    if (FAILED(status))
    {
        if (messages.empty())
            gError->SetError("Failed to compile shader file \"" + name + "\". ", status);
        else
            gError->SetError("Failed to compile shader file \"" + name + "\". " + messages);
        SafeRelease(result);
        return false;
    }

    if (!messages.empty() && !reflect)
        fprintf(stderr, "Warnings from compiling shader file \"%s\":\n%s\n", name.c_str(), messages.c_str());

    IDxcBlob* object = nullptr;
    result->GetOutput(DXC_OUT_OBJECT, __uuidof(IDxcBlob), (void**)&object, nullptr);
    if (object)
    {
        code.resize(object->GetBufferSize() / sizeof(uint32_t));
        memcpy(code.data(), object->GetBufferPointer(), code.size() * sizeof(uint32_t));
    }
    SafeRelease(object);
    SafeRelease(result);

    return !code.empty();
}

} // namespace DGL

using namespace DGL;

//*************************************************************************************************
ShaderBase::~ShaderBase()
{
// The renderer destroys the module (and any pipelines using it) once the GPU is no longer using them
if (module)
    gGraphics->Renderer.DestroyShaderModule(module);
}

namespace DGL
{

//----------------------------------------------------------------------------------- ShaderManager

// These are here because ShaderCompiler is only defined in this file
ShaderManager::ShaderManager() = default;
ShaderManager::~ShaderManager() = default;

//*************************************************************************************************
bool ShaderManager::Initialize(const DGL_SysInitInfo& sysInitInfo)
{
    std::string shaderPath{ sysInitInfo.defaultShaderPath };
    mVertexShader = LoadVertexShader(shaderPath + sysInitInfo.defaultVertexShader);
    mPixelShader = LoadPixelShader(shaderPath + sysInitInfo.defaultPixelShader);

    return mVertexShader && mPixelShader;
}

//*************************************************************************************************
void ShaderManager::ShutDown()
{
    mVertexShader.reset();
    mPixelShader.reset();
    mPixelCustomShader = nullptr;
    mVertexCustomShader = nullptr;
    mCompiler.reset();
}

//*************************************************************************************************
bool ShaderManager::CompileHlsl(std::string_view filename, const wchar_t* profile, bool reflect,
    std::vector<uint32_t>& code)
{
    if (!mCompiler)
    {
        auto compiler = std::make_unique<ShaderCompiler>();
        if (!compiler->Initialize())
            return false;
        mCompiler = std::move(compiler);
    }

    return mCompiler->Compile(filename, profile, reflect, code);
}

//*************************************************************************************************
std::unique_ptr<DGL_PixelShader> ShaderManager::LoadPixelShader(std::string_view filename)
{
    std::vector<uint32_t> code;
    bool loaded = IsSpirvFile(filename) ? LoadSpirvFile(filename, code) : CompileHlsl(filename, L"ps_6_0", false, code);
    if (!loaded)
        return nullptr;

    auto shader = std::make_unique<DGL_PixelShader>(filename);
    shader->module = gGraphics->Renderer.CreateShaderModule(code);
    if (!shader->module)
        return nullptr;

    return shader;
}

//*************************************************************************************************
std::unique_ptr<DGL_VertexShader> ShaderManager::LoadVertexShader(std::string_view filename)
{
    std::vector<uint32_t> code;
    // Compiled SPIR-V files must use input locations 0, 1, and 2 for the position, color, and texture
    // coordinates (which is what DXC does when they are declared in that order)
    VertexInputLocations inputs;

    if (IsSpirvFile(filename))
    {
        if (!LoadSpirvFile(filename, code))
            return nullptr;
    }
    else
    {
        // Compile once with the HLSL semantics to find the input locations, then again without them
        // (the semantics need extensions that aren't needed to run the shader)
        std::vector<uint32_t> reflectionCode;
        if (!CompileHlsl(filename, L"vs_6_0", true, reflectionCode))
            return nullptr;

        std::string error;
        if (!FindVertexInputs(reflectionCode, inputs, error))
        {
            gError->SetError("Problem with vertex shader file \"" + std::string(filename) + "\". " + error);
            return nullptr;
        }

        if (!CompileHlsl(filename, L"vs_6_0", false, code))
            return nullptr;
    }

    auto shader = std::make_unique<DGL_VertexShader>(filename);
    shader->inputs = inputs;
    shader->module = gGraphics->Renderer.CreateShaderModule(code);
    if (!shader->module)
        return nullptr;

    return shader;
}

//*************************************************************************************************
void ShaderManager::SetCustomPixelShader(const DGL_PixelShader* shader)
{
    mPixelCustomShader = shader;
}

//*************************************************************************************************
void ShaderManager::SetCustomVertexShader(const DGL_VertexShader* shader)
{
    mVertexCustomShader = shader;
}

//*************************************************************************************************
void ShaderManager::StopUsing(const DGL_PixelShader* shader)
{
    if (mPixelCustomShader == shader)
        mPixelCustomShader = nullptr;
}

//*************************************************************************************************
void ShaderManager::StopUsing(const DGL_VertexShader* shader)
{
    if (mVertexCustomShader == shader)
        mVertexCustomShader = nullptr;
}

//*************************************************************************************************
const DGL_PixelShader* ShaderManager::GetCurrentPixelShader() const
{
    return mCurrentPixelShaderMode == DGL_PSM_CUSTOM ? mPixelCustomShader : mPixelShader.get();
}

//*************************************************************************************************
const DGL_VertexShader* ShaderManager::GetCurrentVertexShader() const
{
    return mCurrentVertexShaderMode == DGL_VSM_CUSTOM ? mVertexCustomShader : mVertexShader.get();
}

}   // namespace DGL
