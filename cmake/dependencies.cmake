# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

set(CPM_USE_LOCAL_PACKAGES)
include(cmake/CPM.cmake)

CPMAddPackage(
    NAME fmt
    GIT_TAG 9.1.0
    GITHUB_REPOSITORY fmtlib/fmt
)

CPMAddPackage(
    NAME glaze
    VERSION 1.1.0
    GITHUB_REPOSITORY stephenberry/glaze
)

find_package(Vulkan)
if (NOT VULKAN_FOUND)
    CPMAddPackage("gh:khronosgroup/vulkan-hpp@1.3.246")
endif()

CPMAddPackage(
    NAME glfw3
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

CPMAddPackage(
    NAME glm
    GIT_TAG 0.9.9.8
    GITHUB_REPOSITORY g-truc/glm
)

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