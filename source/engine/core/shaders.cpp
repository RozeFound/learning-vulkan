#include <fstream>
#include <stdexcept>

#include "shaders.hpp"

#include "device.hpp"

#include "../utils/logging.hpp"

namespace engine {

    Shader::Shader (std::string path) {

        logi("Creating Shader...");

        auto vertex_path = std::filesystem::path(path + ".vert.spv");
        auto fragment_path = std::filesystem::path(path + ".frag.spv");
        auto compute_path = std::filesystem::path(path + ".comp.spv");

        if (std::filesystem::exists(vertex_path))
            add_stage(vk::ShaderStageFlagBits::eVertex, vertex_path);
        if (std::filesystem::exists(fragment_path))
            add_stage(vk::ShaderStageFlagBits::eFragment, fragment_path);
        if (std::filesystem::exists(compute_path))
            add_stage(vk::ShaderStageFlagBits::eCompute, compute_path);

    }

    Shader::~Shader ( ) {

        for (auto& module : modules)
            Device::get()->get_handle().destroyShaderModule(module);

    }

    void Shader::add_stage (vk::ShaderStageFlagBits stage, std::filesystem::path path) {

        auto code = read_from_file(path);

        auto module_create_info = vk::ShaderModuleCreateInfo {
                .flags = vk::ShaderModuleCreateFlags(),
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
            };

        try {
            auto result = Device::get()->get_handle().createShaderModule(module_create_info);
            modules.push_back(result);
            logi("Successfully created shader module");
        } catch (vk::SystemError err) {
            throw std::runtime_error("Failed to create shader module");
        };

        auto stage_create_info = vk::PipelineShaderStageCreateInfo {
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = stage,
            .module = modules.back(),
            .pName = "main"
        };

        stages.push_back(stage_create_info);

    }

    std::vector<std::byte> Shader::read_from_file (std::filesystem::path path) {

        auto file = std::ifstream(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) loge("Failed to load data from {}", path.string());

        std::size_t size = file.tellg(); file.seekg(0);
        auto buffer = std::vector<std::byte>(size);

        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();

        return buffer;

    }

}