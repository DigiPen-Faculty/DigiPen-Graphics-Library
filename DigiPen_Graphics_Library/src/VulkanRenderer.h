//-------------------------------------------------------------------------------------------------
// file:    VulkanRenderer.ixx
// author:  Andy Ellinger
// brief:   Wrapper for Vulkan functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

#include "DGL.h"
#include "InternalTypes.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <functional>
#include <memory>

namespace DGL
{

namespace Texture
{
    std::unique_ptr<DGL_Texture> CreateTexture(const unsigned char* pixels, int width, int height);
    std::unique_ptr<DGL_Texture> LoadTextureFile(const char* filename);
    std::unique_ptr<DGL_Texture> CreateRenderTexture(int width, int height);
}

//---------------------------------------------------------------------------------- VulkanRenderer

class VulkanRenderer
{
public:
    // Creates the Vulkan instance, device, swap chain, and other objects.
    // Returns 0 on success and 1 if there was an error.
    int Initialize(const DGL_SysInitInfo& sysInitInfo);

    // Waits for the GPU to finish and releases all Vulkan objects
    void Release();

    // Start a new drawing session
    void StartUpdate();

    // End the current drawing session and present the image
    void EndUpdate();

    // Returns true if StartUpdate has been called and EndUpdate has not been called
    bool IsUpdateStarted() const;

    // Set the blend mode to use on the next draw
    void SetBlendMode(DGL_BlendMode mode);

    // Set the sampler state to use on the next draw
    void SetSamplerState(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode);

    // Adjust to a change in window size
    void ResetOnSizeChange();

    // Draws the mesh with the current state, to the window or to the render target if one is provided.
    // The arguments must already be checked: this only skips drawing while the window is minimized.
    void DrawMesh(const DGL_Mesh& mesh, DGL_DrawMode mode, const GpuImage& texture,
        const DGL_VertexShader& vertexShader, const DGL_PixelShader& pixelShader, const GpuImage* renderTarget);

    // Creates a buffer in memory the CPU can write to, and copies the data into it (if data is not null).
    // Returns false if there was an error, in which case buffer may be partly created.
    bool CreateBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, GpuBuffer& buffer);

    // Destroys the buffer once the GPU is no longer using it (it may be empty or partly created)
    void DestroyBuffer(const GpuBuffer& buffer);

    // Creates an image and its view. If pixels is not null it must contain width * height RGBA values
    // (8 bits per channel), otherwise the image is cleared to transparent black. Returns false if there
    // was an error, in which case image may be partly created.
    bool CreateImage(uint32_t width, uint32_t height, VkFormat format, bool renderTarget, const void* pixels,
        GpuImage& image);

    // Destroys the image and its view once the GPU is no longer using them (it may be empty or partly created)
    void DestroyImage(const GpuImage& image);

    // Clears an image created as a render target to the specified color
    void ClearImage(const GpuImage& image, const DGL_Color& color);

    // Returns the format to use for render textures
    VkFormat GetRenderTextureFormat() const;

    // Creates a shader module from SPIR-V code
    VkShaderModule CreateShaderModule(const std::vector<uint32_t>& spirv);

    // Destroys the shader module (and any pipelines using it) once the GPU is no longer using it
    void DestroyShaderModule(VkShaderModule module);

    // Stores the constant buffer data that will be applied
    DGL_ConstantBuffer mConstantBuffer;

    // The color that will be used to clear the window
    float mBackgroundColor[4]{ 0.0f, 0.0f, 0.0f, 1.0f };

private:
    // The number of frames that can be processed by the GPU at the same time
    static constexpr int cFramesInFlight{ 2 };

    // The size of each block of memory used for constant buffer data
    static constexpr VkDeviceSize cUniformBlockSize{ 256 * 1024 };

    // A block of memory for constant buffer data
    struct UniformBlock
    {
        GpuBuffer buffer;
        uint8_t* mapped{ nullptr };
    };

    // The objects needed for each frame in flight
    struct FrameData
    {
        VkCommandPool commandPool{ VK_NULL_HANDLE };
        VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
        // The timeline value signaled when the GPU finishes this frame's last submitted commands
        uint64_t submitValue{ 0 };
        // Signaled when the swap chain image is ready to be drawn to
        VkSemaphore imageAvailable{ VK_NULL_HANDLE };
        // Memory for constant buffer data, filled during the frame
        std::vector<UniformBlock> uniformBlocks;
        size_t uniformBlockIndex{ 0 };
        VkDeviceSize uniformOffset{ 0 };
    };

    // An object to destroy once the timeline reaches the value of the last frame that might use it
    struct PendingDeletion
    {
        uint64_t submitValue{ 0 };
        std::function<void()> destroy;
    };

    // Identifies a pipeline: the shaders and the fixed state
    struct PipelineKey
    {
        VkShaderModule vertexShader{ VK_NULL_HANDLE };
        VkShaderModule pixelShader{ VK_NULL_HANDLE };
        DGL_BlendMode blendMode{ DGL_BM_NONE };
        DGL_DrawMode drawMode{ DGL_DM_TRIANGLELIST };
        VkFormat format{ VK_FORMAT_UNDEFINED };

        bool operator==(const PipelineKey& other) const = default;
    };

    struct PipelineKeyHash
    {
        size_t operator()(const PipelineKey& key) const;
    };

    // Initialization steps. Each returns false if there was an error.
    bool CreateInstance();
    bool ChooseDevice();
    bool CreateDevice();
    bool CreateFrameObjects();
    bool CreateSamplers();
    bool CreateDescriptorAndPipelineLayouts();
    void ChooseRenderTextureFormat();

    // Creates the swap chain (replacing the existing one). Returns false if there was an error.
    bool CreateSwapchain();
    // Destroys the swap chain image views and semaphores
    void DestroySwapchainObjects();

    // Runs the commands recorded by the function immediately, and waits for them to finish
    bool RunImmediateCommands(const std::function<void(VkCommandBuffer)>& record);

    // Records a layout transition for the image
    void TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
        VkImageLayout newLayout);

    // Starts rendering to the window (if renderTarget is null) or to the render target image
    void BeginRendering(const GpuImage* renderTarget, bool clear);
    // Ends the current rendering, if there is one
    void EndRendering();

    // Returns a pipeline for the key, creating it if needed
    VkPipeline GetPipeline(const PipelineKey& key, const DGL_VertexShader& vertexShader);

    // Gets space for the constant buffer data in the current frame's memory
    bool AllocateUniformData(VkBuffer& buffer, VkDeviceSize& offset);

    // Adds an object to be destroyed once the GPU is no longer using it
    void DestroyLater(std::function<void()> destroy);
    // Destroys objects the GPU is no longer using (or all of them, if waitForAll is true)
    void ProcessPendingDeletions(bool waitForAll);

    // Vulkan objects
    VkInstance mInstance{ VK_NULL_HANDLE };
    VkDebugUtilsMessengerEXT mDebugMessenger{ VK_NULL_HANDLE };
    VkSurfaceKHR mSurface{ VK_NULL_HANDLE };
    VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };
    VkDevice mDevice{ VK_NULL_HANDLE };
    VkQueue mQueue{ VK_NULL_HANDLE };
    uint32_t mQueueFamily{ 0 };
    VmaAllocator mAllocator{ nullptr };
    PFN_vkCmdPushDescriptorSetKHR mCmdPushDescriptorSet{ nullptr };
    bool mHasBresenhamLines{ false };
    VkDeviceSize mUniformAlignment{ 256 };

    // Swap chain objects
    VkSwapchainKHR mSwapchain{ VK_NULL_HANDLE };
    VkFormat mSwapchainFormat{ VK_FORMAT_UNDEFINED };
    VkExtent2D mSwapchainExtent{ 0, 0 };
    std::vector<VkImage> mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    // Signaled when rendering to each swap chain image is finished, so it can be presented
    std::vector<VkSemaphore> mRenderFinished;
    bool mSwapchainDirty{ false };

    // Frame objects
    FrameData mFrames[cFramesInFlight];
    int mFrameIndex{ 0 };
    uint32_t mImageIndex{ 0 };
    // A timeline semaphore that counts the frames the GPU has finished. Each frame submit signals
    // the next value, so the semaphore's value tells which frames' objects are no longer in use.
    VkSemaphore mTimeline{ VK_NULL_HANDLE };
    // The number of frames submitted (the value the timeline will reach once the GPU catches up)
    uint64_t mSubmitCount{ 0 };
    // Objects waiting to be destroyed, in order of submitValue
    std::vector<PendingDeletion> mPendingDeletions;

    // Command pool for running commands immediately
    VkCommandPool mImmediateCommandPool{ VK_NULL_HANDLE };

    // Pipeline objects
    VkDescriptorSetLayout mDescriptorSetLayout{ VK_NULL_HANDLE };
    VkPipelineLayout mPipelineLayout{ VK_NULL_HANDLE };
    std::unordered_map<PipelineKey, VkPipeline, PipelineKeyHash> mPipelines;
    VkFormat mRenderTextureFormat{ VK_FORMAT_R32G32B32A32_SFLOAT };

    // Samplers for each sample mode (linear, point) and address mode (wrap, mirror, clamp, mirror once)
    VkSampler mSamplers[2][4]{};
    VkSampler mCurrentSampler{ VK_NULL_HANDLE };
    DGL_BlendMode mBlendMode{ DGL_BM_NONE };

    // Drawing state
    // Set between StartUpdate and EndUpdate
    bool mUpdateStarted{ false };
    // Set when the current frame is being recorded (it is not while the window is minimized)
    bool mFrameActive{ false };
    // Set while rendering is started on the command buffer
    bool mRenderingActive{ false };
    // The current render target image, or null for the window
    const GpuImage* mRenderTarget{ nullptr };
    VkPipeline mBoundPipeline{ VK_NULL_HANDLE };
};

} // namespace DGL
