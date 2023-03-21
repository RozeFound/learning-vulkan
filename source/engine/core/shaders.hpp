#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>

#include "../utils/logging.hpp"

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

        constexpr const auto get_stage_info ( ) const {
            return std::array { vertex_stage, fragment_stage };
        }

    };

}