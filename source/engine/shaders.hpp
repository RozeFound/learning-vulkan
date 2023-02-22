#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

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