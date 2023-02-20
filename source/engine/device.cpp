#include <string>
#include <utility>
#include <ranges>
#include <set>

#include "device.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace engine {

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

    vk::PhysicalDevice get_physical_device (vk::Instance& instance) {

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

            if constexpr (debug) {

                auto[name, type] = get_device_properties(device);

                LOG_INFO("Device Name: {}", name);
                LOG_INFO("Device Type: {}", type);

                auto extensions = device.enumerateDeviceExtensionProperties();

                LOG_VERBOSE("Device can support extensions: ");
                for (auto& extension : extensions)
                    LOG_VERBOSE("\t{}", extension.extensionName);
            }

            return device;

        }

        LOG_ERROR("Failed to get Physical Device");
        return nullptr;

    }

    vk::Device create_logical_device (vk::PhysicalDevice& device, vk::SurfaceKHR& surface) {

        auto indices = get_queue_family_indices(device, surface);

        auto queue_info = std::vector<vk::DeviceQueueCreateInfo>();
        auto queue_piority = 1.f;

        auto unique_indices = std::set<uint32_t>{indices.graphics_family.value(), indices.present_family.value()};

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
            auto result = device.createDevice(device_info);
            LOG_INFO("Device was successfully abstracted.");
            return result;
        } catch (vk::SystemError err) {
            LOG_ERROR("Failed to abstract Physical Device");
            return nullptr;
        }

    }

}