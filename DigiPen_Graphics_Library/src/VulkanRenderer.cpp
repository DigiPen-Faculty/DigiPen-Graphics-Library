//-------------------------------------------------------------------------------------------------
// file:    VulkanRenderer.cpp
// author:  Andy Ellinger
// brief:   Wrapper for Vulkan functionality
//
// Copyright © 2026 DigiPen, All rights reserved.
//-------------------------------------------------------------------------------------------------

#include "VulkanRenderer.h"
#include "Error.h"
#include "GraphicsSystem.h"
#include "WindowSystem.h"
#include "DGLMath.h"
#include "Camera.h"

// Turns off warnings from the third-party implementations compiled in this file
// (GCC and Clang don't have extra warnings turned on in the CMake build, so nothing is needed there)
#if defined(_MSC_VER)
#define DGL_DISABLE_WARNINGS __pragma(warning(push, 0))
#define DGL_RESTORE_WARNINGS __pragma(warning(pop))
#else
#define DGL_DISABLE_WARNINGS
#define DGL_RESTORE_WARNINGS
#endif

// The Vulkan Memory Allocator implementation is compiled here
DGL_DISABLE_WARNINGS
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>
DGL_RESTORE_WARNINGS

#include <vulkan/vk_enum_string_helper.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

// The stb_image implementation is compiled here
DGL_DISABLE_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
DGL_RESTORE_WARNINGS

//*************************************************************************************************
DGL_Texture::~DGL_Texture()
{
    // The renderer destroys the image once the GPU is no longer using it
    DGL::gGraphics->Renderer.DestroyImage(image);
}

namespace DGL
{

namespace Texture
{
    //*************************************************************************************************
    std::unique_ptr<DGL_Texture> CreateTexture(const unsigned char* pixels, int width, int height)
    {
        // 8 bits per channel, without sRGB conversion, like D3D used. If this fails, deleting the
        // texture releases whatever was created (and the renderer set the error message).
        auto texture = std::make_unique<DGL_Texture>();
        if (!gGraphics->Renderer.CreateImage((uint32_t)width, (uint32_t)height, VK_FORMAT_R8G8B8A8_UNORM, false, pixels,
            texture->image))
            return nullptr;

        return texture;
    }

    //*************************************************************************************************
    std::unique_ptr<DGL_Texture> LoadTextureFile(const char* filename)
    {
        // Load the image as 8-bit RGBA pixels
        int width, height, channels;
        unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4);
        if (!pixels)
        {
            std::stringstream stream;
            stream << "Failed to load texture from file \"" << filename << "\". " << stbi_failure_reason();
            gError->SetError(stream.str());
            return nullptr;
        }

        std::unique_ptr<DGL_Texture> texture = CreateTexture(pixels, width, height);
        stbi_image_free(pixels);
        return texture;
    }

    //*************************************************************************************************
    std::unique_ptr<DGL_Texture> CreateRenderTexture(int width, int height)
    {
        auto texture = std::make_unique<DGL_Texture>();
        if (!gGraphics->Renderer.CreateImage((uint32_t)width, (uint32_t)height,
            gGraphics->Renderer.GetRenderTextureFormat(), true, nullptr, texture->image))
            return nullptr;

        // Drawing to the texture uses a camera at its center, showing the whole texture
        texture->renderWorldMatrix = gGraphics->Camera.CreateWorldMatrix({ 0.0f, 0.0f }, 0.0f, 
            (float)width, (float)height);
        return texture;
    }
} // namespace Texture

namespace
{

    //*************************************************************************************************
    // Sets the error message if the result is not VK_SUCCESS. Returns true if the result is VK_SUCCESS.
    bool CheckResult(VkResult result, const char* description)
    {
        if (result == VK_SUCCESS)
            return true;

        std::string msg = description;
        msg += " (";
        msg += string_VkResult(result);
        msg += ")";
        gError->SetError(msg);
        return false;
    }

    //*************************************************************************************************
    // Sends validation layer messages to the error handler (errors) or stderr (warnings)
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
    {
        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            gError->SetError(std::string("Vulkan validation error: ") + data->pMessage);
        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            fprintf(stderr, "Vulkan validation warning: %s\n", data->pMessage);

        return VK_FALSE;
    }

#ifdef _DEBUG
    //*************************************************************************************************
    bool HasInstanceLayer(const char* name)
    {
        uint32_t count = 0;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        std::vector<VkLayerProperties> layers(count);
        vkEnumerateInstanceLayerProperties(&count, layers.data());

        for (const VkLayerProperties& layer : layers)
        {
            if (strcmp(layer.layerName, name) == 0)
                return true;
        }
        return false;
    }
#endif

    //*************************************************************************************************
    bool HasInstanceExtension(const char* name)
    {
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> extensions(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());

        for (const VkExtensionProperties& extension : extensions)
        {
            if (strcmp(extension.extensionName, name) == 0)
                return true;
        }
        return false;
    }

    //*************************************************************************************************
    bool HasDeviceExtension(const std::vector<VkExtensionProperties>& extensions, const char* name)
    {
        for (const VkExtensionProperties& extension : extensions)
        {
            if (strcmp(extension.extensionName, name) == 0)
                return true;
        }
        return false;
    }

    //*************************************************************************************************
    // The device feature structs DGL checks and turns on. Used both to ask the GPU what it supports and
    // to turn features on when creating the device.
    struct DeviceFeatures
    {
        VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
        VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
        VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
        VkPhysicalDeviceLineRasterizationFeaturesEXT lineRasterization{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES_EXT };

        // The structs point to each other, so they can't be copied
        DeviceFeatures() = default;
        DeviceFeatures(const DeviceFeatures&) = delete;
        DeviceFeatures& operator=(const DeviceFeatures&) = delete;

        // Links the structs into a pNext chain and returns the start of it. The line rasterization
        // struct is only included if its extension is being used.
        VkPhysicalDeviceFeatures2* Link(bool useLineRasterization)
        {
            void** next = &features.pNext;
            auto append = [&next](auto& s)
                {
                    *next = &s;
                    next = &s.pNext;
                };

            append(features12);
            append(features13);
            if (useLineRasterization)
                append(lineRasterization);
            *next = nullptr;
            return &features;
        }
    };

    //*************************************************************************************************
    // Returns the value of the DGL_VULKAN_DEVICE environment variable, or -1 if it isn't set.
    // This can be used to choose a GPU on computers with more than one.
    int GetRequestedDeviceIndex()
    {
#if defined(_MSC_VER)
        char* value = nullptr;
        size_t length = 0;
        if (_dupenv_s(&value, &length, "DGL_VULKAN_DEVICE") != 0 || !value)
            return -1;

        int index = atoi(value);
        free(value);
        return index;
#else
        const char* value = std::getenv("DGL_VULKAN_DEVICE");
        return value ? atoi(value) : -1;
#endif
    }

    //*************************************************************************************************
    VkPrimitiveTopology GetTopology(DGL_DrawMode mode)
    {
        switch (mode)
        {
        case DGL_DM_TRIANGLESTRIP:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case DGL_DM_LINELIST:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case DGL_DM_LINESTRIP:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case DGL_DM_TRIANGLELIST:
        default:                    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    //*************************************************************************************************
    VkPipelineColorBlendAttachmentState GetBlendState(DGL_BlendMode mode)
    {
        VkPipelineColorBlendAttachmentState state{};
        state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        state.colorBlendOp = VK_BLEND_OP_ADD;
        state.alphaBlendOp = VK_BLEND_OP_ADD;

        switch (mode)
        {
        case DGL_BM_BLEND:
            state.blendEnable = VK_TRUE;
            state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;
        case DGL_BM_ADD:
            state.blendEnable = VK_TRUE;
            state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
            state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            break;
        case DGL_BM_MULTIPLY:
            state.blendEnable = VK_TRUE;
            state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
            state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            break;
        case DGL_BM_NONE:
        default:
            state.blendEnable = VK_FALSE;
            break;
        }

        return state;
    }

} // namespace

//------------------------------------------------------------------------------------ VulkanRenderer

//*************************************************************************************************
size_t VulkanRenderer::PipelineKeyHash::operator()(const PipelineKey& key) const
{
    size_t hash = std::hash<void*>()(key.vertexShader);
    hash = hash * 31 + std::hash<void*>()(key.pixelShader);
    hash = hash * 31 + (size_t)key.blendMode;
    hash = hash * 31 + (size_t)key.drawMode;
    hash = hash * 31 + (size_t)key.format;
    return hash;
}

//*************************************************************************************************
int VulkanRenderer::Initialize(const DGL_SysInitInfo&)
{
    if (!CreateInstance() || !ChooseDevice() || !CreateDevice() || !CreateFrameObjects() || !CreateSamplers() ||
        !CreateDescriptorAndPipelineLayouts())
        return 1;

    ChooseRenderTextureFormat();

    if (!CreateSwapchain())
        return 1;

    // Set the default constant buffer values
    memset(&mConstantBuffer, 0, sizeof(DGL_ConstantBuffer));
    mConstantBuffer.worldMatrix.m[0][0] = 1;
    mConstantBuffer.worldMatrix.m[1][1] = 1;
    mConstantBuffer.worldMatrix.m[2][2] = 1;
    mConstantBuffer.worldMatrix.m[3][3] = 1;
    mConstantBuffer.alpha = 1.0f;
    mConstantBuffer.tintColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    mConstantBuffer.worldMatrix = gGraphics->Camera.GetWorldMatrix();

    // Set the default draw settings
    mBlendMode = DGL_BM_NONE;
    SetSamplerState(DGL_TSM_LINEAR, DGL_AM_WRAP);

    return 0;
}

//*************************************************************************************************
bool VulkanRenderer::CreateInstance()
{
    // Check the Vulkan version supported by the installed Vulkan loader
    uint32_t version = 0;
    if (vkEnumerateInstanceVersion(&version) != VK_SUCCESS || version < VK_API_VERSION_1_3)
    {
        gError->SetError("Vulkan 1.3 is not available. Try updating your graphics driver.");
        return false;
    }

    // Get the extensions needed to draw to the window
    uint32_t count = 0;
    const char** extensionsRaw = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> extensions(extensionsRaw, extensionsRaw + count);
    if (extensions.empty())
    {
        gError->SetError("Vulkan can't draw to windows on this computer. Try updating your graphics driver.");
        return false;
    }

    // In debug builds, use the validation layer if it is installed (it comes with the Vulkan SDK)
    std::vector<const char*> layers;
    bool useValidation = false;
#ifdef _DEBUG
    useValidation = HasInstanceLayer("VK_LAYER_KHRONOS_validation") &&
        HasInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    if (useValidation)
    {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "DGL Application";
    appInfo.pEngineName = "DigiPen Graphics Library";
    appInfo.engineVersion = VK_MAKE_VERSION(2, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugInfo.pfnUserCallback = DebugCallback;

    VkInstanceCreateInfo instanceInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };

    // On macOS, Vulkan runs on top of Metal using MoltenVK, which doesn't support all of Vulkan.
    // The Vulkan loader hides these "portability" GPUs unless the program asks to see them.
    if (HasInstanceExtension(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
    {
        extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        instanceInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }

    // Passing the debug info here also reports problems with creating and destroying the instance
    instanceInfo.pNext = useValidation ? &debugInfo : nullptr;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = (uint32_t)layers.size();
    instanceInfo.ppEnabledLayerNames = layers.data();
    instanceInfo.enabledExtensionCount = (uint32_t)extensions.size();
    instanceInfo.ppEnabledExtensionNames = extensions.data();

    if (!CheckResult(vkCreateInstance(&instanceInfo, nullptr, &mInstance), "Problem creating Vulkan instance."))
        return false;

    if (useValidation)
    {
        auto createMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance,
            "vkCreateDebugUtilsMessengerEXT");
        if (createMessenger)
            createMessenger(mInstance, &debugInfo, nullptr, &mDebugMessenger);
    }

    // Create the surface for drawing to the window
    if (!CheckResult(glfwCreateWindowSurface(mInstance, gWinSys->GetWindow(), nullptr, &mSurface), 
        "Problem creating Vulkan surface."))
        return false;

    return true;
}

//*************************************************************************************************
bool VulkanRenderer::ChooseDevice()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(mInstance, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(mInstance, &count, devices.data());

    int requestedIndex = GetRequestedDeviceIndex();
    int bestScore = 0;
    std::string rejected;

    for (uint32_t i = 0; i < count; ++i)
    {
        if (requestedIndex >= 0 && (int)i != requestedIndex)
            continue;

        VkPhysicalDevice device = devices[i];
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        std::string reason;

        // Check the version and extensions
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());
        bool hasLineExtension = HasDeviceExtension(extensions, VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);

        if (props.apiVersion < VK_API_VERSION_1_3)
            reason = "does not support Vulkan 1.3";
        else if (!HasDeviceExtension(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
            reason = "can't draw to windows";
        else if (!HasDeviceExtension(extensions, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME))
            reason = "does not support " VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME;

        // Check the features
        DeviceFeatures supported;
        if (reason.empty())
        {
            vkGetPhysicalDeviceFeatures2(device, supported.Link(hasLineExtension));
            if (!supported.features13.dynamicRendering || !supported.features13.synchronization2 ||
                !supported.features13.shaderDemoteToHelperInvocation ||
                !supported.features12.samplerMirrorClampToEdge || !supported.features12.timelineSemaphore)
                reason = "is missing required features";
        }

        // Find a queue that can draw and present to the window
        uint32_t queueFamily = UINT32_MAX;
        if (reason.empty())
        {
            uint32_t familyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
            std::vector<VkQueueFamilyProperties> families(familyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());
            for (uint32_t f = 0; f < familyCount && queueFamily == UINT32_MAX; ++f)
            {
                VkBool32 canPresent = VK_FALSE;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, f, mSurface, &canPresent);
                if ((families[f].queueFlags & VK_QUEUE_GRAPHICS_BIT) && canPresent)
                    queueFamily = f;
            }
            if (queueFamily == UINT32_MAX)
                reason = "can't draw to this window";
        }

        if (!reason.empty())
        {
            rejected += "\n  ";
            rejected += props.deviceName;
            rejected += " ";
            rejected += reason;
            continue;
        }

        // Prefer a discrete GPU
        int score = props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 3 :
            props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? 2 : 1;
        if (score > bestScore)
        {
            bestScore = score;
            mPhysicalDevice = device;
            mQueueFamily = queueFamily;
            mHasBresenhamLines = hasLineExtension && supported.lineRasterization.bresenhamLines;
            mUniformAlignment = std::max<VkDeviceSize>(props.limits.minUniformBufferOffsetAlignment, 16);
        }
    }

    if (!mPhysicalDevice)
    {
        std::string msg = "Couldn't find a GPU that supports the Vulkan features DGL needs. Try updating your graphics driver.";
        msg += rejected;
        gError->SetError(msg);
        return false;
    }

    // Confirm which GPU is used when one was requested
    if (requestedIndex >= 0)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);
        printf("DGL: using GPU %d (%s)\n", requestedIndex, props.deviceName);
    }

    return true;
}

//*************************************************************************************************
bool VulkanRenderer::CreateDevice()
{
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    queueInfo.queueFamilyIndex = mQueueFamily;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &priority;

    // Turn on the features DGL uses
    DeviceFeatures enabled;
    enabled.features12.samplerMirrorClampToEdge = VK_TRUE;
    enabled.features12.timelineSemaphore = VK_TRUE;
    enabled.features13.dynamicRendering = VK_TRUE;
    enabled.features13.synchronization2 = VK_TRUE;
    // Needed for clip() and discard in HLSL pixel shaders
    enabled.features13.shaderDemoteToHelperInvocation = VK_TRUE;
    enabled.lineRasterization.bresenhamLines = VK_TRUE;

    std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME };
    if (mHasBresenhamLines)
        extensions.push_back(VK_EXT_LINE_RASTERIZATION_EXTENSION_NAME);

    // Vulkan requires turning on the portability subset extension for GPUs that have it (MoltenVK on macOS)
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> available(extensionCount);
    vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, available.data());
    if (HasDeviceExtension(available, "VK_KHR_portability_subset"))
        extensions.push_back("VK_KHR_portability_subset");

    VkDeviceCreateInfo deviceInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    deviceInfo.pNext = enabled.Link(mHasBresenhamLines);
    deviceInfo.queueCreateInfoCount = 1;
    deviceInfo.pQueueCreateInfos = &queueInfo;
    deviceInfo.enabledExtensionCount = (uint32_t)extensions.size();
    deviceInfo.ppEnabledExtensionNames = extensions.data();

    if (!CheckResult(vkCreateDevice(mPhysicalDevice, &deviceInfo, nullptr, &mDevice), "Problem creating Vulkan device."))
        return false;

    vkGetDeviceQueue(mDevice, mQueueFamily, 0, &mQueue);

    mCmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(mDevice, "vkCmdPushDescriptorSetKHR");
    if (!mCmdPushDescriptorSet)
    {
        gError->SetError("Problem loading vkCmdPushDescriptorSetKHR.");
        return false;
    }

    // Create the memory allocator
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.instance = mInstance;
    allocatorInfo.physicalDevice = mPhysicalDevice;
    allocatorInfo.device = mDevice;
    if (!CheckResult(vmaCreateAllocator(&allocatorInfo, &mAllocator), "Problem creating Vulkan memory allocator."))
        return false;

    return true;
}

//*************************************************************************************************
bool VulkanRenderer::CreateFrameObjects()
{
    for (FrameData& frame : mFrames)
    {
        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.queueFamilyIndex = mQueueFamily;
        if (!CheckResult(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &frame.commandPool),
            "Problem creating command pool."))
            return false;

        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = frame.commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        if (!CheckResult(vkAllocateCommandBuffers(mDevice, &allocInfo, &frame.commandBuffer),
            "Problem creating command buffer."))
            return false;

        VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        if (!CheckResult(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &frame.imageAvailable),
            "Problem creating semaphore."))
            return false;
    }

    // The timeline starts at 0, meaning no frames have finished
    VkSemaphoreTypeCreateInfo typeInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
    typeInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    typeInfo.initialValue = 0;
    VkSemaphoreCreateInfo timelineInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    timelineInfo.pNext = &typeInfo;
    if (!CheckResult(vkCreateSemaphore(mDevice, &timelineInfo, nullptr, &mTimeline),
        "Problem creating timeline semaphore."))
        return false;

    // Command pool for running commands immediately (uploading textures, etc.)
    VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = mQueueFamily;
    return CheckResult(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mImmediateCommandPool),
        "Problem creating command pool.");
}

//*************************************************************************************************
bool VulkanRenderer::CreateSamplers()
{
    const VkFilter filters[2] = { VK_FILTER_LINEAR, VK_FILTER_NEAREST };
    const VkSamplerMipmapMode mipModes[2] = { VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST };
    const VkSamplerAddressMode addressModes[4] = {
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE
    };

    for (int sample = 0; sample < 2; ++sample)
    {
        for (int address = 0; address < 4; ++address)
        {
            VkSamplerCreateInfo info{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
            info.magFilter = filters[sample];
            info.minFilter = filters[sample];
            info.mipmapMode = mipModes[sample];
            info.addressModeU = addressModes[address];
            info.addressModeV = addressModes[address];
            info.addressModeW = addressModes[address];
            info.minLod = 0.0f;
            info.maxLod = VK_LOD_CLAMP_NONE;
            if (!CheckResult(vkCreateSampler(mDevice, &info, nullptr, &mSamplers[sample][address]),
                "Problem creating sampler."))
                return false;
        }
    }

    return true;
}

//*************************************************************************************************
bool VulkanRenderer::CreateDescriptorAndPipelineLayouts()
{
    // Binding 0 is the constant buffer (register b0 in HLSL), binding 1 is the texture (t0),
    // and binding 2 is the sampler (s0). The descriptors are pushed with each draw.
    VkDescriptorSetLayoutBinding bindings[3]{};
    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo setInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    setInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
    setInfo.bindingCount = 3;
    setInfo.pBindings = bindings;
    if (!CheckResult(vkCreateDescriptorSetLayout(mDevice, &setInfo, nullptr, &mDescriptorSetLayout),
        "Problem creating descriptor set layout."))
        return false;

    VkPipelineLayoutCreateInfo layoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    layoutInfo.setLayoutCount = 1;
    layoutInfo.pSetLayouts = &mDescriptorSetLayout;
    if (!CheckResult(vkCreatePipelineLayout(mDevice, &layoutInfo, nullptr, &mPipelineLayout),
        "Problem creating pipeline layout."))
        return false;

    return true;
}

//*************************************************************************************************
void VulkanRenderer::ChooseRenderTextureFormat()
{
    // Use 32-bit float channels if the GPU can render to, blend, and filter them, otherwise 16-bit floats
    // (which every GPU supports)
    const VkFormatFeatureFlags needed = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT;

    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(mPhysicalDevice, VK_FORMAT_R32G32B32A32_SFLOAT, &props);
    mRenderTextureFormat = (props.optimalTilingFeatures & needed) == needed ?
        VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R16G16B16A16_SFLOAT;
}

//*************************************************************************************************
bool VulkanRenderer::CreateSwapchain()
{
    VkSurfaceCapabilitiesKHR caps;
    if (!CheckResult(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &caps),
        "Problem getting window surface information."))
        return false;

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        // The window system lets us choose the size, so use the window size
        DGL_Vec2 size = gWinSys->GetWindowSize();
        extent.width = std::clamp((uint32_t)size.x, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = std::clamp((uint32_t)size.y, caps.minImageExtent.height, caps.maxImageExtent.height);
    }

    // If the window is minimized, wait until it is restored
    if (extent.width == 0 || extent.height == 0)
    {
        mSwapchainDirty = true;
        return true;
    }

    // Choose an 8-bit format without sRGB conversion, which matches how colors were drawn with D3D
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, formats.data());
    if (formats.empty())
    {
        gError->SetError("The window surface has no formats.");
        return false;
    }
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const VkSurfaceFormatKHR& format : formats)
    {
        if ((format.format == VK_FORMAT_B8G8R8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_UNORM) &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = format;
            break;
        }
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
        imageCount = caps.maxImageCount;

    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if (!(caps.supportedCompositeAlpha & compositeAlpha))
        compositeAlpha = (VkCompositeAlphaFlagBitsKHR)(caps.supportedCompositeAlpha & ~(caps.supportedCompositeAlpha - 1));

    VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    info.surface = mSurface;
    info.minImageCount = imageCount;
    info.imageFormat = surfaceFormat.format;
    info.imageColorSpace = surfaceFormat.colorSpace;
    info.imageExtent = extent;
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.preTransform = caps.currentTransform;
    info.compositeAlpha = compositeAlpha;
    // FIFO waits for vertical sync, like D3D's Present(1, 0)
    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    info.clipped = VK_TRUE;
    info.oldSwapchain = mSwapchain;

    // Make sure the GPU isn't using the old swap chain
    vkDeviceWaitIdle(mDevice);

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    if (!CheckResult(vkCreateSwapchainKHR(mDevice, &info, nullptr, &newSwapchain), "Problem creating swap chain."))
        return false;

    // Replace the old swap chain
    DestroySwapchainObjects();
    if (mSwapchain)
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
    mSwapchain = newSwapchain;
    mSwapchainFormat = surfaceFormat.format;
    mSwapchainExtent = extent;

    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
    mSwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

    for (VkImage image : mSwapchainImages)
    {
        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = mSwapchainFormat;
        viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        VkImageView view = VK_NULL_HANDLE;
        if (!CheckResult(vkCreateImageView(mDevice, &viewInfo, nullptr, &view), "Problem creating swap chain image view."))
            return false;
        mSwapchainImageViews.push_back(view);

        VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkSemaphore semaphore = VK_NULL_HANDLE;
        if (!CheckResult(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &semaphore), "Problem creating semaphore."))
            return false;
        mRenderFinished.push_back(semaphore);
    }

    mSwapchainDirty = false;
    return true;
}

//*************************************************************************************************
void VulkanRenderer::DestroySwapchainObjects()
{
    for (VkImageView view : mSwapchainImageViews)
        vkDestroyImageView(mDevice, view, nullptr);
    mSwapchainImageViews.clear();

    for (VkSemaphore semaphore : mRenderFinished)
        vkDestroySemaphore(mDevice, semaphore, nullptr);
    mRenderFinished.clear();

    mSwapchainImages.clear();
}

//*************************************************************************************************
void VulkanRenderer::Release()
{
    if (mDevice)
    {
        // Wait for the GPU to finish, then destroy everything
        vkDeviceWaitIdle(mDevice);
        ProcessPendingDeletions(true);

        for (auto& [key, pipeline] : mPipelines)
            vkDestroyPipeline(mDevice, pipeline, nullptr);
        mPipelines.clear();

        for (auto& samplers : mSamplers)
        {
            for (VkSampler& sampler : samplers)
            {
                vkDestroySampler(mDevice, sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
        }

        vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);

        for (FrameData& frame : mFrames)
        {
            for (UniformBlock& block : frame.uniformBlocks)
                vmaDestroyBuffer(mAllocator, block.buffer.handle, block.buffer.allocation);
            frame.uniformBlocks.clear();
            vkDestroyCommandPool(mDevice, frame.commandPool, nullptr);
            vkDestroySemaphore(mDevice, frame.imageAvailable, nullptr);
            frame = FrameData{};
        }

        vkDestroySemaphore(mDevice, mTimeline, nullptr);
        vkDestroyCommandPool(mDevice, mImmediateCommandPool, nullptr);

        DestroySwapchainObjects();
        vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);

        if (mAllocator)
            vmaDestroyAllocator(mAllocator);
        vkDestroyDevice(mDevice, nullptr);
    }

    if (mInstance)
    {
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

        if (mDebugMessenger)
        {
            auto destroyMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance,
                "vkDestroyDebugUtilsMessengerEXT");
            if (destroyMessenger)
                destroyMessenger(mInstance, mDebugMessenger, nullptr);
        }

        vkDestroyInstance(mInstance, nullptr);
    }

    // Reset everything to the starting values
    *this = VulkanRenderer{};
}

//*************************************************************************************************
void VulkanRenderer::StartUpdate()
{
    if (!mDevice)
    {
        gError->SetError("Started Graphics update when not initialized.");
        return;
    }

    if (mUpdateStarted)
    {
        gError->SetError("Called DGL_Graphics_StartDrawing again without calling DGL_Graphics_FinishDrawing.");
        return;
    }

    mUpdateStarted = true;
    mFrameActive = false;

    ProcessPendingDeletions(false);

    // Recreate the swap chain if the window has changed
    if (mSwapchainDirty && !CreateSwapchain())
        return;

    // Don't draw anything while the window is minimized
    if (mSwapchainDirty)
        return;

    FrameData& frame = mFrames[mFrameIndex];

    // Wait for the GPU to finish the last frame that used this frame's objects. If that frame's
    // submit failed, submitValue wasn't updated, so this doesn't wait for a value that never comes.
    VkSemaphoreWaitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &mTimeline;
    waitInfo.pValues = &frame.submitValue;
    vkWaitSemaphores(mDevice, &waitInfo, UINT64_MAX);

    // Get the next swap chain image to draw to
    VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, frame.imageAvailable,
        VK_NULL_HANDLE, &mImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // The window changed, so recreate the swap chain and try again
        mSwapchainDirty = true;
        if (!CreateSwapchain() || mSwapchainDirty)
            return;
        result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, frame.imageAvailable,
            VK_NULL_HANDLE, &mImageIndex);
    }
    if (result == VK_SUBOPTIMAL_KHR)
        mSwapchainDirty = true;
    else if (!CheckResult(result, "Problem getting the next swap chain image."))
        return;

    // Reuse this frame's constant buffer memory from the beginning
    frame.uniformBlockIndex = 0;
    frame.uniformOffset = 0;

    // Start recording commands
    vkResetCommandPool(mDevice, frame.commandPool, 0);
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(frame.commandBuffer, &beginInfo);
    mFrameActive = true;

    // Start rendering to the window, clearing it to the background color
    BeginRendering(nullptr, true);
}

//*************************************************************************************************
void VulkanRenderer::EndUpdate()
{
    if (!mDevice)
    {
        gError->SetError("Ended Graphics update when not initialized.");
        return;
    }
    else if (!mUpdateStarted)
    {
        gError->SetError("Called DGL_Graphics_FinishDrawing without calling DGL_Graphics_StartDrawing.");
        return;
    }

    mUpdateStarted = false;

    // Nothing was recorded if the window is minimized
    if (!mFrameActive)
        return;
    mFrameActive = false;

    FrameData& frame = mFrames[mFrameIndex];
    VkCommandBuffer commandBuffer = frame.commandBuffer;

    // Finish rendering and get the image ready to show. The window image stays in the color
    // attachment layout for the whole frame, since it is only ever drawn to.
    EndRendering();
    TransitionImage(commandBuffer, mSwapchainImages[mImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    vkEndCommandBuffer(commandBuffer);

    // Make sure the GPU sees the constant buffer data
    for (size_t i = 0; i <= frame.uniformBlockIndex && i < frame.uniformBlocks.size(); ++i)
        vmaFlushAllocation(mAllocator, frame.uniformBlocks[i].buffer.allocation, 0, VK_WHOLE_SIZE);

    // Submit the commands, waiting for the image to be available. When finished, signal that the
    // image can be presented, and move the timeline to this frame's value.
    VkSemaphoreSubmitInfo waitInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO };
    waitInfo.semaphore = frame.imageAvailable;
    waitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkSemaphoreSubmitInfo signalInfos[2]{};
    signalInfos[0].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalInfos[0].semaphore = mRenderFinished[mImageIndex];
    signalInfos[0].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    signalInfos[1].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalInfos[1].semaphore = mTimeline;
    signalInfos[1].value = mSubmitCount + 1;
    signalInfos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    VkCommandBufferSubmitInfo commandInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    commandInfo.commandBuffer = commandBuffer;

    VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandInfo;
    submitInfo.signalSemaphoreInfoCount = 2;
    submitInfo.pSignalSemaphoreInfos = signalInfos;
    if (!CheckResult(vkQueueSubmit2(mQueue, 1, &submitInfo, VK_NULL_HANDLE), "Problem submitting commands."))
    {
        // Nothing will signal the render finished semaphore, so the image can't be presented.
        // Recreating the swap chain on the next frame gets every image back.
        mSwapchainDirty = true;
        return;
    }
    frame.submitValue = ++mSubmitCount;

    // Show the image in the window
    VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &mRenderFinished[mImageIndex];
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;
    presentInfo.pImageIndices = &mImageIndex;
    VkResult result = vkQueuePresentKHR(mQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        mSwapchainDirty = true;
    else
        CheckResult(result, "Problem presenting the swap chain image.");

    mFrameIndex = (mFrameIndex + 1) % cFramesInFlight;
}

//*************************************************************************************************
bool VulkanRenderer::IsUpdateStarted() const
{
    return mUpdateStarted;
}

//*************************************************************************************************
void VulkanRenderer::SetBlendMode(DGL_BlendMode mode)
{
    if (!mDevice)
    {
        gError->SetError("Tried to set blend mode when Graphics not initialized.");
        return;
    }

    switch (mode)
    {
    case DGL_BM_NONE:
    case DGL_BM_BLEND:
    case DGL_BM_ADD:
    case DGL_BM_MULTIPLY:
        mBlendMode = mode;
        break;
    default:
        gError->SetError("Passed in an invalid DGL_BlendMode value to DGL_Graphics_SetBlendMode.");
        break;
    }
}

//*************************************************************************************************
void VulkanRenderer::SetSamplerState(DGL_TextureSampleMode sampleMode, DGL_TextureAddressMode addressMode)
{
    if (!mDevice)
    {
        gError->SetError("Tried to set sampler state when Graphics not initialized.");
        return;
    }

    if (sampleMode != DGL_TSM_LINEAR && sampleMode != DGL_TSM_POINT)
    {
        gError->SetError("Passed in an invalid DGL_TextureSampleMode value to DGL_Graphics_SetTextureSamplerData.");
        return;
    }

    int addressIndex;
    switch (addressMode)
    {
    case DGL_AM_WRAP:           addressIndex = 0; break;
    case DGL_AM_MIRROR:         addressIndex = 1; break;
    case DGL_AM_CLAMP:          addressIndex = 2; break;
    case DGL_AM_MIRROR_ONCE:    addressIndex = 3; break;
    default:
        gError->SetError("Passed in an invalid DGL_TextureAddressMode value to DGL_Graphics_SetTextureSamplerData.");
        return;
    }

    mCurrentSampler = mSamplers[sampleMode == DGL_TSM_LINEAR ? 0 : 1][addressIndex];
}

//*************************************************************************************************
void VulkanRenderer::ResetOnSizeChange()
{
    if (!mDevice)
        return;

    // The swap chain will be recreated at the start of the next frame
    mSwapchainDirty = true;

    // Get the updated world matrix for the constant buffer
    mConstantBuffer.worldMatrix = gGraphics->Camera.GetWorldMatrix();
}

//*************************************************************************************************
void VulkanRenderer::DrawMesh(const DGL_Mesh& mesh, DGL_DrawMode mode, const GpuImage& texture,
    const DGL_VertexShader& vertexShader, const DGL_PixelShader& pixelShader, const GpuImage* renderTarget)
{
    // Nothing is drawn while the window is minimized
    if (!mFrameActive)
        return;

    VkCommandBuffer commandBuffer = mFrames[mFrameIndex].commandBuffer;

    // Make sure the texture is ready to be sampled. Layout changes can't happen while rendering.
    if (texture.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        EndRendering();
        TransitionImage(commandBuffer, texture.handle, texture.layout, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        texture.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    // Make sure we're rendering to the right place
    if (!mRenderingActive || mRenderTarget != renderTarget)
    {
        EndRendering();
        BeginRendering(renderTarget, false);
    }

    // Bind the pipeline for the current shaders and settings
    PipelineKey key;
    key.vertexShader = vertexShader.module;
    key.pixelShader = pixelShader.module;
    key.blendMode = mBlendMode;
    key.drawMode = mode;
    key.format = renderTarget ? renderTarget->format : mSwapchainFormat;
    VkPipeline pipeline = GetPipeline(key, vertexShader);
    if (!pipeline)
        return;
    if (pipeline != mBoundPipeline)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        mBoundPipeline = pipeline;
    }

    // Copy the constant buffer data for this draw
    VkBuffer uniformBuffer;
    VkDeviceSize uniformOffset;
    if (!AllocateUniformData(uniformBuffer, uniformOffset))
        return;

    // Set the constant buffer, texture, and sampler
    VkDescriptorBufferInfo bufferInfo{ uniformBuffer, uniformOffset, sizeof(DGL_ConstantBuffer) };
    VkDescriptorImageInfo imageInfo{ VK_NULL_HANDLE, texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    VkDescriptorImageInfo samplerInfo{ mCurrentSampler, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED };

    VkWriteDescriptorSet writes[3]{};
    for (uint32_t i = 0; i < 3; ++i)
    {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
    }
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &bufferInfo;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writes[1].pImageInfo = &imageInfo;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writes[2].pImageInfo = &samplerInfo;
    mCmdPushDescriptorSet(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 3, writes);

    // Draw the mesh
    VkDeviceSize vertexOffset = 0;
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer.handle, &vertexOffset);
    if (mesh.indexCount == 0)
        vkCmdDraw(commandBuffer, mesh.vertexCount, 1, 0, 0);
    else
    {
        vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, mesh.indexCount, 1, 0, 0, 0);
    }
}

//*************************************************************************************************
bool VulkanRenderer::CreateBuffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, GpuBuffer& buffer)
{
    // DGL meshes are small, so they are kept in memory the CPU can write to directly
    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VmaAllocationInfo info{};
    if (!CheckResult(vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo, &buffer.handle, &buffer.allocation, &info),
        "Problem creating buffer."))
        return false;

    if (data)
    {
        memcpy(info.pMappedData, data, (size_t)size);
        vmaFlushAllocation(mAllocator, buffer.allocation, 0, VK_WHOLE_SIZE);
    }

    return true;
}

//*************************************************************************************************
void VulkanRenderer::DestroyBuffer(const GpuBuffer& buffer)
{
    if (!buffer.handle || !mDevice)
        return;

    DestroyLater([this, buffer]() { vmaDestroyBuffer(mAllocator, buffer.handle, buffer.allocation); });
}

//*************************************************************************************************
bool VulkanRenderer::CreateImage(uint32_t width, uint32_t height, VkFormat format, bool renderTarget,
    const void* pixels, GpuImage& image)
{
    // Create the image in GPU memory
    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = { width, height, 1 };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (renderTarget)
        imageInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    if (!CheckResult(vmaCreateImage(mAllocator, &imageInfo, &allocInfo, &image.handle, &image.allocation, nullptr),
        "Problem creating texture image."))
        return false;
    image.format = format;
    image.extent = { width, height };

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = image.handle;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    if (!CheckResult(vkCreateImageView(mDevice, &viewInfo, nullptr, &image.view), "Problem creating texture image view."))
        return false;

    // Copy the pixels into a buffer the CPU can write to, so they can be copied to the image
    GpuBuffer staging;
    if (pixels && !CreateBuffer(pixels, (VkDeviceSize)width * height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, staging))
        return false;

    // Fill the image with the pixels (or clear it), then get it ready for sampling
    bool success = RunImmediateCommands([&](VkCommandBuffer commandBuffer)
        {
            TransitionImage(commandBuffer, image.handle, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            if (pixels)
            {
                VkBufferImageCopy region{};
                region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
                region.imageExtent = { width, height, 1 };
                vkCmdCopyBufferToImage(commandBuffer, staging.handle, image.handle,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
            }
            else
            {
                VkClearColorValue clearColor{};
                VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                vkCmdClearColorImage(commandBuffer, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    &clearColor, 1, &range);
            }
            TransitionImage(commandBuffer, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        });

    // The commands are finished, so the staging buffer can be destroyed now
    if (staging.handle)
        vmaDestroyBuffer(mAllocator, staging.handle, staging.allocation);

    image.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return success;
}

//*************************************************************************************************
void VulkanRenderer::DestroyImage(const GpuImage& image)
{
    if (!image.handle || !mDevice)
        return;

    DestroyLater([this, image]()
        {
            if (image.view)
                vkDestroyImageView(mDevice, image.view, nullptr);
            vmaDestroyImage(mAllocator, image.handle, image.allocation);
        });
}

//*************************************************************************************************
void VulkanRenderer::ClearImage(const GpuImage& image, const DGL_Color& color)
{
    auto record = [&](VkCommandBuffer commandBuffer)
        {
            TransitionImage(commandBuffer, image.handle, image.layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            image.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

            VkClearColorValue clearColor{ { color.r, color.g, color.b, color.a } };
            VkImageSubresourceRange range{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            vkCmdClearColorImage(commandBuffer, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &clearColor, 1, &range);
        };

    if (mFrameActive)
    {
        // Add the clear to this frame's commands (images can't be cleared while rendering)
        EndRendering();
        record(mFrames[mFrameIndex].commandBuffer);
    }
    else
        RunImmediateCommands(record);
}

//*************************************************************************************************
VkFormat VulkanRenderer::GetRenderTextureFormat() const
{
    return mRenderTextureFormat;
}

//*************************************************************************************************
VkShaderModule VulkanRenderer::CreateShaderModule(const std::vector<uint32_t>& spirv)
{
    VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    info.codeSize = spirv.size() * sizeof(uint32_t);
    info.pCode = spirv.data();
    VkShaderModule module = VK_NULL_HANDLE;
    CheckResult(vkCreateShaderModule(mDevice, &info, nullptr, &module), "Problem creating shader module.");
    return module;
}

//*************************************************************************************************
void VulkanRenderer::DestroyShaderModule(VkShaderModule module)
{
    if (!module || !mDevice)
        return;

    // Remove any pipelines that use this shader
    for (auto it = mPipelines.begin(); it != mPipelines.end();)
    {
        if (it->first.vertexShader == module || it->first.pixelShader == module)
        {
            VkPipeline pipeline = it->second;
            if (pipeline == mBoundPipeline)
                mBoundPipeline = VK_NULL_HANDLE;
            DestroyLater([this, pipeline]() { vkDestroyPipeline(mDevice, pipeline, nullptr); });
            it = mPipelines.erase(it);
        }
        else
            ++it;
    }

    DestroyLater([this, module]() { vkDestroyShaderModule(mDevice, module, nullptr); });
}

//*************************************************************************************************
bool VulkanRenderer::RunImmediateCommands(const std::function<void(VkCommandBuffer)>& record)
{
    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = mImmediateCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    if (!CheckResult(vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer), "Problem creating command buffer."))
        return false;

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    record(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    VkCommandBufferSubmitInfo commandInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO };
    commandInfo.commandBuffer = commandBuffer;
    VkSubmitInfo2 submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO_2 };
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandInfo;

    // The queue runs submits in order, so these commands finish after any earlier frames anyway.
    // Waiting for the whole queue costs nothing extra and doesn't need a fence.
    bool success = CheckResult(vkQueueSubmit2(mQueue, 1, &submitInfo, VK_NULL_HANDLE), "Problem submitting commands.");
    if (success)
        vkQueueWaitIdle(mQueue);

    vkFreeCommandBuffers(mDevice, mImmediateCommandPool, 1, &commandBuffer);
    return success;
}

//*************************************************************************************************
void VulkanRenderer::TransitionImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    // DGL keeps synchronization simple: wait for all earlier work before any later work uses the image.
    // A barrier is recorded even if the layout doesn't change, so earlier writes are finished first.
    VkImageMemoryBarrier2 barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
    barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dependency{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
    dependency.imageMemoryBarrierCount = 1;
    dependency.pImageMemoryBarriers = &barrier;
    vkCmdPipelineBarrier2(commandBuffer, &dependency);
}

//*************************************************************************************************
void VulkanRenderer::BeginRendering(const GpuImage* renderTarget, bool clear)
{
    VkCommandBuffer commandBuffer = mFrames[mFrameIndex].commandBuffer;

    VkImage image;
    VkImageView view;
    VkImageLayout oldLayout;
    VkExtent2D extent;
    if (renderTarget)
    {
        image = renderTarget->handle;
        view = renderTarget->view;
        oldLayout = renderTarget->layout;
        renderTarget->layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        extent = renderTarget->extent;
    }
    else
    {
        // The window image is cleared when the frame starts (so its old contents don't matter),
        // and stays in the color attachment layout for the rest of the frame
        image = mSwapchainImages[mImageIndex];
        view = mSwapchainImageViews[mImageIndex];
        oldLayout = clear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        extent = mSwapchainExtent;
    }

    TransitionImage(commandBuffer, image, oldLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderingAttachmentInfo attachment{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
    attachment.imageView = view;
    attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    memcpy(attachment.clearValue.color.float32, mBackgroundColor, sizeof(mBackgroundColor));

    VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    renderingInfo.renderArea = { { 0, 0 }, extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &attachment;
    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    // Use a negative viewport height so that +Y is up, matching D3D (and the DGL camera matrices)
    VkViewport viewport{ 0.0f, (float)extent.height, (float)extent.width, -(float)extent.height, 0.0f, 1.0f };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{ { 0, 0 }, extent };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    mRenderingActive = true;
    mRenderTarget = renderTarget;
    mBoundPipeline = VK_NULL_HANDLE;
}

//*************************************************************************************************
void VulkanRenderer::EndRendering()
{
    if (!mRenderingActive)
        return;

    vkCmdEndRendering(mFrames[mFrameIndex].commandBuffer);
    mRenderingActive = false;
}

//*************************************************************************************************
VkPipeline VulkanRenderer::GetPipeline(const PipelineKey& key, const DGL_VertexShader& vertexShader)
{
    auto found = mPipelines.find(key);
    if (found != mPipelines.end())
        return found->second;

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = key.vertexShader;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = key.pixelShader;
    stages[1].pName = "main";

    // The vertex data layout, matched to the inputs the vertex shader uses
    VkVertexInputBindingDescription binding{ 0, sizeof(VertexData), VK_VERTEX_INPUT_RATE_VERTEX };
    std::vector<VkVertexInputAttributeDescription> attributes;
    if (vertexShader.inputs.position >= 0)
        attributes.push_back({ (uint32_t)vertexShader.inputs.position, 0, VK_FORMAT_R32G32_SFLOAT,
            (uint32_t)offsetof(VertexData, mPosition) });
    if (vertexShader.inputs.color >= 0)
        attributes.push_back({ (uint32_t)vertexShader.inputs.color, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
            (uint32_t)offsetof(VertexData, mColor) });
    if (vertexShader.inputs.texCoord >= 0)
        attributes.push_back({ (uint32_t)vertexShader.inputs.texCoord, 0, VK_FORMAT_R32G32_SFLOAT,
            (uint32_t)offsetof(VertexData, mTexCoord) });

    VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInput.vertexBindingDescriptionCount = 1;
    vertexInput.pVertexBindingDescriptions = &binding;
    vertexInput.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
    vertexInput.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = GetTopology(key.drawMode);

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    // Draw lines with the same rules as D3D, if the GPU supports it (otherwise it is up to the GPU)
    VkPipelineRasterizationLineStateCreateInfoEXT lineState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT };
    lineState.lineRasterizationMode = VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
    if (mHasBresenhamLines && (key.drawMode == DGL_DM_LINELIST || key.drawMode == DGL_DM_LINESTRIP))
        rasterizer.pNext = &lineState;

    VkPipelineMultisampleStateCreateInfo multisample{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blendAttachment = GetBlendState(key.blendMode);
    VkPipelineColorBlendStateCreateInfo blend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    blend.attachmentCount = 1;
    blend.pAttachments = &blendAttachment;

    VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic.dynamicStateCount = 2;
    dynamic.pDynamicStates = dynamicStates;

    VkPipelineRenderingCreateInfo rendering{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    rendering.colorAttachmentCount = 1;
    rendering.pColorAttachmentFormats = &key.format;

    VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    info.pNext = &rendering;
    info.stageCount = 2;
    info.pStages = stages;
    info.pVertexInputState = &vertexInput;
    info.pInputAssemblyState = &inputAssembly;
    info.pViewportState = &viewportState;
    info.pRasterizationState = &rasterizer;
    info.pMultisampleState = &multisample;
    info.pColorBlendState = &blend;
    info.pDynamicState = &dynamic;
    info.layout = mPipelineLayout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    if (!CheckResult(vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline),
        "Problem creating graphics pipeline."))
        return VK_NULL_HANDLE;

    mPipelines[key] = pipeline;
    return pipeline;
}

//*************************************************************************************************
bool VulkanRenderer::AllocateUniformData(VkBuffer& buffer, VkDeviceSize& offset)
{
    FrameData& frame = mFrames[mFrameIndex];
    VkDeviceSize size = (sizeof(DGL_ConstantBuffer) + mUniformAlignment - 1) / mUniformAlignment * mUniformAlignment;

    // Move to the next block if this one is full
    if (frame.uniformOffset + size > cUniformBlockSize)
    {
        ++frame.uniformBlockIndex;
        frame.uniformOffset = 0;
    }

    // Create a new block if needed
    if (frame.uniformBlockIndex == frame.uniformBlocks.size())
    {
        UniformBlock block;
        if (!CreateBuffer(nullptr, cUniformBlockSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, block.buffer))
            return false;
        VmaAllocationInfo info;
        vmaGetAllocationInfo(mAllocator, block.buffer.allocation, &info);
        block.mapped = (uint8_t*)info.pMappedData;
        frame.uniformBlocks.push_back(block);
    }

    UniformBlock& block = frame.uniformBlocks[frame.uniformBlockIndex];
    memcpy(block.mapped + frame.uniformOffset, &mConstantBuffer, sizeof(DGL_ConstantBuffer));
    buffer = block.buffer.handle;
    offset = frame.uniformOffset;
    frame.uniformOffset += size;
    return true;
}

//*************************************************************************************************
void VulkanRenderer::DestroyLater(std::function<void()> destroy)
{
    // The last frame that might use the object is the one being recorded (which will be submitted
    // with the next value), or the last one submitted if no frame is being recorded
    uint64_t submitValue = mFrameActive ? mSubmitCount + 1 : mSubmitCount;
    mPendingDeletions.push_back({ submitValue, std::move(destroy) });
}

//*************************************************************************************************
void VulkanRenderer::ProcessPendingDeletions(bool waitForAll)
{
    uint64_t finished = UINT64_MAX;
    if (waitForAll)
        vkDeviceWaitIdle(mDevice);
    else
        vkGetSemaphoreCounterValue(mDevice, mTimeline, &finished);

    // The deletions are in order, so stop at the first one that isn't finished
    auto it = mPendingDeletions.begin();
    for (; it != mPendingDeletions.end() && it->submitValue <= finished; ++it)
        it->destroy();
    mPendingDeletions.erase(mPendingDeletions.begin(), it);
}

} // namespace DGL
