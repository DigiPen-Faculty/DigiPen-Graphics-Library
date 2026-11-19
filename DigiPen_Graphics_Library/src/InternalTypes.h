//-------------------------------------------------------------------------------------------------
// file:    InternalTypes.h
// author:  Andy Ellinger
// brief:   Definitions of exposed types that are not defined in the public header file
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#pragma once

#include "DGL.h"
#include <vulkan/vulkan.h>
#pragma warning(push, 0)
#include <vma/vk_mem_alloc.h>
#pragma warning(pop)
#include <optional>
#include <string>
#include <string_view>

//------------------------------------------------------------------------------------- GPU objects
// These are created and destroyed by the renderer.

// A Vulkan buffer and its memory
struct GpuBuffer
{
    VkBuffer handle{ VK_NULL_HANDLE };
    VmaAllocation allocation{ nullptr };
};

// A Vulkan image, its memory, and the view used to sample and render to it
struct GpuImage
{
    VkImage handle{ VK_NULL_HANDLE };
    VmaAllocation allocation{ nullptr };
    VkImageView view{ VK_NULL_HANDLE };
    VkFormat format{ VK_FORMAT_UNDEFINED };
    VkExtent2D extent{ 0, 0 };
    // The layout the image will be in when the commands recorded so far have run.
    // This changes as the image is drawn to and drawn with, even through a const reference.
    mutable VkImageLayout layout{ VK_IMAGE_LAYOUT_UNDEFINED };
};

//------------------------------------------------------------------------------------ DGL resources
// The types the public header declares. Each one releases its GPU objects when it is deleted,
// once the GPU is no longer using them. They can't be copied, since only one copy may release them.

// Stores the data for a single vertex, including position, color, and texture coordinates.
typedef struct VertexData
{
    // The position of this vertex
    DGL_Vec2 mPosition;
    // The color associated with this vertex
    DGL_Color mColor;
    // The texture coordinates associated with this vertex
    DGL_Vec2 mTexCoord;
} VertexData;

// A mesh stores the vertex buffer and the index buffer for an indexed mesh
typedef struct DGL_Mesh
{
    DGL_Mesh() = default;
    DGL_Mesh(const DGL_Mesh&) = delete;
    DGL_Mesh& operator=(const DGL_Mesh&) = delete;
    ~DGL_Mesh();

    GpuBuffer vertexBuffer;
    unsigned vertexCount{ 0 };
    // Only used by indexed meshes (indexCount is 0 for other meshes)
    GpuBuffer indexBuffer;
    unsigned indexCount{ 0 };
} DGL_Mesh;

// A texture loaded from a file or pixel data, or a render texture
typedef struct DGL_Texture
{
    DGL_Texture() = default;
    DGL_Texture(const DGL_Texture&) = delete;
    DGL_Texture& operator=(const DGL_Texture&) = delete;
    ~DGL_Texture();

    // Returns true if the texture can be drawn to
    bool IsRenderTexture() const { return renderWorldMatrix.has_value(); }

    GpuImage image;
    // For render textures, the world matrix to use when drawing to the texture (a camera showing
    // the whole texture). Empty for other textures.
    std::optional<DGL_Mat4> renderWorldMatrix;
} DGL_Texture;

// The parts shared by pixel and vertex shaders
struct ShaderBase
{
    explicit ShaderBase(std::string_view filename) :
        filename(filename)
    {}
    ShaderBase(const ShaderBase&) = delete;
    ShaderBase& operator=(const ShaderBase&) = delete;
    // Destroys the shader module (and any pipelines using it)
    ~ShaderBase();

    VkShaderModule module{ VK_NULL_HANDLE };
    // The file the shader was loaded from
    std::string filename;
};

// A pixel shader
typedef struct DGL_PixelShader : ShaderBase
{
    using ShaderBase::ShaderBase;
} DGL_PixelShader;

// The vertex shader input locations for each part of the vertex data (-1 if the shader doesn't use it)
typedef struct VertexInputLocations
{
    int position{ 0 };
    int color{ 1 };
    int texCoord{ 2 };
} VertexInputLocations;

// A vertex shader
typedef struct DGL_VertexShader : ShaderBase
{
    using ShaderBase::ShaderBase;

    // Where the shader expects each part of the vertex data
    VertexInputLocations inputs;
} DGL_VertexShader;
