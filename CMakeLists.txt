#--------------------------------------------------------------------------------------------------
# CMake build for the DigiPen Graphics Library on Windows, Linux, and macOS.
#
# Requirements:
#   - The Vulkan SDK (https://vulkan.lunarg.com), with the VULKAN_SDK environment variable set.
#     It provides the Vulkan loader, the DXC shader compiler, and (on macOS) MoltenVK.
#   - vcpkg, with the VCPKG_ROOT environment variable set. It provides GLFW, stb, and VMA.
#
# Build:
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build
#
# The library, the DXC library, the test and template programs, and their shaders folders are all
# put in build/bin (build/bin/<Config> with Visual Studio). Run the programs from that folder,
# since the default shaders are loaded from "shaders/" relative to the working folder.
#--------------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.21)

# Use vcpkg automatically if VCPKG_ROOT is set, using the manifest in the library folder
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
endif()
if(NOT DEFINED VCPKG_MANIFEST_DIR)
    set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/DigiPen_Graphics_Library" CACHE PATH "")
endif()
# Match the Visual Studio project: static dependencies with the dynamic C runtime
if(WIN32 AND NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
        set(VCPKG_TARGET_TRIPLET "x86-windows-static-md" CACHE STRING "")
    else()
        set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")
    endif()
endif()

project(DGL VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put everything in one folder so programs can find the libraries and shaders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

#----------------------------------------------------------------------------------------- Library

set(DGL_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/DigiPen_Graphics_Library/src")

add_library(DGL SHARED
    ${DGL_SRC_DIR}/Camera.cpp
    ${DGL_SRC_DIR}/DGL.cpp
    ${DGL_SRC_DIR}/Error.cpp
    ${DGL_SRC_DIR}/FrameRateController.cpp
    ${DGL_SRC_DIR}/GraphicsSystem.cpp
    ${DGL_SRC_DIR}/InputSystem.cpp
    ${DGL_SRC_DIR}/Math.cpp
    ${DGL_SRC_DIR}/Mesh.cpp
    ${DGL_SRC_DIR}/Shader.cpp
    ${DGL_SRC_DIR}/VulkanRenderer.cpp
    ${DGL_SRC_DIR}/WindowSystem.cpp
)

# Same names as the Visual Studio project: DGL and DGL_d
set_target_properties(DGL PROPERTIES
    DEBUG_POSTFIX "_d"
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

target_include_directories(DGL
    PUBLIC ${DGL_SRC_DIR}
    PRIVATE ${Stb_INCLUDE_DIR}
)

target_compile_definitions(DGL PRIVATE DGL_EXPORTS)
if(WIN32)
    target_compile_definitions(DGL PRIVATE WIN32_LEAN_AND_MEAN)
endif()
# MSVC defines _DEBUG for debug builds automatically; other compilers need it added
if(NOT MSVC)
    target_compile_definitions(DGL PRIVATE $<$<CONFIG:Debug>:_DEBUG>)
endif()

# DXC is loaded at runtime, so only its headers (from the Vulkan SDK) are needed to build
target_link_libraries(DGL PRIVATE
    Vulkan::Vulkan
    glfw
    GPUOpen::VulkanMemoryAllocator
    ${CMAKE_DL_LIBS}
)

# Find the DXC library from the Vulkan SDK so it can be copied next to the DGL library
set(DGL_SDK_HINTS "$ENV{VULKAN_SDK}")
if(Vulkan_INCLUDE_DIR)
    get_filename_component(DGL_SDK_FROM_INCLUDE "${Vulkan_INCLUDE_DIR}" DIRECTORY)
    list(APPEND DGL_SDK_HINTS "${DGL_SDK_FROM_INCLUDE}")
endif()
if(WIN32)
    find_file(DGL_DXC_RUNTIME dxcompiler.dll HINTS ${DGL_SDK_HINTS} PATH_SUFFIXES Bin bin)
elseif(APPLE)
    find_file(DGL_DXC_RUNTIME libdxcompiler.dylib HINTS ${DGL_SDK_HINTS} PATH_SUFFIXES lib)
else()
    find_file(DGL_DXC_RUNTIME libdxcompiler.so HINTS ${DGL_SDK_HINTS} PATH_SUFFIXES lib)
endif()

if(DGL_DXC_RUNTIME)
    add_custom_command(TARGET DGL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DGL_DXC_RUNTIME}" "$<TARGET_FILE_DIR:DGL>"
        COMMENT "Copying the DXC shader compiler next to the DGL library")
else()
    message(WARNING "The DXC shader compiler library was not found in the Vulkan SDK. "
        "DGL can only load precompiled .spv shaders without it.")
endif()

#---------------------------------------------------------------------------------------- Programs

# Adds a program that uses DGL, copying its shaders folder next to it
if(APPLE)
    set(DGL_PROGRAM_RPATH "@executable_path")
else()
    set(DGL_PROGRAM_RPATH "$ORIGIN")
endif()

function(dgl_add_program name folder)
    add_executable(${name} "${CMAKE_CURRENT_SOURCE_DIR}/${folder}/main.cpp")
    target_link_libraries(${name} PRIVATE DGL)
    set_target_properties(${name} PROPERTIES
        DEBUG_POSTFIX "_d"
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${name}>"
        # Find the DGL library in the program's own folder
        BUILD_RPATH "${DGL_PROGRAM_RPATH}"
        INSTALL_RPATH "${DGL_PROGRAM_RPATH}"
    )
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/${folder}/shaders" "$<TARGET_FILE_DIR:${name}>/shaders"
        COMMENT "Copying shaders for ${name}")
endfunction()

dgl_add_program(DGL_Template DGL_Template_Project)
dgl_add_program(DGL_Test DGL_Test_Project)
