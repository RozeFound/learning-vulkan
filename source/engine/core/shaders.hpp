#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>

namespace engine {

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