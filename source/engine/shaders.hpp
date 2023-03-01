#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>

#include "essentials.hpp"

namespace engine {

        struct Vertex {

        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 texture_coordinates;

        static auto get_binding_description ( ) {

            auto description = vk::VertexInputBindingDescription {
                .binding = 0,
                .stride = sizeof(Vertex),
                .inputRate = vk::VertexInputRate::eVertex
            };

            return description;

        }

        static auto get_attribute_descriptions ( ) {

            auto descriptions = std::array {
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

    struct UniformBufferObject {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 projection;
    };

    class Shader {

        vk::UniqueShaderModule vertex_module;
        vk::UniqueShaderModule fragment_module;

        vk::PipelineShaderStageCreateInfo vertex_stage;
        vk::PipelineShaderStageCreateInfo fragment_stage;

        std::vector<std::byte> read (std::filesystem::path path);
        vk::UniqueShaderModule create_module (std::vector<std::byte> code);

        public:

        Shader (std::filesystem::path path); 

        std::vector<vk::PipelineShaderStageCreateInfo> get_stage_info ( );

    };

}