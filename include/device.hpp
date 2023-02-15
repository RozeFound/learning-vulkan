#pragma once

#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace vk::device {

    struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_family;
		std::optional<uint32_t> present_family;
	};

    void log_device_properties (vk::PhysicalDevice& device);
    auto get_device_properties (vk::PhysicalDevice& device);

    std::optional<vk::PhysicalDevice> get_physical_device (vk::Instance& instance);
    QueueFamilyIndices get_queue_family_indices (vk::PhysicalDevice& device, vk::SurfaceKHR& surface);

    std::optional<vk::Device> create_logical_device (vk::PhysicalDevice& device, vk::SurfaceKHR& surface);

}