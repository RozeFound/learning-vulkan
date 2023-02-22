#pragma once

#include <vector>

#include <glm/glm.hpp>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "memory.hpp"
#include "device.hpp"

namespace engine {

    struct Vertex {

        glm::vec2 position;
        glm::vec3 color;

        static auto get_binding_description ( ) {

            auto description = vk::VertexInputBindingDescription {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = vk::VertexInputRate::eVertex
            };

            return description;

        }

        static auto get_attribute_descriptions ( ) {

            auto descriptions = std::array<vk::VertexInputAttributeDescription, 2>();

            descriptions.at(0) = {
                .location = 0,
                .binding = 0,
                .format = vk::Format::eR32G32Sfloat,
                .offset = offsetof(Vertex, position)
            };

            descriptions.at(1) = {
                .location = 1,
                .binding = 0,
                .format = vk::Format::eR32G32B32Sfloat,
                .offset = offsetof(Vertex, color)
            };

            return descriptions;

        }
    };

    struct Mesh {

        std::vector<Vertex> vertices;
        Buffer vertex_buffer;

        Device device;

        Mesh (Device& device);
        ~Mesh( );

    };

}