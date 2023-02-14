#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include <fmt/core.h>


namespace logging {

#ifdef DEBUG 
    constexpr bool debug = true;
#else
    constexpr bool debug = false;
#endif

    enum class level {
        info, warning, error
    };

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, 
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*,void*);

    vk::DebugUtilsMessengerEXT make_debug_messenger (vk::Instance&, vk::DispatchLoaderDynamic&);

    void log(std::string_view message, level level);

}

#ifdef DEBUG 

#define LOG_INFO(FORMAT_STRING ...) logging::log(fmt::format(FORMAT_STRING), logging::level::info);
#define LOG_WARNING(FORMAT_STRING ...) logging::log(fmt::format(FORMAT_STRING), logging::level::warning);
#define LOG_ERROR(FORMAT_STRING ...) logging::log(fmt::format(FORMAT_STRING), logging::level::error);

#else

#define LOG_INFO(FORMAT_STRING ...)
#define LOG_WARNING(FORMAT_STRING ...)  
#define LOG_ERROR(FORMAT_STRING ...)

#endif