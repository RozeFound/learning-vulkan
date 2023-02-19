#include <GLFW/glfw3.h>

#include "instance.hpp"
#include "logging.hpp"

namespace engine {

    void log_supported_features ( ) {

        auto extensions = vk::enumerateInstanceExtensionProperties();

        LOG_VERBOSE("Platform can support this extensions: ");
        for (auto& extension : extensions) 
            LOG_VERBOSE("\t{}", extension.extensionName);

        auto layers = vk::enumerateInstanceLayerProperties();

        LOG_VERBOSE("Platform can support this Layers: ");
        for (auto& layer : layers) 
            LOG_VERBOSE("\t{}", layer.layerName);

    }

    std::optional<vk::Instance> create_instance ( ) {

        auto app_info = vk::ApplicationInfo {
            .apiVersion = VK_VERSION_1_3
        };

        uint32_t glfw_extension_count = 0;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        if (!glfw_extensions) LOG_WARNING("Failed to fetch required GLFW Extensions!");

        auto extensions = std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);
        auto layers = std::vector<const char*>();


        if constexpr (debug) {

            extensions.push_back("VK_EXT_debug_utils");

            LOG_VERBOSE("Extensions to be requested: ");
            for (auto& extension : extensions) {
                LOG_VERBOSE("\t{}", extension);
            }

            layers.push_back("VK_LAYER_KHRONOS_validation");

            log_supported_features();
        }

        auto create_info = vk::InstanceCreateInfo {
            .flags = vk::InstanceCreateFlags(),
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };

        try {
            auto result = vk::createInstance(create_info);
            LOG_INFO("Successfully created Instance");
            return result;
            
        } catch(vk::SystemError err) {
            LOG_ERROR("Failed to create Instance");
            return std::nullopt;
        }

    }

}