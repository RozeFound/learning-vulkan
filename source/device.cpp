#include <optional>
#include <range/v3/all.hpp>

#include "device.hpp"
#include "logging.hpp"

namespace vk::device {

    std::optional<vk::PhysicalDevice> get_physical_device (vk::Instance& instance) {

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

            LOG_INFO("Device Name: {}", device.getProperties().deviceName);

            return device;

        }

        LOG_ERROR("Failed to get Physical Device");
        return std::nullopt;

    }

    uint32_t get_graphics_queue_index (vk::PhysicalDevice& device) {

        auto queue_family_properties = device.getQueueFamilyProperties();
        uint32_t queue_family_index = 0;

        for (std::size_t i = 0; i < queue_family_properties.size(); i++)
            if (queue_family_properties.at(i).queueFlags & vk::QueueFlagBits::eGraphics) {
                queue_family_index = i; break;
            }

        return queue_family_index;

    }

    std::optional<vk::Device> create_logical_device (vk::PhysicalDevice& device) {

        auto queue_family_index = vk::device::get_graphics_queue_index(device);
        auto queue_piority = 1.f;

        auto queue_info = vk::DeviceQueueCreateInfo {
            .flags = vk::DeviceQueueCreateFlags(),
            .queueFamilyIndex = queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_piority
        };

        auto device_features = vk::PhysicalDeviceFeatures();

        auto layers = std::vector<const char*>();

        if (logging::debug) layers.push_back("VK_LAYER_KHRONOS_validation");

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
            return device.createDevice(device_info);
            LOG_INFO("Device was successfully abstracted.");
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to abstract Physical Device");
            return std::nullopt;
        }

    }

}