#pragma once

#include <source_location>
#include <functional>
#include <chrono>

#include <fmt/core.h>

namespace engine {

    enum class log_level {
        verbose, info, warning, error
    };

#if defined(DEBUG) 
    constexpr bool debug = true;
    constexpr auto default_debug_level = log_level::verbose;
#else
    constexpr bool debug = false;
    constexpr auto default_debug_level = log_level::error;
#endif

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback (VkDebugUtilsMessageSeverityFlagBitsEXT, 
        VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*,void*);

    vk::DebugUtilsMessengerEXT make_debug_messenger (const vk::Instance&, vk::DispatchLoaderDynamic&);

    void log (std::string_view message, log_level level, std::source_location = std::source_location::current());

    void log_device_properties (vk::PhysicalDevice& physical_device);
    void log_instance_properties ( );

    class ScopedTimer {

        using hrc = std::chrono::high_resolution_clock;

        hrc::time_point start;
        std::function<void(double)> callback;

        public:

        ScopedTimer (decltype(callback) callback) : callback(callback) { start = hrc::now(); }
        ~ScopedTimer ( ) { callback((hrc::now() - start).count() * 0.000001); }

    };
    
    ScopedTimer add_perf_counter (std::source_location = std::source_location::current());

}

#if defined(DEBUG) 

#define logv(FS...) engine::log(fmt::format(FS), engine::log_level::verbose)
#define logi(FS...) engine::log(fmt::format(FS), engine::log_level::info)
#define logw(FS...) engine::log(fmt::format(FS), engine::log_level::warning)

#else 

#define logv(FS...)
#define logi(FS...)
#define logw(FS...)

#endif

#define loge(FS...) engine::log(fmt::format(FS), engine::log_level::error)
#define SCOPED_PERF_LOG auto timer##__LINE__ = engine::add_perf_counter()