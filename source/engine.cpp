#include <iostream>
#include <stdexcept>
#include <vector>

#include <fmt/core.h>

#include "engine.hpp"
#include "logging.hpp"

Engine::Engine ( ) {

    if (debug) std::cout << "Creating Engine instance..." << std::endl;

    create_window();
    create_instance();
    
}

Engine::~Engine ( ) {

    if (debug) std::cout << "Destroying Engine..." << std::endl;

    if (debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

    instance.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();

}

void Engine::create_window ( ) {

    if (debug) std::cout << "Creating window..." << std::endl;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);

    if (debug) {
        if (window) std::cout << "Successfully created " << title << " window." << std::endl;
        else std::cout << "Failed to create " << title << " window." << std::endl;
    }

}

void Engine::create_instance ( ) {

    auto version = vk::enumerateInstanceVersion();

    if (debug) {
        auto major = VK_API_VERSION_MAJOR(version);
        auto minor = VK_API_VERSION_MINOR(version);
        auto patch = VK_API_VERSION_PATCH(version);

        auto str_version = fmt::format("{}.{}.{}", major, minor, patch);
        std::cout << "System support up to Vulkan " << str_version << std::endl;
    }

    version &= ~(0xFFFU);

    auto app_info = vk::ApplicationInfo {
        .pApplicationName = title.data(),
        .apiVersion = version
    };

    uint32_t glfw_extension_count = 0;
    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if (!glfw_extensions && debug) std::cout << "Failed to fetch required GLFW Extensions!" << std::endl;

    auto extensions = std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);

    if (debug) extensions.push_back("VK_EXT_debug_utils");

    if (debug) {
        std::cout << "GLFW Extensions to be requested: " << std::endl;
        for (auto extension : extensions)
            std::cout << extension << std::endl; 
    }

    auto layers = std::vector<const char*>();

    if (debug) layers.push_back("VK_LAYER_KHRONOS_validation");

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
        dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
        debug_messenger = logging::make_debug_messenger(instance, dldi);
    } catch(vk::SystemError err) {
        std::cout << "Failed to create vk::Instance" << std::endl;
    }

}