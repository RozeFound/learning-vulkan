#include <iostream>
#include <stdexcept>
#include <vector>

#include <fmt/core.h>
#include <range/v3/all.hpp>

#include "engine.hpp"
#include "logging.hpp"

Engine::Engine ( ) {

    if (debug) std::cout << "Creating Engine instance..." << std::endl;

    create_window();
    create_instance();
    create_device();
    
}

Engine::~Engine ( ) {

    if (debug) std::cout << "Destroying Engine..." << std::endl;

    if (debug) instance.destroyDebugUtilsMessengerEXT(debug_messenger, nullptr, dldi);

    device.destroy();
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
        else std::cerr << "Failed to create " << title << " window." << std::endl;
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
    if (!glfw_extensions && debug) std::cerr << "Failed to fetch required GLFW Extensions!" << std::endl;

    auto extensions = std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);

    if (debug) extensions.push_back("VK_EXT_debug_utils");

    if (debug) {
        std::cout << "Extensions to be requested: " << std::endl;
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
        std::cerr << "Failed to create vk::Instance" << std::endl;
    }

}

void Engine::create_device ( ) {

    auto suitable = [](vk::PhysicalDevice& device){

        for (auto& extension_properies : device.enumerateDeviceExtensionProperties()) {

            if (std::strcmp(extension_properies.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
                continue;
            
            return true;
        }  
        return false;

    };

    auto devices = instance.enumeratePhysicalDevices();

    for (auto& device : devices | ranges::views::filter(suitable)) {

        if (debug) {
            auto device_properties = device.getProperties();
            std::cout << "Device Name: " << device_properties.deviceName << std::endl;
        }

        physical_device = device;
        break;

    }

    auto queue_family_properties = physical_device.getQueueFamilyProperties();
    auto queue_piority = 1.f; uint32_t queue_family_index = 0;

    for (std::size_t i = 0; i < queue_family_properties.size(); i++)
        if (queue_family_properties.at(i).queueFlags & vk::QueueFlagBits::eGraphics) {
            queue_family_index = i; break;
        }

    auto queue_info = vk::DeviceQueueCreateInfo {
        .flags = vk::DeviceQueueCreateFlags(),
        .queueFamilyIndex = queue_family_index, // TODO 
        .queueCount = 1,
        .pQueuePriorities = &queue_piority
    };

    auto device_features = vk::PhysicalDeviceFeatures();

    auto layers = std::vector<const char*>();

    if (debug) layers.push_back("VK_LAYER_KHRONOS_validation");

    auto device_info = vk::DeviceCreateInfo {
        .flags = vk::DeviceCreateFlags(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
        .pEnabledFeatures = &device_features
    };

    try {
        device = physical_device.createDevice(device_info);
        if (debug) std::cout << "Device was successfully abstracted." << std::endl;
        graphics_queue = device.getQueue(queue_family_index, 0);
    } catch (vk::SystemError err) {
        std::cerr << "Failed to abstract device." << std::endl;
    }

}