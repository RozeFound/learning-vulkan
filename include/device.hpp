#pragma once

#include <optional>

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace vk::device {

    std::optional<vk::PhysicalDevice> get_physical_device (vk::Instance& instance);
    uint32_t get_graphics_queue_index (vk::PhysicalDevice& device);

    std::optional<vk::Device> create_logical_device (vk::PhysicalDevice& device);

}