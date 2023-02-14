#include <iostream>

#include "logging.hpp"

namespace logging {

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, 
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void*) {

        std::cerr << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    vk::DebugUtilsMessengerEXT make_debug_messenger (vk::Instance& instance, vk::DispatchLoaderDynamic& dldi) {

        using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
        using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;

        auto create_info = vk::DebugUtilsMessengerCreateInfoEXT {
                .flags = vk::DebugUtilsMessengerCreateFlagsEXT(),
                .messageSeverity = eWarning | eError,
                .messageType = eGeneral | eValidation | ePerformance,
                .pfnUserCallback = debug_callback, .pUserData = nullptr
        };

        return instance.createDebugUtilsMessengerEXT(create_info, nullptr, dldi);

    }

    void log (std::string_view message, level level) {

        switch (level) {
            case (level::info): std::cout << "Info: " << message << std::endl; break;
            case (level::warning): std::cout << "Warning: " << message << std::endl; break;
            case (level::error): std::cerr << "Error: " << message << std::endl; break;
        }

    }

}