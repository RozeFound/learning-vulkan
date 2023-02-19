#pragma once

#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    auto get_device_properties (vk::PhysicalDevice& device);

    std::optional<vk::PhysicalDevice> get_physical_device (vk::Instance& instance);
    std::optional<vk::Device> create_logical_device (vk::PhysicalDevice& device, vk::SurfaceKHR& surface);

}