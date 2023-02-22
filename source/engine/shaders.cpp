#include <fstream>
#include <vector>

#include "logging.hpp"
#include "shaders.hpp"

namespace engine {

    Shader::Shader (const vk::Device& device, std::filesystem::path path)
        : device(device) {

        LOG_INFO("Creating Shader...");

        auto vertex_path = path.string() + ".vert.spv";
        auto fragment_path = path.string() + ".frag.spv";

        vertex_module = create_module(read(vertex_path));
        fragment_module = create_module(read(fragment_path));

        vertex_stage = vk::PipelineShaderStageCreateInfo {
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vertex_module,
            .pName = "main"
        };

        fragment_stage = vk::PipelineShaderStageCreateInfo {
            .flags = vk::PipelineShaderStageCreateFlags(),
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fragment_module,
            .pName = "main"
        };
    }

    void Shader::destroy ( ) {

        device.destroyShaderModule(vertex_module);
        device.destroyShaderModule(fragment_module);

    }

    vk::ShaderModule Shader::create_module (std::vector<std::byte> code) {

        auto create_info = vk::ShaderModuleCreateInfo {
                .flags = vk::ShaderModuleCreateFlags(),
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data())
            };

        try {
            auto result = device.createShaderModule(create_info);
            LOG_INFO("Successfully created shader module");
            return result;
        } catch (vk::SystemError err) {
            LOG_WARNING("Failed to create shader module");
            return nullptr;
        };

    }

    std::vector<vk::PipelineShaderStageCreateInfo> Shader::get_stage_info ( ) {

        return { vertex_stage, fragment_stage };

    }

    std::vector<std::byte> Shader::read (std::filesystem::path path) {

        auto file = std::ifstream(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) LOG_ERROR("Failed to load data from {}", path.string());

        std::size_t size = file.tellg(); file.seekg(0);
        auto buffer = std::vector<std::byte>(size);

        file.read(reinterpret_cast<char*>(buffer.data()), size);
        file.close();

        return buffer;

    }

}