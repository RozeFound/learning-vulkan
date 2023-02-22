#pragma once

#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

#include "device.hpp"

namespace engine {

    struct Buffer {
        vk::Buffer buffer;
        vk::DeviceMemory memory;
    };

    uint32_t get_memory_index (vk::PhysicalDevice&, vk::MemoryRequirements, vk::MemoryPropertyFlags);
    Buffer create_buffer (Device& device, vk::DeviceSize, vk::BufferUsageFlags, vk::MemoryPropertyFlags);
    void copy_buffer (Device& device, vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);

}