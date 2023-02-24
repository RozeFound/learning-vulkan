#pragma once

#include <vector>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "memory.hpp"
#include "device.hpp"
#include "shaders.hpp"


namespace engine {

    struct Mesh {

        std::vector<Vertex> vertices;
        Buffer vertex_buffer;

        Device device;

        Mesh (Device& device);
        ~Mesh( );

    };

}