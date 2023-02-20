#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace engine {

    auto get_device_properties (vk::PhysicalDevice& device);

    vk::PhysicalDevice get_physical_device (vk::Instance& instance);
    vk::Device create_logical_device (vk::PhysicalDevice& device, vk::SurfaceKHR& surface);

}