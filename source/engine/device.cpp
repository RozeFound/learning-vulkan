#include <ranges>
#include <set>

#include "device.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

    Device::Device (GLFWwindow* window) : window(window) {

        make_instance();

        VkSurfaceKHR c_surface;

        if (glfwCreateWindowSurface(instance, window, nullptr, &c_surface) != VK_SUCCESS)
            LOG_ERROR("Cannot abstract GLFW surface for Vulkan");

        surface = c_surface;

        choose_physical_device();
        create_handle();

    }

    void Device::destroy ( ) {

        LOG_INFO("Destroying Device");
        handle.destroy();
        LOG_INFO("Destroying Surface");
        instance.destroySurfaceKHR(surface);
        LOG_INFO("Destroying Instance");
        instance.destroy();

    }

    void Device::make_instance ( ) {

        auto app_info = vk::ApplicationInfo {
            .apiVersion = VK_VERSION_1_3
        };

        uint32_t glfw_extension_count = 0;
        auto glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        if (!glfw_extensions) LOG_WARNING("Failed to fetch required GLFW Extensions!");

        auto extensions = std::vector<const char*>(glfw_extensions, glfw_extensions + glfw_extension_count);
        auto layers = std::vector<const char*>();


        if constexpr (debug) {

            extensions.push_back("VK_EXT_debug_utils");

            LOG_VERBOSE("Extensions to be requested: ");
            for (auto& extension : extensions) {
                LOG_VERBOSE("\t{}", extension);
            }

            layers.push_back("VK_LAYER_KHRONOS_validation");

        }

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
            LOG_INFO("Successfully created Instance");
            
        } catch(vk::SystemError err) {
            LOG_ERROR("Failed to create Instance");
        }

    } 

    void Device::choose_physical_device ( ) {

        auto suitable = [](vk::PhysicalDevice& device){

            for (auto& extension_properies : device.enumerateDeviceExtensionProperties()) {

                if (std::strcmp(extension_properies.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
                    continue;
                
                return true;
            }  
            return false;

        };

        auto devices = instance.enumeratePhysicalDevices();

        auto device = std::ranges::find_if(devices, suitable);
        if (device != std::end(devices)) gpu = *device;
        else LOG_ERROR("Failed to get Physical Device");

        auto properties = device->getProperties();

        std::string device_type;

        switch (properties.deviceType) {
            case (vk::PhysicalDeviceType::eCpu): device_type = "CPU"; break;
            case (vk::PhysicalDeviceType::eDiscreteGpu): device_type = "Discrete GPU"; break;
            case (vk::PhysicalDeviceType::eIntegratedGpu): device_type = "Integrated GPU"; break;
            case (vk::PhysicalDeviceType::eVirtualGpu): device_type = "Virtual GPU"; break;
            default: device_type = "Other";
        };

        LOG_INFO("Device Name: {}", properties.deviceName);
        LOG_INFO("Device Type: {}", device_type);

        auto extensions = gpu.enumerateDeviceExtensionProperties();

        LOG_VERBOSE("Device can support extensions: ");
        for (auto& extension : extensions)
            LOG_VERBOSE("\t{}", extension.extensionName);

    }

    void Device::create_handle ( ) {


        auto indices = get_queue_family_indices(gpu, surface);

        auto queue_info = std::vector<vk::DeviceQueueCreateInfo>();
        auto queue_piority = 1.f;

        auto unique_indices = std::set<uint32_t> {
            indices.transfer_family.value(),
            indices.graphics_family.value(), 
            indices.present_family.value()
        };

        for (const auto& queue_family : unique_indices) {

            auto create_info = vk::DeviceQueueCreateInfo {
                .flags = vk::DeviceQueueCreateFlags(),
                .queueFamilyIndex = queue_family,
                .queueCount = 1,
                .pQueuePriorities = &queue_piority
            }; queue_info.push_back(create_info);

        }

        auto device_features = vk::PhysicalDeviceFeatures();

        auto extensions = std::vector {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        auto layers = std::vector<const char*>();

        if constexpr (debug) layers.push_back("VK_LAYER_KHRONOS_validation");

        auto device_info = vk::DeviceCreateInfo {
            .flags = vk::DeviceCreateFlags(),
            .queueCreateInfoCount = static_cast<uint32_t>(queue_info.size()),
            .pQueueCreateInfos = queue_info.data(),
            .enabledLayerCount = static_cast<uint32_t>(layers.size()),
            .ppEnabledLayerNames = layers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = &device_features
        };

        try {
            handle = gpu.createDevice(device_info);
            LOG_INFO("Device was successfully abstracted.");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to abstract Physical Device");
        }

    }

}