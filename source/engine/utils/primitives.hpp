#pragma once

#include <array>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <imgui.h>

namespace engine {

    struct Vertex {

        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texture_coordinates;

        auto operator== (const Vertex& other) const {

            auto pos_eq = position == other.position;
            auto col_eq = color == other.color;
            auto tex_eq = texture_coordinates == other.texture_coordinates;

            return pos_eq && col_eq && tex_eq;
            
        }

        static auto get_binding_description ( ) {

            auto description = vk::VertexInputBindingDescription {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = vk::VertexInputRate::eVertex
            };

            return description;

        }

        static auto get_attribute_descriptions ( ) {

            auto descriptions = std::vector {
                vk::VertexInputAttributeDescription {
                    .location = 0,
                    .binding = 0,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, position)
                },
                vk::VertexInputAttributeDescription {
                    .location = 1,
                    .binding = 0,
                    .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, color)
                },
                vk::VertexInputAttributeDescription {
                    .location = 2,
                    .binding = 0,
                    .format = vk::Format::eR32G32Sfloat,
                    .offset = offsetof(Vertex, texture_coordinates)
                }
            };

            return descriptions;

        }

    };

    struct ImVertex : ImDrawVert {

        static auto get_binding_description ( ) {

            auto description = vk::VertexInputBindingDescription {
                .binding = 0,
                .stride = sizeof(ImVertex),
                .inputRate = vk::VertexInputRate::eVertex
            };

            return description;

        }

        static auto get_attribute_descriptions ( ) {

            auto descriptions = std::vector {
                vk::VertexInputAttributeDescription {
                    .location = 0,
                    .binding = 0,
                    .format = vk::Format::eR32G32Sfloat,
                    .offset = offsetof(ImVertex, pos)
                },
                vk::VertexInputAttributeDescription {
                    .location = 1,
                    .binding = 0,
                    .format = vk::Format::eR32G32Sfloat,
                    .offset = offsetof(ImVertex, uv)
                },
                vk::VertexInputAttributeDescription {
                    .location = 2,
                    .binding = 0,
                    .format = vk::Format::eR8G8B8A8Unorm,
                    .offset = offsetof(ImVertex, col)
                }
            };

            return descriptions;

        }

    };

    struct Particle {

        glm::vec2 position;
        glm::vec2 velocity;
        glm::vec4 color;

        static auto get_binding_description ( ) {

            auto description = vk::VertexInputBindingDescription {
                .binding = 0,
                .stride = sizeof(Particle),
                .inputRate = vk::VertexInputRate::eVertex
            };

            return description;

        }

        static auto get_attribute_descriptions ( ) {

            auto descriptions = std::vector {
                vk::VertexInputAttributeDescription {
                    .location = 0,
                    .binding = 0,
                    .format = vk::Format::eR32G32Sfloat,
                    .offset = offsetof(Particle, position)
                },
                vk::VertexInputAttributeDescription {
                    .location = 1,
                    .binding = 0,
                    .format = vk::Format::eR32G32B32A32Sfloat,
                    .offset = offsetof(Particle, color)
                }
            };

            return descriptions;

        }

        static auto get_descriptor_set_layout_bindings ( ) {

            auto bindings = std::array {
                vk::DescriptorSetLayoutBinding {
                    .binding = 0,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .descriptorCount = 1,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute
                },
                vk::DescriptorSetLayoutBinding {
                    .binding =1,
                    .descriptorType = vk::DescriptorType::eStorageBuffer,
                    .descriptorCount = 1,
                    .stageFlags = vk::ShaderStageFlagBits::eCompute
                }
            };

            return bindings;

        }

    };

    struct MVPMatrix {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 projection;
        glm::mat4x4 pvm = projection * view * model;
    };

};

template <> struct std::hash<engine::Vertex> {
    std::size_t operator() (const engine::Vertex& vertex) const noexcept {

        auto hash_position = std::hash<glm::vec3>{}(vertex.position);
        auto hash_color = std::hash<glm::vec2>{}(vertex.color);
        auto hash_coords = std::hash<glm::vec2>{}(vertex.texture_coordinates);

        return ((hash_position ^ hash_color << 1) >> 1) ^ (hash_coords << 1);

    };
};