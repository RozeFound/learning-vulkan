#include <iostream>
#include <stdexcept>
#include <vector>

#include "device.hpp"
#include "engine.hpp"
#include "logging.hpp"

Engine::Engine ( ) {

    LOG_INFO("Creating Engine instance...");

    make_window();
    make_instance();
    make_device();
    
}

Engine::~Engine ( ) {

    LOG_INFO("Destroying Engine...");

    if (logging::debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

    device.destroy();
    instance.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();

}

void Engine::make_window ( ) {

    LOG_INFO("Creating window...");

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

    if (window) {LOG_INFO("Successfully created {} window.", title);}
    else LOG_ERROR("Failed to create {} window.", title);

}

void Engine::make_instance ( ) {

    auto version = vk::enumerateInstanceVersion();

    if constexpr (logging::debug) {

        auto major = VK_API_VERSION_MAJOR(version);
        auto minor = VK_API_VERSION_MINOR(version);
        auto patch = VK_API_VERSION_PATCH(version);

        LOG_INFO("System support up to Vulkan {}.{}.{}", major, minor, patch);

    }

    version &= ~(0xFFFU);

    auto app_info = vk::ApplicationInfo {
        .pApplicationName = title.data(),
        .apiVersion = version
    };

    uint32_t glfw_extension_count = 0;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if (!glfw_extensions) LOG_WARNING("Failed to fetch required GLFW Extensions!");

    auto extensions = std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);

    if constexpr (logging::debug) extensions.push_back("VK_EXT_debug_utils");

    if constexpr (logging::debug) {
        LOG_INFO("Extensions to be requested: ");
        for (auto& extension : extensions)
            std::cout << extension << std::endl;
    }

    auto layers = std::vector<const char*>();

    if (logging::debug) layers.push_back("VK_LAYER_KHRONOS_validation");

    auto create_info = vk::InstanceCreateInfo {
        .flags = vk::InstanceCreateFlags(),
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    try {
        instance = vk::createInstance(create_info);
        
        if constexpr (logging::debug) {
            dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
            debug_messenger = logging::make_debug_messenger(instance, dldi);
        }
        
    } catch(vk::SystemError err) {
        LOG_ERROR("Failed to create vk::Instance");
    }

}

void Engine::make_device ( ) {

    physical_device = vk::device::get_physical_device(instance).value_or(nullptr);
    device = vk::device::create_logical_device(physical_device).value_or(nullptr);

    auto queue_family_index = vk::device::get_graphics_queue_index(physical_device);
    graphics_queue = device.getQueue(queue_family_index, 0);

}