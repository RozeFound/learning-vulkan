#include <string>
#include <utility>
#include <ranges>

#include "device.hpp"
#include "logging.hpp"

namespace vk::device {

    void log_device_properties (vk::PhysicalDevice& device) {

        auto extensions = device.enumerateDeviceExtensionProperties();

        LOG_VERBOSE("Device can support extensions: ");
        for (auto& extension : extensions)
            LOG_VERBOSE("\t{}", extension.extensionName);

    }

    auto get_device_properties (vk::PhysicalDevice& device) {

        auto properties = device.getProperties();

        std::string device_name = properties.deviceName;
        std::string device_type;

        switch (properties.deviceType) {
            case (vk::PhysicalDeviceType::eCpu): device_type = "CPU"; break;
            case (vk::PhysicalDeviceType::eDiscreteGpu): device_type = "Discrete GPU"; break;
            case (vk::PhysicalDeviceType::eIntegratedGpu): device_type = "Integrated GPU"; break;
            case (vk::PhysicalDeviceType::eVirtualGpu): device_type = "Virtual GPU"; break;
            default: device_type = "Other";
        }

        return std::make_pair(device_name, device_type);

    }

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

        for (auto& device : devices | std::views::filter(suitable)) {

            if constexpr (logging::debug) {

                auto[name, type] = get_device_properties(device);

                LOG_INFO("Device Name: {}", name);
                LOG_INFO("Device Type: {}", type);

                log_device_properties(device);
            }

            return device;

        }

        LOG_ERROR("Failed to get Physical Device");
        return std::nullopt;

    }

    QueueFamilyIndices get_queue_family_indices (vk::PhysicalDevice& device, vk::SurfaceKHR& surface) {

        QueueFamilyIndices indices;
        auto queue_family_properties = device.getQueueFamilyProperties();

        for (std::size_t i = 0; i < queue_family_properties.size(); i++) {

            auto queue_flags = queue_family_properties.at(i).queueFlags;
            
            if (queue_flags & vk::QueueFlagBits::eGraphics) indices.graphics_family = i;
            if (device.getSurfaceSupportKHR(i, surface)) indices.present_family = i;
            if(indices.graphics_family.has_value() && indices.present_family.has_value()) break;

        }

        return indices;

    }

    std::optional<vk::Device> create_logical_device (vk::PhysicalDevice& device, vk::SurfaceKHR& surface) {

        auto indices = vk::device::get_queue_family_indices(device, surface);

        auto queue_info = std::vector<vk::DeviceQueueCreateInfo>();
        auto queue_piority = 1.f;

        auto graphics_queue_info = vk::DeviceQueueCreateInfo {
            .flags = vk::DeviceQueueCreateFlags(),
            .queueFamilyIndex = indices.graphics_family.value(),
            .queueCount = 1,
            .pQueuePriorities = &queue_piority
        };

        queue_info.push_back(graphics_queue_info);

        if (indices.graphics_family != indices.present_family) {

            auto present_queue_info = vk::DeviceQueueCreateInfo {
                .flags = vk::DeviceQueueCreateFlags(),
                .queueFamilyIndex = indices.present_family.value(),
                .queueCount = 1,
                .pQueuePriorities = &queue_piority
            };

            queue_info.push_back(present_queue_info);
        }

        auto device_features = vk::PhysicalDeviceFeatures();

        auto extensions = std::vector<const char*>{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        auto layers = std::vector<const char*>();

        if constexpr (logging::debug) layers.push_back("VK_LAYER_KHRONOS_validation");

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
            auto result = device.createDevice(device_info);
            LOG_INFO("Device was successfully abstracted.");
            return result;
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to abstract Physical Device");
            return std::nullopt;
        }

    }

}