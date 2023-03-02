#pragma once

#include <fmt/core.h>
#include <vulkan/vulkan.hpp>

namespace engine {

#ifdef DEBUG 
    constexpr bool debug = true;
#else
    constexpr bool debug = false;
#endif

    enum class log_level {
        verbose, info, warning, error
    };

    constexpr auto default_debug_level = log_level::verbose;

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT, 
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*,void*);

    vk::DebugUtilsMessengerEXT make_debug_messenger (const vk::Instance&, vk::DispatchLoaderDynamic&);

    void log(std::string_view message, log_level level);

    void log_device_properties (vk::PhysicalDevice& physical_device);
    void log_instance_properties ( );

}

#ifdef DEBUG 

#define LOG_VERBOSE(FORMAT_STRING ...) engine::log(fmt::format(FORMAT_STRING), engine::log_level::verbose);
#define LOG_INFO(FORMAT_STRING ...) engine::log(fmt::format(FORMAT_STRING), engine::log_level::info);
#define LOG_WARNING(FORMAT_STRING ...) engine::log(fmt::format(FORMAT_STRING), engine::log_level::warning);

#else

#define LOG_VERBOSE(FORMAT_STRING ...)
#define LOG_INFO(FORMAT_STRING ...)
#define LOG_WARNING(FORMAT_STRING ...)  

#endif

#define LOG_ERROR(FORMAT_STRING ...) engine::log(fmt::format(FORMAT_STRING), engine::log_level::error);