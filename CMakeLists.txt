cmake_minimum_required(VERSION 3.16...3.22)

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
CPMAddPackage("gh:khronosgroup/vulkan-hpp@1.3.240")

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

# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE sources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
)

# ---- Enable libc++ ---- 

# if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
# endif()

# ---- Create executable ----

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
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
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE vulkan glfw glm)

