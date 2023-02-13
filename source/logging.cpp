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

}