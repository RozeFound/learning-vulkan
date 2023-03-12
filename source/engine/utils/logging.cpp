#include <iostream>
#include <filesystem>

#include "logging.hpp"

namespace engine {

    std::string to_string(auto type) {         

        if (std::is_same_v<decltype(type), VkDebugUtilsMessageTypeFlagsEXT>) {

            auto flags = static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(type);
            using enum vk::DebugUtilsMessageTypeFlagBitsEXT;

            switch (flags) {
                case eGeneral: return "General";
                case eValidation: return "Validation";
                case ePerformance: return "Performance";
                case eDeviceAddressBinding: return "DeviceAddressBinding";
            }

        }

        if (std::is_same_v<decltype(type), VkDebugUtilsMessageSeverityFlagBitsEXT>) {

            auto flags = static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(type);           
            using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;

            switch (flags) {
                case eVerbose: return "Verbose";
                case eInfo: return "Info";
                case eWarning: return "Warning";
                case eError: return "Error";
            }

        }

    }

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, 
        VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) {

        auto type_flags = static_cast<vk::DebugUtilsMessageTypeFlagBitsEXT>(type);
        auto severity_flags = static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity); 

        using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
        using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;

        log(pCallbackData->pMessage, log_level::verbose);

        return VK_FALSE;
    }

    vk::DebugUtilsMessengerEXT make_debug_messenger (const vk::Instance& instance, vk::DispatchLoaderDynamic& dldi) {

        using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
        using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;

        auto create_info = vk::DebugUtilsMessengerCreateInfoEXT {
                .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
                .messageSeverity = eVerbose | eInfo | eWarning | eError,
                .messageType = eGeneral | eValidation | ePerformance,
                .pfnUserCallback = debug_callback, .pUserData = nullptr
        };

        return instance.createDebugUtilsMessengerEXT(create_info, nullptr, dldi);

    }

    void log (std::string_view message, log_level level, std::source_location location) {

        if (default_debug_level > level) return; 

        switch (level) {
            case log_level::info: std::clog << "Info: "; break;
            case log_level::warning: std::clog << "Warning in "; break;
            case log_level::error: std::clog << "Error in "; break;
            default: break;
        }

        if (level >= log_level::warning) {

            auto file_name = std::filesystem::path(location.file_name()).filename().string();
            auto output = fmt::format("{} ({}:{}) `{}`: {}", file_name, location.line(), location.column(), location.function_name(), message);

            std::clog << output << std::endl;

        } else std::clog << message << std::endl;

    }

    void log_device_properties(vk::PhysicalDevice &physical_device) {

        auto properties = physical_device.getProperties();

        std::string device_type;

        switch (properties.deviceType) {
            case (vk::PhysicalDeviceType::eCpu): 
                device_type = "CPU"; break;
            case (vk::PhysicalDeviceType::eDiscreteGpu): 
                device_type = "Discrete GPU"; break;
            case (vk::PhysicalDeviceType::eIntegratedGpu): 
                device_type = "Integrated GPU"; break;
            case (vk::PhysicalDeviceType::eVirtualGpu): 
                device_type = "Virtual GPU"; break;
            default: device_type = "Other";
        };

        logi("Device Name: {}", properties.deviceName);
        logi("Device Type: {}", device_type);

        auto extensions = physical_device.enumerateDeviceExtensionProperties();

        logv("Device can support extensions:");
        for (auto& extension : extensions)
            logv("\t{}", extension.extensionName);

    }

    void log_instance_properties ( ) {

        auto extensions = vk::enumerateInstanceExtensionProperties();

        logv("Instance can support extensions:");
        for (const auto& extension : extensions) {
            logv("\t{}", extension.extensionName);
        }

        auto layers = vk::enumerateInstanceLayerProperties();

        logv("Instance can support layers:");
        for (const auto& layer : layers) {
            logv("\t{}", layer.layerName);
        }

    }

}