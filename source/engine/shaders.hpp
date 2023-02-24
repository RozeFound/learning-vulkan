#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

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

    struct UniformBufferObject {
        glm::mat4x4 model;
        glm::mat4x4 view;
        glm::mat4x4 projection;
    };

    class Shader {

        vk::ShaderModule vertex_module;
        vk::ShaderModule fragment_module;

        vk::PipelineShaderStageCreateInfo vertex_stage;
        vk::PipelineShaderStageCreateInfo fragment_stage;

        vk::Device device;

        std::vector<std::byte> read (std::filesystem::path path);
        vk::ShaderModule create_module (std::vector<std::byte> code);

        public:

        Shader (const vk::Device& device, std::filesystem::path path); 

        std::vector<vk::PipelineShaderStageCreateInfo> get_stage_info ( );

        void destroy ( );

    };

}