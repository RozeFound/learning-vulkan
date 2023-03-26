cmake_minimum_required(VERSION 3.19...3.22)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  Learning-Vulkan
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

# ---- Import tools ----

set(USE_CCACHE YES)
include(cmake/tools.cmake)

# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:khronosgroup/vulkan-hpp@1.3.244")

CPMAddPackage(
        NAME GLFW
        VERSION 3.3.9
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 064fa9c6a50422f2d7b450afed1d5eea2b8054db
        OPTIONS
          "GLFW_BUILD_TESTS OFF"
          "GLFW_BUILD_EXAMPLES OFF"
          "GLFW_BULID_DOCS OFF"
          "GLFW_BUILD_WAYLAND ON"
          "GLFW_BUILD_X11 OFF"
)
CPMAddPackage("gh:g-truc/glm#0.9.9.8")

CPMAddPackage(
    NAME imgui
    VERSION 1.89.3
    GITHUB_REPOSITORY ocornut/imgui
    DOWNLOAD_ONLY
)

file(GLOB imgui_headers 
    "${imgui_SOURCE_DIR}/*.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
)

file(GLOB imgui_sources 
    "${imgui_SOURCE_DIR}/*.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
)

add_library(imgui STATIC EXCLUDE_FROM_ALL ${imgui_sources} ${imgui_headers})
target_include_directories(imgui PUBLIC $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>)
target_link_libraries(imgui PRIVATE glfw)

CPMAddPackage(
    NAME stb
    VERSION stable
    GITHUB_REPOSITORY nothings/stb
    GIT_TAG 3ecc60f25ae1391cf6434578ece782afa1458b56
    DOWNLOAD_ONLY
)

add_library(stb_image INTERFACE EXCLUDE_FROM_ALL ${stb_SOURCE_DIR}/stb_image.h)
target_include_directories(stb_image INTERFACE $<BUILD_INTERFACE:${stb_SOURCE_DIR}>)
target_compile_definitions(stb_image INTERFACE "STB_IMAGE_IMPLEMENTATION")

CPMAddPackage(
    NAME TinyOBJLoader
    VERSION 2.0.0-rc4
    GITHUB_REPOSITORY tinyobjloader/tinyobjloader
    DOWNLOAD_ONLY
)

add_library(tinyobj INTERFACE EXCLUDE_FROM_ALL "${TinyOBJLoader_SOURCE_DIR}/tiny_obj_loader.h")
target_include_directories(tinyobj INTERFACE $<BUILD_INTERFACE:${TinyOBJLoader_SOURCE_DIR}>)
target_compile_definitions(tinyobj INTERFACE "TINYOBJLOADER_IMPLEMENTATION")

CPMAddPackage(
    NAME VulkanMemoryAllocator
    VERSION 3.0.1
    GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    DOWNLOAD_ONLY
)

add_library(vma INTERFACE EXCLUDE_FROM_ALL "${VulkanMemoryAllocator_SOURCE_DIR}/include/vk_mem_alloc.h")
target_include_directories(vma INTERFACE $<BUILD_INTERFACE:${VulkanMemoryAllocator_SOURCE_DIR}>/include)
target_compile_options(vma INTERFACE "-Wno-nullability-completeness")

CPMAddPackage("gh:stephenberry/glaze@1.1.0")

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.

file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp")

# ---- Create executable ----

add_executable(${PROJECT_NAME} ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 OUTPUT_NAME "Learning-Vulkan")

# Enable precompiled headers
target_compile_definitions(${PROJECT_NAME} PRIVATE "VULKAN_HPP_NO_CONSTRUCTORS")
target_precompile_headers(${PROJECT_NAME} PRIVATE <vulkan/vulkan.hpp> <glm/glm.hpp> <GLFW/glfw3.h>)

# Enable debug info
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELWITHDEBINFO>:DEBUG>")

# Enable shader comilation
include(cmake/shaders.cmake)

# Add textures & models
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${PROJECT_BINARY_DIR})

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE imgui stb_image tinyobj vma)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt glaze::glaze)
target_link_libraries(${PROJECT_NAME} PRIVATE vulkan glfw glm)

