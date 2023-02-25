#include <iostream>

#include "logging.hpp"

namespace engine {

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, 
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void*) {

        std::cerr << pCallbackData->pMessage << std::endl;

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

    void log (std::string_view message, log_level level) {

        if (default_debug_level > level) return; 

        switch (level) {
            case (log_level::verbose): std::cout << message << std::endl; break;
            case (log_level::info): std::cout << "Info: " << message << std::endl; break;
            case (log_level::warning): std::cout << "Warning: " << message << std::endl; break;
            case (log_level::error): std::cerr << "Error: " << message << std::endl; break;
        }

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

        LOG_INFO("Device Name: {}", properties.deviceName);
        LOG_INFO("Device Type: {}", device_type);

        auto extensions = physical_device.enumerateDeviceExtensionProperties();

        LOG_VERBOSE("Device can support extensions: ");
        for (auto& extension : extensions)
            LOG_VERBOSE("\t{}", extension.extensionName);

    }

    void log_instance_properties ( ) {

        auto extensions = vk::enumerateInstanceExtensionProperties();

        LOG_VERBOSE("Instance can support extensions:")
        for (const auto& extension : extensions) {
            LOG_VERBOSE("\t{}", extension.extensionName);
        }

        auto layers = vk::enumerateInstanceLayerProperties();

        LOG_VERBOSE("Instance can support layers:")
        for (const auto& layer : layers) {
            LOG_VERBOSE("\t{}",layer.layerName);
        }

    }

}