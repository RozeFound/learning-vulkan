#pragma once

#include <cstddef>
#include <vector>
#include <filesystem>
#include <span>

#include "../utils/logging.hpp"

namespace engine {

    class Shader {

        std::vector<vk::PipelineShaderStageCreateInfo> stages;
        std::vector<vk::ShaderModule> modules;

        std::vector<std::byte> read_from_file (std::filesystem::path path);
        void add_stage (vk::ShaderStageFlagBits stage, std::filesystem::path path);

        public:

        Shader (std::string path);
        ~Shader ( );

        constexpr const auto get_stage_info ( ) const { return stages; }

    };

}